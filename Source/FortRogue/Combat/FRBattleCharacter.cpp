// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/FRBattleCharacter.h"

#include "AbilitySystem/FRAbilitySet.h"
#include "AbilitySystem/FRAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/FRCombatSet.h"
#include "Combat/FRDestructibleTerrain.h"
#include "Combat/FRProjectile.h"
#include "FRGameMode.h"
#include "FRGameplayTags.h"
#include "Items/FRItemDefinition.h"
#include "Perks/FRPerkDefinition.h"
#include "Rewards/FRRewardBlueprintLibrary.h"
#include "Run/FRDefaultLoadoutDefinition.h"
#include "Weapons/FRWeaponDefinition.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "PaperFlipbook.h"
#include "PaperFlipbookComponent.h"
#include "UI/FRBattleCharacterStatusWidget.h"
#include "UI/FRFloatingDamageTextActor.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
constexpr float DefaultMaxCharacterSlopeDegrees = 45.0f;
constexpr float MaxUsableSlopeDegrees = 89.0f;

bool DoesShotModifierMatchTag(const FFRShotModifierSpec& Modifier, FGameplayTag ModifierTag)
{
	return (Modifier.ModifierTag.IsValid() && Modifier.ModifierTag.MatchesTagExact(ModifierTag))
		|| Modifier.EffectTags.HasTagExact(ModifierTag);
}

bool DoesAbilitySetMatchTag(const UFRAbilitySet* AbilitySet, FGameplayTag AbilitySetTag)
{
	return AbilitySet && AbilitySetTag.IsValid() && AbilitySet->AbilitySetTag.MatchesTagExact(AbilitySetTag);
}

float GetTrajectorySegmentAlphaXZ(const FVector& StartLocation, const FVector& EndLocation, const FVector& TestLocation)
{
	const FVector2D Segment(EndLocation.X - StartLocation.X, EndLocation.Z - StartLocation.Z);
	const FVector2D ToTest(TestLocation.X - StartLocation.X, TestLocation.Z - StartLocation.Z);
	const float SegmentLengthSq = Segment.SizeSquared();
	if (SegmentLengthSq <= KINDA_SMALL_NUMBER)
	{
		return 0.0f;
	}

	return FMath::Clamp(FVector2D::DotProduct(ToTest, Segment) / SegmentLengthSq, 0.0f, 1.0f);
}

FVector GetClosestPointOnTrajectorySegmentXZ(const FVector& StartLocation, const FVector& EndLocation, const FVector& TestLocation)
{
	return FMath::Lerp(StartLocation, EndLocation, GetTrajectorySegmentAlphaXZ(StartLocation, EndLocation, TestLocation));
}

float GetTrajectoryDistanceSquaredXZ(const FVector& First, const FVector& Second)
{
	return FVector2D(First.X - Second.X, First.Z - Second.Z).SizeSquared();
}

}

AFRBattleCharacter::AFRBattleCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	BodyFrame = CreateDefaultSubobject<USceneComponent>(TEXT("BodyFrame"));
	BodyFrame->SetupAttachment(Root);

	Hurtbox = CreateDefaultSubobject<UBoxComponent>(TEXT("Hurtbox"));
	Hurtbox->SetupAttachment(BodyFrame);
	Hurtbox->InitBoxExtent(FVector(28.0f, 16.0f, 42.0f));
	Hurtbox->SetRelativeLocation(FVector(0.0f, 0.0f, 45.0f));
	Hurtbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Hurtbox->SetCollisionResponseToAllChannels(ECR_Ignore);
	Hurtbox->SetGenerateOverlapEvents(false);
	Hurtbox->SetCanEverAffectNavigation(false);

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Body->SetupAttachment(BodyFrame);
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Body->SetRelativeScale3D(FVector(0.65f, 0.08f, 0.9f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Body->SetStaticMesh(CubeMesh.Object);
	}

	BodySprite = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("BodySprite"));
	BodySprite->SetupAttachment(BodyFrame);
	BodySprite->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodySprite->SetVisibility(false);
	UpdateCharacterRotation(0.0f);

	StatusWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("StatusWidget"));
	StatusWidgetComponent->SetupAttachment(Root);
	StatusWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StatusWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	StatusWidgetComponent->SetDrawAtDesiredSize(false);
	StatusWidgetComponent->SetDrawSize(FVector2D(220.0f, 76.0f));
	StatusWidgetComponent->SetPivot(FVector2D(0.5f, 1.0f));
	StatusWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 118.0f));

	FloatingDamageTextActorClass = AFRFloatingDamageTextActor::StaticClass();

	AbilitySystemComponent = CreateDefaultSubobject<UFRAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	CombatSet = CreateDefaultSubobject<UFRCombatSet>(TEXT("CombatSet"));

	CharacterDisplayName = FText::FromString(TEXT("Rookie Tank"));
}

void AFRBattleCharacter::BeginPlay()
{
	Super::BeginPlay();

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	AbilitySystemComponent->AddAttributeSetSubobject(CombatSet.Get());
	InitializeFromDefinition(CharacterDefinition);
	EnsureDefaultLoadout();
	GrantStartupAbilitySets();
	UpdateCharacterRotation(GetActorPitchDegrees());
	SnapToTerrain();
	InitializeStatusWidget();
}


void AFRBattleCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ApplyTerrainGravity(DeltaSeconds);
}

UAbilitySystemComponent* AFRBattleCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AFRBattleCharacter::InitializeFromDefinition(UFRCharacterDefinition* InCharacterDefinition)
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
	CombatSet->SetMinAimAngle(InCharacterDefinition->MinAimAngle);
	CombatSet->SetMaxAimAngle(InCharacterDefinition->MaxAimAngle);
	AimAngle = FMath::Clamp(AimAngle, GetMinAimAngle(), GetMaxAimAngle());

	GrantedShotModifiers.Reset();
	PendingShotModifiers.Reset();
	WeaponLoadout.Reset();
	bHasSpecialAttackSlot = InCharacterDefinition->SpecialAttackDefinition != nullptr;
	bSpecialAttackEnabled = InCharacterDefinition->bCanUseSpecialAttack;
	if (InCharacterDefinition->BasicAttackDefinition)
	{
		AddWeaponDefinition(InCharacterDefinition->BasicAttackDefinition);
	}
	if (InCharacterDefinition->SpecialAttackDefinition)
	{
		AddWeaponDefinition(InCharacterDefinition->SpecialAttackDefinition);
	}
	for (UFRWeaponDefinition* WeaponDefinition : InCharacterDefinition->WeaponLoadout)
	{
		if (WeaponDefinition != InCharacterDefinition->BasicAttackDefinition && WeaponDefinition != InCharacterDefinition->SpecialAttackDefinition)
		{
			AddWeaponDefinition(WeaponDefinition);
		}
	}
	SelectedWeaponIndex = 0;

	ItemLoadout = InCharacterDefinition->ItemLoadout;
	for (FFRItemStack& ItemStack : ItemLoadout)
	{
		if (ItemStack.ItemDefinition && ItemStack.Charges <= 0)
		{
			ItemStack.Charges = ItemStack.ItemDefinition->InitialCharges;
		}
	}
}

void AFRBattleCharacter::ConfigureAsEnemy(bool bNewEnemy)
{
	bEnemy = bNewEnemy;
	bFacingRight = !bEnemy;
	UpdateCharacterRotation(GetActorPitchDegrees());
}

void AFRBattleCharacter::BeginTurn()
{
	bActiveTurn = true;
	bFiredThisTurn = false;
	bChargingShot = false;
	ShotChargeElapsed = 0.0f;
	ShotPower = 0.0f;
	CombatSet->ResetTurnBudget();
}

void AFRBattleCharacter::EndTurn()
{
	bActiveTurn = false;
	bChargingShot = false;
}

void AFRBattleCharacter::MoveHorizontal(float Axis, float DeltaSeconds)
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

	if (AFRDestructibleTerrain* Terrain = FindTerrain())
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
				if (!TryResolveFootprintBlock(*Terrain, StepLocation, SurfaceZ))
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

