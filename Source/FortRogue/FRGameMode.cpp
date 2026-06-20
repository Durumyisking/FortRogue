// Copyright Epic Games, Inc. All Rights Reserved.

#include "FRGameMode.h"

#include "Characters/FRCharacterDefinition.h"
#include "Combat/FRBattleCharacter.h"
#include "Combat/FRDestructibleTerrain.h"
#include "Combat/FRProjectile.h"
#include "Combat/FRTerrainMapDefinition.h"
#include "FortRogue.h"
#include "FRGameplayTags.h"
#include "FRPlayerController.h"
#include "Items/FRItemDefinition.h"
#include "Perks/FRPerkDefinition.h"
#include "Run/FRDefaultLoadoutDefinition.h"
#include "Run/FRStageRunDefinition.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

namespace
{
constexpr float TerrainCameraPadding = 240.0f;
constexpr float MinimumTerrainCameraOrthoWidth = 1200.0f;
constexpr float ExpectedWideViewportAspectRatio = 16.0f / 9.0f;
constexpr int32 MaxRewardChoiceCount = 5;
const FRotator BattleCameraRotation(0.0f, -90.0f, 0.0f);
}

AFRGameMode::AFRGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	PlayerControllerClass = AFRPlayerController::StaticClass();
	DefaultPawnClass = nullptr;
	StatusText = FText::FromString(TEXT("Ready"));
	PlayerCharacterClass = AFRBattleCharacter::StaticClass();
	EnemyCharacterClass = AFRBattleCharacter::StaticClass();
	TerrainClass = AFRDestructibleTerrain::StaticClass();
	CameraClass = ACameraActor::StaticClass();
}

void AFRGameMode::BeginPlay()
{
	Super::BeginPlay();

	CurrentStage = 1;
	if (StageRunDefinition)
	{
		StageRunDefinition->NormalizeStageData();
	}
	EncounteredEnemyDefinitions.Reset();
	ChosenRewardTags.Reset();
	SelectNextEnemyDefinition();
	SpawnMVPBattle();
	StartPlayerTurn();
}

void AFRGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateBattleCamera(DeltaSeconds);
	CheckTurnDefeatState();
}

void AFRGameMode::NotifyProjectileSpawned(AFRProjectile* Projectile, bool bIncreasePendingProjectileCount)
{
	if (Projectile)
	{
		ActiveProjectiles.Add(Projectile);
		if (bIncreasePendingProjectileCount && BattleState == EFRBattleState::ResolvingShot)
		{
			++PendingProjectiles;
		}
	}
}

void AFRGameMode::NotifyShotFired(AFRBattleCharacter* Shooter, int32 ProjectileCount)
{
	if (!Shooter || ProjectileCount <= 0)
	{
		return;
	}

	PendingProjectiles = ProjectileCount;
	LastShooter = Shooter;
	BattleState = EFRBattleState::ResolvingShot;
	Shooter->EndTurn();
	SetStatus(Shooter->IsEnemy() ? TEXT("Enemy shell incoming") : TEXT("Shell fired"));
}

void AFRGameMode::NotifyProjectileResolved(AFRProjectile* Projectile)
{
	if (Projectile)
	{
		LastImpactCameraLocation = Projectile->GetActorLocation();
		ActiveProjectiles.RemoveAll([Projectile](const TWeakObjectPtr<AFRProjectile>& ActiveProjectile)
		{
			return !ActiveProjectile.IsValid() || ActiveProjectile.Get() == Projectile;
		});
	}

	if (BattleState != EFRBattleState::ResolvingShot)
	{
		return;
	}

	PendingProjectiles = FMath::Max(0, PendingProjectiles - 1);
	if (PendingProjectiles == 0)
	{
		bHoldingImpactCamera = true;
		GetWorldTimerManager().SetTimer(ShotResolutionTimerHandle, this, &AFRGameMode::FinishShotResolution, ShotImpactCameraHoldSeconds, false);
	}
}

