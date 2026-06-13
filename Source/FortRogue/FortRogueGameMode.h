// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Rewards/FortRogueRewardTypes.h"
#include "FortRogueGameMode.generated.h"

class AFortRogueBattleCharacter;
class AFortRogueDestructibleTerrain;
class AFortRogueProjectile;
class ACameraActor;
class UFortRogueCharacterDefinition;

UENUM(BlueprintType)
enum class EFortRogueBattleState : uint8
{
	PlayerTurn,
	EnemyTurn,
	ResolvingShot,
	Reward,
	Lost
};

UCLASS()
class FORTROGUE_API AFortRogueGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFortRogueGameMode();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Battle")
	void NotifyShotFired(AFortRogueBattleCharacter* Shooter, int32 ProjectileCount);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Battle")
	void NotifyProjectileResolved(AFortRogueProjectile* Projectile);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Rewards")
	void ApplyRewardChoice(int32 ChoiceIndex);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Battle")
	float GetWind() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Battle")
	EFortRogueBattleState GetBattleState() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Battle")
	AFortRogueBattleCharacter* GetPlayerCharacter() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Battle")
	AFortRogueBattleCharacter* GetEnemyCharacter() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	TArray<FFortRogueRewardChoice> GetRewardChoices() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Battle")
	FText GetStatusText() const;

private:
	void SpawnMVPBattle();
	void StartPlayerTurn();
	void StartEnemyTurn();
	void RunEnemyTurn();
	void FinishShotResolution();
	void EnterRewardState();
	void BuildRewardChoices();
	void SetStatus(const FString& NewStatus);

	UPROPERTY()
	TObjectPtr<AFortRogueBattleCharacter> PlayerCharacter;

	UPROPERTY()
	TObjectPtr<AFortRogueBattleCharacter> EnemyCharacter;

	UPROPERTY()
	TObjectPtr<AFortRogueDestructibleTerrain> Terrain;

	UPROPERTY()
	TArray<FFortRogueRewardChoice> RewardChoices;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup")
	TSubclassOf<AFortRogueBattleCharacter> PlayerCharacterClass;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup")
	TSubclassOf<AFortRogueBattleCharacter> EnemyCharacterClass;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup")
	TSubclassOf<AFortRogueDestructibleTerrain> TerrainClass;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup")
	TSubclassOf<ACameraActor> CameraClass;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup")
	TObjectPtr<UFortRogueCharacterDefinition> PlayerDefinition;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup")
	TObjectPtr<UFortRogueCharacterDefinition> EnemyDefinition;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup")
	FVector TerrainLocation = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup")
	FVector PlayerSpawnOffset = FVector(-650.0f, 0.0f, 95.0f);

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup")
	FVector EnemySpawnOffset = FVector(650.0f, 0.0f, 95.0f);

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup")
	FVector CameraLocation = FVector(0.0f, -3000.0f, 860.0f);

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup")
	FRotator CameraRotation = FRotator(-5.0f, 90.0f, 0.0f);

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup")
	float CameraOrthoWidth = 2700.0f;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup")
	float MinWind = -180.0f;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Battle Setup")
	float MaxWind = 180.0f;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Rewards")
	TArray<FFortRogueRewardChoice> RewardPool;

	EFortRogueBattleState BattleState = EFortRogueBattleState::PlayerTurn;
	float Wind = 0.0f;
	int32 PendingProjectiles = 0;
	TWeakObjectPtr<AFortRogueBattleCharacter> LastShooter;
	FText StatusText;
};
