// Copyright Epic Games, Inc. All Rights Reserved.

#include "FRGameMode.h"

#include "Characters/FRCharacterDefinition.h"
#include "Combat/FRBattleCamera.h"
#include "Combat/FRBattleCharacter.h"
#include "Combat/FRDestructibleTerrain.h"
#include "Combat/FRProjectile.h"
#include "Combat/FRTerrainMapDefinition.h"
#include "FortRogue.h"
#include "FRGameplayTags.h"
#include "Game/FRGameFlowSubsystem.h"
#include "Game/FRGameModeDataAsset.h"
#include "Game/FRRunSubsystem.h"
#include "FRPlayerController.h"
#include "Items/FRItemDefinition.h"
#include "Perks/FRPerkDefinition.h"
#include "Rewards/FRRewardGrant.h"
#include "Run/FRDefaultLoadoutDefinition.h"
#include "Run/FRStageRunDefinition.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

AFRGameMode::AFRGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	PlayerControllerClass = AFRPlayerController::StaticClass();
	DefaultPawnClass = nullptr;
	PlayerCharacterClass = AFRBattleCharacter::StaticClass();
	EnemyCharacterClass = AFRBattleCharacter::StaticClass();
	TerrainClass = AFRDestructibleTerrain::StaticClass();
	CameraClass = AFRBattleCamera::StaticClass();
	GameStateClass = AFRTurnBasedGameState::StaticClass();
}

void AFRGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	// 레벨/설정이 다른 클래스를 지정해도 FortRogue 흐름에 필요한 클래스를 보장합니다.
	PlayerControllerClass = AFRPlayerController::StaticClass();
	DefaultPawnClass = nullptr;
	GameStateClass = AFRTurnBasedGameState::StaticClass();
	Super::InitGame(MapName, Options, ErrorMessage);
}

void AFRGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (AFRTurnBasedGameState* TurnState = GetTurnBasedGameState())
	{
		TurnState->ResetTurnState();
	}

	// 자동화 테스트는 게임 플로우(메뉴 진입/레벨 이동) 없이 테스트가 구성한 데이터로 바로 전투를 시작합니다.
	if (!GIsAutomationTesting)
	{
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (UFRGameFlowSubsystem* GameFlow = GameInstance->GetSubsystem<UFRGameFlowSubsystem>())
			{
				GameFlow->EnsureStartupMode();
				if (const UFRGameModeDataAsset* ModeData = GameFlow->GetCurrentModeData())
				{
					EnterGameFlowMode(ModeData);
					return;
				}
			}
		}
	}

	StartBattleRun();
}

void AFRGameMode::EnterGameFlowMode(const UFRGameModeDataAsset* ModeData)
{
	ApplyGameModeData(ModeData);

	if (!ModeData || ModeData->bStartBattleOnEnter)
	{
		StartBattleRun();
		return;
	}

	if (bBattleStarted)
	{
		ClearBattleStage(false);
		bBattleStarted = false;
		if (UFRRunSubsystem* RunSubsystem = GetRunSubsystem())
		{
			RunSubsystem->EndRun();
		}
	}

	const FString Status = ModeData->DisplayName.IsEmpty()
		? FString(TEXT("Main menu"))
		: ModeData->DisplayName.ToString();
	SetStatus(Status);
}

void AFRGameMode::StartBattleRun()
{
	if (bBattleStarted)
	{
		return;
	}

	bBattleStarted = true;
	if (UFRRunSubsystem* RunSubsystem = GetRunSubsystem())
	{
		RunSubsystem->StartRun();
		// 캐릭터 선택 화면에서 고른 캐릭터가 있으면 모드 데이터 기본값보다 우선합니다.
		if (UFRCharacterDefinition* SelectedDefinition = RunSubsystem->GetPendingPlayerDefinition())
		{
			PlayerDefinition = SelectedDefinition;
		}
	}
	if (StageRunDefinition)
	{
		StageRunDefinition->NormalizeStageData();
	}
	SelectNextEnemyDefinition();
	SpawnMVPBattle();
	StartPlayerTurn();
}