void AFRGameMode::ApplyRewardChoice(int32 ChoiceIndex)
{
	if (!CanApplyRewardChoice(ChoiceIndex))
	{
		return;
	}

	const FFRRewardChoice Reward = RewardChoices[ChoiceIndex];
	ApplyRewardToPlayer(Reward);
	if (Reward.RewardTag.IsValid())
	{
		ChosenRewardTags.AddUnique(Reward.RewardTag);
	}

	SetStatus(FString::Printf(TEXT("Reward chosen: %s"), *Reward.DisplayName.ToString()));
	RewardChoices.Reset();
	AdvanceToNextStage();
}

bool AFRGameMode::CanApplyRewardChoice(int32 ChoiceIndex) const
{
	return BattleState == EFRBattleState::Reward
		&& PlayerCharacter
		&& RewardChoices.IsValidIndex(ChoiceIndex)
		&& RewardChoices[ChoiceIndex].MeetsRewardTagConditions(GetChosenRewardTags());
}

float AFRGameMode::GetWind() const
{
	return Wind;
}

FText AFRGameMode::GetWindSummary() const
{
	return FText::FromString(FMath::IsNearlyZero(Wind)
		? FString(TEXT("Wind 0"))
		: FString::Printf(TEXT("Wind %+.0f"), Wind));
}

EFRBattleState AFRGameMode::GetBattleState() const
{
	return BattleState;
}

AFRBattleCharacter* AFRGameMode::GetPlayerCharacter() const
{
	return PlayerCharacter;
}

AFRBattleCharacter* AFRGameMode::GetEnemyCharacter() const
{
	return GetFirstAliveEnemyCharacter();
}

TArray<AFRBattleCharacter*> AFRGameMode::GetEnemyCharacters() const
{
	TArray<AFRBattleCharacter*> Result;
	for (AFRBattleCharacter* Enemy : EnemyCharacters)
	{
		if (Enemy)
		{
			Result.Add(Enemy);
		}
	}
	return Result;
}

TArray<FFRRewardChoice> AFRGameMode::GetRewardChoices() const
{
	return RewardChoices;
}

int32 AFRGameMode::GetRewardChoiceCount() const
{
	return RewardChoices.Num();
}

FFRRewardChoice AFRGameMode::GetRewardChoice(int32 ChoiceIndex) const
{
	return RewardChoices.IsValidIndex(ChoiceIndex) ? RewardChoices[ChoiceIndex] : FFRRewardChoice();
}

FText AFRGameMode::GetRewardChoiceSummary(int32 ChoiceIndex) const
{
	return RewardChoices.IsValidIndex(ChoiceIndex) ? GetRewardChoice(ChoiceIndex).GetEffectSummary() : FText::GetEmpty();
}

FText AFRGameMode::GetRewardChoiceConditionFailureSummary(int32 ChoiceIndex) const
{
	return RewardChoices.IsValidIndex(ChoiceIndex) ? GetRewardChoice(ChoiceIndex).GetRewardTagConditionFailureSummary(GetChosenRewardTags()) : FText::GetEmpty();
}

FGameplayTagContainer AFRGameMode::GetChosenRewardTags() const
{
	FGameplayTagContainer ChosenRewardTagContainer;
	for (const FGameplayTag& ChosenRewardTag : ChosenRewardTags)
	{
		if (ChosenRewardTag.IsValid())
		{
			ChosenRewardTagContainer.AddTag(ChosenRewardTag);
		}
	}
	return ChosenRewardTagContainer;
}

FText AFRGameMode::GetStatusText() const
{
	return StatusText;
}

FText AFRGameMode::GetRunProgressSummary() const
{
	return FText::FromString(FString::Printf(
		TEXT("Stage %d/%d | %s"),
		GetCurrentStage(),
		GetMaxStages(),
		*GetStatusText().ToString()));
}

int32 AFRGameMode::GetCurrentStage() const
{
	return CurrentStage;
}

int32 AFRGameMode::GetMaxStages() const
{
	return GetConfiguredStageCount();
}