void AFRBattleCharacter::AdjustAim(float Axis, float DeltaSeconds)
{
	if (!bActiveTurn || bFiredThisTurn || !IsSupportedByTerrain() || FMath::IsNearlyZero(Axis))
	{
		return;
	}

	AimAngle = FMath::Clamp(AimAngle + Axis * 70.0f * DeltaSeconds, GetMinAimAngle(), GetMaxAimAngle());
}

void AFRBattleCharacter::AdjustPower(float Axis, float DeltaSeconds)
{
	if (!bActiveTurn || bFiredThisTurn || !IsSupportedByTerrain() || FMath::IsNearlyZero(Axis))
	{
		return;
	}

	ShotPower = FMath::Clamp(ShotPower + Axis * 0.75f * DeltaSeconds, 0.0f, 1.0f);
}

void AFRBattleCharacter::BeginShotCharge()
{
	if (!CanBeginShotCharge())
	{
		return;
	}

	bChargingShot = true;
	ShotChargeElapsed = 0.0f;
	ShotPower = 0.0f;
}

bool AFRBattleCharacter::CanBeginShotCharge() const
{
	return CanFireSelectedWeapon();
}

void AFRBattleCharacter::UpdateShotCharge(float DeltaSeconds)
{
	if (!bChargingShot || !bActiveTurn || bFiredThisTurn || IsDefeated() || !IsSupportedByTerrain())
	{
		return;
	}

	ShotChargeElapsed += DeltaSeconds;
	const float ChargeDuration = FMath::Max(ShotChargeSeconds, KINDA_SMALL_NUMBER);
	ShotPower = FMath::Clamp(ShotChargeElapsed / ChargeDuration, 0.0f, 1.0f);
}

int32 AFRBattleCharacter::ReleaseShotCharge()
{
	if (!bChargingShot)
	{
		return 0;
	}

	bChargingShot = false;
	return FireSelectedWeapon();
}

void AFRBattleCharacter::SelectWeapon(int32 WeaponIndex)
{
	if (CanSelectWeapon(WeaponIndex))
	{
		SelectedWeaponIndex = WeaponIndex;
	}
}

void AFRBattleCharacter::SelectBasicAttack()
{
	SelectWeapon(0);
}

bool AFRBattleCharacter::SelectSpecialAttack()
{
	if (!CanSelectSpecialAttack())
	{
		return false;
	}

	SelectedWeaponIndex = 1;
	return true;
}

bool AFRBattleCharacter::SelectWeaponByTag(FGameplayTag WeaponTag)
{
	const int32 WeaponIndex = GetWeaponIndexByTag(WeaponTag);
	if (!CanSelectWeapon(WeaponIndex))
	{
		return false;
	}

	SelectedWeaponIndex = WeaponIndex;
	return true;
}

bool AFRBattleCharacter::CanSelectWeapon(int32 WeaponIndex) const
{
	return WeaponLoadout.IsValidIndex(WeaponIndex)
		&& (!bHasSpecialAttackSlot || WeaponIndex != 1 || bSpecialAttackEnabled);
}

bool AFRBattleCharacter::CanSelectWeaponByTag(FGameplayTag WeaponTag) const
{
	return CanSelectWeapon(GetWeaponIndexByTag(WeaponTag));
}

bool AFRBattleCharacter::CanSelectSpecialAttack() const
{
	return bHasSpecialAttackSlot && bSpecialAttackEnabled && WeaponLoadout.IsValidIndex(1);
}

int32 AFRBattleCharacter::GetWeaponIndexByTag(FGameplayTag WeaponTag) const
{
	if (!WeaponTag.IsValid())
	{
		return INDEX_NONE;
	}

	for (int32 WeaponIndex = 0; WeaponIndex < WeaponLoadout.Num(); ++WeaponIndex)
	{
		if (WeaponLoadout[WeaponIndex].WeaponTag.MatchesTagExact(WeaponTag))
		{
			return WeaponIndex;
		}
	}

	return INDEX_NONE;
}

int32 AFRBattleCharacter::FireSelectedWeapon()
{
	if (!CanFireSelectedWeapon())
	{
		return 0;
	}

	bChargingShot = false;
	bFiredThisTurn = true;

	const FFRWeaponSpec& Weapon = GetCurrentWeapon();
	const FFRShotSpec ShotSpec = BuildShotSpec(Weapon);
	const int32 ProjectileCount = ShotSpec.ProjectileCount;
	const int32 SalvoCount = FMath::Max(1, ShotSpec.SalvoCount);
	const float SalvoInterval = FMath::Max(0.0f, ShotSpec.SalvoInterval);
	PendingAttackMultiplier = 1.0f;
	PendingShotModifiers.Reset();

	int32 ExpectedProjectiles = SpawnShotSpecProjectiles(ShotSpec, false);
	for (int32 SalvoIndex = 1; SalvoIndex < SalvoCount; ++SalvoIndex)
	{
		ExpectedProjectiles += ProjectileCount;
		FTimerHandle SalvoTimerHandle;
		FTimerDelegate SalvoDelegate;
		SalvoDelegate.BindWeakLambda(this, [this, ShotSpec, ProjectileCount]()
		{
			const int32 SpawnedProjectiles = SpawnShotSpecProjectiles(ShotSpec, false);
			const int32 FailedProjectiles = FMath::Max(0, ProjectileCount - SpawnedProjectiles);
			if (FailedProjectiles > 0)
			{
				if (AFRGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFRGameMode>() : nullptr)
				{
					GameMode->NotifyProjectileSpawnFailed(FailedProjectiles);
				}
			}
		});
		GetWorldTimerManager().SetTimer(SalvoTimerHandle, SalvoDelegate, SalvoInterval * SalvoIndex, false);
	}

	return ExpectedProjectiles;
}

bool AFRBattleCharacter::CanFireSelectedWeapon() const
{
	return bActiveTurn && !bFiredThisTurn && !IsDefeated() && IsSupportedByTerrain() && CanSelectWeapon(SelectedWeaponIndex);
}

