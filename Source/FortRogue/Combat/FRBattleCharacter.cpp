// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/FRBattleCharacter.h"

#include "AbilitySystem/FRAbilitySet.h"
#include "AbilitySystem/FRAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/FRCombatSet.h"
#include "Combat/FRCharacterSpriteAnimator.h"
#include "Combat/FRDestructibleTerrain.h"
#include "Combat/FRProjectile.h"
#include "Combat/FRShotAimSolver.h"
#include "Combat/FRTerrainMovementComponent.h"
#include "FRGameMode.h"
#include "FRGameplayTags.h"
#include "Game/FRGameFlowSettings.h"
#include "Game/FRRunSubsystem.h"
#include "Items/FRItemDefinition.h"
#include "Items/FRItemEffect.h"
#include "Perks/FRPerkDefinition.h"
#include "Rewards/FRRewardBlueprintLibrary.h"
#include "Run/FRDefaultLoadoutDefinition.h"
#include "Weapons/FRWeaponDefinition.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/StaticMesh.h"
#include "PaperFlipbook.h"
#include "PaperFlipbookComponent.h"
#include "UI/FRBattleCharacterAimIndicatorWidget.h"
#include "UI/FRBattleCharacterStatusWidget.h"
#include "UI/FRFloatingDamageTextActor.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
bool DoesShotModifierMatchTag(const FFRShotModifierSpec& Modifier, FGameplayTag ModifierTag)
{
	return (Modifier.ModifierTag.IsValid() && Modifier.ModifierTag.MatchesTagExact(ModifierTag))
		|| Modifier.EffectTags.HasTagExact(ModifierTag);
}

bool DoesAbilitySetMatchTag(const UFRAbilitySet* AbilitySet, FGameplayTag AbilitySetTag)
{
	return AbilitySet && AbilitySetTag.IsValid() && AbilitySet->AbilitySetTag.MatchesTagExact(AbilitySetTag);
}

// 태그 하나가 값 읽기와 델타 적용 동작 한 쌍에 대응합니다. 새 어트리뷰트는 여기 한 줄만 추가하면 됩니다.
struct FFRCombatAttributeHandlers
{
	float (*GetValue)(const UFRCombatSet&) = nullptr;
	void (*ApplyDelta)(UFRCombatSet&, float) = nullptr;
};

const TMap<FGameplayTag, FFRCombatAttributeHandlers>& GetCombatAttributeHandlers()
{
	static const TMap<FGameplayTag, FFRCombatAttributeHandlers> Handlers = []()
	{
		TMap<FGameplayTag, FFRCombatAttributeHandlers> Map;
		Map.Add(FRGameplayTags::Attribute_Health, {
			[](const UFRCombatSet& Set) { return Set.GetHealth(); },
			nullptr }); // Health 델타는 회복/피해 연출이 달라 캐릭터에서 직접 처리합니다.
		Map.Add(FRGameplayTags::Attribute_MaxHealth, {
			[](const UFRCombatSet& Set) { return Set.GetMaxHealth(); },
			[](UFRCombatSet& Set, float Delta) { Set.AddMaxHealth(Delta); } });
		Map.Add(FRGameplayTags::Attribute_MoveBudget, {
			[](const UFRCombatSet& Set) { return Set.GetMoveBudget(); },
			[](UFRCombatSet& Set, float Delta) { Set.SetMoveBudget(FMath::Clamp(Set.GetMoveBudget() + Delta, 0.0f, Set.GetMaxMoveBudget())); } });
		Map.Add(FRGameplayTags::Attribute_MaxMoveBudget, {
			[](const UFRCombatSet& Set) { return Set.GetMaxMoveBudget(); },
			[](UFRCombatSet& Set, float Delta) { Set.AddMaxMoveBudget(Delta); } });
		Map.Add(FRGameplayTags::Attribute_Damage, {
			[](const UFRCombatSet& Set) { return Set.GetDamage(); },
			[](UFRCombatSet& Set, float Delta) { Set.AddDamage(Delta); } });
		Map.Add(FRGameplayTags::Attribute_ShotPowerMultiplier, {
			[](const UFRCombatSet& Set) { return Set.GetShotPowerMultiplier(); },
			[](UFRCombatSet& Set, float Delta) { Set.AddShotPowerMultiplier(Delta); } });
		Map.Add(FRGameplayTags::Attribute_ProjectileCount, {
			[](const UFRCombatSet& Set) { return Set.GetProjectileCount(); },
			[](UFRCombatSet& Set, float Delta) { Set.AddProjectileCount(Delta); } });
		Map.Add(FRGameplayTags::Attribute_MinAimAngle, {
			[](const UFRCombatSet& Set) { return FMath::Min(Set.GetMinAimAngle(), Set.GetMaxAimAngle()); },
			[](UFRCombatSet& Set, float Delta) { Set.AddMinAimAngle(Delta); } });
		Map.Add(FRGameplayTags::Attribute_MaxAimAngle, {
			[](const UFRCombatSet& Set) { return FMath::Max(Set.GetMinAimAngle(), Set.GetMaxAimAngle()); },
			[](UFRCombatSet& Set, float Delta) { Set.AddMaxAimAngle(Delta); } });
		return Map;
	}();
	return Handlers;
}
}