void AFRGameMode::SpawnMVPBattle()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	ClearBattleStage(PlayerCharacter != nullptr);

	const TSubclassOf<AFRDestructibleTerrain> ActualTerrainClass = TerrainClass ? TerrainClass : TSubclassOf<AFRDestructibleTerrain>(AFRDestructibleTerrain::StaticClass());
	Terrain = World->SpawnActorDeferred<AFRDestructibleTerrain>(ActualTerrainClass, FTransform(FRotator::ZeroRotator, TerrainLocation));
	if (Terrain)
	{
		Terrain->MapDefinition = GetStageTerrainMapDefinition();
		UGameplayStatics::FinishSpawningActor(Terrain, FTransform(FRotator::ZeroRotator, TerrainLocation));
	}

	FVector PlayerLocation = TerrainLocation + PlayerSpawnOffset;
	FVector EnemyLocation = TerrainLocation + EnemySpawnOffset;
	if (Terrain)
	{
		PlayerLocation = Terrain->GetPlayerSpawnWorldLocation();
		EnemyLocation = Terrain->GetEnemySpawnWorldLocation();
	}
	else
	{
		PlayerLocation.Z = TerrainLocation.Z + 520.0f + PlayerSpawnOffset.Z;
		EnemyLocation.Z = TerrainLocation.Z + 520.0f + EnemySpawnOffset.Z;
	}

	if (PlayerCharacter)
	{
		PlayerCharacter->SetActorLocation(PlayerLocation);
		PlayerCharacter->SetTerrain(Terrain);
	}
	else
	{
		const TSubclassOf<AFRBattleCharacter> ActualPlayerClass = PlayerCharacterClass ? PlayerCharacterClass : TSubclassOf<AFRBattleCharacter>(AFRBattleCharacter::StaticClass());
		PlayerCharacter = World->SpawnActorDeferred<AFRBattleCharacter>(ActualPlayerClass, FTransform(FRotator::ZeroRotator, PlayerLocation));
		if (PlayerCharacter)
		{
			PlayerCharacter->CharacterDefinition = PlayerDefinition;
			PlayerCharacter->DefaultLoadoutDefinition = GetDefaultLoadoutDefinition();
			PlayerCharacter->SetTerrain(Terrain);
			UGameplayStatics::FinishSpawningActor(PlayerCharacter, FTransform(FRotator::ZeroRotator, PlayerLocation));
		}
	}

	EnemyCharacters.Reset();
	EnemyCharacter = nullptr;
	UFRCharacterDefinition* DefaultEnemyDefinition = CurrentEnemyDefinition ? CurrentEnemyDefinition.Get() : EnemyDefinition.Get();
	const UFRTerrainMapDefinition* ActiveMapDefinition = Terrain ? Terrain->MapDefinition.Get() : nullptr;
	if (Terrain && ActiveMapDefinition && ActiveMapDefinition->EnemyPlacements.Num() > 0)
	{
		for (const FFREnemyPlacement& EnemyPlacement : ActiveMapDefinition->EnemyPlacements)
		{
			const FVector PlacementLocation = Terrain->ResolveMapLocalSpawnWorldLocation(EnemyPlacement.SpawnLocal, EnemySpawnOffset);
			UFRCharacterDefinition* PlacementDefinition = EnemyPlacement.CharacterDefinition ? EnemyPlacement.CharacterDefinition.Get() : DefaultEnemyDefinition;
			SpawnEnemyCharacter(World, PlacementLocation, PlacementDefinition, true, EnemyPlacement.bUseSpecialAttack);
		}
	}
	else
	{
		SpawnEnemyCharacter(World, EnemyLocation, DefaultEnemyDefinition, false, true);
	}

	if (PlayerCharacter)
	{
		PlayerCharacter->ConfigureAsEnemy(false);
	}
	const TSubclassOf<ACameraActor> ActualCameraClass = CameraClass ? CameraClass : TSubclassOf<ACameraActor>(ACameraActor::StaticClass());
	BattleCamera = World->SpawnActor<ACameraActor>(ActualCameraClass, GetDesiredCameraLocation(), GetBattleCameraRotation());
	if (BattleCamera && BattleCamera->GetCameraComponent())
	{
		BattleCamera->SetActorRotation(GetBattleCameraRotation());
		BattleCamera->GetCameraComponent()->ProjectionMode = ECameraProjectionMode::Orthographic;
		BattleCamera->GetCameraComponent()->OrthoWidth = GetInitialCameraOrthoWidth();
	}

	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0))
	{
		PlayerController->Possess(PlayerCharacter);
		PlayerController->SetViewTarget(BattleCamera);
	}
}