void AFRBattleCharacter::FireAtTarget(AFRBattleCharacter* Target, const FFRStageDifficultyData& DifficultyData)
{
	if (!Target)
	{
		return;
	}

	const FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
	bFacingRight = ToTarget.X >= 0.0f;
	UpdateCharacterRotation(GetActorPitchDegrees());
	const float AimError = DifficultyData.AimAngleErrorDegrees > 0.0f ? FMath::RandRange(-DifficultyData.AimAngleErrorDegrees, DifficultyData.AimAngleErrorDegrees) : 0.0f;
	const float PowerError = DifficultyData.ShotPowerError > 0.0f ? FMath::RandRange(-DifficultyData.ShotPowerError, DifficultyData.ShotPowerError) : 0.0f;
	const float CharacterMinAim = GetMinAimAngle();
	const float CharacterMaxAim = GetMaxAimAngle();
	const float DifficultyMinAim = FMath::Min(DifficultyData.MinAimAngleDegrees, DifficultyData.MaxAimAngleDegrees);
	const float DifficultyMaxAim = FMath::Max(DifficultyData.MinAimAngleDegrees, DifficultyData.MaxAimAngleDegrees);
	const float MinAim = FMath::Clamp(DifficultyMinAim, CharacterMinAim, CharacterMaxAim);
	const float MaxAim = FMath::Clamp(DifficultyMaxAim, MinAim, CharacterMaxAim);
	const float MinPower = FMath::Min(DifficultyData.MinShotPower, DifficultyData.MaxShotPower);
	const float MaxPower = FMath::Max(DifficultyData.MinShotPower, DifficultyData.MaxShotPower);

	const FFRWeaponSpec& Weapon = GetCurrentWeapon();
	const AFRGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFRGameMode>() : nullptr;
	const float Wind = GameMode ? GameMode->GetWind() : 0.0f;
	const FVector TargetLocation = Target->GetActorLocation();
	float BestAimAngle = AimAngle;
	float BestShotPower = ShotPower;
	float BestScore = MAX_flt;
	constexpr int32 AimSamples = 24;
	constexpr int32 PowerSamples = 24;
	constexpr int32 SimulationSteps = 150;
	constexpr float SimulationDeltaSeconds = 1.0f / 30.0f;

	for (int32 AimSampleIndex = 0; AimSampleIndex <= AimSamples; ++AimSampleIndex)
	{
		const float AimAlpha = static_cast<float>(AimSampleIndex) / static_cast<float>(AimSamples);
		const float CandidateAimAngle = FMath::Lerp(MinAim, MaxAim, AimAlpha);
		for (int32 PowerSampleIndex = 0; PowerSampleIndex <= PowerSamples; ++PowerSampleIndex)
		{
			const float PowerAlpha = static_cast<float>(PowerSampleIndex) / static_cast<float>(PowerSamples);
			const float CandidateShotPower = FMath::Lerp(MinPower, MaxPower, PowerAlpha);
			AimAngle = CandidateAimAngle;
			ShotPower = CandidateShotPower;

			const FFRShotSpec CandidateShotSpec = BuildShotSpec(Weapon);
			if (CandidateShotSpec.LaunchSpeed <= KINDA_SMALL_NUMBER)
			{
				continue;
			}

			const FVector LaunchDirection = GetProjectileLaunchDirection(0.0f);
			FVector SimulatedLocation = GetProjectileSpawnLocation(LaunchDirection);
			FVector SimulatedVelocity = LaunchDirection * CandidateShotSpec.LaunchSpeed;
			float CandidateScore = GetTrajectoryDistanceSquaredXZ(SimulatedLocation, TargetLocation);
			for (int32 StepIndex = 0; StepIndex < SimulationSteps; ++StepIndex)
			{
				const FVector PreviousLocation = SimulatedLocation;
				SimulatedVelocity += FVector(Wind, 0.0f, -CandidateShotSpec.Gravity) * SimulationDeltaSeconds;
				SimulatedLocation += SimulatedVelocity * SimulationDeltaSeconds;
				const FVector ClosestPoint = GetClosestPointOnTrajectorySegmentXZ(PreviousLocation, SimulatedLocation, TargetLocation);
				CandidateScore = FMath::Min(CandidateScore, GetTrajectoryDistanceSquaredXZ(ClosestPoint, TargetLocation));

				const float HorizontalOvershoot = bFacingRight ? SimulatedLocation.X - TargetLocation.X : TargetLocation.X - SimulatedLocation.X;
				const bool bMovingPastTarget = bFacingRight ? SimulatedVelocity.X > 0.0f : SimulatedVelocity.X < 0.0f;
				if ((SimulatedLocation.Z < TargetLocation.Z - 2000.0f && SimulatedVelocity.Z < 0.0f) || (HorizontalOvershoot > 1400.0f && bMovingPastTarget))
				{
					break;
				}
			}

			if (CandidateScore < BestScore)
			{
				BestScore = CandidateScore;
				BestAimAngle = CandidateAimAngle;
				BestShotPower = CandidateShotPower;
			}
		}
	}

	if (BestScore < MAX_flt)
	{
		AimAngle = FMath::Clamp(BestAimAngle + AimError, MinAim, MaxAim);
		ShotPower = FMath::Clamp(BestShotPower + PowerError, MinPower, MaxPower);
	}
	else
	{
		const float AimArcHeight = FMath::Max(DifficultyData.MinAimArcHeight, FMath::Abs(ToTarget.Z) + DifficultyData.AimHeightOffset);
		const float Distance = FMath::Max(1.0f, FMath::Abs(ToTarget.X));
		AimAngle = FMath::Clamp(FMath::RadiansToDegrees(FMath::Atan2(AimArcHeight, Distance)) + AimError, MinAim, MaxAim);
		ShotPower = FMath::Clamp(Distance / FMath::Max(1.0f, DifficultyData.ShotPowerDistanceScale) + PowerError, MinPower, MaxPower);
	}
}

void AFRBattleCharacter::ApplyDamage(float DamageAmount)
{
	const float HealthBeforeDamage = GetHealth();
	CombatSet->ApplyDamage(DamageAmount);
	const float AppliedDamage = FMath::Max(0.0f, HealthBeforeDamage - GetHealth());
	if (AppliedDamage > KINDA_SMALL_NUMBER)
	{
		SpawnFloatingDamageText(AppliedDamage);
	}
	RefreshStatusWidget();
}

bool AFRBattleCharacter::FindHurtboxImpactAlongSegmentXZ(const FVector& StartLocation, const FVector& EndLocation, FVector& OutImpactLocation) const
{
	if (!Hurtbox)
	{
		return false;
	}

	const float HurtboxWorldY = Hurtbox->GetComponentLocation().Y;
	const FVector ProjectedStart(StartLocation.X, HurtboxWorldY, StartLocation.Z);
	const FVector ProjectedEnd(EndLocation.X, HurtboxWorldY, EndLocation.Z);
	FHitResult HitResult;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CharacterHurtbox), false);
	if (!Hurtbox->LineTraceComponent(HitResult, ProjectedStart, ProjectedEnd, QueryParams))
	{
		return false;
	}

	OutImpactLocation = FMath::Lerp(StartLocation, EndLocation, HitResult.Time);
	return true;
}

float AFRBattleCharacter::GetDistanceToHurtboxXZ(const FVector& WorldLocation) const
{
	if (!Hurtbox)
	{
		return FVector2D(WorldLocation.X - GetActorLocation().X, WorldLocation.Z - GetActorLocation().Z).Size();
	}

	const FVector ProjectedLocation(WorldLocation.X, Hurtbox->GetComponentLocation().Y, WorldLocation.Z);
	FVector ClosestPoint = ProjectedLocation;
	if (Hurtbox->GetClosestPointOnCollision(ProjectedLocation, ClosestPoint) < 0.0f)
	{
		return FVector2D(WorldLocation.X - GetActorLocation().X, WorldLocation.Z - GetActorLocation().Z).Size();
	}

	return FVector2D(WorldLocation.X - ClosestPoint.X, WorldLocation.Z - ClosestPoint.Z).Size();
}

