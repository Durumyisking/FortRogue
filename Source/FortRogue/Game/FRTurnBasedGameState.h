// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "FRTurnBasedGameState.generated.h"

class AFRBattleCharacter;

UENUM(BlueprintType)
enum class EFRBattleState : uint8
{
	PlayerTurn,
	EnemyTurn,
	ResolvingShot,
	Reward,
	Won,
	Lost
};

UCLASS()
class FORTROGUE_API AFRTurnBasedGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AFRTurnBasedGameState();

	void ResetTurnState();
	void ResetShotResolution();
	void StartPlayerTurn(AFRBattleCharacter* PlayerCharacter, float NewWind, const FText& NewStatusText);
	void StartEnemyTurn(const TArray<AFRBattleCharacter*>& EnemyCharacters, float NewWind, const FText& NewStatusText);
	AFRBattleCharacter* AdvanceToNextEnemyTurnCharacter(const TArray<AFRBattleCharacter*>& EnemyCharacters);
	AFRBattleCharacter* GetActiveEnemyTurnCharacter(const TArray<AFRBattleCharacter*>& EnemyCharacters) const;
	void EnterShotResolution(AFRBattleCharacter* Shooter, int32 ProjectileCount, const FText& NewStatusText);
	void EnterEnemyTurnContinuation();
	void EnterRewardState(const FText& NewStatusText);
	void EnterWonState(const FText& NewStatusText);
	void EnterLostState(const FText& NewStatusText);
	void RegisterProjectileSpawned(bool bIncreasePendingProjectileCount);
	int32 RegisterProjectileSpawnFailed(int32 ProjectileCount);
	int32 RegisterProjectileResolved();
	bool CanCheckTurnDefeatState() const;
	void SetStatusText(const FText& NewStatusText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Battle")
	EFRBattleState GetBattleState() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Battle")
	float GetWind() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Battle")
	int32 GetPendingProjectileCount() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Battle")
	int32 GetActiveEnemyTurnIndex() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Battle")
	AFRBattleCharacter* GetLastShooter() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Battle")
	FText GetStatusText() const;

private:
	UPROPERTY(VisibleAnywhere, Category = "FortRogue|Battle")
	EFRBattleState BattleState = EFRBattleState::PlayerTurn;

	UPROPERTY(VisibleAnywhere, Category = "FortRogue|Battle")
	float Wind = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "FortRogue|Battle")
	int32 PendingProjectiles = 0;

	UPROPERTY(VisibleAnywhere, Category = "FortRogue|Battle")
	int32 ActiveEnemyTurnIndex = INDEX_NONE;

	UPROPERTY()
	TWeakObjectPtr<AFRBattleCharacter> LastShooter;

	UPROPERTY(VisibleAnywhere, Category = "FortRogue|Battle")
	FText StatusText;
};