AFRBattleCharacter* AFRGameMode::SpawnEnemyCharacter(UWorld* World, const FVector& SpawnLocation, UFRCharacterDefinition* CharacterDefinition, bool bOverrideSpecialAttack, bool bUseSpecialAttack)
{
	if (!World)
	{
		return nullptr;
	}

	const TSubclassOf<AFRBattleCharacter> ActualEnemyClass = EnemyCharacterClass ? EnemyCharacterClass : TSubclassOf<AFRBattleCharacter>(AFRBattleCharacter::StaticClass());
	AFRBattleCharacter* SpawnedEnemy = World->SpawnActorDeferred<AFRBattleCharacter>(ActualEnemyClass, FTransform(FRotator::ZeroRotator, SpawnLocation));
	if (!SpawnedEnemy)
	{
		return nullptr;
	}

	SpawnedEnemy->CharacterDefinition = CharacterDefinition;
	SpawnedEnemy->DefaultLoadoutDefinition = GetDefaultLoadoutDefinition();
	SpawnedEnemy->SetTerrain(Terrain);
	UGameplayStatics::FinishSpawningActor(SpawnedEnemy, FTransform(FRotator::ZeroRotator, SpawnLocation));
	SpawnedEnemy->ConfigureAsEnemy(true);
	if (bOverrideSpecialAttack)
	{
		SpawnedEnemy->SetSpecialAttackEnabled(bUseSpecialAttack);
	}

	EnemyCharacters.Add(SpawnedEnemy);
	if (!EnemyCharacter)
	{
		EnemyCharacter = SpawnedEnemy;
	}
	return SpawnedEnemy;
}

void AFRGameMode::ClearBattleStage(bool bKeepPlayerCharacter)
{
	for (const TWeakObjectPtr<AFRProjectile>& ActiveProjectile : ActiveProjectiles)
	{
		if (ActiveProjectile.IsValid())
		{
			ActiveProjectile->Destroy();
		}
	}
	ActiveProjectiles.Reset();
	GetWorldTimerManager().ClearTimer(ShotResolutionTimerHandle);
	PendingProjectiles = 0;
	LastShooter.Reset();
	bHoldingImpactCamera = false;

	for (AFRBattleCharacter* Enemy : EnemyCharacters)
	{
		if (Enemy)
		{
			Enemy->Destroy();
		}
	}
	EnemyCharacters.Reset();
	EnemyCharacter = nullptr;
	ActiveEnemyTurnIndex = INDEX_NONE;

	if (Terrain)
	{
		Terrain->Destroy();
		Terrain = nullptr;
	}

	if (BattleCamera)
	{
		BattleCamera->Destroy();
		BattleCamera = nullptr;
	}

	if (!bKeepPlayerCharacter && PlayerCharacter)
	{
		PlayerCharacter->Destroy();
		PlayerCharacter = nullptr;
	}
}