void AFRBattleCharacter::ReevaluateTerrainSupport()
{
	AFRDestructibleTerrain* Terrain = FindTerrain();
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

void AFRBattleCharacter::ApplyImpactKnockback(float HorizontalDistance, FVector ImpactLocation, FVector ImpactVelocity)
{
	if (HorizontalDistance <= 0.0f || IsDefeated())
	{
		return;
	}

	FVector NewLocation = GetActorLocation();
	float Direction = FMath::Sign(NewLocation.X - ImpactLocation.X);
	if (FMath::IsNearlyZero(Direction))
	{
		Direction = ImpactVelocity.X >= 0.0f ? 1.0f : -1.0f;
	}
	NewLocation.X += Direction * HorizontalDistance;
	if (AFRDestructibleTerrain* Terrain = FindTerrain())
	{
		NewLocation.X = ClampWorldXToTerrainBounds(*Terrain, NewLocation.X);
	}
	SetActorLocation(NewLocation);
	ReevaluateTerrainSupport();
}

void AFRBattleCharacter::SetTerrain(AFRDestructibleTerrain* InTerrain)
{
	AssignedTerrain = InTerrain;
	if (HasActorBegunPlay())
	{
		SnapToTerrain();
	}
}

bool AFRBattleCharacter::UseItemByType(EFRItemType ItemType)
{
	if (!CanUseAnyItem())
	{
		return false;
	}

	for (FFRItemStack& ItemStack : ItemLoadout)
	{
		const UFRItemDefinition* ItemDefinition = ItemStack.ItemDefinition;
		if (!ItemDefinition || ItemDefinition->ItemType != ItemType || !CanUseItemStack(ItemStack))
		{
			continue;
		}

		return UseItemStack(ItemStack);
	}

	return false;
}

bool AFRBattleCharacter::UseItemByTag(FGameplayTag ItemTag)
{
	if (!ItemTag.IsValid() || !CanUseAnyItem())
	{
		return false;
	}

	for (FFRItemStack& ItemStack : ItemLoadout)
	{
		const UFRItemDefinition* ItemDefinition = ItemStack.ItemDefinition;
		if (!ItemDefinition || !ItemDefinition->ItemTag.MatchesTagExact(ItemTag) || !CanUseItemStack(ItemStack))
		{
			continue;
		}

		return UseItemStack(ItemStack);
	}

	return false;
}

bool AFRBattleCharacter::UseItemByIndex(int32 ItemIndex)
{
	if (!CanUseItemByIndex(ItemIndex))
	{
		return false;
	}

	return UseItemStack(ItemLoadout[ItemIndex]);
}

bool AFRBattleCharacter::CanUseItemByType(EFRItemType ItemType) const
{
	if (!CanUseAnyItem())
	{
		return false;
	}

	for (const FFRItemStack& ItemStack : ItemLoadout)
	{
		const UFRItemDefinition* ItemDefinition = ItemStack.ItemDefinition;
		if (ItemDefinition && ItemDefinition->ItemType == ItemType && CanUseItemStack(ItemStack))
		{
			return true;
		}
	}
	return false;
}

bool AFRBattleCharacter::CanUseItemByTag(FGameplayTag ItemTag) const
{
	if (!ItemTag.IsValid() || !CanUseAnyItem())
	{
		return false;
	}

	for (const FFRItemStack& ItemStack : ItemLoadout)
	{
		const UFRItemDefinition* ItemDefinition = ItemStack.ItemDefinition;
		if (ItemDefinition && ItemDefinition->ItemTag.MatchesTagExact(ItemTag) && CanUseItemStack(ItemStack))
		{
			return true;
		}
	}
	return false;
}

bool AFRBattleCharacter::CanUseItemByIndex(int32 ItemIndex) const
{
	return CanUseAnyItem() && ItemLoadout.IsValidIndex(ItemIndex) && CanUseItemStack(ItemLoadout[ItemIndex]);
}

int32 AFRBattleCharacter::GetItemIndexByTag(FGameplayTag ItemTag) const
{
	if (!ItemTag.IsValid())
	{
		return INDEX_NONE;
	}

	for (int32 ItemIndex = 0; ItemIndex < ItemLoadout.Num(); ++ItemIndex)
	{
		const UFRItemDefinition* ItemDefinition = ItemLoadout[ItemIndex].ItemDefinition;
		if (ItemDefinition && ItemDefinition->ItemTag.MatchesTagExact(ItemTag))
		{
			return ItemIndex;
		}
	}

	return INDEX_NONE;
}

bool AFRBattleCharacter::CanUseAnyItem() const
{
	return bActiveTurn && !IsDefeated() && IsSupportedByTerrain();
}

bool AFRBattleCharacter::CanUseItemStack(const FFRItemStack& ItemStack) const
{
	return ItemStack.ItemDefinition && ItemStack.Charges > 0;
}

bool AFRBattleCharacter::UseItemStack(FFRItemStack& ItemStack)
{
	const UFRItemDefinition* ItemDefinition = ItemStack.ItemDefinition;
	if (!ItemDefinition || ItemStack.Charges <= 0)
	{
		return false;
	}

	--ItemStack.Charges;
	if (ItemDefinition->ItemType == EFRItemType::Heal)
	{
		CombatSet->Heal(ItemDefinition->HealAmount);
	}
	else if (ItemDefinition->ItemType == EFRItemType::AttackMultiplier)
	{
		PendingAttackMultiplier = FMath::Max(PendingAttackMultiplier, ItemDefinition->AttackMultiplier);
	}
	GrantAbilitySet(ItemDefinition->UseAbilitySet);
	PendingShotModifiers.Append(ItemDefinition->UseShotModifiers);
	return true;
}

void AFRBattleCharacter::ApplyRewardDamage(float BonusDamage)
{
	CombatSet->AddDamage(BonusDamage);
}

void AFRBattleCharacter::ApplyRewardHealth(float BonusHealth)
{
	CombatSet->AddMaxHealth(BonusHealth);
}

void AFRBattleCharacter::ApplyRewardMoveBudget(float BonusMoveBudget)
{
	CombatSet->AddMaxMoveBudget(BonusMoveBudget);
}

void AFRBattleCharacter::ApplyRewardProjectiles(int32 BonusProjectiles)
{
	CombatSet->AddProjectileCount(BonusProjectiles);
}

void AFRBattleCharacter::ApplyRewardShotPowerMultiplier(float BonusMultiplier)
{
	CombatSet->AddShotPowerMultiplier(BonusMultiplier);
}

void AFRBattleCharacter::ApplyPerkDefinition(UFRPerkDefinition* PerkDefinition)
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

void AFRBattleCharacter::GrantAbilitySet(UFRAbilitySet* AbilitySet)
{
	if (!AbilitySet || !AbilitySystemComponent)
	{
		return;
	}

	FFRGrantedAbilitySetEntry Entry;
	Entry.AbilitySet = AbilitySet;
	AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, &Entry.Handles, this);
	GrantedAbilitySetEntries.Add(MoveTemp(Entry));
}

