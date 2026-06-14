// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/FortRogueBattleCharacter.h"

#include "AbilitySystem/FortRogueAbilitySet.h"
#include "AbilitySystem/FortRogueAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/FortRogueCombatSet.h"
#include "Combat/FortRogueDestructibleTerrain.h"
#include "Combat/FortRogueProjectile.h"
#include "FortRogueGameMode.h"
#include "FortRogueGameplayTags.h"
#include "Items/FortRogueItemDefinition.h"
#include "Perks/FortRoguePerkDefinition.h"
#include "Weapons/FortRogueWeaponDefinition.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"

AFortRogueBattleCharacter::AFortRogueBattleCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Body->SetupAttachment(Root);
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Body->SetRelativeScale3D(FVector(0.65f, 0.08f, 0.9f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Body->SetStaticMesh(CubeMesh.Object);
	}

	AbilitySystemComponent = CreateDefaultSubobject<UFortRogueAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	CombatSet = CreateDefaultSubobject<UFortRogueCombatSet>(TEXT("CombatSet"));

	CharacterDisplayName = FText::FromString(TEXT("Rookie Tank"));
}

void AFortRogueBattleCharacter::BeginPlay()
{
	Super::BeginPlay();

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	AbilitySystemComponent->AddAttributeSetSubobject(CombatSet.Get());
	InitializeFromDefinition(CharacterDefinition);
	EnsureDefaultLoadout();
	GrantStartupAbilitySets();
	SnapToTerrain();
}

void AFortRogueBattleCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ApplyTerrainGravity(DeltaSeconds);
}

UAbilitySystemComponent* AFortRogueBattleCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AFortRogueBattleCharacter::InitializeFromDefinition(UFortRogueCharacterDefinition* InCharacterDefinition)
{
	if (!InCharacterDefinition)
	{
		return;
	}

	CharacterDefinition = InCharacterDefinition;
	CharacterDisplayName = InCharacterDefinition->DisplayName;
	StartupAbilitySets = InCharacterDefinition->StartupAbilitySets;

	CombatSet->SetMaxHealth(InCharacterDefinition->MaxHealth);
	CombatSet->SetHealth(InCharacterDefinition->MaxHealth);
	CombatSet->SetDamage(InCharacterDefinition->BonusDamage);
	CombatSet->SetMaxMoveBudget(InCharacterDefinition->MaxMoveBudget);
	CombatSet->SetMoveBudget(InCharacterDefinition->MaxMoveBudget);

	WeaponLoadout.Reset();
	for (UFortRogueWeaponDefinition* WeaponDefinition : InCharacterDefinition->WeaponLoadout)
	{
		AddWeaponDefinition(WeaponDefinition);
	}

	ItemLoadout = InCharacterDefinition->ItemLoadout;
	for (FFortRogueItemStack& ItemStack : ItemLoadout)
	{
		if (ItemStack.ItemDefinition && ItemStack.Charges <= 0)
		{
			ItemStack.Charges = ItemStack.ItemDefinition->InitialCharges;
		}
	}
}

void AFortRogueBattleCharacter::ConfigureAsEnemy(bool bNewEnemy)
{
	bEnemy = bNewEnemy;
	bFacingRight = !bEnemy;
}

void AFortRogueBattleCharacter::BeginTurn()
{
	bActiveTurn = true;
	bFiredThisTurn = false;
	bChargingShot = false;
	ShotChargeElapsed = 0.0f;
	CombatSet->ResetTurnBudget();
}

void AFortRogueBattleCharacter::EndTurn()
{
	bActiveTurn = false;
	bChargingShot = false;
}