void AFRGameMode::SelectNextEnemyDefinition()
{
	TArray<UFRCharacterDefinition*> Candidates;
	const TArray<TObjectPtr<UFRCharacterDefinition>>* EnemyPool = StageRunDefinition ? &StageRunDefinition->EnemyDefinitionPool : nullptr;
	if (EnemyPool)
	{
		for (UFRCharacterDefinition* Candidate : *EnemyPool)
		{
			if (Candidate && !EncounteredEnemyDefinitions.Contains(Candidate))
			{
				Candidates.Add(Candidate);
			}
		}
	}

	if (Candidates.Num() == 0 && EnemyPool && EnemyPool->Num() > 0)
	{
		EncounteredEnemyDefinitions.Reset();
		for (UFRCharacterDefinition* Candidate : *EnemyPool)
		{
			if (Candidate)
			{
				Candidates.Add(Candidate);
			}
		}
	}

	if (Candidates.Num() > 0)
	{
		CurrentEnemyDefinition = Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
		EncounteredEnemyDefinitions.AddUnique(CurrentEnemyDefinition);
	}
	else
	{
		CurrentEnemyDefinition = EnemyDefinition;
	}
}

UFRTerrainMapDefinition* AFRGameMode::GetStageTerrainMapDefinition() const
{
	if (CurrentEnemyDefinition && CurrentEnemyDefinition->BattleMapDefinition)
	{
		return CurrentEnemyDefinition->BattleMapDefinition;
	}

	if (StageRunDefinition && StageRunDefinition->DefaultTerrainMapDefinition)
	{
		return StageRunDefinition->DefaultTerrainMapDefinition;
	}

	return TerrainMapDefinition;
}

UFRDefaultLoadoutDefinition* AFRGameMode::GetDefaultLoadoutDefinition() const
{
	return StageRunDefinition ? StageRunDefinition->DefaultLoadoutDefinition : nullptr;
}

const FFRStageDifficultyData& AFRGameMode::GetCurrentStageDifficulty() const
{
	if (StageRunDefinition)
	{
		return StageRunDefinition->GetStageDifficulty(CurrentStage);
	}

	static const FFRStageDifficultyData DefaultDifficulty;
	return DefaultDifficulty;
}

int32 AFRGameMode::GetConfiguredStageCount() const
{
	return StageRunDefinition ? FMath::Max(1, StageRunDefinition->StageCount) : 1;
}

AFRBattleCharacter* AFRGameMode::GetFirstAliveEnemyCharacter() const
{
	for (AFRBattleCharacter* Enemy : EnemyCharacters)
	{
		if (Enemy && !Enemy->IsDefeated())
		{
			return Enemy;
		}
	}

	return EnemyCharacter && !EnemyCharacter->IsDefeated() ? EnemyCharacter.Get() : nullptr;
}

AFRBattleCharacter* AFRGameMode::GetActiveEnemyTurnCharacter() const
{
	if (EnemyCharacters.IsValidIndex(ActiveEnemyTurnIndex))
	{
		AFRBattleCharacter* ActiveEnemy = EnemyCharacters[ActiveEnemyTurnIndex];
		if (ActiveEnemy && !ActiveEnemy->IsDefeated())
		{
			return ActiveEnemy;
		}
	}

	return GetFirstAliveEnemyCharacter();
}

bool AFRGameMode::AreAllEnemiesDefeated() const
{
	if (EnemyCharacters.Num() == 0)
	{
		return !EnemyCharacter || EnemyCharacter->IsDefeated();
	}

	for (AFRBattleCharacter* Enemy : EnemyCharacters)
	{
		if (Enemy && !Enemy->IsDefeated())
		{
			return false;
		}
	}
	return true;
}

void AFRGameMode::HandleEnemyDefeated()
{
	ResetShotCameraState();
	if (CurrentStage >= GetConfiguredStageCount())
	{
		BattleState = EFRBattleState::Won;
		SetStatus(TEXT("Run complete"));
		return;
	}

	EnterRewardState();
	if (RewardChoices.Num() <= 0)
	{
		UE_LOG(LogFortRogue, Log, TEXT("No reward choices configured after stage %d."), CurrentStage);
		AdvanceToNextStage();
	}
}

void AFRGameMode::AdvanceToNextStage()
{
	++CurrentStage;
	SelectNextEnemyDefinition();
	SpawnMVPBattle();
	StartPlayerTurn();
}