AFRBattleCharacter::AFRBattleCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	Root->SetRelativeScale3D(FVector::OneVector);

	BodyFrame = CreateDefaultSubobject<USceneComponent>(TEXT("BodyFrame"));
	BodyFrame->SetupAttachment(Root);
	BodyFrame->SetRelativeScale3D(FVector::OneVector);

	Hurtbox = CreateDefaultSubobject<UBoxComponent>(TEXT("Hurtbox"));
	Hurtbox->SetupAttachment(BodyFrame);
	Hurtbox->SetRelativeScale3D(FVector::OneVector);
	Hurtbox->InitBoxExtent(FVector(28.0f, 16.0f, 42.0f));
	Hurtbox->SetRelativeLocation(FVector(0.0f, 0.0f, 45.0f));
	Hurtbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Hurtbox->SetCollisionResponseToAllChannels(ECR_Ignore);
	Hurtbox->SetGenerateOverlapEvents(false);
	Hurtbox->SetCanEverAffectNavigation(false);

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Body->SetupAttachment(BodyFrame);
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Body->SetRelativeScale3D(FVector::OneVector);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Body->SetStaticMesh(CubeMesh.Object);
	}

	BodySprite = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("BodySprite"));
	BodySprite->SetupAttachment(BodyFrame);
	BodySprite->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodySprite->SetRelativeScale3D(FVector::OneVector);
	BodySprite->SetVisibility(false);

	SpriteAnimator = CreateDefaultSubobject<UFRCharacterSpriteAnimator>(TEXT("SpriteAnimator"));

	Muzzle = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle"));
	Muzzle->SetupAttachment(BodyFrame);
	Muzzle->SetRelativeScale3D(FVector::OneVector);

	TerrainMovement = CreateDefaultSubobject<UFRTerrainMovementComponent>(TEXT("TerrainMovement"));
	TerrainMovement->OnSurfacePitchChanged.BindUObject(this, &AFRBattleCharacter::HandleSurfacePitchChanged);
	TerrainMovement->OnSupportLost.BindUObject(this, &AFRBattleCharacter::HandleSupportLost);
	TerrainMovement->OnFellToDeath.BindUObject(this, &AFRBattleCharacter::HandleFellToDeath);
	TerrainMovement->CanApplyGravity.BindWeakLambda(this, [this]() { return !IsDefeated(); });

	AngleIndicatorWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("AngleIndicatorWidget"));
	AngleIndicatorWidget->SetupAttachment(BodyFrame);
	AngleIndicatorWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AngleIndicatorWidget->SetWidgetSpace(EWidgetSpace::World);
	AngleIndicatorWidget->SetDrawAtDesiredSize(false);
	AngleIndicatorWidget->SetDrawSize(FVector2D(128.0f, 128.0f));
	AngleIndicatorWidget->SetPivot(FVector2D(0.5f, 0.5f));
	AngleIndicatorWidget->SetRelativeScale3D(FVector::OneVector);
	AngleIndicatorWidget->SetTranslucentSortPriority(10);
	AngleIndicatorWidget->SetTwoSided(true);

	HpWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HpWidget"));
	HpWidget->SetupAttachment(Root);
	HpWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HpWidget->SetWidgetSpace(EWidgetSpace::Screen);
	HpWidget->SetDrawAtDesiredSize(false);
	HpWidget->SetDrawSize(FVector2D(160.0f, 18.0f));
	HpWidget->SetPivot(FVector2D(0.5f, 0.0f));
	HpWidget->SetRelativeScale3D(FVector::OneVector);
	HpWidget->SetRelativeLocation(FVector(0.0f, 0.0f, -62.0f));

	FloatingDamageTextActorClass = AFRFloatingDamageTextActor::StaticClass();

	AbilitySystemComponent = CreateDefaultSubobject<UFRAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	CombatSet = CreateDefaultSubobject<UFRCombatSet>(TEXT("CombatSet"));

	CharacterDisplayName = FText::FromString(TEXT("Rookie Tank"));
	UpdateCharacterRotation(0.0f);
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
	TerrainMovement->SnapToTerrain();
	InitializeCharacterWidgets();
}

void AFRBattleCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	TerrainMovement->TickGravity(DeltaSeconds);
}

UAbilitySystemComponent* AFRBattleCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UFRTerrainMovementComponent* AFRBattleCharacter::GetTerrainMovement() const
{
	return TerrainMovement;
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
		const bool bHasAnySprite = InCharacterDefinition->BodyFlipbook != nullptr
			|| InCharacterDefinition->AnimationSet.HasAnyAnimation();
		BodySprite->SetFlipbook(InCharacterDefinition->BodyFlipbook);
		BodySprite->SetVisibility(bHasAnySprite);
		if (Body)
		{
			Body->SetVisibility(!bHasAnySprite);
		}
		if (SpriteAnimator)
		{
			SpriteAnimator->Initialize(BodySprite, InCharacterDefinition->AnimationSet, InCharacterDefinition->BodyFlipbook);
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
	UpdateAimDrivenComponents();

	GrantedShotModifierEntries.Reset();
	PendingShotModifiers.Reset();
	AcquiredPerkEntries.Reset();
	WeaponLoadout.Reset();
	BasicAttackIndex = INDEX_NONE;
	SpecialAttackIndex = INDEX_NONE;
	bSpecialAttackEnabled = InCharacterDefinition->bCanUseSpecialAttack;
	auto AddAttackDefinition = [this](UFRWeaponDefinition* WeaponDefinition) -> int32
	{
		if (!WeaponDefinition)
		{
			return INDEX_NONE;
		}

		const int32 PreviousCount = WeaponLoadout.Num();
		AddWeaponDefinition(WeaponDefinition);
		if (WeaponLoadout.Num() > PreviousCount)
		{
			return PreviousCount;
		}
		return GetWeaponIndexByTag(WeaponDefinition->Weapon.WeaponTag);
	};
	BasicAttackIndex = AddAttackDefinition(InCharacterDefinition->BasicAttackDefinition);
	SpecialAttackIndex = AddAttackDefinition(InCharacterDefinition->SpecialAttackDefinition);
	for (UFRWeaponDefinition* WeaponDefinition : InCharacterDefinition->WeaponLoadout)
	{
		if (WeaponDefinition != InCharacterDefinition->BasicAttackDefinition && WeaponDefinition != InCharacterDefinition->SpecialAttackDefinition)
		{
			AddWeaponDefinition(WeaponDefinition);
		}
	}
	SelectedWeaponIndex = BasicAttackIndex != INDEX_NONE ? BasicAttackIndex : 0;

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
	const float TerrainPitchDegrees = GetActorPitchDegrees();
	bEnemy = bNewEnemy;
	bFacingRight = !bEnemy;
	UpdateCharacterRotation(TerrainPitchDegrees);
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

	const float RequestedDelta = Axis * TerrainMovement->MoveSpeed * DeltaSeconds;
	const float Budget = CombatSet->GetMoveBudget();
	const float AppliedDelta = FMath::Clamp(RequestedDelta, -Budget, Budget);
	if (FMath::IsNearlyZero(AppliedDelta))
	{
		return;
	}

	const float ActualDelta = TerrainMovement->MoveHorizontal(AppliedDelta);
	if (FMath::IsNearlyZero(ActualDelta))
	{
		return;
	}

	CombatSet->SetMoveBudget(Budget - FMath::Abs(ActualDelta));
	if (SpriteAnimator)
	{
		SpriteAnimator->NotifyMoving();
	}
}

void AFRBattleCharacter::AdjustAim(float Axis, float DeltaSeconds)
{
	if (!bActiveTurn || bFiredThisTurn || !IsSupportedByTerrain() || FMath::IsNearlyZero(Axis))
	{
		return;
	}

	AimAngle = FMath::Clamp(AimAngle + Axis * 70.0f * DeltaSeconds, GetMinAimAngle(), GetMaxAimAngle());
	UpdateAimDrivenComponents();
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
	SelectWeapon(BasicAttackIndex != INDEX_NONE ? BasicAttackIndex : 0);
}

bool AFRBattleCharacter::SelectSpecialAttack()
{
	if (!CanSelectSpecialAttack())
	{
		return false;
	}

	SelectedWeaponIndex = SpecialAttackIndex;
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
		&& (WeaponIndex != SpecialAttackIndex || bSpecialAttackEnabled);
}

bool AFRBattleCharacter::CanSelectWeaponByTag(FGameplayTag WeaponTag) const
{
	return CanSelectWeapon(GetWeaponIndexByTag(WeaponTag));
}

bool AFRBattleCharacter::CanSelectSpecialAttack() const
{
	return SpecialAttackIndex != INDEX_NONE && bSpecialAttackEnabled && WeaponLoadout.IsValidIndex(SpecialAttackIndex);
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

int32 AFRBattleCharacter::GetBasicAttackIndex() const
{
	return BasicAttackIndex;
}

int32 AFRBattleCharacter::GetSpecialAttackIndex() const
{
	return SpecialAttackIndex;
}

int32 AFRBattleCharacter::FireSelectedWeapon()
{
	if (!CanFireSelectedWeapon())
	{
		return 0;
	}

	bChargingShot = false;
	bFiredThisTurn = true;

	if (SpriteAnimator)
	{
		const bool bSpecialShot = SpecialAttackIndex != INDEX_NONE && SelectedWeaponIndex == SpecialAttackIndex;
		SpriteAnimator->NotifyShoot(bSpecialShot);
	}

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
	const float TerrainPitchDegrees = GetActorPitchDegrees();
	bFacingRight = ToTarget.X >= 0.0f;
	UpdateCharacterRotation(TerrainPitchDegrees);

	UFRRunSubsystem* RunSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFRRunSubsystem>() : nullptr;
	auto RandInRange = [RunSubsystem](float Min, float Max)
	{
		return RunSubsystem ? RunSubsystem->RandRange(Min, Max) : FMath::RandRange(Min, Max);
	};

	const float AimError = DifficultyData.AimAngleErrorDegrees > 0.0f ? RandInRange(-DifficultyData.AimAngleErrorDegrees, DifficultyData.AimAngleErrorDegrees) : 0.0f;
	const float PowerError = DifficultyData.ShotPowerError > 0.0f ? RandInRange(-DifficultyData.ShotPowerError, DifficultyData.ShotPowerError) : 0.0f;
	const float CharacterMinAim = GetMinAimAngle();
	const float CharacterMaxAim = GetMaxAimAngle();
	const float DifficultyMinAim = FMath::Min(DifficultyData.MinAimAngleDegrees, DifficultyData.MaxAimAngleDegrees);
	const float DifficultyMaxAim = FMath::Max(DifficultyData.MinAimAngleDegrees, DifficultyData.MaxAimAngleDegrees);
	const float MinAim = FMath::Clamp(DifficultyMinAim, CharacterMinAim, CharacterMaxAim);
	const float MaxAim = FMath::Clamp(DifficultyMaxAim, MinAim, CharacterMaxAim);
	const float MinPower = FMath::Min(DifficultyData.MinShotPower, DifficultyData.MaxShotPower);
	const float MaxPower = FMath::Max(DifficultyData.MinShotPower, DifficultyData.MaxShotPower);

	const FFRWeaponSpec& Weapon = GetCurrentWeapon();

	FFRShotAimSolverParams SolverParams;
	SolverParams.TargetLocation = Target->GetActorLocation();
	SolverParams.Wind = GetWorldWind();
	SolverParams.bFacingRight = bFacingRight;
	SolverParams.MinAimAngle = MinAim;
	SolverParams.MaxAimAngle = MaxAim;
	SolverParams.MinShotPower = MinPower;
	SolverParams.MaxShotPower = MaxPower;
	SolverParams.BuildShotSpec = [this, &Weapon](float CandidateAim, float CandidatePower)
	{
		return BuildShotSpecForAim(Weapon, CandidateAim, CandidatePower);
	};
	SolverParams.GetLaunchDirection = [this](float CandidateAim)
	{
		return GetLaunchDirectionForAim(CandidateAim);
	};
	SolverParams.GetSpawnLocation = [this](const FVector& LaunchDirection)
	{
		return GetProjectileSpawnLocation(LaunchDirection);
	};

	float BestAimAngle = AimAngle;
	float BestShotPower = ShotPower;
	if (FRShotAimSolver::SolveBallisticAim(SolverParams, BestAimAngle, BestShotPower))
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
	UpdateAimDrivenComponents();
}

void AFRBattleCharacter::ApplyDamage(float DamageAmount)
{
	const float HealthBeforeDamage = GetHealth();
	CombatSet->ApplyDamage(DamageAmount);
	const float AppliedDamage = FMath::Max(0.0f, HealthBeforeDamage - GetHealth());
	if (AppliedDamage > KINDA_SMALL_NUMBER)
	{
		SpawnFloatingDamageText(AppliedDamage);
		if (SpriteAnimator)
		{
			SpriteAnimator->NotifyHurt();
		}
	}
	RefreshCharacterWidgets();
}

void AFRBattleCharacter::ApplyHeal(float HealAmount)
{
	CombatSet->Heal(HealAmount);
	RefreshCharacterWidgets();
}

void AFRBattleCharacter::ApplyPendingAttackMultiplier(float Multiplier)
{
	PendingAttackMultiplier = FMath::Max(PendingAttackMultiplier, Multiplier);
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
	TerrainMovement->ReevaluateSupport();
}

void AFRBattleCharacter::ApplyImpactKnockback(float HorizontalDistance, FVector ImpactLocation, FVector ImpactVelocity)
{
	if (HorizontalDistance <= 0.0f || IsDefeated())
	{
		return;
	}

	TerrainMovement->ApplyKnockback(HorizontalDistance, ImpactLocation, ImpactVelocity);
}

void AFRBattleCharacter::SetTerrain(AFRDestructibleTerrain* InTerrain)
{
	TerrainMovement->SetTerrain(InTerrain);
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
	for (const UFRItemEffect* UseEffect : ItemDefinition->UseEffects)
	{
		if (UseEffect)
		{
			UseEffect->ApplyToCharacter(*this);
		}
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
	AppendGrantedShotModifiers(PerkDefinition->ShotModifiers, PerkDefinition);
	GrantAbilitySet(PerkDefinition->GrantedAbilitySet);

	FFRAcquiredPerkEntry& Entry = AcquiredPerkEntries.AddDefaulted_GetRef();
	Entry.PerkDefinition = PerkDefinition;
}

bool AFRBattleCharacter::RemovePerkDefinition(UFRPerkDefinition* PerkDefinition)
{
	if (!PerkDefinition)
	{
		return false;
	}

	int32 EntryIndex = INDEX_NONE;
	for (int32 Index = AcquiredPerkEntries.Num() - 1; Index >= 0; --Index)
	{
		if (AcquiredPerkEntries[Index].PerkDefinition == PerkDefinition)
		{
			EntryIndex = Index;
			break;
		}
	}
	if (EntryIndex == INDEX_NONE)
	{
		return false;
	}

	ApplyRewardDamage(-PerkDefinition->DamageBonus);
	ApplyRewardHealth(-PerkDefinition->MaxHealthBonus);
	ApplyRewardMoveBudget(-PerkDefinition->MaxMoveBudgetBonus);
	ApplyRewardProjectiles(-PerkDefinition->ProjectileBonus);
	ApplyRewardShotPowerMultiplier(-PerkDefinition->ShotPowerMultiplierBonus);
	if (PerkDefinition->GrantedAbilitySet)
	{
		RemoveAbilitySet(PerkDefinition->GrantedAbilitySet);
	}

	int32 ModifiersToRemove = PerkDefinition->ShotModifiers.Num();
	for (int32 Index = GrantedShotModifierEntries.Num() - 1; Index >= 0 && ModifiersToRemove > 0; --Index)
	{
		if (GrantedShotModifierEntries[Index].SourcePerk == PerkDefinition)
		{
			GrantedShotModifierEntries.RemoveAt(Index);
			--ModifiersToRemove;
		}
	}

	AcquiredPerkEntries.RemoveAt(EntryIndex);
	RefreshCharacterWidgets();
	return true;
}

TArray<UFRPerkDefinition*> AFRBattleCharacter::GetAcquiredPerksForBlueprint() const
{
	TArray<UFRPerkDefinition*> Perks;
	for (const FFRAcquiredPerkEntry& Entry : AcquiredPerkEntries)
	{
		if (Entry.PerkDefinition)
		{
			Perks.Add(Entry.PerkDefinition);
		}
	}
	return Perks;
}

int32 AFRBattleCharacter::GetPerkStackCount(const UFRPerkDefinition* PerkDefinition) const
{
	if (!PerkDefinition)
	{
		return 0;
	}

	int32 StackCount = 0;
	for (const FFRAcquiredPerkEntry& Entry : AcquiredPerkEntries)
	{
		if (Entry.PerkDefinition == PerkDefinition)
		{
			++StackCount;
		}
	}
	return StackCount;
}

bool AFRBattleCharacter::HasPerkByTag(FGameplayTag PerkTag) const
{
	if (!PerkTag.IsValid())
	{
		return false;
	}

	for (const FFRAcquiredPerkEntry& Entry : AcquiredPerkEntries)
	{
		if (Entry.PerkDefinition && Entry.PerkDefinition->PerkTag.MatchesTagExact(PerkTag))
		{
			return true;
		}
	}
	return false;
}

FText AFRBattleCharacter::GetAcquiredPerksSummary() const
{
	TArray<FString> Parts;
	for (const FFRAcquiredPerkEntry& Entry : AcquiredPerkEntries)
	{
		if (Entry.PerkDefinition)
		{
			Parts.Add(Entry.PerkDefinition->DisplayName.ToString());
		}
	}
	return Parts.Num() > 0 ? FText::FromString(FString::Join(Parts, TEXT(" | "))) : FText::GetEmpty();
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

void AFRBattleCharacter::AppendGrantedShotModifiers(const TArray<FFRShotModifierSpec>& ShotModifiers, UFRPerkDefinition* SourcePerk)
{
	for (const FFRShotModifierSpec& Modifier : ShotModifiers)
	{
		FFRGrantedShotModifierEntry& Entry = GrantedShotModifierEntries.AddDefaulted_GetRef();
		Entry.Spec = Modifier;
		Entry.SourcePerk = SourcePerk;
	}
}

void AFRBattleCharacter::GrantShotModifiers(const TArray<FFRShotModifierSpec>& ShotModifiers)
{
	AppendGrantedShotModifiers(ShotModifiers, nullptr);
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
	for (int32 Index = GrantedShotModifierEntries.Num() - 1; Index >= 0; --Index)
	{
		if (!DoesShotModifierMatchTag(GrantedShotModifierEntries[Index].Spec, ModifierTag))
		{
			continue;
		}

		GrantedShotModifierEntries.RemoveAt(Index);
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
		SelectBasicAttack();
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

	const FFRCombatAttributeHandlers* Handlers = GetCombatAttributeHandlers().Find(AttributeTag);
	if (!Handlers || !Handlers->GetValue)
	{
		return false;
	}

	OutValue = Handlers->GetValue(*CombatSet);
	return true;
}

bool AFRBattleCharacter::TryApplyCombatAttributeDeltaByTag(FGameplayTag AttributeTag, float DeltaValue)
{
	if (!AttributeTag.IsValid())
	{
		return false;
	}

	// 체력 델타는 회복과 피해가 서로 다른 연출(플로팅 텍스트)을 타므로 별도 처리합니다.
	if (AttributeTag.MatchesTagExact(FRGameplayTags::Attribute_Health))
	{
		if (DeltaValue >= 0.0f)
		{
			ApplyHeal(DeltaValue);
		}
		else
		{
			ApplyDamage(-DeltaValue);
		}
		return true;
	}

	const FFRCombatAttributeHandlers* Handlers = GetCombatAttributeHandlers().Find(AttributeTag);
	if (!Handlers || !Handlers->ApplyDelta)
	{
		return false;
	}

	Handlers->ApplyDelta(*CombatSet, DeltaValue);

	if (AttributeTag.MatchesTagExact(FRGameplayTags::Attribute_MinAimAngle) || AttributeTag.MatchesTagExact(FRGameplayTags::Attribute_MaxAimAngle))
	{
		AimAngle = FMath::Clamp(AimAngle, GetMinAimAngle(), GetMaxAimAngle());
		UpdateAimDrivenComponents();
	}
	return true;
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

bool AFRBattleCharacter::IsFacingRight() const
{
	return bFacingRight;
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
	TArray<FFRShotModifierSpec> Modifiers;
	Modifiers.Reserve(GrantedShotModifierEntries.Num());
	for (const FFRGrantedShotModifierEntry& Entry : GrantedShotModifierEntries)
	{
		Modifiers.Add(Entry.Spec);
	}
	return Modifiers;
}

TArray<FFRShotModifierSpec> AFRBattleCharacter::GetPendingShotModifiersForBlueprint() const
{
	return PendingShotModifiers;
}

FText AFRBattleCharacter::GetGrantedShotModifiersSummary() const
{
	return UFRRewardBlueprintLibrary::GetShotModifierEffectSummary(GetGrantedShotModifiersForBlueprint());
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
	for (const FFRGrantedShotModifierEntry& Entry : GrantedShotModifierEntries)
	{
		if (DoesShotModifierMatchTag(Entry.Spec, ModifierTag))
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

float AFRBattleCharacter::GetWorldWind() const
{
	const AFRGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFRGameMode>() : nullptr;
	return GameMode ? GameMode->GetWind() : 0.0f;
}

bool AFRBattleCharacter::DoesShotModifierMeetCurrentShotConditions(const FFRShotModifierSpec& ShotModifier) const
{
	return ShotModifier.MeetsShotConditions(GetCurrentShotSpec(), AimAngle, GetWorldWind(), bFacingRight);
}

FText AFRBattleCharacter::GetShotModifierCurrentConditionFailureSummary(const FFRShotModifierSpec& ShotModifier) const
{
	return ShotModifier.GetShotConditionFailureSummary(GetCurrentShotSpec(), AimAngle, GetWorldWind(), bFacingRight);
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
	return TerrainMovement->FindTerrain();
}

bool AFRBattleCharacter::IsSupportedByTerrain() const
{
	return TerrainMovement->IsSupportedByTerrain();
}

void AFRBattleCharacter::HandleSurfacePitchChanged(float PitchDegrees)
{
	UpdateCharacterRotation(PitchDegrees);
}

void AFRBattleCharacter::HandleSupportLost()
{
	bChargingShot = false;
}

void AFRBattleCharacter::HandleFellToDeath()
{
	bChargingShot = false;
	ApplyDamage(GetMaxHealth());
}

void AFRBattleCharacter::SetFacingFromAxis(float Axis)
{
	if (!FMath::IsNearlyZero(Axis))
	{
		const float TerrainPitchDegrees = GetActorPitchDegrees();
		bFacingRight = Axis > 0.0f;
		UpdateCharacterRotation(TerrainPitchDegrees);
	}
}

void AFRBattleCharacter::UpdateCharacterRotation(float PitchDegrees)
{
	SetActorRotation(FRotator(0.0f, bFacingRight ? 0.0f : 180.0f, 0.0f));
	if (BodyFrame)
	{
		const float LocalBodyPitch = bFacingRight ? PitchDegrees : -PitchDegrees;
		BodyFrame->SetRelativeRotation(FRotator(LocalBodyPitch, 0.0f, 0.0f));
	}
	UpdateBodyFrameChildrenLocalTransform();
	UpdateAimDrivenComponents();
}

void AFRBattleCharacter::UpdateBodyFrameChildrenLocalTransform()
{
	const float FootOffsetZ = TerrainMovement ? TerrainMovement->GetFootOffsetZ() : 45.0f;
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

void AFRBattleCharacter::UpdateAimDrivenComponents()
{
	const float RelativeAimDegrees = FMath::Clamp(AimAngle, GetMinAimAngle(), GetMaxAimAngle());
	const FRotator AimLocalRotation(RelativeAimDegrees, 0.0f, 0.0f);
	const FVector MuzzleLocalLocation = AimPivotLocalLocation + AimLocalRotation.RotateVector(FVector(MuzzleForwardOffset, 0.0f, 0.0f));

	if (Muzzle)
	{
		Muzzle->SetRelativeLocation(MuzzleLocalLocation);
		Muzzle->SetRelativeRotation(AimLocalRotation);
	}
	if (AngleIndicatorWidget)
	{
		const float CameraLocalYOffset = bFacingRight ? AimIndicatorCameraOffsetY : -AimIndicatorCameraOffsetY;
		AngleIndicatorWidget->SetRelativeLocation(AimPivotLocalLocation + FVector(0.0f, CameraLocalYOffset, 0.0f));
		AngleIndicatorWidget->SetRelativeRotation(FRotator(0.0f, bFacingRight ? 90.0f : -90.0f, 0.0f));
	}
}

void AFRBattleCharacter::InitializeCharacterWidgets()
{
	const UFRGameFlowSettings* GameFlowSettings = GetDefault<UFRGameFlowSettings>();

	if (AngleIndicatorWidget)
	{
		if (!AngleIndicatorWidgetClass && GameFlowSettings)
		{
			AngleIndicatorWidgetClass = GameFlowSettings->DefaultAimIndicatorWidgetClass.LoadSynchronous();
		}
		if (AngleIndicatorWidgetClass)
		{
			AngleIndicatorWidget->SetWidgetClass(AngleIndicatorWidgetClass);
		}
		AngleIndicatorWidget->InitWidget();
	}

	if (HpWidget)
	{
		if (!HpWidgetClass && GameFlowSettings)
		{
			HpWidgetClass = GameFlowSettings->DefaultCharacterStatusWidgetClass.LoadSynchronous();
		}
		if (HpWidgetClass)
		{
			HpWidget->SetWidgetClass(HpWidgetClass);
		}
		HpWidget->InitWidget();
	}

	RefreshCharacterWidgets();
}

void AFRBattleCharacter::RefreshCharacterWidgets()
{
	UpdateAimDrivenComponents();
	if (UFRBattleCharacterAimIndicatorWidget* AimWidget = AngleIndicatorWidget ? Cast<UFRBattleCharacterAimIndicatorWidget>(AngleIndicatorWidget->GetUserWidgetObject()) : nullptr)
	{
		AimWidget->SetBattleCharacter(this);
	}
	if (UFRBattleCharacterStatusWidget* HealthWidget = HpWidget ? Cast<UFRBattleCharacterStatusWidget>(HpWidget->GetUserWidgetObject()) : nullptr)
	{
		HealthWidget->SetBattleCharacter(this);
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
	if (!BodyFrame)
	{
		return GetActorRotation().Pitch;
	}

	const float LocalBodyPitch = BodyFrame->GetRelativeRotation().Pitch;
	return bFacingRight ? LocalBodyPitch : -LocalBodyPitch;
}

FVector AFRBattleCharacter::GetProjectileLaunchDirection(float SpreadDegrees) const
{
	return GetLaunchDirectionForAim(AimAngle + SpreadDegrees);
}

FVector AFRBattleCharacter::GetLaunchDirectionForAim(float AimDegrees) const
{
	const float RelativeAimDegrees = FMath::Clamp(AimDegrees, GetMinAimAngle(), GetMaxAimAngle());
	const FQuat AimLocalRotation = FRotator(RelativeAimDegrees, 0.0f, 0.0f).Quaternion();
	if (BodyFrame)
	{
		return (BodyFrame->GetComponentQuat() * AimLocalRotation).GetForwardVector().GetSafeNormal();
	}

	const FRotator FallbackRotation(RelativeAimDegrees, bFacingRight ? 0.0f : 180.0f, 0.0f);
	return FallbackRotation.Vector().GetSafeNormal();
}

FVector AFRBattleCharacter::GetAimPivotWorldLocation() const
{
	if (BodyFrame)
	{
		return BodyFrame->GetComponentTransform().TransformPosition(AimPivotLocalLocation);
	}

	return GetActorLocation() + AimPivotLocalLocation;
}

FVector AFRBattleCharacter::GetProjectileSpawnLocation(const FVector& LaunchDirection) const
{
	const FVector SafeLaunchDirection = LaunchDirection.GetSafeNormal();
	const float BodyScale = BodyFrame ? BodyFrame->GetComponentScale().GetAbsMax() : 1.0f;
	return GetAimPivotWorldLocation() + SafeLaunchDirection * MuzzleForwardOffset * BodyScale;
}

FVector AFRBattleCharacter::GetMuzzleSpawnLocation() const
{
	return Muzzle ? Muzzle->GetComponentLocation() : GetProjectileSpawnLocation(GetProjectileLaunchDirection(0.0f));
}

float AFRBattleCharacter::GetEffectiveShotPower() const
{
	return GetEffectiveShotPowerFor(ShotPower);
}

float AFRBattleCharacter::GetEffectiveShotPowerFor(float InShotPower) const
{
	const float MinPower = FMath::Clamp(FMath::Min(MinShotPower, MaxShotPower), 0.0f, 1.0f);
	const float MaxPower = FMath::Clamp(FMath::Max(MinShotPower, MaxShotPower), 0.0f, 1.0f);
	return FMath::Clamp(FMath::Max(InShotPower, MinPower), MinPower, MaxPower);
}

int32 AFRBattleCharacter::SpawnShotSpecProjectiles(const FFRShotSpec& ShotSpec, bool bIncreasePendingProjectileCount)
{
	if (!GetWorld())
	{
		return 0;
	}

	UpdateAimDrivenComponents();
	const FVector SpawnLocation = GetMuzzleSpawnLocation();

	int32 SpawnedProjectiles = 0;
	for (int32 Index = 0; Index < ShotSpec.ProjectileCount; ++Index)
	{
		const float Spread = (ShotSpec.ProjectileCount > 1) ? (Index - (ShotSpec.ProjectileCount - 1) * 0.5f) * 5.0f : 0.0f;
		const FVector Direction = GetProjectileLaunchDirection(Spread);

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
	return BuildShotSpecForAim(Weapon, AimAngle, ShotPower);
}

FFRShotSpec AFRBattleCharacter::BuildShotSpecForAim(const FFRWeaponSpec& Weapon, float InAimAngle, float InShotPower) const
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
	ShotSpec.LaunchSpeed = FMath::Max(0.0f, Weapon.ProjectileSpeed * GetEffectiveShotPowerFor(InShotPower) * CombatSet->GetShotPowerMultiplier());
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
	const float Wind = GetWorldWind();
	auto ApplyShotModifier = [this, &ShotSpec, Wind, InAimAngle](const FFRShotModifierSpec& Modifier)
	{
		if (!Modifier.MeetsShotConditions(ShotSpec, InAimAngle, Wind, bFacingRight))
		{
			return;
		}

		Modifier.ApplyToShotSpec(ShotSpec);
	};
	for (const FFRGrantedShotModifierEntry& Entry : GrantedShotModifierEntries)
	{
		ApplyShotModifier(Entry.Spec);
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
		if (BasicAttackIndex == INDEX_NONE && WeaponLoadout.Num() > 0)
		{
			BasicAttackIndex = 0;
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