void AFortRogueBattleCharacter::MoveHorizontal(float Axis, float DeltaSeconds)
{
	if (!bActiveTurn || bFiredThisTurn || IsDefeated() || FMath::IsNearlyZero(Axis))
	{
		return;
	}

	const float RequestedDelta = Axis * MoveSpeed * DeltaSeconds;
	const float Budget = CombatSet->GetMoveBudget();
	const float AppliedDelta = FMath::Clamp(RequestedDelta, -Budget, Budget);
	if (FMath::IsNearlyZero(AppliedDelta))
	{
		return;
	}

	FVector NewLocation = GetActorLocation();
	float ActualDelta = 0.0f;

	if (AFortRogueDestructibleTerrain* Terrain = FindTerrain())
	{
		const float StepLength = FMath::Max(1.0f, Terrain->CellSize * 0.5f);
		const int32 StepCount = FMath::Max(1, FMath::CeilToInt(FMath::Abs(AppliedDelta) / StepLength));
		const float StepDelta = AppliedDelta / static_cast<float>(StepCount);

		for (int32 Step = 0; Step < StepCount; ++Step)
		{
			FVector StepLocation = NewLocation;
			StepLocation.X += StepDelta;

			const float CurrentFootZ = NewLocation.Z - FootOffsetZ;
			float SurfaceZ = 0.0f;
			if (FindFootprintSurfaceZ(*Terrain, StepLocation.X, CurrentFootZ + MaxStepUp, MaxStepUp + MaxStepDown, SurfaceZ))
			{
				if (!IsSlopeTraversable(CurrentFootZ, SurfaceZ, StepDelta, Terrain->CellSize))
				{
					break;
				}

				StepLocation.Z = SurfaceZ + FootOffsetZ;
				if (IsFootprintBlocked(*Terrain, StepLocation, SurfaceZ))
				{
					break;
				}
				VerticalVelocity = 0.0f;
			}
			else if (IsFootprintBlocked(*Terrain, StepLocation, CurrentFootZ))
			{
				break;
			}

			NewLocation = StepLocation;
			ActualDelta += StepDelta;
		}
	}
	else
	{
		NewLocation.X += AppliedDelta;
		ActualDelta = AppliedDelta;
	}

	if (FMath::IsNearlyZero(ActualDelta))
	{
		return;
	}

	SetActorLocation(NewLocation);
	UpdateBodyTerrainAlignment(FindTerrain());
	CombatSet->SetMoveBudget(Budget - FMath::Abs(ActualDelta));
	bFacingRight = ActualDelta >= 0.0f;
}

void AFortRogueBattleCharacter::AdjustAim(float Axis, float DeltaSeconds)
{
	if (!bActiveTurn || bFiredThisTurn || FMath::IsNearlyZero(Axis))
	{
		return;
	}

	AimAngle = FMath::Clamp(AimAngle + Axis * 70.0f * DeltaSeconds, 5.0f, 85.0f);
}

void AFortRogueBattleCharacter::AdjustPower(float Axis, float DeltaSeconds)
{
	if (!bActiveTurn || bFiredThisTurn || FMath::IsNearlyZero(Axis))
	{
		return;
	}

	ShotPower = FMath::Clamp(ShotPower + Axis * 0.75f * DeltaSeconds, FMath::Min(MinShotPower, MaxShotPower), FMath::Max(MinShotPower, MaxShotPower));
}

void AFortRogueBattleCharacter::BeginShotCharge()
{
	if (!bActiveTurn || bFiredThisTurn || IsDefeated())
	{
		return;
	}

	bChargingShot = true;
	ShotChargeElapsed = 0.0f;
	ShotPower = FMath::Min(MinShotPower, MaxShotPower);
}

void AFortRogueBattleCharacter::UpdateShotCharge(float DeltaSeconds)
{
	if (!bChargingShot || !bActiveTurn || bFiredThisTurn || IsDefeated())
	{
		return;
	}

	ShotChargeElapsed += DeltaSeconds;
	const float ChargeDuration = FMath::Max(ShotChargeSeconds, KINDA_SMALL_NUMBER);
	const float ChargeAlpha = FMath::Clamp(ShotChargeElapsed / ChargeDuration, 0.0f, 1.0f);
	ShotPower = FMath::Lerp(FMath::Min(MinShotPower, MaxShotPower), FMath::Max(MinShotPower, MaxShotPower), ChargeAlpha);
}