void AFRGameMode::ApplyRewardToPlayer(const FFRRewardChoice& Reward)
{
	if (!PlayerCharacter)
	{
		return;
	}

	if (Reward.PerkReward)
	{
		PlayerCharacter->ApplyPerkDefinition(Reward.PerkReward);
	}
	if (Reward.WeaponReward)
	{
		PlayerCharacter->AddWeaponDefinition(Reward.WeaponReward);
	}
	if (Reward.ItemReward)
	{
		PlayerCharacter->AddItemDefinition(Reward.ItemReward);
	}
}

void AFRGameMode::StartPlayerTurn()
{
	ResetShotCameraState();
	BattleState = EFRBattleState::PlayerTurn;
	Wind = FMath::RandRange(MinWind, MaxWind);
	if (PlayerCharacter)
	{
		PlayerCharacter->BeginTurn();
	}
	SetStatus(FString::Printf(TEXT("Stage %d/%d - Player turn"), CurrentStage, GetConfiguredStageCount()));
}

void AFRGameMode::StartEnemyTurn()
{
	ResetShotCameraState();
	BattleState = EFRBattleState::EnemyTurn;
	Wind = FMath::RandRange(MinWind, MaxWind);
	ActiveEnemyTurnIndex = INDEX_NONE;
	for (AFRBattleCharacter* Enemy : EnemyCharacters)
	{
		if (Enemy && !Enemy->IsDefeated())
		{
			Enemy->BeginTurn();
		}
	}
	SetStatus(FString::Printf(TEXT("Stage %d/%d - Enemy turn"), CurrentStage, GetConfiguredStageCount()));

	FTimerHandle EnemyTimerHandle;
	GetWorldTimerManager().SetTimer(EnemyTimerHandle, this, &AFRGameMode::RunEnemyTurn, GetCurrentStageDifficulty().EnemyTurnDelaySeconds, false);
}

void AFRGameMode::RunEnemyTurn()
{
	if (BattleState != EFRBattleState::EnemyTurn || !PlayerCharacter)
	{
		return;
	}

	AFRBattleCharacter* ActingEnemy = nullptr;
	for (int32 EnemyIndex = ActiveEnemyTurnIndex + 1; EnemyIndex < EnemyCharacters.Num(); ++EnemyIndex)
	{
		AFRBattleCharacter* Candidate = EnemyCharacters[EnemyIndex];
		if (Candidate && !Candidate->IsDefeated())
		{
			ActiveEnemyTurnIndex = EnemyIndex;
			ActingEnemy = Candidate;
			break;
		}
	}

	if (!ActingEnemy)
	{
		StartPlayerTurn();
		return;
	}

	ActingEnemy->FireAtTarget(PlayerCharacter, GetCurrentStageDifficulty());
	if (!ActingEnemy->SelectSpecialAttack())
	{
		ActingEnemy->SelectBasicAttack();
	}
	const int32 SpawnedProjectiles = ActingEnemy->FireSelectedWeapon();
	if (SpawnedProjectiles <= 0)
	{
		ActingEnemy->EndTurn();
		RunEnemyTurn();
		return;
	}

	NotifyShotFired(ActingEnemy, SpawnedProjectiles);
}

void AFRGameMode::FinishShotResolution()
{
	if (AreAllEnemiesDefeated())
	{
		HandleEnemyDefeated();
		return;
	}

	if (PlayerCharacter && PlayerCharacter->IsDefeated())
	{
		BattleState = EFRBattleState::Lost;
		SetStatus(TEXT("Defeat"));
		return;
	}

	if (LastShooter.IsValid() && LastShooter->IsEnemy())
	{
		BattleState = EFRBattleState::EnemyTurn;
		RunEnemyTurn();
	}
	else
	{
		StartEnemyTurn();
	}
}

void AFRGameMode::EnterRewardState()
{
	ResetShotCameraState();
	BattleState = EFRBattleState::Reward;
	BuildRewardChoices();
	SetStatus(TEXT("Victory - choose a reward"));
}