bool AFRBattleCharacter::RemoveAbilitySet(UFRAbilitySet* AbilitySet)
{
	if (!AbilitySet || !AbilitySystemComponent)
	{
		return false;
	}

	for (int32 Index = GrantedAbilitySetEntries.Num() - 1; Index >= 0; --Index)
	{
		FFRGrantedAbilitySetEntry& Entry = GrantedAbilitySetEntries[Index];
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

int32 AFRBattleCharacter::RemoveAbilitySetsByTag(FGameplayTag AbilitySetTag)
{
	if (!AbilitySetTag.IsValid() || !AbilitySystemComponent)
	{
		return 0;
	}

	int32 RemovedCount = 0;
	for (int32 Index = GrantedAbilitySetEntries.Num() - 1; Index >= 0; --Index)
	{
		FFRGrantedAbilitySetEntry& Entry = GrantedAbilitySetEntries[Index];
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

int32 AFRBattleCharacter::GetGrantedAbilitySetCount(UFRAbilitySet* AbilitySet) const
{
	if (!AbilitySet)
	{
		return 0;
	}

	int32 GrantedCount = 0;
	for (const FFRGrantedAbilitySetEntry& Entry : GrantedAbilitySetEntries)
	{
		if (Entry.AbilitySet == AbilitySet)
		{
			++GrantedCount;
		}
	}
	return GrantedCount;
}

int32 AFRBattleCharacter::GetGrantedAbilitySetCountByTag(FGameplayTag AbilitySetTag) const
{
	if (!AbilitySetTag.IsValid())
	{
		return 0;
	}

	int32 GrantedCount = 0;
	for (const FFRGrantedAbilitySetEntry& Entry : GrantedAbilitySetEntries)
	{
		if (DoesAbilitySetMatchTag(Entry.AbilitySet, AbilitySetTag))
		{
			++GrantedCount;
		}
	}
	return GrantedCount;
}

bool AFRBattleCharacter::HasGrantedAbilitySetByTag(FGameplayTag AbilitySetTag) const
{
	return GetGrantedAbilitySetCountByTag(AbilitySetTag) > 0;
}

TArray<UFRAbilitySet*> AFRBattleCharacter::GetGrantedAbilitySetsForBlueprint() const
{
	TArray<UFRAbilitySet*> AbilitySets;
	for (const FFRGrantedAbilitySetEntry& Entry : GrantedAbilitySetEntries)
	{
		if (Entry.AbilitySet)
		{
			AbilitySets.Add(Entry.AbilitySet);
		}
	}
	return AbilitySets;
}

FText AFRBattleCharacter::GetGrantedAbilitySetsSummary() const
{
	TArray<FString> Parts;
	for (const FFRGrantedAbilitySetEntry& Entry : GrantedAbilitySetEntries)
	{
		if (!Entry.AbilitySet)
		{
			continue;
		}

		const FString AbilitySetSummary = Entry.AbilitySet->GetEffectSummary().ToString();
		Parts.Add(AbilitySetSummary.IsEmpty() ? Entry.AbilitySet->GetName() : AbilitySetSummary);
	}
	return Parts.Num() > 0 ? FText::FromString(FString::Join(Parts, TEXT(" | "))) : FText::GetEmpty();
}

void AFRBattleCharacter::GrantShotModifiers(const TArray<FFRShotModifierSpec>& ShotModifiers)
{
	if (ShotModifiers.Num() <= 0)
	{
		return;
	}

	GrantedShotModifiers.Append(ShotModifiers);
}

void AFRBattleCharacter::GrantPendingShotModifiers(const TArray<FFRShotModifierSpec>& ShotModifiers)
{
	if (ShotModifiers.Num() <= 0)
	{
		return;
	}

	PendingShotModifiers.Append(ShotModifiers);
}

int32 AFRBattleCharacter::RemoveGrantedShotModifiersByTag(FGameplayTag ModifierTag)
{
	if (!ModifierTag.IsValid())
	{
		return 0;
	}

	int32 RemovedCount = 0;
	for (int32 Index = GrantedShotModifiers.Num() - 1; Index >= 0; --Index)
	{
		const FFRShotModifierSpec& Modifier = GrantedShotModifiers[Index];
		if (!DoesShotModifierMatchTag(Modifier, ModifierTag))
		{
			continue;
		}

		GrantedShotModifiers.RemoveAt(Index);
		++RemovedCount;
	}

	return RemovedCount;
}

int32 AFRBattleCharacter::RemovePendingShotModifiersByTag(FGameplayTag ModifierTag)
{
	if (!ModifierTag.IsValid())
	{
		return 0;
	}

	int32 RemovedCount = 0;
	for (int32 Index = PendingShotModifiers.Num() - 1; Index >= 0; --Index)
	{
		const FFRShotModifierSpec& Modifier = PendingShotModifiers[Index];
		if (!DoesShotModifierMatchTag(Modifier, ModifierTag))
		{
			continue;
		}

		PendingShotModifiers.RemoveAt(Index);
		++RemovedCount;
	}

	return RemovedCount;
}

void AFRBattleCharacter::AddWeaponDefinition(UFRWeaponDefinition* WeaponDefinition)
{
	if (WeaponDefinition)
	{
		WeaponLoadout.Add(WeaponDefinition->Weapon);
	}
}

void AFRBattleCharacter::SetSpecialAttackEnabled(bool bEnabled)
{
	bSpecialAttackEnabled = bEnabled;
	if (!CanSelectWeapon(SelectedWeaponIndex))
	{
		SelectedWeaponIndex = 0;
	}
}

void AFRBattleCharacter::AddItemDefinition(UFRItemDefinition* ItemDefinition, int32 ChargesOverride)
{
	if (!ItemDefinition)
	{
		return;
	}

	for (FFRItemStack& ItemStack : ItemLoadout)
	{
		if (ItemStack.ItemDefinition == ItemDefinition)
		{
			ItemStack.Charges += ChargesOverride == INDEX_NONE ? ItemDefinition->InitialCharges : FMath::Max(0, ChargesOverride);
			return;
		}
	}

	FFRItemStack NewStack;
	NewStack.ItemDefinition = ItemDefinition;
	NewStack.Charges = ChargesOverride == INDEX_NONE ? ItemDefinition->InitialCharges : FMath::Max(0, ChargesOverride);
	ItemLoadout.Add(NewStack);
}

bool AFRBattleCharacter::IsEnemy() const
{
	return bEnemy;
}

bool AFRBattleCharacter::IsActiveTurn() const
{
	return bActiveTurn;
}

bool AFRBattleCharacter::HasFiredThisTurn() const
{
	return bFiredThisTurn;
}

bool AFRBattleCharacter::IsDefeated() const
{
	return CombatSet->GetHealth() <= 0.0f;
}

float AFRBattleCharacter::GetHealth() const
{
	return CombatSet->GetHealth();
}

float AFRBattleCharacter::GetMaxHealth() const
{
	return CombatSet->GetMaxHealth();
}

float AFRBattleCharacter::GetMoveBudget() const
{
	return CombatSet->GetMoveBudget();
}

float AFRBattleCharacter::GetMaxMoveBudget() const
{
	return CombatSet->GetMaxMoveBudget();
}

float AFRBattleCharacter::GetDamageBonus() const
{
	return CombatSet->GetDamage();
}

float AFRBattleCharacter::GetShotPowerMultiplier() const
{
	return CombatSet->GetShotPowerMultiplier();
}

float AFRBattleCharacter::GetProjectileCount() const
{
	return CombatSet->GetProjectileCount();
}

float AFRBattleCharacter::GetMinAimAngle() const
{
	return FMath::Min(CombatSet->GetMinAimAngle(), CombatSet->GetMaxAimAngle());
}

float AFRBattleCharacter::GetMaxAimAngle() const
{
	return FMath::Max(CombatSet->GetMinAimAngle(), CombatSet->GetMaxAimAngle());
}

bool AFRBattleCharacter::TryGetCombatAttributeValueByTag(FGameplayTag AttributeTag, float& OutValue) const
{
	OutValue = 0.0f;
	if (!AttributeTag.IsValid())
	{
		return false;
	}

	if (AttributeTag.MatchesTagExact(FRGameplayTags::Attribute_Health))
	{
		OutValue = GetHealth();
		return true;
	}
	if (AttributeTag.MatchesTagExact(FRGameplayTags::Attribute_MaxHealth))
	{
		OutValue = GetMaxHealth();
		return true;
	}
	if (AttributeTag.MatchesTagExact(FRGameplayTags::Attribute_MoveBudget))
	{
		OutValue = GetMoveBudget();
		return true;
	}
	if (AttributeTag.MatchesTagExact(FRGameplayTags::Attribute_MaxMoveBudget))
	{
		OutValue = GetMaxMoveBudget();
		return true;
	}
	if (AttributeTag.MatchesTagExact(FRGameplayTags::Attribute_Damage))
	{
		OutValue = GetDamageBonus();
		return true;
	}
	if (AttributeTag.MatchesTagExact(FRGameplayTags::Attribute_ShotPowerMultiplier))
	{
		OutValue = GetShotPowerMultiplier();
		return true;
	}
	if (AttributeTag.MatchesTagExact(FRGameplayTags::Attribute_ProjectileCount))
	{
		OutValue = GetProjectileCount();
		return true;
	}
	if (AttributeTag.MatchesTagExact(FRGameplayTags::Attribute_MinAimAngle))
	{
		OutValue = GetMinAimAngle();
		return true;
	}
	if (AttributeTag.MatchesTagExact(FRGameplayTags::Attribute_MaxAimAngle))
	{
		OutValue = GetMaxAimAngle();
		return true;
	}

	return false;
}

bool AFRBattleCharacter::TryApplyCombatAttributeDeltaByTag(FGameplayTag AttributeTag, float DeltaValue)
{
	if (!AttributeTag.IsValid())
	{
		return false;
	}

	if (AttributeTag.MatchesTagExact(FRGameplayTags::Attribute_Health))
	{
		if (DeltaValue >= 0.0f)
		{
			CombatSet->Heal(DeltaValue);
		}
		else
		{
			ApplyDamage(-DeltaValue);
		}
		return true;
	}
	if (AttributeTag.MatchesTagExact(FRGameplayTags::Attribute_MaxHealth))
	{
		CombatSet->AddMaxHealth(DeltaValue);
		return true;
	}
	if (AttributeTag.MatchesTagExact(FRGameplayTags::Attribute_MoveBudget))
	{
		CombatSet->SetMoveBudget(FMath::Clamp(GetMoveBudget() + DeltaValue, 0.0f, GetMaxMoveBudget()));
		return true;
	}
	if (AttributeTag.MatchesTagExact(FRGameplayTags::Attribute_MaxMoveBudget))
	{
		CombatSet->AddMaxMoveBudget(DeltaValue);
		return true;
	}
	if (AttributeTag.MatchesTagExact(FRGameplayTags::Attribute_Damage))
	{
		CombatSet->AddDamage(DeltaValue);
		return true;
	}
	if (AttributeTag.MatchesTagExact(FRGameplayTags::Attribute_ShotPowerMultiplier))
	{
		CombatSet->AddShotPowerMultiplier(DeltaValue);
		return true;
	}
	if (AttributeTag.MatchesTagExact(FRGameplayTags::Attribute_ProjectileCount))
	{
		CombatSet->AddProjectileCount(DeltaValue);
		return true;
	}
	if (AttributeTag.MatchesTagExact(FRGameplayTags::Attribute_MinAimAngle))
	{
		CombatSet->AddMinAimAngle(DeltaValue);
		AimAngle = FMath::Clamp(AimAngle, GetMinAimAngle(), GetMaxAimAngle());
		return true;
	}
	if (AttributeTag.MatchesTagExact(FRGameplayTags::Attribute_MaxAimAngle))
	{
		CombatSet->AddMaxAimAngle(DeltaValue);
		AimAngle = FMath::Clamp(AimAngle, GetMinAimAngle(), GetMaxAimAngle());
		return true;
	}

	return false;
}

FText AFRBattleCharacter::GetCombatStatsSummary() const
{
	return FText::FromString(FString::Printf(
		TEXT("HP %.0f/%.0f | move %.0f/%.0f | damage +%.0f | shot power x%.2g | projectiles %.0f | aim %.0f-%.0f"),
		GetHealth(),
		GetMaxHealth(),
		GetMoveBudget(),
		GetMaxMoveBudget(),
		GetDamageBonus(),
		GetShotPowerMultiplier(),
		GetProjectileCount(),
		GetMinAimAngle(),
		GetMaxAimAngle()));
}

float AFRBattleCharacter::GetAimAngle() const
{
	return AimAngle;
}

float AFRBattleCharacter::GetShotPower() const
{
	return ShotPower;
}

float AFRBattleCharacter::GetShotChargeAlpha() const
{
	return FMath::Clamp(ShotPower, 0.0f, 1.0f);
}

bool AFRBattleCharacter::IsChargingShot() const
{
	return bChargingShot;
}

int32 AFRBattleCharacter::GetItemCharges(EFRItemType ItemType) const
{
	int32 TotalCharges = 0;
	for (const FFRItemStack& ItemStack : ItemLoadout)
	{
		if (ItemStack.ItemDefinition && ItemStack.ItemDefinition->ItemType == ItemType)
		{
			TotalCharges += ItemStack.Charges;
		}
	}
	return TotalCharges;
}

int32 AFRBattleCharacter::GetItemChargesByTag(FGameplayTag ItemTag) const
{
	if (!ItemTag.IsValid())
	{
		return 0;
	}

	int32 TotalCharges = 0;
	for (const FFRItemStack& ItemStack : ItemLoadout)
	{
		if (ItemStack.ItemDefinition && ItemStack.ItemDefinition->ItemTag.MatchesTagExact(ItemTag))
		{
			TotalCharges += ItemStack.Charges;
		}
	}
	return TotalCharges;
}

const TArray<FFRItemStack>& AFRBattleCharacter::GetItemLoadout() const
{
	return ItemLoadout;
}

TArray<FFRItemStack> AFRBattleCharacter::GetItemLoadoutForBlueprint() const
{
	return ItemLoadout;
}

TArray<FFRShotModifierSpec> AFRBattleCharacter::GetGrantedShotModifiersForBlueprint() const
{
	return GrantedShotModifiers;
}

TArray<FFRShotModifierSpec> AFRBattleCharacter::GetPendingShotModifiersForBlueprint() const
{
	return PendingShotModifiers;
}

FText AFRBattleCharacter::GetGrantedShotModifiersSummary() const
{
	return UFRRewardBlueprintLibrary::GetShotModifierEffectSummary(GrantedShotModifiers);
}

FText AFRBattleCharacter::GetPendingShotModifiersSummary() const
{
	return UFRRewardBlueprintLibrary::GetShotModifierEffectSummary(PendingShotModifiers);
}

int32 AFRBattleCharacter::GetGrantedShotModifierCountByTag(FGameplayTag ModifierTag) const
{
	if (!ModifierTag.IsValid())
	{
		return 0;
	}

	int32 TotalCount = 0;
	for (const FFRShotModifierSpec& Modifier : GrantedShotModifiers)
	{
		if (DoesShotModifierMatchTag(Modifier, ModifierTag))
		{
			++TotalCount;
		}
	}
	return TotalCount;
}

int32 AFRBattleCharacter::GetPendingShotModifierCountByTag(FGameplayTag ModifierTag) const
{
	if (!ModifierTag.IsValid())
	{
		return 0;
	}

	int32 TotalCount = 0;
	for (const FFRShotModifierSpec& Modifier : PendingShotModifiers)
	{
		if (DoesShotModifierMatchTag(Modifier, ModifierTag))
		{
			++TotalCount;
		}
	}
	return TotalCount;
}

bool AFRBattleCharacter::HasGrantedShotModifierByTag(FGameplayTag ModifierTag) const
{
	return GetGrantedShotModifierCountByTag(ModifierTag) > 0;
}

bool AFRBattleCharacter::HasPendingShotModifierByTag(FGameplayTag ModifierTag) const
{
	return GetPendingShotModifierCountByTag(ModifierTag) > 0;
}

const FFRWeaponSpec& AFRBattleCharacter::GetCurrentWeapon() const
{
	if (WeaponLoadout.IsValidIndex(SelectedWeaponIndex))
	{
		return WeaponLoadout[SelectedWeaponIndex];
	}

	static const FFRWeaponSpec FallbackWeapon = []()
	{
		FFRWeaponSpec Weapon;
		Weapon.DisplayName = FText::FromString(TEXT("No Weapon"));
		Weapon.Description = FText::GetEmpty();
		Weapon.HitDamage = 0.0f;
		Weapon.Damage = 0.0f;
		Weapon.BlastRadius = 0.0f;
		Weapon.ExplosionFullDamageRadius = 0.0f;
		Weapon.TerrainDamage = 0.0f;
		Weapon.ProjectileSpeed = 0.0f;
		Weapon.Gravity = 0.0f;
		Weapon.ProjectilesPerShot = 0;
		return Weapon;
	}();
	return FallbackWeapon;
}

FFRWeaponSpec AFRBattleCharacter::GetCurrentWeaponSpec() const
{
	return GetCurrentWeapon();
}

FFRShotSpec AFRBattleCharacter::GetCurrentShotSpec() const
{
	return BuildShotSpec(GetCurrentWeapon());
}

FText AFRBattleCharacter::GetCurrentShotSummary() const
{
	const FFRShotSpec ShotSpec = GetCurrentShotSpec();
	const bool bFillsTerrain = ShotSpec.TerrainFillRadius > 0.0f;

	return FText::FromString(FString::Printf(TEXT("Hit %.0f | Blast Dmg %.0f | Blast %.0f | %s %.0f | Projectiles %d"),
		ShotSpec.HitDamage,
		ShotSpec.Damage,
		ShotSpec.BlastRadius,
		bFillsTerrain ? TEXT("Fill") : TEXT("Carve"),
		bFillsTerrain ? ShotSpec.TerrainFillRadius : ShotSpec.TerrainDamage,
		ShotSpec.ProjectileCount));
}

bool AFRBattleCharacter::DoesShotModifierMeetCurrentShotConditions(const FFRShotModifierSpec& ShotModifier) const
{
	const AFRGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFRGameMode>() : nullptr;
	const float Wind = GameMode ? GameMode->GetWind() : 0.0f;
	return ShotModifier.MeetsShotConditions(GetCurrentShotSpec(), AimAngle, Wind, bFacingRight);
}

FText AFRBattleCharacter::GetShotModifierCurrentConditionFailureSummary(const FFRShotModifierSpec& ShotModifier) const
{
	const AFRGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFRGameMode>() : nullptr;
	const float Wind = GameMode ? GameMode->GetWind() : 0.0f;
	return ShotModifier.GetShotConditionFailureSummary(GetCurrentShotSpec(), AimAngle, Wind, bFacingRight);
}

const TArray<FFRWeaponSpec>& AFRBattleCharacter::GetWeaponLoadout() const
{
	return WeaponLoadout;
}

TArray<FFRWeaponSpec> AFRBattleCharacter::GetWeaponLoadoutForBlueprint() const
{
	return WeaponLoadout;
}

int32 AFRBattleCharacter::GetSelectedWeaponIndex() const
{
	return SelectedWeaponIndex;
}

bool AFRBattleCharacter::IsSpecialAttackEnabled() const
{
	return bSpecialAttackEnabled;
}

FText AFRBattleCharacter::GetCharacterDisplayName() const
{
	return CharacterDisplayName;
}

AFRDestructibleTerrain* AFRBattleCharacter::FindTerrain() const
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

	for (TActorIterator<AFRDestructibleTerrain> It(World); It; ++It)
	{
		return *It;
	}

	return nullptr;
}

bool AFRBattleCharacter::IsSupportedByTerrain() const
{
	AFRDestructibleTerrain* Terrain = FindTerrain();
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

bool AFRBattleCharacter::FindFootprintSurfaceZ(const AFRDestructibleTerrain& Terrain, float CenterWorldX, float StartWorldZ, float SearchDistance, float& OutSurfaceZ) const
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

bool AFRBattleCharacter::IsFootprintBlocked(const AFRDestructibleTerrain& Terrain, const FVector& CenterLocation, float FootWorldZ) const
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

bool AFRBattleCharacter::TryResolveFootprintBlock(const AFRDestructibleTerrain& Terrain, FVector& InOutCenterLocation, float& InOutFootWorldZ) const
{
	if (!IsFootprintBlocked(Terrain, InOutCenterLocation, InOutFootWorldZ))
	{
		return true;
	}

	const float LiftStep = FMath::Max(1.0f, Terrain.CellSize);
	for (float Lift = LiftStep; Lift <= MaxStepUp + KINDA_SMALL_NUMBER; Lift += LiftStep)
	{
		const float CandidateFootWorldZ = InOutFootWorldZ + Lift;
		FVector CandidateLocation = InOutCenterLocation;
		CandidateLocation.Z = CandidateFootWorldZ + FootOffsetZ;
		if (!IsFootprintBlocked(Terrain, CandidateLocation, CandidateFootWorldZ))
		{
			InOutCenterLocation = CandidateLocation;
			InOutFootWorldZ = CandidateFootWorldZ;
			return true;
		}
	}

	return false;
}

bool AFRBattleCharacter::IsSlopeTraversable(float CurrentFootWorldZ, float NextSurfaceWorldZ, float HorizontalDistance, float TerrainCellSize) const
{
	if (NextSurfaceWorldZ <= CurrentFootWorldZ)
	{
		return true;
	}

	const float HorizontalSpan = FMath::Max(FMath::Abs(HorizontalDistance), FMath::Max(TerrainCellSize, 1.0f));
	const float MaxVerticalDelta = FMath::Tan(FMath::DegreesToRadians(GetMaxCharacterSlopeDegrees())) * HorizontalSpan;
	return NextSurfaceWorldZ - CurrentFootWorldZ <= MaxVerticalDelta + KINDA_SMALL_NUMBER;
}

float AFRBattleCharacter::GetMaxCharacterSlopeDegrees() const
{
	const AFRGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFRGameMode>() : nullptr;
	const float MaxSlopeDegrees = GameMode ? GameMode->GetMaxCharacterSlopeDegrees() : DefaultMaxCharacterSlopeDegrees;
	return FMath::Clamp(MaxSlopeDegrees, 0.0f, MaxUsableSlopeDegrees);
}

float AFRBattleCharacter::ClampWorldXToTerrainBounds(const AFRDestructibleTerrain& Terrain, float WorldX) const
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

void AFRBattleCharacter::UpdateBodyTerrainAlignment(const AFRDestructibleTerrain* Terrain)
{
	if (!BodyFrame)
	{
		return;
	}

	if (!Terrain)
	{
		UpdateCharacterRotation(0.0f);
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
		UpdateCharacterRotation(0.0f);
		return;
	}

	const float SurfaceDeltaZ = RightSurfaceZ - LeftSurfaceZ;
	const float AbsSurfaceDeltaZ = FMath::Abs(SurfaceDeltaZ);
	if (AbsSurfaceDeltaZ <= BodySlopeVisualDeadZoneHeight)
	{
		UpdateCharacterRotation(0.0f);
		return;
	}

	const float SlopeAngleDegrees = FMath::RadiansToDegrees(FMath::Atan2(SurfaceDeltaZ, ProbeHalfWidth * 2.0f));
	const float MaxSlopeDegrees = GetMaxCharacterSlopeDegrees();
	const float ClampedPitch = FMath::Clamp(SlopeAngleDegrees, -MaxSlopeDegrees, MaxSlopeDegrees);
	UpdateCharacterRotation(ClampedPitch);
}

void AFRBattleCharacter::ApplyTerrainGravity(float DeltaSeconds)
{
	if (IsDefeated())
	{
		return;
	}

	AFRDestructibleTerrain* Terrain = FindTerrain();
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

bool AFRBattleCharacter::CheckFallDeath()
{
	AFRDestructibleTerrain* Terrain = FindTerrain();
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

void AFRBattleCharacter::SnapToTerrain()
{
	AFRDestructibleTerrain* Terrain = FindTerrain();
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

void AFRBattleCharacter::SetFacingFromAxis(float Axis)
{
	if (!FMath::IsNearlyZero(Axis))
	{
		bFacingRight = Axis > 0.0f;
		UpdateCharacterRotation(GetActorPitchDegrees());
	}
}

void AFRBattleCharacter::UpdateCharacterRotation(float PitchDegrees)
{
	SetActorRotation(FRotator(0.0f, bFacingRight ? 0.0f : 180.0f, 0.0f));
	if (BodyFrame)
	{
		BodyFrame->SetRelativeRotation(FRotator(PitchDegrees, 0.0f, 0.0f));
	}
	UpdateBodySpriteLocalTransform();
}

void AFRBattleCharacter::UpdateBodySpriteLocalTransform()
{
	if (BodyFrame)
	{
		BodyFrame->SetRelativeLocation(FVector(0.0f, 0.0f, -FootOffsetZ));
	}
	if (Body)
	{
		Body->SetRelativeLocation(FVector(0.0f, 0.0f, FootOffsetZ));
		Body->SetRelativeRotation(FRotator::ZeroRotator);
	}
	if (BodySprite)
	{
		BodySprite->SetRelativeLocation(FVector::ZeroVector);
		BodySprite->SetRelativeRotation(FRotator::ZeroRotator);
	}
}

void AFRBattleCharacter::InitializeStatusWidget()
{
	if (!StatusWidgetComponent)
	{
		return;
	}

	if (StatusWidgetClass)
	{
		StatusWidgetComponent->SetWidgetClass(StatusWidgetClass);
	}
	else if (UClass* LoadedStatusWidgetClass = LoadClass<UFRBattleCharacterStatusWidget>(nullptr, TEXT("/Game/FortRogue/Widget/Character/WBP_BattleCharacterStatus.WBP_BattleCharacterStatus_C")))
	{
		StatusWidgetClass = LoadedStatusWidgetClass;
		StatusWidgetComponent->SetWidgetClass(StatusWidgetClass);
	}
	StatusWidgetComponent->InitWidget();
	RefreshStatusWidget();
}

void AFRBattleCharacter::RefreshStatusWidget()
{
	if (UFRBattleCharacterStatusWidget* StatusWidget = StatusWidgetComponent ? Cast<UFRBattleCharacterStatusWidget>(StatusWidgetComponent->GetUserWidgetObject()) : nullptr)
	{
		StatusWidget->SetBattleCharacter(this);
	}
}

void AFRBattleCharacter::SpawnFloatingDamageText(float DamageAmount)
{
	if (!GetWorld())
	{
		return;
	}

	TSubclassOf<AFRFloatingDamageTextActor> SpawnClass = FloatingDamageTextActorClass;
	if (!SpawnClass)
	{
		SpawnClass = AFRFloatingDamageTextActor::StaticClass();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	AFRFloatingDamageTextActor* DamageTextActor = GetWorld()->SpawnActor<AFRFloatingDamageTextActor>(
		SpawnClass,
		GetActorLocation() + FVector(0.0f, 0.0f, 128.0f),
		FRotator::ZeroRotator,
		SpawnParams);
	if (DamageTextActor)
	{
		DamageTextActor->InitializeDamageText(DamageAmount);
	}
}

float AFRBattleCharacter::GetActorPitchDegrees() const
{
	return BodyFrame ? BodyFrame->GetRelativeRotation().Pitch : GetActorRotation().Pitch;
}

FVector AFRBattleCharacter::GetProjectileLaunchDirection(float SpreadDegrees) const
{
	const float RelativeAimDegrees = FMath::Clamp(AimAngle + SpreadDegrees, GetMinAimAngle(), GetMaxAimAngle());
	const FRotator ActorRotation = GetActorRotation();
	const bool bActorFacingLeft = FMath::IsNearlyEqual(FMath::Abs(FRotator::NormalizeAxis(ActorRotation.Yaw)), 180.0f);
	const float BodyPitchDegrees = GetActorPitchDegrees();
	const float WorldAngleDegrees = bActorFacingLeft
		? BodyPitchDegrees + 180.0f - RelativeAimDegrees
		: BodyPitchDegrees + RelativeAimDegrees;
	const float WorldAngleRadians = FMath::DegreesToRadians(WorldAngleDegrees);
	return FVector(FMath::Cos(WorldAngleRadians), 0.0f, FMath::Sin(WorldAngleRadians)).GetSafeNormal();
}

FVector AFRBattleCharacter::GetProjectileSpawnLocation(const FVector& LaunchDirection) const
{
	return GetActorLocation() + LaunchDirection * 70.0f + FVector(0.0f, 0.0f, 35.0f);
}

float AFRBattleCharacter::GetEffectiveShotPower() const
{
	const float MinPower = FMath::Clamp(FMath::Min(MinShotPower, MaxShotPower), 0.0f, 1.0f);
	const float MaxPower = FMath::Clamp(FMath::Max(MinShotPower, MaxShotPower), 0.0f, 1.0f);
	return FMath::Clamp(FMath::Max(ShotPower, MinPower), MinPower, MaxPower);
}

int32 AFRBattleCharacter::SpawnShotSpecProjectiles(const FFRShotSpec& ShotSpec, bool bIncreasePendingProjectileCount)
{
	if (!GetWorld())
	{
		return 0;
	}

	int32 SpawnedProjectiles = 0;
	for (int32 Index = 0; Index < ShotSpec.ProjectileCount; ++Index)
	{
		const float Spread = (ShotSpec.ProjectileCount > 1) ? (Index - (ShotSpec.ProjectileCount - 1) * 0.5f) * 5.0f : 0.0f;
		const FVector Direction = GetProjectileLaunchDirection(Spread);
		const FVector SpawnLocation = GetProjectileSpawnLocation(Direction);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		AFRProjectile* Projectile = GetWorld()->SpawnActor<AFRProjectile>(ShotSpec.ProjectileClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
		if (!Projectile)
		{
			continue;
		}

		Projectile->InitializeProjectileFromShotSpec(this, FindTerrain(), Direction * ShotSpec.LaunchSpeed, ShotSpec);
		if (AFRGameMode* GameMode = GetWorld()->GetAuthGameMode<AFRGameMode>())
		{
			GameMode->NotifyProjectileSpawned(Projectile, bIncreasePendingProjectileCount);
		}
		++SpawnedProjectiles;
	}
	return SpawnedProjectiles;
}

FFRShotSpec AFRBattleCharacter::BuildShotSpec(const FFRWeaponSpec& Weapon) const
{
	FFRShotSpec ShotSpec;
	ShotSpec.WeaponTag = Weapon.WeaponTag;
	ShotSpec.EffectTags = Weapon.ShotEffectTags;
	if (Weapon.WeaponTag.IsValid())
	{
		ShotSpec.EffectTags.AddTag(Weapon.WeaponTag);
	}
	ShotSpec.HitDamage = FMath::Max(0.0f, Weapon.HitDamage * PendingAttackMultiplier);
	ShotSpec.Damage = FMath::Max(0.0f, (Weapon.Damage + CombatSet->GetDamage()) * PendingAttackMultiplier);
	ShotSpec.BlastRadius = FMath::Max(0.0f, Weapon.BlastRadius);
	ShotSpec.ExplosionFullDamageRadius = FMath::Clamp(Weapon.ExplosionFullDamageRadius, 0.0f, ShotSpec.BlastRadius);
	ShotSpec.TerrainDamage = FMath::Max(0.0f, Weapon.TerrainDamage);
	ShotSpec.TerrainFillRadius = 0.0f;
	ShotSpec.LaunchSpeed = FMath::Max(0.0f, Weapon.ProjectileSpeed * GetEffectiveShotPower() * CombatSet->GetShotPowerMultiplier());
	ShotSpec.Gravity = FMath::Max(0.0f, Weapon.Gravity);
	ShotSpec.ProjectileCount = FMath::Max(1, Weapon.ProjectilesPerShot + FMath::RoundToInt(CombatSet->GetProjectileCount()) - 1);
	ShotSpec.SalvoCount = FMath::Max(1, Weapon.SalvoCount);
	ShotSpec.SalvoInterval = FMath::Max(0.0f, Weapon.SalvoInterval);
	ShotSpec.ProjectileClass = Weapon.ProjectileClass ? Weapon.ProjectileClass : TSubclassOf<AFRProjectile>(AFRProjectile::StaticClass());
	for (const FFRProjectileEffectSpec& ProjectileEffect : Weapon.ProjectileEffects)
	{
		ProjectileEffect.ApplyToShotSpec(ShotSpec);
		if (ProjectileEffect.RequiresProjectileRuntime())
		{
			ShotSpec.ProjectileEffects.Add(ProjectileEffect);
		}
	}
	const AFRGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFRGameMode>() : nullptr;
	const float Wind = GameMode ? GameMode->GetWind() : 0.0f;
	auto ApplyShotModifier = [this, &ShotSpec, Wind](const FFRShotModifierSpec& Modifier)
	{
		if (!Modifier.MeetsShotConditions(ShotSpec, AimAngle, Wind, bFacingRight))
		{
			return;
		}

		Modifier.ApplyToShotSpec(ShotSpec);
	};
	for (const FFRShotModifierSpec& Modifier : GrantedShotModifiers)
	{
		ApplyShotModifier(Modifier);
	}
	for (const FFRShotModifierSpec& Modifier : PendingShotModifiers)
	{
		ApplyShotModifier(Modifier);
	}
	return ShotSpec;
}

void AFRBattleCharacter::GrantStartupAbilitySets()
{
	for (UFRAbilitySet* AbilitySet : StartupAbilitySets)
	{
		GrantAbilitySet(AbilitySet);
	}
}

void AFRBattleCharacter::EnsureDefaultLoadout()
{
	if (DefaultLoadoutDefinition && WeaponLoadout.Num() == 0)
	{
		for (UFRWeaponDefinition* WeaponDefinition : DefaultLoadoutDefinition->WeaponDefinitions)
		{
			AddWeaponDefinition(WeaponDefinition);
		}
	}

	if (DefaultLoadoutDefinition && ItemLoadout.Num() == 0)
	{
		for (const FFRItemStack& ItemStack : DefaultLoadoutDefinition->ItemDefinitions)
		{
			AddItemDefinition(ItemStack.ItemDefinition, ItemStack.Charges);
		}
	}
}