int32 AFortRogueBattleCharacter::ReleaseShotCharge()
{
	if (!bChargingShot)
	{
		return 0;
	}

	bChargingShot = false;
	return FireSelectedWeapon();
}

void AFortRogueBattleCharacter::SelectWeapon(int32 WeaponIndex)
{
	if (WeaponLoadout.IsValidIndex(WeaponIndex))
	{
		SelectedWeaponIndex = WeaponIndex;
	}
}

int32 AFortRogueBattleCharacter::FireSelectedWeapon()
{
	if (!bActiveTurn || bFiredThisTurn || IsDefeated())
	{
		return 0;
	}

	bChargingShot = false;
	bFiredThisTurn = true;

	const FFortRogueWeaponSpec& Weapon = GetCurrentWeapon();
	const int32 ProjectileCount = FMath::Max(1, Weapon.ProjectilesPerShot + FMath::RoundToInt(CombatSet->GetProjectileCount()) - 1);
	const float FacingSign = bFacingRight ? 1.0f : -1.0f;
	const float Damage = (Weapon.Damage + CombatSet->GetDamage()) * PendingAttackMultiplier;
	const float Speed = Weapon.ProjectileSpeed * ShotPower * CombatSet->GetShotPowerMultiplier();
	int32 SpawnedProjectiles = 0;
	PendingAttackMultiplier = 1.0f;

	for (int32 Index = 0; Index < ProjectileCount; ++Index)
	{
		const float Spread = (ProjectileCount > 1) ? (Index - (ProjectileCount - 1) * 0.5f) * 5.0f : 0.0f;
		const float AngleRadians = FMath::DegreesToRadians(FMath::Clamp(AimAngle + Spread, 5.0f, 85.0f));
		const FVector Direction(FMath::Cos(AngleRadians) * FacingSign, 0.0f, FMath::Sin(AngleRadians));
		const FVector SpawnLocation = GetActorLocation() + FVector(FacingSign * 70.0f, 0.0f, 70.0f);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		const TSubclassOf<AFortRogueProjectile> ProjectileClass = Weapon.ProjectileClass ? Weapon.ProjectileClass : TSubclassOf<AFortRogueProjectile>(AFortRogueProjectile::StaticClass());
		AFortRogueProjectile* Projectile = GetWorld()->SpawnActor<AFortRogueProjectile>(ProjectileClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
		if (Projectile)
		{
			Projectile->InitializeProjectile(this, Direction * Speed, Damage, Weapon.BlastRadius, Weapon.Gravity);
			if (AFortRogueGameMode* GameMode = GetWorld()->GetAuthGameMode<AFortRogueGameMode>())
			{
				GameMode->NotifyProjectileSpawned(Projectile);
			}
			++SpawnedProjectiles;
		}
	}

	return SpawnedProjectiles;
}

void AFortRogueBattleCharacter::FireAtTarget(AFortRogueBattleCharacter* Target)
{
	if (!Target)
	{
		return;
	}

	const FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
	bFacingRight = ToTarget.X >= 0.0f;
	AimAngle = FMath::Clamp(FMath::RadiansToDegrees(FMath::Atan2(FMath::Max(220.0f, FMath::Abs(ToTarget.Z) + 300.0f), FMath::Max(1.0f, FMath::Abs(ToTarget.X)))), 35.0f, 72.0f);
	ShotPower = FMath::Clamp(FMath::Abs(ToTarget.X) / 1450.0f, 0.45f, 0.92f);
}

void AFortRogueBattleCharacter::ApplyDamage(float DamageAmount)
{
	CombatSet->ApplyDamage(DamageAmount);
}

void AFortRogueBattleCharacter::ReevaluateTerrainSupport()
{
	ApplyTerrainGravity(1.0f / 60.0f);
}

bool AFortRogueBattleCharacter::UseItemByType(EFortRogueItemType ItemType)
{
	if (!bActiveTurn || IsDefeated())
	{
		return false;
	}

	for (FFortRogueItemStack& ItemStack : ItemLoadout)
	{
		const UFortRogueItemDefinition* ItemDefinition = ItemStack.ItemDefinition;
		if (!ItemDefinition || ItemDefinition->ItemType != ItemType || ItemStack.Charges <= 0)
		{
			continue;
		}

		--ItemStack.Charges;
		if (ItemType == EFortRogueItemType::Heal)
		{
			CombatSet->Heal(ItemDefinition->HealAmount);
		}
		else if (ItemType == EFortRogueItemType::AttackMultiplier)
		{
			PendingAttackMultiplier = FMath::Max(PendingAttackMultiplier, ItemDefinition->AttackMultiplier);
		}
		return true;
	}

	return false;
}

void AFortRogueBattleCharacter::ApplyRewardDamage(float BonusDamage)
{
	CombatSet->AddDamage(BonusDamage);
}

void AFortRogueBattleCharacter::ApplyRewardHealth(float BonusHealth)
{
	CombatSet->AddMaxHealth(BonusHealth);
}

void AFortRogueBattleCharacter::ApplyRewardProjectiles(int32 BonusProjectiles)
{
	CombatSet->AddProjectileCount(BonusProjectiles);
}

void AFortRogueBattleCharacter::ApplyPerkDefinition(UFortRoguePerkDefinition* PerkDefinition)
{
	if (!PerkDefinition)
	{
		return;
	}

	ApplyRewardDamage(PerkDefinition->DamageBonus);
	ApplyRewardHealth(PerkDefinition->MaxHealthBonus);
	ApplyRewardProjectiles(PerkDefinition->ProjectileBonus);

	if (PerkDefinition->GrantedAbilitySet)
	{
		FFortRogueAbilitySet_GrantedHandles Handles;
		PerkDefinition->GrantedAbilitySet->GiveToAbilitySystem(AbilitySystemComponent, &Handles, this);
		StartupAbilitySetHandles.Add(Handles);
	}
}

void AFortRogueBattleCharacter::AddWeaponDefinition(UFortRogueWeaponDefinition* WeaponDefinition)
{
	if (WeaponDefinition)
	{
		WeaponLoadout.Add(WeaponDefinition->Weapon);
	}
}

void AFortRogueBattleCharacter::AddItemDefinition(UFortRogueItemDefinition* ItemDefinition, int32 ChargesOverride)
{
	if (!ItemDefinition)
	{
		return;
	}

	for (FFortRogueItemStack& ItemStack : ItemLoadout)
	{
		if (ItemStack.ItemDefinition == ItemDefinition)
		{
			ItemStack.Charges += ChargesOverride == INDEX_NONE ? ItemDefinition->InitialCharges : FMath::Max(0, ChargesOverride);
			return;
		}
	}

	FFortRogueItemStack NewStack;
	NewStack.ItemDefinition = ItemDefinition;
	NewStack.Charges = ChargesOverride == INDEX_NONE ? ItemDefinition->InitialCharges : FMath::Max(0, ChargesOverride);
	ItemLoadout.Add(NewStack);
}

bool AFortRogueBattleCharacter::IsEnemy() const
{
	return bEnemy;
}

bool AFortRogueBattleCharacter::IsActiveTurn() const
{
	return bActiveTurn;
}

bool AFortRogueBattleCharacter::HasFiredThisTurn() const
{
	return bFiredThisTurn;
}

bool AFortRogueBattleCharacter::IsDefeated() const
{
	return CombatSet->GetHealth() <= 0.0f;
}

float AFortRogueBattleCharacter::GetHealth() const
{
	return CombatSet->GetHealth();
}

float AFortRogueBattleCharacter::GetMaxHealth() const
{
	return CombatSet->GetMaxHealth();
}

float AFortRogueBattleCharacter::GetMoveBudget() const
{
	return CombatSet->GetMoveBudget();
}

float AFortRogueBattleCharacter::GetAimAngle() const
{
	return AimAngle;
}

float AFortRogueBattleCharacter::GetShotPower() const
{
	return ShotPower;
}

float AFortRogueBattleCharacter::GetShotChargeAlpha() const
{
	const float MinPower = FMath::Min(MinShotPower, MaxShotPower);
	const float MaxPower = FMath::Max(MinShotPower, MaxShotPower);
	if (FMath::IsNearlyEqual(MinPower, MaxPower))
	{
		return 1.0f;
	}

	return FMath::Clamp((ShotPower - MinPower) / (MaxPower - MinPower), 0.0f, 1.0f);
}

bool AFortRogueBattleCharacter::IsChargingShot() const
{
	return bChargingShot;
}

int32 AFortRogueBattleCharacter::GetItemCharges(EFortRogueItemType ItemType) const
{
	int32 TotalCharges = 0;
	for (const FFortRogueItemStack& ItemStack : ItemLoadout)
	{
		if (ItemStack.ItemDefinition && ItemStack.ItemDefinition->ItemType == ItemType)
		{
			TotalCharges += ItemStack.Charges;
		}
	}
	return TotalCharges;
}

const FFortRogueWeaponSpec& AFortRogueBattleCharacter::GetCurrentWeapon() const
{
	if (WeaponLoadout.IsValidIndex(SelectedWeaponIndex))
	{
		return WeaponLoadout[SelectedWeaponIndex];
	}

	static const FFortRogueWeaponSpec FallbackWeapon;
	return FallbackWeapon;
}

FFortRogueWeaponSpec AFortRogueBattleCharacter::GetCurrentWeaponSpec() const
{
	return GetCurrentWeapon();
}

const TArray<FFortRogueWeaponSpec>& AFortRogueBattleCharacter::GetWeaponLoadout() const
{
	return WeaponLoadout;
}

TArray<FFortRogueWeaponSpec> AFortRogueBattleCharacter::GetWeaponLoadoutForBlueprint() const
{
	return WeaponLoadout;
}

int32 AFortRogueBattleCharacter::GetSelectedWeaponIndex() const
{
	return SelectedWeaponIndex;
}

FText AFortRogueBattleCharacter::GetCharacterDisplayName() const
{
	return CharacterDisplayName;
}

AFortRogueDestructibleTerrain* AFortRogueBattleCharacter::FindTerrain() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<AFortRogueDestructibleTerrain> It(World); It; ++It)
	{
		return *It;
	}

	return nullptr;
}