void AFRGameMode::BuildRewardChoices()
{
	RewardChoices.Reset();
	if (!StageRunDefinition || StageRunDefinition->RewardPool.Num() <= 0)
	{
		return;
	}

	const FGameplayTagContainer ChosenRewardTagContainer = GetChosenRewardTags();
	TArray<FFRRewardChoice> CompatibleRewards;
	TArray<FFRRewardChoice> CandidateRewards;
	for (const FFRRewardChoice& Reward : StageRunDefinition->RewardPool)
	{
		if (!Reward.MeetsRewardTagConditions(ChosenRewardTagContainer))
		{
			continue;
		}

		CompatibleRewards.Add(Reward);
		if (Reward.bOfferOncePerRun && Reward.RewardTag.IsValid() && ChosenRewardTags.Contains(Reward.RewardTag))
		{
			continue;
		}

		CandidateRewards.Add(Reward);
	}
	if (CandidateRewards.Num() <= 0)
	{
		CandidateRewards = CompatibleRewards;
	}
	if (CandidateRewards.Num() <= 0)
	{
		return;
	}

	const int32 MaxAvailableChoices = FMath::Min(MaxRewardChoiceCount, CandidateRewards.Num());
	const int32 ChoiceCount = FMath::Clamp(StageRunDefinition->RewardChoiceCount, 1, MaxAvailableChoices);
	for (int32 ChoiceIndex = 0; ChoiceIndex < ChoiceCount; ++ChoiceIndex)
	{
		float TotalWeight = 0.0f;
		for (const FFRRewardChoice& CandidateReward : CandidateRewards)
		{
			TotalWeight += FMath::Max(0.0f, CandidateReward.RewardWeight);
		}

		int32 CandidateIndex = FMath::RandRange(0, CandidateRewards.Num() - 1);
		if (TotalWeight > KINDA_SMALL_NUMBER)
		{
			float WeightRoll = FMath::FRandRange(0.0f, TotalWeight);
			for (int32 Index = 0; Index < CandidateRewards.Num(); ++Index)
			{
				const float CandidateWeight = FMath::Max(0.0f, CandidateRewards[Index].RewardWeight);
				if (CandidateWeight <= 0.0f)
				{
					continue;
				}

				WeightRoll -= CandidateWeight;
				if (WeightRoll <= 0.0f)
				{
					CandidateIndex = Index;
					break;
				}
			}
		}

		RewardChoices.Add(CandidateRewards[CandidateIndex]);
		CandidateRewards.RemoveAtSwap(CandidateIndex, 1, EAllowShrinking::No);
	}
}

void AFRGameMode::CheckTurnDefeatState()
{
	if (BattleState != EFRBattleState::PlayerTurn && BattleState != EFRBattleState::EnemyTurn)
	{
		return;
	}

	if (AreAllEnemiesDefeated())
	{
		HandleEnemyDefeated();
		return;
	}

	if (PlayerCharacter && PlayerCharacter->IsDefeated())
	{
		BattleState = EFRBattleState::Lost;
		SetStatus(TEXT("Defeat"));
	}
}

void AFRGameMode::SetStatus(const FString& NewStatus)
{
	StatusText = FText::FromString(NewStatus);
}

void AFRGameMode::UpdateBattleCamera(float DeltaSeconds)
{
	if (!BattleCamera)
	{
		return;
	}

	const FVector CurrentLocation = BattleCamera->GetActorLocation();
	const FVector DesiredLocation = GetDesiredCameraLocation();
	const FVector NewLocation = FMath::VInterpTo(CurrentLocation, DesiredLocation, DeltaSeconds, CameraFollowInterpSpeed);
	BattleCamera->SetActorLocation(NewLocation);
	BattleCamera->SetActorRotation(GetBattleCameraRotation());
}