void AFRGameMode::ApplyGameModeData(const UFRGameModeDataAsset* ModeData)
{
	if (!ModeData || !ModeData->bStartBattleOnEnter)
	{
		return;
	}

	if (TSubclassOf<AFRBattleCharacter> LoadedPlayerClass = ModeData->LoadPlayerCharacterClass())
	{
		PlayerCharacterClass = LoadedPlayerClass;
	}
	if (TSubclassOf<AFRBattleCharacter> LoadedEnemyClass = ModeData->LoadEnemyCharacterClass())
	{
		EnemyCharacterClass = LoadedEnemyClass;
	}
	if (TSubclassOf<AFRDestructibleTerrain> LoadedTerrainClass = ModeData->LoadTerrainClass())
	{
		TerrainClass = LoadedTerrainClass;
	}
	if (TSubclassOf<AFRBattleCamera> LoadedCameraClass = ModeData->LoadCameraClass())
	{
		CameraClass = LoadedCameraClass;
	}
	if (ModeData->PlayerDefinition)
	{
		PlayerDefinition = ModeData->PlayerDefinition;
	}
	if (ModeData->StageRunDefinition)
	{
		StageRunDefinition = ModeData->StageRunDefinition;
	}
	if (ModeData->TerrainMapDefinition)
	{
		TerrainMapDefinition = ModeData->TerrainMapDefinition;
	}

	TerrainLocation = ModeData->TerrainLocation;
	PlayerSpawnOffset = ModeData->PlayerSpawnOffset;
	EnemySpawnOffset = ModeData->EnemySpawnOffset;
	CameraLocation = ModeData->CameraLocation;
	CameraOrthoWidth = ModeData->CameraOrthoWidth;
	CameraFollowInterpSpeed = ModeData->CameraFollowInterpSpeed;
	CameraManualPanSpeed = ModeData->CameraManualPanSpeed;
	CameraProjectileZOffset = ModeData->CameraProjectileZOffset;
	CameraTurnZOffset = ModeData->CameraTurnZOffset;
	ShotImpactCameraHoldSeconds = ModeData->ShotImpactCameraHoldSeconds;
	MinWind = ModeData->MinWind;
	MaxWind = ModeData->MaxWind;
}

void AFRGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bBattleStarted)
	{
		CheckTurnDefeatState();
	}
}

UFRRunSubsystem* AFRGameMode::GetRunSubsystem() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UFRRunSubsystem>() : nullptr;
}

float AFRGameMode::RollWind()
{
	if (UFRRunSubsystem* RunSubsystem = GetRunSubsystem())
	{
		return RunSubsystem->RandRange(MinWind, MaxWind);
	}
	return FMath::RandRange(MinWind, MaxWind);
}

void AFRGameMode::NotifyProjectileSpawned(AFRProjectile* Projectile, bool bIncreasePendingProjectileCount)
{
	if (Projectile)
	{
		ActiveProjectiles.Add(Projectile);
		RequestAutoCameraFocus(Projectile, Projectile->GetActorLocation(), CameraProjectileZOffset);
		if (AFRTurnBasedGameState* TurnState = GetTurnBasedGameState())
		{
			TurnState->RegisterProjectileSpawned(bIncreasePendingProjectileCount);
		}
	}
}

void AFRGameMode::NotifyShotFired(AFRBattleCharacter* Shooter, int32 ProjectileCount)
{
	if (!Shooter || ProjectileCount <= 0)
	{
		return;
	}

	if (AFRTurnBasedGameState* TurnState = GetTurnBasedGameState())
	{
		TurnState->EnterShotResolution(
			Shooter,
			ProjectileCount,
			FText::FromString(Shooter->IsEnemy() ? TEXT("Enemy shell incoming") : TEXT("Shell fired")));
	}
}

void AFRGameMode::NotifyProjectileSpawnFailed(int32 ProjectileCount)
{
	if (ProjectileCount <= 0)
	{
		return;
	}

	AFRTurnBasedGameState* TurnState = GetTurnBasedGameState();
	if (!TurnState || TurnState->GetBattleState() != EFRBattleState::ResolvingShot)
	{
		return;
	}

	if (TurnState->RegisterProjectileSpawnFailed(ProjectileCount) <= 0)
	{
		FinishShotResolution();
	}
}