bool AFortRogueBattleCharacter::FindFootprintSurfaceZ(const AFortRogueDestructibleTerrain& Terrain, float CenterWorldX, float StartWorldZ, float SearchDistance, float& OutSurfaceZ) const
{
	bool bFoundSurface = false;
	float BestSurfaceZ = -TNumericLimits<float>::Max();
	const float SampleOffsets[] = { -FootProbeHalfWidth, 0.0f, FootProbeHalfWidth };
	for (const float SampleOffset : SampleOffsets)
	{
		float SampleSurfaceZ = 0.0f;
		if (Terrain.FindSurfaceZAtWorldX(CenterWorldX + SampleOffset, StartWorldZ, SearchDistance, SampleSurfaceZ))
		{
			BestSurfaceZ = FMath::Max(BestSurfaceZ, SampleSurfaceZ);
			bFoundSurface = true;
		}
	}

	if (bFoundSurface)
	{
		OutSurfaceZ = BestSurfaceZ;
	}

	return bFoundSurface;
}

bool AFortRogueBattleCharacter::IsFootprintBlocked(const AFortRogueDestructibleTerrain& Terrain, const FVector& CenterLocation, float FootWorldZ) const
{
	const float SampleOffsets[] = { -FootProbeHalfWidth, 0.0f, FootProbeHalfWidth };
	for (const float SampleOffset : SampleOffsets)
	{
		const FVector FootProbe(CenterLocation.X + SampleOffset, CenterLocation.Y, FootWorldZ + 1.0f);
		const FVector BodyProbe(CenterLocation.X + SampleOffset, CenterLocation.Y, FootWorldZ + FootOffsetZ * 0.5f);
		if (Terrain.IsSolidAtWorldLocation(FootProbe) || Terrain.IsSolidAtWorldLocation(BodyProbe))
		{
			return true;
		}
	}

	return false;
}