void AFRGameMode::ResetShotCameraState()
{
	ActiveProjectiles.Reset();
	bHoldingImpactCamera = false;
	GetWorldTimerManager().ClearTimer(ShotResolutionTimerHandle);
}

float AFRGameMode::GetInitialCameraOrthoWidth() const
{
	if (!Terrain)
	{
		return CameraOrthoWidth;
	}

	const float WidthFit = Terrain->Width + TerrainCameraPadding;
	const float HeightFit = Terrain->Height * ExpectedWideViewportAspectRatio + TerrainCameraPadding;
	return FMath::Max(MinimumTerrainCameraOrthoWidth, FMath::Max(WidthFit, HeightFit));
}

FRotator AFRGameMode::GetBattleCameraRotation() const
{
	return BattleCameraRotation;
}

FVector AFRGameMode::GetDesiredCameraLocation() const
{
	FVector DesiredLocation = CameraLocation;
	if (BattleCamera)
	{
		DesiredLocation = BattleCamera->GetActorLocation();
		DesiredLocation.Y = CameraLocation.Y;
	}

	const FVector FocusLocation = GetCameraFocusLocation();
	DesiredLocation.X = FocusLocation.X;
	DesiredLocation.Z = FocusLocation.Z;
	return ClampCameraLocationToTerrainBounds(DesiredLocation);
}

FVector AFRGameMode::GetCameraFocusLocation() const
{
	if (BattleState == EFRBattleState::ResolvingShot)
	{
		for (const TWeakObjectPtr<AFRProjectile>& Projectile : ActiveProjectiles)
		{
			if (Projectile.IsValid())
			{
				const FVector ProjectileLocation = Projectile->GetActorLocation();
				return FVector(ProjectileLocation.X, CameraLocation.Y, ProjectileLocation.Z + CameraProjectileZOffset);
			}
		}

		if (bHoldingImpactCamera)
		{
			return FVector(LastImpactCameraLocation.X, CameraLocation.Y, LastImpactCameraLocation.Z + CameraProjectileZOffset);
		}
	}

	const AFRBattleCharacter* FocusCharacter = nullptr;
	if (BattleState == EFRBattleState::EnemyTurn)
	{
		FocusCharacter = GetActiveEnemyTurnCharacter();
	}
	else
	{
		FocusCharacter = PlayerCharacter;
	}

	if (FocusCharacter)
	{
		const FVector CharacterLocation = FocusCharacter->GetActorLocation();
		return FVector(CharacterLocation.X, CameraLocation.Y, CharacterLocation.Z + CameraTurnZOffset);
	}

	return CameraLocation;
}

FVector AFRGameMode::ClampCameraLocationToTerrainBounds(const FVector& DesiredLocation) const
{
	if (!Terrain)
	{
		return DesiredLocation;
	}

	float OrthoWidth = CameraOrthoWidth;
	if (BattleCamera && BattleCamera->GetCameraComponent())
	{
		OrthoWidth = BattleCamera->GetCameraComponent()->OrthoWidth;
	}
	else
	{
		OrthoWidth = GetInitialCameraOrthoWidth();
	}

	const float ViewHalfWidth = OrthoWidth * 0.5f;
	const float TerrainHalfWidth = Terrain->Width * 0.5f;
	const float PaddingHalf = TerrainCameraPadding * 0.5f;
	const float TerrainCenterX = Terrain->GetActorLocation().X;

	FVector ClampedLocation = DesiredLocation;
	if (ViewHalfWidth >= TerrainHalfWidth + PaddingHalf)
	{
		ClampedLocation.X = TerrainCenterX;
	}
	else
	{
		const float MinCameraX = TerrainCenterX - TerrainHalfWidth - PaddingHalf + ViewHalfWidth;
		const float MaxCameraX = TerrainCenterX + TerrainHalfWidth + PaddingHalf - ViewHalfWidth;
		ClampedLocation.X = FMath::Clamp(static_cast<float>(DesiredLocation.X), MinCameraX, MaxCameraX);
	}

	return ClampedLocation;
}