void AFRGameMode::NotifyProjectileResolved(AFRProjectile* Projectile)
{
	if (Projectile)
	{
		ActiveProjectiles.RemoveAll([Projectile](const TWeakObjectPtr<AFRProjectile>& ActiveProjectile)
		{
			return !ActiveProjectile.IsValid() || ActiveProjectile.Get() == Projectile;
		});
	}

	AFRTurnBasedGameState* TurnState = GetTurnBasedGameState();
	if (!TurnState || TurnState->GetBattleState() != EFRBattleState::ResolvingShot)
	{
		return;
	}

	if (Projectile)
	{
		RequestAutoCameraFocus(nullptr, Projectile->GetActorLocation(), CameraProjectileZOffset);
	}

	if (TurnState->RegisterProjectileResolved() == 0)
	{
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
	if (UFRRunSubsystem* RunSubsystem = GetRunSubsystem())
	{
		RunSubsystem->RecordChosenRewardTag(Reward.GetResolvedRewardTag());
	}

	SetStatus(FString::Printf(TEXT("Reward chosen: %s"), *Reward.GetResolvedDisplayName().ToString()));
	RewardChoices.Reset();
	AdvanceToNextStage();
}

bool AFRGameMode::CanApplyRewardChoice(int32 ChoiceIndex) const
{
	return GetBattleState() == EFRBattleState::Reward
		&& PlayerCharacter
		&& RewardChoices.IsValidIndex(ChoiceIndex)
		&& RewardChoices[ChoiceIndex].MeetsRewardTagConditions(GetChosenRewardTags());
}

float AFRGameMode::GetWind() const
{
	return GetTurnBasedGameState() ? GetTurnBasedGameState()->GetWind() : 0.0f;
}

FText AFRGameMode::GetWindSummary() const
{
	const float CurrentWind = GetWind();
	return FText::FromString(FMath::IsNearlyZero(CurrentWind)
		? FString(TEXT("Wind 0"))
		: FString::Printf(TEXT("Wind %+.0f"), CurrentWind));
}

AFRTurnBasedGameState* AFRGameMode::GetTurnBasedGameState() const
{
	return GetWorld() ? GetWorld()->GetGameState<AFRTurnBasedGameState>() : nullptr;
}

EFRBattleState AFRGameMode::GetBattleState() const
{
	return GetTurnBasedGameState() ? GetTurnBasedGameState()->GetBattleState() : EFRBattleState::PlayerTurn;
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

AFRBattleCamera* AFRGameMode::GetBattleCamera() const
{
	return BattleCamera;
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
	if (const UFRRunSubsystem* RunSubsystem = GetRunSubsystem())
	{
		return RunSubsystem->GetChosenRewardTags();
	}
	return FGameplayTagContainer();
}

FText AFRGameMode::GetStatusText() const
{
	return GetTurnBasedGameState() ? GetTurnBasedGameState()->GetStatusText() : FText::GetEmpty();
}

FText AFRGameMode::GetRunProgressSummary() const
{
	return FText::FromString(FString::Printf(
		TEXT("Stage %d/%d"),
		GetCurrentStage(),
		GetMaxStages()));
}

int32 AFRGameMode::GetCurrentStage() const
{
	const UFRRunSubsystem* RunSubsystem = GetRunSubsystem();
	return RunSubsystem ? RunSubsystem->GetCurrentStage() : 1;
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
	UFRCharacterDefinition* DefaultEnemyDefinition = CurrentEnemyDefinition.Get();
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

	const TSubclassOf<AFRBattleCamera> ActualCameraClass = CameraClass ? CameraClass : TSubclassOf<AFRBattleCamera>(AFRBattleCamera::StaticClass());
	BattleCamera = World->SpawnActor<AFRBattleCamera>(ActualCameraClass, CameraLocation, AFRBattleCamera::GetBattleRotation());
	if (BattleCamera)
	{
		BattleCamera->ConfigureBattle(CameraLocation, CameraOrthoWidth, CameraFollowInterpSpeed, CameraManualPanSpeed);
		BattleCamera->SetTerrainActor(Terrain);
	}
	RequestAutoCameraFocus(PlayerCharacter, PlayerCharacter ? PlayerCharacter->GetActorLocation() : CameraLocation, CameraTurnZOffset);

	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0))
	{
		PlayerController->Possess(PlayerCharacter);
		PlayerController->SetViewTarget(BattleCamera);
	}
}