bool AFortRogueBattleCharacter::IsSlopeTraversable(float CurrentFootWorldZ, float NextSurfaceWorldZ, float HorizontalDistance, float TerrainCellSize) const
{
	if (NextSurfaceWorldZ <= CurrentFootWorldZ)
	{
		return true;
	}

	const float HorizontalSpan = FMath::Max(FMath::Abs(HorizontalDistance), FMath::Max(TerrainCellSize, 1.0f));
	const float MaxVerticalDelta = FMath::Tan(FMath::DegreesToRadians(MaxSlopeAngleDegrees)) * HorizontalSpan;
	return NextSurfaceWorldZ - CurrentFootWorldZ <= MaxVerticalDelta + KINDA_SMALL_NUMBER;
}

void AFortRogueBattleCharacter::UpdateBodyTerrainAlignment(const AFortRogueDestructibleTerrain* Terrain)
{
	if (!Body)
	{
		return;
	}

	if (!Terrain)
	{
		Body->SetRelativeRotation(FRotator::ZeroRotator);
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	const float CurrentFootZ = CurrentLocation.Z - FootOffsetZ;
	const float ProbeHalfWidth = FMath::Max(BodySlopeProbeHalfWidth, Terrain->CellSize);
	float LeftSurfaceZ = 0.0f;
	float RightSurfaceZ = 0.0f;
	const bool bLeftSurface = Terrain->FindSurfaceZAtWorldX(CurrentLocation.X - ProbeHalfWidth, CurrentFootZ + GroundSnapDistance, GroundSnapDistance + MaxStepDown, LeftSurfaceZ);
	const bool bRightSurface = Terrain->FindSurfaceZAtWorldX(CurrentLocation.X + ProbeHalfWidth, CurrentFootZ + GroundSnapDistance, GroundSnapDistance + MaxStepDown, RightSurfaceZ);
	if (!bLeftSurface || !bRightSurface)
	{
		Body->SetRelativeRotation(FRotator::ZeroRotator);
		return;
	}

	const float SlopeAngleDegrees = FMath::RadiansToDegrees(FMath::Atan2(RightSurfaceZ - LeftSurfaceZ, ProbeHalfWidth * 2.0f));
	const float ClampedPitch = FMath::Clamp(SlopeAngleDegrees, -MaxBodySlopeVisualDegrees, MaxBodySlopeVisualDegrees);
	Body->SetRelativeRotation(FRotator(ClampedPitch, 0.0f, 0.0f));
}

void AFortRogueBattleCharacter::ApplyTerrainGravity(float DeltaSeconds)
{
	if (IsDefeated())
	{
		return;
	}

	AFortRogueDestructibleTerrain* Terrain = FindTerrain();
	if (!Terrain)
	{
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	const float CurrentFootZ = CurrentLocation.Z - FootOffsetZ;
	float SurfaceZ = 0.0f;
	if (FindFootprintSurfaceZ(*Terrain, CurrentLocation.X, CurrentFootZ + GroundSnapDistance, GroundSnapDistance + 1.0f, SurfaceZ))
	{
		SetActorLocation(FVector(CurrentLocation.X, CurrentLocation.Y, SurfaceZ + FootOffsetZ));
		VerticalVelocity = 0.0f;
		UpdateBodyTerrainAlignment(Terrain);
		return;
	}

	VerticalVelocity = FMath::Max(VerticalVelocity - GravityAcceleration * DeltaSeconds, -MaxFallSpeed);
	const float FallDistance = FMath::Abs(VerticalVelocity * DeltaSeconds);
	if (FindFootprintSurfaceZ(*Terrain, CurrentLocation.X, CurrentFootZ, FallDistance + GroundSnapDistance, SurfaceZ))
	{
		SetActorLocation(FVector(CurrentLocation.X, CurrentLocation.Y, SurfaceZ + FootOffsetZ));
		VerticalVelocity = 0.0f;
		UpdateBodyTerrainAlignment(Terrain);
		return;
	}

	AddActorWorldOffset(FVector(0.0f, 0.0f, VerticalVelocity * DeltaSeconds));
	UpdateBodyTerrainAlignment(nullptr);
}

void AFortRogueBattleCharacter::SnapToTerrain()
{
	AFortRogueDestructibleTerrain* Terrain = FindTerrain();
	if (!Terrain)
	{
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	float SurfaceZ = 0.0f;
	if (FindFootprintSurfaceZ(*Terrain, CurrentLocation.X, CurrentLocation.Z, 2000.0f, SurfaceZ))
	{
		SetActorLocation(FVector(CurrentLocation.X, CurrentLocation.Y, SurfaceZ + FootOffsetZ));
		VerticalVelocity = 0.0f;
		UpdateBodyTerrainAlignment(Terrain);
	}
}

void AFortRogueBattleCharacter::GrantStartupAbilitySets()
{
	for (const UFortRogueAbilitySet* AbilitySet : StartupAbilitySets)
	{
		if (!AbilitySet)
		{
			continue;
		}

		FFortRogueAbilitySet_GrantedHandles Handles;
		AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, &Handles, this);
		StartupAbilitySetHandles.Add(Handles);
	}
}

void AFortRogueBattleCharacter::EnsureDefaultLoadout()
{
	if (WeaponLoadout.Num() == 0)
	{
		FFortRogueWeaponSpec Shell1;
		Shell1.DisplayName = FText::FromString(TEXT("Cannon Shot 1"));
		Shell1.Description = FText::FromString(TEXT("Fortress-style basic weapon 1: stable direct shell."));
		Shell1.WeaponTag = FortRogueGameplayTags::Weapon_Shell;
		Shell1.Damage = 35.0f;
		Shell1.BlastRadius = 145.0f;
		Shell1.ProjectileSpeed = 1180.0f;
		Shell1.ProjectilesPerShot = 1;
		WeaponLoadout.Add(Shell1);

		FFortRogueWeaponSpec Shell2;
		Shell2.DisplayName = FText::FromString(TEXT("Cannon Shot 2"));
		Shell2.Description = FText::FromString(TEXT("Fortress-style basic weapon 2: heavier arc and wider crater."));
		Shell2.WeaponTag = FortRogueGameplayTags::Weapon_Cluster;
		Shell2.Damage = 25.0f;
		Shell2.BlastRadius = 185.0f;
		Shell2.ProjectileSpeed = 980.0f;
		Shell2.ProjectilesPerShot = 2;
		WeaponLoadout.Add(Shell2);
	}

	if (ItemLoadout.Num() == 0)
	{
		UFortRogueItemDefinition* AttackItem = NewObject<UFortRogueItemDefinition>(this, TEXT("DefaultAttackMultiplierItem"));
		AttackItem->DisplayName = FText::FromString(TEXT("Attack Amp"));
		AttackItem->ItemTag = FortRogueGameplayTags::Trait_Damage;
		AttackItem->ItemType = EFortRogueItemType::AttackMultiplier;
		AttackItem->InitialCharges = 1;
		AttackItem->AttackMultiplier = 1.5f;

		FFortRogueItemStack AttackStack;
		AttackStack.ItemDefinition = AttackItem;
		AttackStack.Charges = AttackItem->InitialCharges;
		ItemLoadout.Add(AttackStack);

		UFortRogueItemDefinition* HealItem = NewObject<UFortRogueItemDefinition>(this, TEXT("DefaultHealItem"));
		HealItem->DisplayName = FText::FromString(TEXT("Repair Kit"));
		HealItem->ItemTag = FortRogueGameplayTags::Item_Repair;
		HealItem->ItemType = EFortRogueItemType::Heal;
		HealItem->InitialCharges = 1;
		HealItem->HealAmount = 35.0f;

		FFortRogueItemStack HealStack;
		HealStack.ItemDefinition = HealItem;
		HealStack.Charges = HealItem->InitialCharges;
		ItemLoadout.Add(HealStack);
	}
}
