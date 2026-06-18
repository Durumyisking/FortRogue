// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/FortRogueBattleCharacter.h"

#include "AbilitySystem/FortRogueAbilitySet.h"
#include "AbilitySystem/FortRogueAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/FortRogueCombatSet.h"
#include "Combat/FortRogueDestructibleTerrain.h"
#include "Combat/FortRogueProjectile.h"
#include "FortRogueGameMode.h"
#include "Items/FortRogueItemDefinition.h"
#include "Perks/FortRoguePerkDefinition.h"
#include "Run/FortRogueDefaultLoadoutDefinition.h"
#include "Weapons/FortRogueWeaponDefinition.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "PaperFlipbook.h"
#include "PaperFlipbookComponent.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
bool DoesShotModifierMatchTag(const FFortRogueShotModifierSpec& Modifier, FGameplayTag ModifierTag)
{
	return (Modifier.ModifierTag.IsValid() && Modifier.ModifierTag.MatchesTagExact(ModifierTag))
		|| Modifier.EffectTags.HasTagExact(ModifierTag);
}

bool DoesAbilitySetMatchTag(const UFortRogueAbilitySet* AbilitySet, FGameplayTag AbilitySetTag)
{
	return AbilitySet && AbilitySetTag.IsValid() && AbilitySet->AbilitySetTag.MatchesTagExact(AbilitySetTag);
}
}

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

	BodySprite = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("BodySprite"));
	BodySprite->SetupAttachment(Root);
	BodySprite->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodySprite->SetVisibility(false);
	BodySprite->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));

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
	DrawProjectileTrajectory();
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
	if (BodySprite)
	{
		BodySprite->SetFlipbook(InCharacterDefinition->BodyFlipbook);
		BodySprite->SetVisibility(InCharacterDefinition->BodyFlipbook != nullptr);
		if (Body)
		{
			Body->SetVisibility(InCharacterDefinition->BodyFlipbook == nullptr);
		}
	}

	CombatSet->SetMaxHealth(InCharacterDefinition->MaxHealth);
	CombatSet->SetHealth(InCharacterDefinition->MaxHealth);
	CombatSet->SetDamage(InCharacterDefinition->BonusDamage);
	CombatSet->SetMaxMoveBudget(InCharacterDefinition->MaxMoveBudget);
	CombatSet->SetMoveBudget(InCharacterDefinition->MaxMoveBudget);
	CombatSet->SetShotPowerMultiplier(InCharacterDefinition->ShotPowerMultiplier);

	GrantedShotModifiers.Reset();
	PendingShotModifiers.Reset();
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
	if (!bActiveTurn || bFiredThisTurn || IsDefeated() || !IsSupportedByTerrain() || FMath::IsNearlyZero(Axis))
	{
		return;
	}

	SetFacingFromAxis(Axis);

	const float RequestedDelta = Axis * MoveSpeed * DeltaSeconds;
	const float Budget = CombatSet->GetMoveBudget();
	const float AppliedDelta = FMath::Clamp(RequestedDelta, -Budget, Budget);
	if (FMath::IsNearlyZero(AppliedDelta))
	{
		return;
	}

	FVector NewLocation = GetActorLocation();
	float ActualDelta = 0.0f;
	bool bLostSupport = false;

	if (AFortRogueDestructibleTerrain* Terrain = FindTerrain())
	{
		const float StepLength = FMath::Max(1.0f, Terrain->CellSize * 0.5f);
		const int32 StepCount = FMath::Max(1, FMath::CeilToInt(FMath::Abs(AppliedDelta) / StepLength));
		const float StepDelta = AppliedDelta / static_cast<float>(StepCount);

		for (int32 Step = 0; Step < StepCount; ++Step)
		{
			FVector StepLocation = NewLocation;
			StepLocation.X += StepDelta;
			StepLocation.X = ClampWorldXToTerrainBounds(*Terrain, StepLocation.X);
			const float ClampedStepDelta = StepLocation.X - NewLocation.X;
			if (FMath::IsNearlyZero(ClampedStepDelta))
			{
				break;
			}

			const float CurrentFootZ = NewLocation.Z - FootOffsetZ;
			float SurfaceZ = 0.0f;
			if (FindFootprintSurfaceZ(*Terrain, StepLocation.X, CurrentFootZ + MaxStepUp, MaxStepUp + MaxStepDown, SurfaceZ))
			{
				if (!IsSlopeTraversable(CurrentFootZ, SurfaceZ, ClampedStepDelta, Terrain->CellSize))
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
			else
			{
				NewLocation = StepLocation;
				ActualDelta += ClampedStepDelta;
				bLostSupport = true;
				break;
			}

			NewLocation = StepLocation;
			ActualDelta += ClampedStepDelta;
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
	if (bLostSupport)
	{
		ReevaluateTerrainSupport();
	}
	else
	{
		UpdateBodyTerrainAlignment(FindTerrain());
	}
	CombatSet->SetMoveBudget(Budget - FMath::Abs(ActualDelta));
}

void AFortRogueBattleCharacter::AdjustAim(float Axis, float DeltaSeconds)
{
	if (!bActiveTurn || bFiredThisTurn || !IsSupportedByTerrain() || FMath::IsNearlyZero(Axis))
	{
		return;
	}

	AimAngle = FMath::Clamp(AimAngle + Axis * 70.0f * DeltaSeconds, 5.0f, 90.0f);
}

void AFortRogueBattleCharacter::AdjustPower(float Axis, float DeltaSeconds)
{
	if (!bActiveTurn || bFiredThisTurn || !IsSupportedByTerrain() || FMath::IsNearlyZero(Axis))
	{
		return;
	}

	ShotPower = FMath::Clamp(ShotPower + Axis * 0.75f * DeltaSeconds, FMath::Min(MinShotPower, MaxShotPower), FMath::Max(MinShotPower, MaxShotPower));
}

void AFortRogueBattleCharacter::BeginShotCharge()
{
	if (!bActiveTurn || bFiredThisTurn || IsDefeated() || !IsSupportedByTerrain())
	{
		return;
	}

	bChargingShot = true;
	ShotChargeElapsed = 0.0f;
	ShotPower = FMath::Min(MinShotPower, MaxShotPower);
}

void AFortRogueBattleCharacter::UpdateShotCharge(float DeltaSeconds)
{
	if (!bChargingShot || !bActiveTurn || bFiredThisTurn || IsDefeated() || !IsSupportedByTerrain())
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

bool AFortRogueBattleCharacter::SelectWeaponByTag(FGameplayTag WeaponTag)
{
	if (!WeaponTag.IsValid())
	{
		return false;
	}

	for (int32 WeaponIndex = 0; WeaponIndex < WeaponLoadout.Num(); ++WeaponIndex)
	{
		if (WeaponLoadout[WeaponIndex].WeaponTag.MatchesTagExact(WeaponTag))
		{
			SelectedWeaponIndex = WeaponIndex;
			return true;
		}
	}

	return false;
}

int32 AFortRogueBattleCharacter::FireSelectedWeapon()
{
	if (!bActiveTurn || bFiredThisTurn || IsDefeated() || !IsSupportedByTerrain())
	{
		return 0;
	}
	if (!WeaponLoadout.IsValidIndex(SelectedWeaponIndex))
	{
		return 0;
	}

	bChargingShot = false;
	bFiredThisTurn = true;

	const FFortRogueWeaponSpec& Weapon = GetCurrentWeapon();
	const FFortRogueShotSpec ShotSpec = BuildShotSpec(Weapon);
	const int32 ProjectileCount = ShotSpec.ProjectileCount;
	int32 SpawnedProjectiles = 0;
	PendingAttackMultiplier = 1.0f;
	PendingShotModifiers.Reset();

	for (int32 Index = 0; Index < ProjectileCount; ++Index)
	{
		const float Spread = (ProjectileCount > 1) ? (Index - (ProjectileCount - 1) * 0.5f) * 5.0f : 0.0f;
		const FVector Direction = GetProjectileLaunchDirection(Spread);
		const FVector SpawnLocation = GetProjectileSpawnLocation(Direction);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		AFortRogueProjectile* Projectile = GetWorld()->SpawnActor<AFortRogueProjectile>(ShotSpec.ProjectileClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
		if (Projectile)
		{
			Projectile->InitializeProjectileFromShotSpec(this, FindTerrain(), Direction * ShotSpec.LaunchSpeed, ShotSpec);
			if (AFortRogueGameMode* GameMode = GetWorld()->GetAuthGameMode<AFortRogueGameMode>())
			{
				GameMode->NotifyProjectileSpawned(Projectile);
			}
			++SpawnedProjectiles;
		}
	}

	return SpawnedProjectiles;
}

void AFortRogueBattleCharacter::FireAtTarget(AFortRogueBattleCharacter* Target, const FFortRogueStageDifficultyData& DifficultyData)
{
	if (!Target)
	{
		return;
	}

	const FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
	bFacingRight = ToTarget.X >= 0.0f;
	const float AimArcHeight = FMath::Max(DifficultyData.MinAimArcHeight, FMath::Abs(ToTarget.Z) + DifficultyData.AimHeightOffset);
	const float Distance = FMath::Max(1.0f, FMath::Abs(ToTarget.X));
	const float AimError = DifficultyData.AimAngleErrorDegrees > 0.0f ? FMath::RandRange(-DifficultyData.AimAngleErrorDegrees, DifficultyData.AimAngleErrorDegrees) : 0.0f;
	const float PowerError = DifficultyData.ShotPowerError > 0.0f ? FMath::RandRange(-DifficultyData.ShotPowerError, DifficultyData.ShotPowerError) : 0.0f;
	const float MinAim = FMath::Min(DifficultyData.MinAimAngleDegrees, DifficultyData.MaxAimAngleDegrees);
	const float MaxAim = FMath::Max(DifficultyData.MinAimAngleDegrees, DifficultyData.MaxAimAngleDegrees);
	const float MinPower = FMath::Min(DifficultyData.MinShotPower, DifficultyData.MaxShotPower);
	const float MaxPower = FMath::Max(DifficultyData.MinShotPower, DifficultyData.MaxShotPower);
	AimAngle = FMath::Clamp(FMath::RadiansToDegrees(FMath::Atan2(AimArcHeight, Distance)) + AimError, MinAim, MaxAim);
	ShotPower = FMath::Clamp(Distance / FMath::Max(1.0f, DifficultyData.ShotPowerDistanceScale) + PowerError, MinPower, MaxPower);
}

void AFortRogueBattleCharacter::ApplyDamage(float DamageAmount)
{
	CombatSet->ApplyDamage(DamageAmount);
}

void AFortRogueBattleCharacter::ReevaluateTerrainSupport()
{
	AFortRogueDestructibleTerrain* Terrain = FindTerrain();
	if (!Terrain)
	{
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	const float ClampedWorldX = ClampWorldXToTerrainBounds(*Terrain, CurrentLocation.X);
	const float CurrentFootZ = CurrentLocation.Z - FootOffsetZ;
	float SurfaceZ = 0.0f;
	if (FindFootprintSurfaceZ(*Terrain, ClampedWorldX, CurrentFootZ + GroundSnapDistance, GroundSnapDistance + MaxStepDown, SurfaceZ))
	{
		SetActorLocation(FVector(ClampedWorldX, CurrentLocation.Y, SurfaceZ + FootOffsetZ));
		VerticalVelocity = 0.0f;
		UpdateBodyTerrainAlignment(Terrain);
		return;
	}

	if (!FMath::IsNearlyEqual(ClampedWorldX, static_cast<float>(CurrentLocation.X)))
	{
		SetActorLocation(FVector(ClampedWorldX, CurrentLocation.Y, CurrentLocation.Z));
	}
	bChargingShot = false;
	ApplyTerrainGravity(1.0f / 60.0f);
}

void AFortRogueBattleCharacter::SetTerrain(AFortRogueDestructibleTerrain* InTerrain)
{
	AssignedTerrain = InTerrain;
	if (HasActorBegunPlay())
	{
		SnapToTerrain();
	}
}

bool AFortRogueBattleCharacter::UseItemByType(EFortRogueItemType ItemType)
{
	if (!bActiveTurn || IsDefeated() || !IsSupportedByTerrain())
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

		return UseItemStack(ItemStack);
	}

	return false;
}

bool AFortRogueBattleCharacter::UseItemByTag(FGameplayTag ItemTag)
{
	if (!ItemTag.IsValid() || !bActiveTurn || IsDefeated() || !IsSupportedByTerrain())
	{
		return false;
	}

	for (FFortRogueItemStack& ItemStack : ItemLoadout)
	{
		const UFortRogueItemDefinition* ItemDefinition = ItemStack.ItemDefinition;
		if (!ItemDefinition || !ItemDefinition->ItemTag.MatchesTagExact(ItemTag) || ItemStack.Charges <= 0)
		{
			continue;
		}

		return UseItemStack(ItemStack);
	}

	return false;
}

bool AFortRogueBattleCharacter::UseItemByIndex(int32 ItemIndex)
{
	if (!bActiveTurn || IsDefeated() || !IsSupportedByTerrain() || !ItemLoadout.IsValidIndex(ItemIndex))
	{
		return false;
	}

	return UseItemStack(ItemLoadout[ItemIndex]);
}

bool AFortRogueBattleCharacter::UseItemStack(FFortRogueItemStack& ItemStack)
{
	const UFortRogueItemDefinition* ItemDefinition = ItemStack.ItemDefinition;
	if (!ItemDefinition || ItemStack.Charges <= 0)
	{
		return false;
	}

	--ItemStack.Charges;
	if (ItemDefinition->ItemType == EFortRogueItemType::Heal)
	{
		CombatSet->Heal(ItemDefinition->HealAmount);
	}
	else if (ItemDefinition->ItemType == EFortRogueItemType::AttackMultiplier)
	{
		PendingAttackMultiplier = FMath::Max(PendingAttackMultiplier, ItemDefinition->AttackMultiplier);
	}
	GrantAbilitySet(ItemDefinition->UseAbilitySet);
	PendingShotModifiers.Append(ItemDefinition->UseShotModifiers);
	return true;
}

void AFortRogueBattleCharacter::ApplyRewardDamage(float BonusDamage)
{
	CombatSet->AddDamage(BonusDamage);
}

void AFortRogueBattleCharacter::ApplyRewardHealth(float BonusHealth)
{
	CombatSet->AddMaxHealth(BonusHealth);
}

void AFortRogueBattleCharacter::ApplyRewardMoveBudget(float BonusMoveBudget)
{
	CombatSet->AddMaxMoveBudget(BonusMoveBudget);
}

void AFortRogueBattleCharacter::ApplyRewardProjectiles(int32 BonusProjectiles)
{
	CombatSet->AddProjectileCount(BonusProjectiles);
}

void AFortRogueBattleCharacter::ApplyRewardShotPowerMultiplier(float BonusMultiplier)
{
	CombatSet->AddShotPowerMultiplier(BonusMultiplier);
}

void AFortRogueBattleCharacter::ApplyPerkDefinition(UFortRoguePerkDefinition* PerkDefinition)
{
	if (!PerkDefinition)
	{
		return;
	}

	ApplyRewardDamage(PerkDefinition->DamageBonus);
	ApplyRewardHealth(PerkDefinition->MaxHealthBonus);
	ApplyRewardMoveBudget(PerkDefinition->MaxMoveBudgetBonus);
	ApplyRewardProjectiles(PerkDefinition->ProjectileBonus);
	ApplyRewardShotPowerMultiplier(PerkDefinition->ShotPowerMultiplierBonus);
	GrantShotModifiers(PerkDefinition->ShotModifiers);

	GrantAbilitySet(PerkDefinition->GrantedAbilitySet);
}

void AFortRogueBattleCharacter::GrantAbilitySet(UFortRogueAbilitySet* AbilitySet)
{
	if (!AbilitySet || !AbilitySystemComponent)
	{
		return;
	}

	FFortRogueGrantedAbilitySetEntry Entry;
	Entry.AbilitySet = AbilitySet;
	AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, &Entry.Handles, this);
	GrantedAbilitySetEntries.Add(MoveTemp(Entry));
}

bool AFortRogueBattleCharacter::RemoveAbilitySet(UFortRogueAbilitySet* AbilitySet)
{
	if (!AbilitySet || !AbilitySystemComponent)
	{
		return false;
	}

	for (int32 Index = GrantedAbilitySetEntries.Num() - 1; Index >= 0; --Index)
	{
		FFortRogueGrantedAbilitySetEntry& Entry = GrantedAbilitySetEntries[Index];
		if (Entry.AbilitySet != AbilitySet)
		{
			continue;
		}

		Entry.Handles.TakeFromAbilitySystem(AbilitySystemComponent);
		GrantedAbilitySetEntries.RemoveAt(Index);
		return true;
	}

	return false;
}

int32 AFortRogueBattleCharacter::RemoveAbilitySetsByTag(FGameplayTag AbilitySetTag)
{
	if (!AbilitySetTag.IsValid() || !AbilitySystemComponent)
	{
		return 0;
	}

	int32 RemovedCount = 0;
	for (int32 Index = GrantedAbilitySetEntries.Num() - 1; Index >= 0; --Index)
	{
		FFortRogueGrantedAbilitySetEntry& Entry = GrantedAbilitySetEntries[Index];
		if (!DoesAbilitySetMatchTag(Entry.AbilitySet, AbilitySetTag))
		{
			continue;
		}

		Entry.Handles.TakeFromAbilitySystem(AbilitySystemComponent);
		GrantedAbilitySetEntries.RemoveAt(Index);
		++RemovedCount;
	}
	return RemovedCount;
}

int32 AFortRogueBattleCharacter::GetGrantedAbilitySetCount(UFortRogueAbilitySet* AbilitySet) const
{
	if (!AbilitySet)
	{
		return 0;
	}

	int32 GrantedCount = 0;
	for (const FFortRogueGrantedAbilitySetEntry& Entry : GrantedAbilitySetEntries)
	{
		if (Entry.AbilitySet == AbilitySet)
		{
			++GrantedCount;
		}
	}
	return GrantedCount;
}

int32 AFortRogueBattleCharacter::GetGrantedAbilitySetCountByTag(FGameplayTag AbilitySetTag) const
{
	if (!AbilitySetTag.IsValid())
	{
		return 0;
	}

	int32 GrantedCount = 0;
	for (const FFortRogueGrantedAbilitySetEntry& Entry : GrantedAbilitySetEntries)
	{
		if (DoesAbilitySetMatchTag(Entry.AbilitySet, AbilitySetTag))
		{
			++GrantedCount;
		}
	}
	return GrantedCount;
}

void AFortRogueBattleCharacter::GrantShotModifiers(const TArray<FFortRogueShotModifierSpec>& ShotModifiers)
{
	if (ShotModifiers.Num() <= 0)
	{
		return;
	}

	GrantedShotModifiers.Append(ShotModifiers);
}

int32 AFortRogueBattleCharacter::RemoveGrantedShotModifiersByTag(FGameplayTag ModifierTag)
{
	if (!ModifierTag.IsValid())
	{
		return 0;
	}

	int32 RemovedCount = 0;
	for (int32 Index = GrantedShotModifiers.Num() - 1; Index >= 0; --Index)
	{
		const FFortRogueShotModifierSpec& Modifier = GrantedShotModifiers[Index];
		if (!DoesShotModifierMatchTag(Modifier, ModifierTag))
		{
			continue;
		}

		GrantedShotModifiers.RemoveAt(Index);
		++RemovedCount;
	}

	return RemovedCount;
}

int32 AFortRogueBattleCharacter::RemovePendingShotModifiersByTag(FGameplayTag ModifierTag)
{
	if (!ModifierTag.IsValid())
	{
		return 0;
	}

	int32 RemovedCount = 0;
	for (int32 Index = PendingShotModifiers.Num() - 1; Index >= 0; --Index)
	{
		const FFortRogueShotModifierSpec& Modifier = PendingShotModifiers[Index];
		if (!DoesShotModifierMatchTag(Modifier, ModifierTag))
		{
			continue;
		}

		PendingShotModifiers.RemoveAt(Index);
		++RemovedCount;
	}

	return RemovedCount;
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

float AFortRogueBattleCharacter::GetMaxMoveBudget() const
{
	return CombatSet->GetMaxMoveBudget();
}

float AFortRogueBattleCharacter::GetDamageBonus() const
{
	return CombatSet->GetDamage();
}

float AFortRogueBattleCharacter::GetShotPowerMultiplier() const
{
	return CombatSet->GetShotPowerMultiplier();
}

float AFortRogueBattleCharacter::GetProjectileCount() const
{
	return CombatSet->GetProjectileCount();
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

int32 AFortRogueBattleCharacter::GetItemChargesByTag(FGameplayTag ItemTag) const
{
	if (!ItemTag.IsValid())
	{
		return 0;
	}

	int32 TotalCharges = 0;
	for (const FFortRogueItemStack& ItemStack : ItemLoadout)
	{
		if (ItemStack.ItemDefinition && ItemStack.ItemDefinition->ItemTag.MatchesTagExact(ItemTag))
		{
			TotalCharges += ItemStack.Charges;
		}
	}
	return TotalCharges;
}

const TArray<FFortRogueItemStack>& AFortRogueBattleCharacter::GetItemLoadout() const
{
	return ItemLoadout;
}

TArray<FFortRogueItemStack> AFortRogueBattleCharacter::GetItemLoadoutForBlueprint() const
{
	return ItemLoadout;
}

int32 AFortRogueBattleCharacter::GetGrantedShotModifierCountByTag(FGameplayTag ModifierTag) const
{
	if (!ModifierTag.IsValid())
	{
		return 0;
	}

	int32 TotalCount = 0;
	for (const FFortRogueShotModifierSpec& Modifier : GrantedShotModifiers)
	{
		if (DoesShotModifierMatchTag(Modifier, ModifierTag))
		{
			++TotalCount;
		}
	}
	return TotalCount;
}

int32 AFortRogueBattleCharacter::GetPendingShotModifierCountByTag(FGameplayTag ModifierTag) const
{
	if (!ModifierTag.IsValid())
	{
		return 0;
	}

	int32 TotalCount = 0;
	for (const FFortRogueShotModifierSpec& Modifier : PendingShotModifiers)
	{
		if (DoesShotModifierMatchTag(Modifier, ModifierTag))
		{
			++TotalCount;
		}
	}
	return TotalCount;
}

const FFortRogueWeaponSpec& AFortRogueBattleCharacter::GetCurrentWeapon() const
{
	if (WeaponLoadout.IsValidIndex(SelectedWeaponIndex))
	{
		return WeaponLoadout[SelectedWeaponIndex];
	}

	static const FFortRogueWeaponSpec FallbackWeapon = []()
	{
		FFortRogueWeaponSpec Weapon;
		Weapon.DisplayName = FText::FromString(TEXT("No Weapon"));
		Weapon.Description = FText::GetEmpty();
		Weapon.Damage = 0.0f;
		Weapon.BlastRadius = 0.0f;
		Weapon.ProjectileSpeed = 0.0f;
		Weapon.Gravity = 0.0f;
		Weapon.ProjectilesPerShot = 0;
		return Weapon;
	}();
	return FallbackWeapon;
}

FFortRogueWeaponSpec AFortRogueBattleCharacter::GetCurrentWeaponSpec() const
{
	return GetCurrentWeapon();
}

FFortRogueShotSpec AFortRogueBattleCharacter::GetCurrentShotSpec() const
{
	return BuildShotSpec(GetCurrentWeapon());
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
	if (AssignedTerrain)
	{
		return AssignedTerrain;
	}

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

bool AFortRogueBattleCharacter::IsSupportedByTerrain() const
{
	AFortRogueDestructibleTerrain* Terrain = FindTerrain();
	if (!Terrain)
	{
		return true;
	}

	const FVector CurrentLocation = GetActorLocation();
	const float ClampedWorldX = ClampWorldXToTerrainBounds(*Terrain, CurrentLocation.X);
	const float CurrentFootZ = CurrentLocation.Z - FootOffsetZ;
	float SurfaceZ = 0.0f;
	return FindFootprintSurfaceZ(*Terrain, ClampedWorldX, CurrentFootZ + GroundSnapDistance, GroundSnapDistance + 1.0f, SurfaceZ);
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

float AFortRogueBattleCharacter::ClampWorldXToTerrainBounds(const AFortRogueDestructibleTerrain& Terrain, float WorldX) const
{
	const float HalfWidth = Terrain.Width * 0.5f;
	const float EdgePadding = FMath::Clamp(FootProbeHalfWidth, 0.0f, HalfWidth);
	const float MinX = Terrain.GetActorLocation().X - HalfWidth + EdgePadding;
	const float MaxX = Terrain.GetActorLocation().X + HalfWidth - EdgePadding;
	if (MinX > MaxX)
	{
		return Terrain.GetActorLocation().X;
	}

	return FMath::Clamp(WorldX, MinX, MaxX);
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
	const float ClampedWorldX = ClampWorldXToTerrainBounds(*Terrain, CurrentLocation.X);
	const float CurrentFootZ = CurrentLocation.Z - FootOffsetZ;
	float SurfaceZ = 0.0f;
	if (FindFootprintSurfaceZ(*Terrain, ClampedWorldX, CurrentFootZ + GroundSnapDistance, GroundSnapDistance + 1.0f, SurfaceZ))
	{
		SetActorLocation(FVector(ClampedWorldX, CurrentLocation.Y, SurfaceZ + FootOffsetZ));
		VerticalVelocity = 0.0f;
		UpdateBodyTerrainAlignment(Terrain);
		return;
	}

	bChargingShot = false;
	VerticalVelocity = FMath::Max(VerticalVelocity - GravityAcceleration * DeltaSeconds, -MaxFallSpeed);
	const float FallDistance = FMath::Abs(VerticalVelocity * DeltaSeconds);
	if (FindFootprintSurfaceZ(*Terrain, ClampedWorldX, CurrentFootZ, FallDistance + GroundSnapDistance, SurfaceZ))
	{
		SetActorLocation(FVector(ClampedWorldX, CurrentLocation.Y, SurfaceZ + FootOffsetZ));
		VerticalVelocity = 0.0f;
		UpdateBodyTerrainAlignment(Terrain);
		return;
	}

	SetActorLocation(FVector(ClampedWorldX, CurrentLocation.Y, CurrentLocation.Z + VerticalVelocity * DeltaSeconds));
	UpdateBodyTerrainAlignment(nullptr);
	CheckFallDeath();
}

bool AFortRogueBattleCharacter::CheckFallDeath()
{
	AFortRogueDestructibleTerrain* Terrain = FindTerrain();
	if (!Terrain || IsDefeated())
	{
		return false;
	}

	if (GetActorLocation().Z >= Terrain->GetActorLocation().Z - FallDeathDepth)
	{
		return false;
	}

	bChargingShot = false;
	VerticalVelocity = 0.0f;
	ApplyDamage(GetMaxHealth());
	return true;
}

void AFortRogueBattleCharacter::SnapToTerrain()
{
	AFortRogueDestructibleTerrain* Terrain = FindTerrain();
	if (!Terrain)
	{
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	const float ClampedWorldX = ClampWorldXToTerrainBounds(*Terrain, CurrentLocation.X);
	float SurfaceZ = 0.0f;
	if (FindFootprintSurfaceZ(*Terrain, ClampedWorldX, CurrentLocation.Z, 2000.0f, SurfaceZ))
	{
		SetActorLocation(FVector(ClampedWorldX, CurrentLocation.Y, SurfaceZ + FootOffsetZ));
		VerticalVelocity = 0.0f;
		UpdateBodyTerrainAlignment(Terrain);
	}
}

void AFortRogueBattleCharacter::SetFacingFromAxis(float Axis)
{
	if (!FMath::IsNearlyZero(Axis))
	{
		bFacingRight = Axis > 0.0f;
	}
}

float AFortRogueBattleCharacter::GetBodyPitchDegrees() const
{
	return Body ? Body->GetRelativeRotation().Pitch : 0.0f;
}

FVector AFortRogueBattleCharacter::GetProjectileLaunchDirection(float SpreadDegrees) const
{
	const float RelativeAimDegrees = FMath::Clamp(AimAngle + SpreadDegrees, 5.0f, 90.0f);
	const float WorldAngleDegrees = bFacingRight ? GetBodyPitchDegrees() + RelativeAimDegrees : GetBodyPitchDegrees() + 180.0f - RelativeAimDegrees;
	const float WorldAngleRadians = FMath::DegreesToRadians(WorldAngleDegrees);
	return FVector(FMath::Cos(WorldAngleRadians), 0.0f, FMath::Sin(WorldAngleRadians)).GetSafeNormal();
}

FVector AFortRogueBattleCharacter::GetProjectileSpawnLocation(const FVector& LaunchDirection) const
{
	return GetActorLocation() + LaunchDirection * 70.0f + FVector(0.0f, 0.0f, 35.0f);
}

FFortRogueShotSpec AFortRogueBattleCharacter::BuildShotSpec(const FFortRogueWeaponSpec& Weapon) const
{
	FFortRogueShotSpec ShotSpec;
	ShotSpec.WeaponTag = Weapon.WeaponTag;
	ShotSpec.EffectTags = Weapon.ShotEffectTags;
	if (Weapon.WeaponTag.IsValid())
	{
		ShotSpec.EffectTags.AddTag(Weapon.WeaponTag);
	}
	ShotSpec.Damage = FMath::Max(0.0f, (Weapon.Damage + CombatSet->GetDamage()) * PendingAttackMultiplier);
	ShotSpec.BlastRadius = FMath::Max(0.0f, Weapon.BlastRadius);
	ShotSpec.TerrainCarveRadius = ShotSpec.BlastRadius;
	ShotSpec.TerrainFillRadius = 0.0f;
	ShotSpec.LaunchSpeed = FMath::Max(0.0f, Weapon.ProjectileSpeed * ShotPower * CombatSet->GetShotPowerMultiplier());
	ShotSpec.Gravity = FMath::Max(0.0f, Weapon.Gravity);
	ShotSpec.ProjectileCount = FMath::Max(1, Weapon.ProjectilesPerShot + FMath::RoundToInt(CombatSet->GetProjectileCount()) - 1);
	ShotSpec.ProjectileClass = Weapon.ProjectileClass ? Weapon.ProjectileClass : TSubclassOf<AFortRogueProjectile>(AFortRogueProjectile::StaticClass());
	ShotSpec.ImpactSpawns = Weapon.ImpactSpawns;
	auto ApplyShotModifier = [this, &ShotSpec](const FFortRogueShotModifierSpec& Modifier)
	{
		if (Modifier.bUseAimAngleRange)
		{
			const float MinAngle = FMath::Min(Modifier.MinAimAngle, Modifier.MaxAimAngle);
			const float MaxAngle = FMath::Max(Modifier.MinAimAngle, Modifier.MaxAimAngle);
			if (AimAngle < MinAngle || AimAngle > MaxAngle)
			{
				return;
			}
		}
		if (Modifier.bRequireWindAligned)
		{
			const AFortRogueGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFortRogueGameMode>() : nullptr;
			const float Wind = GameMode ? GameMode->GetWind() : 0.0f;
			const float ShotDirection = bFacingRight ? 1.0f : -1.0f;
			if (FMath::Abs(Wind) < Modifier.MinWindMagnitude || Wind * ShotDirection <= 0.0f)
			{
				return;
			}
		}
		if (!Modifier.RequiredShotTags.IsEmpty() && !ShotSpec.EffectTags.HasAny(Modifier.RequiredShotTags))
		{
			return;
		}
		if (!Modifier.BlockedShotTags.IsEmpty() && ShotSpec.EffectTags.HasAny(Modifier.BlockedShotTags))
		{
			return;
		}

		ShotSpec.EffectTags.AppendTags(Modifier.EffectTags);
		ShotSpec.Damage = FMath::Max(0.0f, (ShotSpec.Damage + Modifier.DamageBonus) * Modifier.DamageMultiplier);
		ShotSpec.BlastRadius = FMath::Max(0.0f, (ShotSpec.BlastRadius + Modifier.BlastRadiusBonus) * Modifier.BlastRadiusMultiplier);
		ShotSpec.TerrainCarveRadius = FMath::Max(0.0f, (ShotSpec.TerrainCarveRadius + Modifier.TerrainCarveRadiusBonus) * Modifier.TerrainCarveRadiusMultiplier);
		ShotSpec.TerrainFillRadius = FMath::Max(0.0f, (ShotSpec.TerrainFillRadius + Modifier.TerrainFillRadiusBonus) * Modifier.TerrainFillRadiusMultiplier);
		ShotSpec.LaunchSpeed = FMath::Max(0.0f, ShotSpec.LaunchSpeed * Modifier.LaunchSpeedMultiplier);
		ShotSpec.Gravity = FMath::Max(0.0f, ShotSpec.Gravity * Modifier.GravityMultiplier);
		ShotSpec.ProjectileCount = FMath::Max(1, ShotSpec.ProjectileCount + Modifier.ProjectileCountBonus);
		ShotSpec.ImpactSpawns.Append(Modifier.ImpactSpawns);
	};
	for (const FFortRogueShotModifierSpec& Modifier : Weapon.ShotModifiers)
	{
		ApplyShotModifier(Modifier);
	}
	for (const FFortRogueShotModifierSpec& Modifier : GrantedShotModifiers)
	{
		ApplyShotModifier(Modifier);
	}
	for (const FFortRogueShotModifierSpec& Modifier : PendingShotModifiers)
	{
		ApplyShotModifier(Modifier);
	}
	return ShotSpec;
}

void AFortRogueBattleCharacter::DrawProjectileTrajectory() const
{
	if (!bDrawProjectileTrajectory || !bActiveTurn || bFiredThisTurn || IsDefeated() || !IsSupportedByTerrain() || !GetWorld())
	{
		return;
	}
	if (!WeaponLoadout.IsValidIndex(SelectedWeaponIndex))
	{
		return;
	}

	const FFortRogueWeaponSpec& Weapon = GetCurrentWeapon();
	const FFortRogueShotSpec ShotSpec = BuildShotSpec(Weapon);
	FVector Velocity = GetProjectileLaunchDirection(0.0f) * ShotSpec.LaunchSpeed;
	FVector Location = GetProjectileSpawnLocation(Velocity.GetSafeNormal());
	const float StepSeconds = FMath::Max(TrajectoryDebugTimeStep, KINDA_SMALL_NUMBER);
	const int32 StepCount = FMath::Max(1, TrajectoryDebugSteps);
	const AFortRogueGameMode* GameMode = GetWorld()->GetAuthGameMode<AFortRogueGameMode>();
	const float Wind = GameMode ? GameMode->GetWind() : 0.0f;
	const AFortRogueDestructibleTerrain* Terrain = FindTerrain();

	for (int32 Step = 0; Step < StepCount; ++Step)
	{
		const FVector PreviousLocation = Location;
		Velocity += FVector(Wind, 0.0f, -ShotSpec.Gravity) * StepSeconds;
		Location += Velocity * StepSeconds;
		DrawDebugLine(GetWorld(), PreviousLocation, Location, FColor::Yellow, false, 0.0f, 0, 2.0f);

		if (Terrain && Terrain->IsSolidAtWorldLocation(Location))
		{
			DrawDebugSphere(GetWorld(), Location, 16.0f, 12, FColor::Red, false, 0.0f, 0, 2.0f);
			break;
		}
	}
}

void AFortRogueBattleCharacter::GrantStartupAbilitySets()
{
	for (UFortRogueAbilitySet* AbilitySet : StartupAbilitySets)
	{
		GrantAbilitySet(AbilitySet);
	}
}

void AFortRogueBattleCharacter::EnsureDefaultLoadout()
{
	if (DefaultLoadoutDefinition && WeaponLoadout.Num() == 0)
	{
		for (UFortRogueWeaponDefinition* WeaponDefinition : DefaultLoadoutDefinition->WeaponDefinitions)
		{
			AddWeaponDefinition(WeaponDefinition);
		}
	}

	if (DefaultLoadoutDefinition && ItemLoadout.Num() == 0)
	{
		for (const FFortRogueDefaultItemStack& ItemStack : DefaultLoadoutDefinition->ItemDefinitions)
		{
			AddItemDefinition(ItemStack.ItemDefinition, ItemStack.Charges);
		}
	}
}