AFRBattleCharacter* AFRGameMode::SpawnEnemyCharacter(UWorld* World, const FVector& SpawnLocation, UFRCharacterDefinition* CharacterDefinition, bool bOverrideSpecialAttack, bool bUseSpecialAttack)
{
	if (!World || !CharacterDefinition)
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
	if (AFRTurnBasedGameState* TurnState = GetTurnBasedGameState())
	{
		TurnState->ResetShotResolution();
	}
	for (AFRBattleCharacter* Enemy : EnemyCharacters)
	{
		if (Enemy)
		{
			Enemy->Destroy();
		}
	}
	EnemyCharacters.Reset();

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
	if (!StageRunDefinition)
	{
		CurrentEnemyDefinition = nullptr;
		return;
	}

	if (UFRRunSubsystem* RunSubsystem = GetRunSubsystem())
	{
		CurrentEnemyDefinition = RunSubsystem->PickNextEnemyDefinition(StageRunDefinition->EnemyDefinitionPool);
		return;
	}

	// 게임 인스턴스가 없는 환경(예: 일부 테스트)에서는 풀의 첫 항목을 사용합니다.
	CurrentEnemyDefinition = StageRunDefinition->EnemyDefinitionPool.Num() > 0 ? StageRunDefinition->EnemyDefinitionPool[0].Get() : nullptr;
}

UFRTerrainMapDefinition* AFRGameMode::GetStageTerrainMapDefinition() const
{
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
		return StageRunDefinition->GetStageDifficulty(GetCurrentStage());
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

	return nullptr;
}

AFRBattleCharacter* AFRGameMode::GetActiveEnemyTurnCharacter() const
{
	return GetTurnBasedGameState() ? GetTurnBasedGameState()->GetActiveEnemyTurnCharacter(GetEnemyCharacters()) : GetFirstAliveEnemyCharacter();
}

bool AFRGameMode::AreAllEnemiesDefeated() const
{
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
	if (GetCurrentStage() >= GetConfiguredStageCount())
	{
		if (AFRTurnBasedGameState* TurnState = GetTurnBasedGameState())
		{
			TurnState->EnterWonState(FText::FromString(TEXT("Run complete")));
		}
		if (UFRRunSubsystem* RunSubsystem = GetRunSubsystem())
		{
			RunSubsystem->EndRun();
		}
		return;
	}

	EnterRewardState();
	if (RewardChoices.Num() <= 0)
	{
		UE_LOG(LogFortRogue, Log, TEXT("No reward choices configured after stage %d."), GetCurrentStage());
		AdvanceToNextStage();
	}
}

void AFRGameMode::AdvanceToNextStage()
{
	if (UFRRunSubsystem* RunSubsystem = GetRunSubsystem())
	{
		RunSubsystem->AdvanceStage();
	}
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

	Reward.ApplyToCharacter(*PlayerCharacter);

	if (UFRRunSubsystem* RunSubsystem = GetRunSubsystem())
	{
		for (const UFRRewardGrant* Grant : Reward.Grants)
		{
			if (const UFRRewardGrant_Perk* PerkGrant = Cast<UFRRewardGrant_Perk>(Grant))
			{
				RunSubsystem->RecordAcquiredPerk(PerkGrant->PerkDefinition);
			}
		}
	}
}

void AFRGameMode::StartPlayerTurn()
{
	ResetShotCameraState();
	if (AFRTurnBasedGameState* TurnState = GetTurnBasedGameState())
	{
		TurnState->StartPlayerTurn(PlayerCharacter, RollWind(), FText::FromString(TEXT("Player turn")));
	}
	RequestAutoCameraFocus(PlayerCharacter, PlayerCharacter ? PlayerCharacter->GetActorLocation() : CameraLocation, CameraTurnZOffset);
}

