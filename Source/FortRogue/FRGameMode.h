// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Game/FRTurnBasedGameState.h"
#include "GameFramework/GameModeBase.h"
#include "Rewards/FRRewardTypes.h"
#include "TimerManager.h"
#include "FRGameMode.generated.h"

class AFRBattleCharacter;
class AFRDestructibleTerrain;
class AFRProjectile;
class ACameraActor;
class UFRCharacterDefinition;
class UFRDefaultLoadoutDefinition;
class UFRStageRunDefinition;
class UFRTerrainMapDefinition;
struct FFRStageDifficultyData;


UCLASS()
class FORTROGUE_API AFRGameMode : public AGameModeBase
{
	GENERATED_BODY()

#if WITH_DEV_AUTOMATION_TESTS
	friend class FFRGameModeEnemyTurnContinuationTest;
	friend class FFRTerrainGameModeMapDefinitionTest;
#endif

public:
	AFRGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Battle")
	void NotifyProjectileSpawned(AFRProjectile* Projectile, bool bIncreasePendingProjectileCount = true);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Battle")
	void NotifyShotFired(AFRBattleCharacter* Shooter, int32 ProjectileCount);

	void NotifyProjectileSpawnFailed(int32 ProjectileCount);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Battle")
	void NotifyProjectileResolved(AFRProjectile* Projectile);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Rewards")
	void ApplyRewardChoice(int32 ChoiceIndex);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	bool CanApplyRewardChoice(int32 ChoiceIndex) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Battle")
	float GetWind() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Battle")
	FText GetWindSummary() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Battle")
	AFRTurnBasedGameState* GetTurnBasedGameState() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Battle")
	EFRBattleState GetBattleState() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Battle")
	AFRBattleCharacter* GetPlayerCharacter() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Battle")
	AFRBattleCharacter* GetEnemyCharacter() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Battle")
	TArray<AFRBattleCharacter*> GetEnemyCharacters() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	TArray<FFRRewardChoice> GetRewardChoices() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	int32 GetRewardChoiceCount() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	FFRRewardChoice GetRewardChoice(int32 ChoiceIndex) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	FText GetRewardChoiceSummary(int32 ChoiceIndex) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	FText GetRewardChoiceConditionFailureSummary(int32 ChoiceIndex) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	FGameplayTagContainer GetChosenRewardTags() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Battle")
	FText GetStatusText() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Run")
	FText GetRunProgressSummary() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Run")
	int32 GetCurrentStage() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Run")
	int32 GetMaxStages() const;

private:
	void SpawnMVPBattle();
	void ClearBattleStage(bool bKeepPlayerCharacter);
	void SelectNextEnemyDefinition();
	UFRTerrainMapDefinition* GetStageTerrainMapDefinition() const;
	UFRDefaultLoadoutDefinition* GetDefaultLoadoutDefinition() const;
	const FFRStageDifficultyData& GetCurrentStageDifficulty() const;
	int32 GetConfiguredStageCount() const;
	void HandleEnemyDefeated();
	void AdvanceToNextStage();
	void ApplyRewardToPlayer(const FFRRewardChoice& Reward);
	void StartPlayerTurn();
	void StartEnemyTurn();
	void RunEnemyTurn();
	void FinishShotResolution();
	void EnterRewardState();
	void BuildRewardChoices();
	void CheckTurnDefeatState();
	AFRBattleCharacter* SpawnEnemyCharacter(UWorld* World, const FVector& SpawnLocation, UFRCharacterDefinition* CharacterDefinition, bool bOverrideSpecialAttack, bool bUseSpecialAttack);
	AFRBattleCharacter* GetFirstAliveEnemyCharacter() const;
	AFRBattleCharacter* GetActiveEnemyTurnCharacter() const;
	bool AreAllEnemiesDefeated() const;
	void SetStatus(const FString& NewStatus);
	void UpdateBattleCamera(float DeltaSeconds);
	void ResetShotCameraState();
	float GetInitialCameraOrthoWidth() const;
	FRotator GetBattleCameraRotation() const;
	FVector GetDesiredCameraLocation() const;
	FVector GetCameraFocusLocation() const;
	FVector ClampCameraLocationToTerrainBounds(const FVector& DesiredLocation) const;

	UPROPERTY()
	TObjectPtr<AFRBattleCharacter> PlayerCharacter;

	UPROPERTY()
	TObjectPtr<AFRBattleCharacter> EnemyCharacter;

	UPROPERTY()
	TArray<TObjectPtr<AFRBattleCharacter>> EnemyCharacters;

	UPROPERTY()
	TObjectPtr<AFRDestructibleTerrain> Terrain;

