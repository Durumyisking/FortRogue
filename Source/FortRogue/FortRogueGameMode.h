// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Rewards/FortRogueRewardTypes.h"
#include "TimerManager.h"
#include "FortRogueGameMode.generated.h"

class AFortRogueBattleCharacter;
class AFortRogueDestructibleTerrain;
class AFortRogueProjectile;
class ACameraActor;
class UFortRogueCharacterDefinition;
class UFortRogueDefaultLoadoutDefinition;
class UFortRogueStageRunDefinition;
class UFortRogueTerrainMapDefinition;
struct FFortRogueStageDifficultyData;

UENUM(BlueprintType)
enum class EFortRogueBattleState : uint8
{
	PlayerTurn,
	EnemyTurn,
	ResolvingShot,
	Reward,
	Won,
	Lost
};

UCLASS()
class FORTROGUE_API AFortRogueGameMode : public AGameModeBase
{
	GENERATED_BODY()

#if WITH_DEV_AUTOMATION_TESTS
	friend class FFortRogueTerrainGameModeMapDefinitionTest;
#endif

public:
	AFortRogueGameMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Battle")
	void NotifyProjectileSpawned(AFortRogueProjectile* Projectile);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Battle")
	void NotifyShotFired(AFortRogueBattleCharacter* Shooter, int32 ProjectileCount);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Battle")
	void NotifyProjectileResolved(AFortRogueProjectile* Projectile);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Rewards")
	void ApplyRewardChoice(int32 ChoiceIndex);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	bool CanApplyRewardChoice(int32 ChoiceIndex) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Battle")
	float GetWind() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Battle")
	FText GetWindSummary() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Battle")
	EFortRogueBattleState GetBattleState() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Battle")
	AFortRogueBattleCharacter* GetPlayerCharacter() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Battle")
	AFortRogueBattleCharacter* GetEnemyCharacter() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	TArray<FFortRogueRewardChoice> GetRewardChoices() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	int32 GetRewardChoiceCount() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	FFortRogueRewardChoice GetRewardChoice(int32 ChoiceIndex) const;

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
	UFortRogueTerrainMapDefinition* GetStageTerrainMapDefinition() const;
	UFortRogueDefaultLoadoutDefinition* GetDefaultLoadoutDefinition() const;
	const FFortRogueStageDifficultyData& GetCurrentStageDifficulty() const;
	int32 GetConfiguredStageCount() const;
	void HandleEnemyDefeated();
	void AdvanceToNextStage();
	void ApplyRewardToPlayer(const FFortRogueRewardChoice& Reward);
	void StartPlayerTurn();
	void StartEnemyTurn();
	void RunEnemyTurn();
	void FinishShotResolution();
	void EnterRewardState();
	void BuildRewardChoices();
	void CheckTurnDefeatState();
	void SetStatus(const FString& NewStatus);
	void UpdateBattleCamera(float DeltaSeconds);
	void ResetShotCameraState();
	float GetInitialCameraOrthoWidth() const;
	FRotator GetBattleCameraRotation() const;
	FVector GetDesiredCameraLocation() const;
	FVector GetCameraFocusLocation() const;
	FVector ClampCameraLocationToTerrainBounds(const FVector& DesiredLocation) const;

	UPROPERTY()
	TObjectPtr<AFortRogueBattleCharacter> PlayerCharacter;

	UPROPERTY()
	TObjectPtr<AFortRogueBattleCharacter> EnemyCharacter;

	UPROPERTY()
	TObjectPtr<AFortRogueDestructibleTerrain> Terrain;

	UPROPERTY()
	TObjectPtr<ACameraActor> BattleCamera;

	UPROPERTY()
	TArray<FFortRogueRewardChoice> RewardChoices;

	UPROPERTY()
	TArray<TWeakObjectPtr<AFortRogueProjectile>> ActiveProjectiles;

	UPROPERTY()
	TObjectPtr<UFortRogueCharacterDefinition> CurrentEnemyDefinition;

	UPROPERTY()
	TArray<TObjectPtr<UFortRogueCharacterDefinition>> EncounteredEnemyDefinitions;

	UPROPERTY()
	TArray<FGameplayTag> ChosenRewardTags;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup")
	TSubclassOf<AFortRogueBattleCharacter> PlayerCharacterClass;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup")
	TSubclassOf<AFortRogueBattleCharacter> EnemyCharacterClass;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup")
	TSubclassOf<AFortRogueDestructibleTerrain> TerrainClass;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup")
	TObjectPtr<UFortRogueTerrainMapDefinition> TerrainMapDefinition;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup")
	TSubclassOf<ACameraActor> CameraClass;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup")
	TObjectPtr<UFortRogueCharacterDefinition> PlayerDefinition;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup")
	TObjectPtr<UFortRogueCharacterDefinition> EnemyDefinition;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Run")
	TObjectPtr<UFortRogueStageRunDefinition> StageRunDefinition;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup")
	FVector TerrainLocation = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup")
	FVector PlayerSpawnOffset = FVector(-650.0f, 0.0f, 95.0f);

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup")
	FVector EnemySpawnOffset = FVector(650.0f, 0.0f, 95.0f);

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup")
	FVector CameraLocation = FVector(0.0f, 3000.0f, 860.0f);

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup")
	float CameraOrthoWidth = 2700.0f;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup")
	float CameraFollowInterpSpeed = 4.5f;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup")
	float CameraProjectileZOffset = 120.0f;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup")
	float CameraTurnZOffset = 220.0f;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup")
	float ShotImpactCameraHoldSeconds = 0.65f;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup")
	float MinWind = -180.0f;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup")
	float MaxWind = 180.0f;

	EFortRogueBattleState BattleState = EFortRogueBattleState::PlayerTurn;
	int32 CurrentStage = 1;
	float Wind = 0.0f;
	int32 PendingProjectiles = 0;
	TWeakObjectPtr<AFortRogueBattleCharacter> LastShooter;
	FVector LastImpactCameraLocation = FVector::ZeroVector;
	bool bHoldingImpactCamera = false;
	FTimerHandle ShotResolutionTimerHandle;
	FText StatusText;
};