void AFRGameMode::StartEnemyTurn()
{
	ResetShotCameraState();
	if (AFRTurnBasedGameState* TurnState = GetTurnBasedGameState())
	{
		TurnState->StartEnemyTurn(GetEnemyCharacters(), RollWind(), FText::FromString(TEXT("Enemy turn")));
	}
	AFRBattleCharacter* FocusEnemy = GetActiveEnemyTurnCharacter();
	RequestAutoCameraFocus(FocusEnemy, FocusEnemy ? FocusEnemy->GetActorLocation() : CameraLocation, CameraTurnZOffset);

	FTimerHandle EnemyTimerHandle;
	GetWorldTimerManager().SetTimer(EnemyTimerHandle, this, &AFRGameMode::RunEnemyTurn, GetCurrentStageDifficulty().EnemyTurnDelaySeconds, false);
}

void AFRGameMode::RunEnemyTurn()
{
	AFRTurnBasedGameState* TurnState = GetTurnBasedGameState();
	if (!TurnState || TurnState->GetBattleState() != EFRBattleState::EnemyTurn || !PlayerCharacter)
	{
		return;
	}

	AFRBattleCharacter* ActingEnemy = TurnState->AdvanceToNextEnemyTurnCharacter(GetEnemyCharacters());
	if (!ActingEnemy)
	{
		StartPlayerTurn();
		return;
	}
	RequestAutoCameraFocus(ActingEnemy, ActingEnemy->GetActorLocation(), CameraTurnZOffset);

	if (!ActingEnemy->SelectSpecialAttack())
	{
		ActingEnemy->SelectBasicAttack();
	}
	ActingEnemy->FireAtTarget(PlayerCharacter, GetCurrentStageDifficulty());
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
	AFRTurnBasedGameState* TurnState = GetTurnBasedGameState();
	if (!TurnState)
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
		TurnState->EnterLostState(FText::FromString(TEXT("Defeat")));
		if (UFRRunSubsystem* RunSubsystem = GetRunSubsystem())
		{
			RunSubsystem->EndRun();
		}
		return;
	}

	if (AFRBattleCharacter* LastShooter = TurnState->GetLastShooter())
	{
		if (LastShooter->IsEnemy())
		{
			TurnState->EnterEnemyTurnContinuation();
			RunEnemyTurn();
			return;
		}
	}

	StartEnemyTurn();
}

void AFRGameMode::EnterRewardState()
{
	ResetShotCameraState();
	if (AFRTurnBasedGameState* TurnState = GetTurnBasedGameState())
	{
		TurnState->EnterRewardState(FText::FromString(TEXT("Victory - choose a reward")));
	}
	BuildRewardChoices();
}

void AFRGameMode::BuildRewardChoices()
{
	RewardChoices.Reset();
	if (UFRRunSubsystem* RunSubsystem = GetRunSubsystem())
	{
		RewardChoices = RunSubsystem->BuildRewardChoices(StageRunDefinition, PlayerDefinition);
	}
}

void AFRGameMode::CheckTurnDefeatState()
{
	AFRTurnBasedGameState* TurnState = GetTurnBasedGameState();
	if (!TurnState || !TurnState->CanCheckTurnDefeatState())
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
		TurnState->EnterLostState(FText::FromString(TEXT("Defeat")));
		if (UFRRunSubsystem* RunSubsystem = GetRunSubsystem())
		{
			RunSubsystem->EndRun();
		}
	}
}

void AFRGameMode::SetStatus(const FString& NewStatus)
{
	if (AFRTurnBasedGameState* TurnState = GetTurnBasedGameState())
	{
		TurnState->SetStatusText(FText::FromString(NewStatus));
	}
}

void AFRGameMode::HandleManualCameraInput(const FVector2D& InputAxis, bool bAnyDirectionKeyDown, float DeltaSeconds)
{
	if (BattleCamera)
	{
		BattleCamera->HandleManualInput(InputAxis, bAnyDirectionKeyDown, DeltaSeconds);
	}
}

void AFRGameMode::ResetShotCameraState()
{
	ActiveProjectiles.Reset();
	GetWorldTimerManager().ClearTimer(ShotResolutionTimerHandle);
	if (AFRTurnBasedGameState* TurnState = GetTurnBasedGameState())
	{
		TurnState->ResetShotResolution();
	}
}

void AFRGameMode::RequestAutoCameraFocus(AActor* FocusActor, const FVector& FocusLocation, float ZOffset)
{
	if (BattleCamera)
	{
		BattleCamera->RequestAutoFocus(FocusActor, FocusLocation, ZOffset);
	}
}