	UPROPERTY()
	TObjectPtr<ACameraActor> BattleCamera;

	UPROPERTY()
	TArray<FFRRewardChoice> RewardChoices;

	UPROPERTY()
	TArray<TWeakObjectPtr<AFRProjectile>> ActiveProjectiles;

	UPROPERTY()
	TObjectPtr<UFRCharacterDefinition> CurrentEnemyDefinition;

	UPROPERTY()
	TArray<TObjectPtr<UFRCharacterDefinition>> EncounteredEnemyDefinitions;

	UPROPERTY()
	TArray<FGameplayTag> ChosenRewardTags;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup", meta = (ToolTip = "전투 시작 시 스폰할 플레이어 캐릭터 액터 클래스입니다."))
	TSubclassOf<AFRBattleCharacter> PlayerCharacterClass;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup", meta = (ToolTip = "전투 시작 시 스폰할 적 캐릭터 액터 클래스입니다."))
	TSubclassOf<AFRBattleCharacter> EnemyCharacterClass;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup", meta = (ToolTip = "스테이지 지형으로 스폰할 파괴 가능한 지형 액터 클래스입니다."))
	TSubclassOf<AFRDestructibleTerrain> TerrainClass;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup", meta = (ToolTip = "StageRunDefinition에 맵이 없을 때 사용할 기본 지형 맵 데이터입니다."))
	TObjectPtr<UFRTerrainMapDefinition> TerrainMapDefinition;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup", meta = (ToolTip = "전투 카메라로 스폰할 CameraActor 클래스입니다."))
	TSubclassOf<ACameraActor> CameraClass;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup", meta = (ToolTip = "플레이어에게 적용할 캐릭터 데이터입니다. 비워두면 캐릭터 클래스 기본값을 사용합니다."))
	TObjectPtr<UFRCharacterDefinition> PlayerDefinition;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Run", meta = (ToolTip = "스테이지 수, 적 풀, 보상 풀, 기본 로드아웃을 정의하는 런 데이터입니다."))
	TObjectPtr<UFRStageRunDefinition> StageRunDefinition;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup", meta = (ToolTip = "지형 액터를 스폰할 월드 위치입니다."))
	FVector TerrainLocation = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup", meta = (ToolTip = "지형 맵 데이터가 없을 때 TerrainLocation 기준으로 사용할 플레이어 스폰 오프셋입니다."))
	FVector PlayerSpawnOffset = FVector(-650.0f, 0.0f, 95.0f);

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup", meta = (ToolTip = "지형 맵 데이터가 없을 때 TerrainLocation 기준으로 사용할 적 스폰 오프셋입니다."))
	FVector EnemySpawnOffset = FVector(650.0f, 0.0f, 95.0f);

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup", meta = (ToolTip = "전투 카메라의 기본 월드 위치입니다. 카메라 추적이 시작되기 전 기준 위치로 사용됩니다."))
	FVector CameraLocation = FVector(0.0f, 3000.0f, 860.0f);

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup", meta = (ToolTip = "전투 카메라의 기본 직교 폭입니다. 값이 클수록 더 넓은 전장을 보여줍니다."))
	float CameraOrthoWidth = 2700.0f;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup", meta = (ToolTip = "카메라가 목표 위치로 따라가는 보간 속도입니다. 0이면 거의 따라가지 않고, 값이 클수록 빠르게 따라갑니다."))
	float CameraFollowInterpSpeed = 4.5f;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup", meta = (ToolTip = "투사체를 따라갈 때 카메라가 투사체보다 위를 보도록 더하는 Z 오프셋입니다."))
	float CameraProjectileZOffset = 120.0f;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup", meta = (ToolTip = "턴 시작 시 현재 캐릭터보다 위를 보도록 더하는 Z 오프셋입니다."))
	float CameraTurnZOffset = 220.0f;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup", meta = (ToolTip = "충돌 직후 카메라가 impact 위치를 유지하는 시간입니다. 0이면 바로 다음 상태로 넘어갑니다."))
	float ShotImpactCameraHoldSeconds = 0.65f;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup", meta = (ToolTip = "각 스테이지에서 무작위로 뽑을 최소 바람 값입니다. 음수는 왼쪽 방향 바람입니다."))
	float MinWind = -180.0f;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup", meta = (ToolTip = "각 스테이지에서 무작위로 뽑을 최대 바람 값입니다. 양수는 오른쪽 방향 바람입니다."))
	float MaxWind = 180.0f;

	int32 CurrentStage = 1;
	FVector LastImpactCameraLocation = FVector::ZeroVector;
	bool bHoldingImpactCamera = false;
	FTimerHandle ShotResolutionTimerHandle;
};
