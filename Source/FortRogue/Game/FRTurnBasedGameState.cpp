// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/FRTurnBasedGameState.h"

#include "Combat/FRBattleCharacter.h"

AFRTurnBasedGameState::AFRTurnBasedGameState()
{
	StatusText = FText::FromString(TEXT("Ready"));
}

void AFRTurnBasedGameState::ResetTurnState()
{
	BattleState = EFRBattleState::PlayerTurn;
	Wind = 0.0f;
	PendingProjectiles = 0;
	ActiveEnemyTurnIndex = INDEX_NONE;
	LastShooter.Reset();
	StatusText = FText::FromString(TEXT("Ready"));
}

void AFRTurnBasedGameState::ResetShotResolution()
{
	PendingProjectiles = 0;
	LastShooter.Reset();
}

void AFRTurnBasedGameState::StartPlayerTurn(AFRBattleCharacter* PlayerCharacter, float NewWind, const FText& NewStatusText)
{
	BattleState = EFRBattleState::PlayerTurn;
	Wind = NewWind;
	PendingProjectiles = 0;
	ActiveEnemyTurnIndex = INDEX_NONE;
	LastShooter.Reset();
	if (PlayerCharacter)
	{
		PlayerCharacter->BeginTurn();
	}
	SetStatusText(NewStatusText);
}

void AFRTurnBasedGameState::StartEnemyTurn(const TArray<AFRBattleCharacter*>& EnemyCharacters, float NewWind, const FText& NewStatusText)
{
	BattleState = EFRBattleState::EnemyTurn;
	Wind = NewWind;
	PendingProjectiles = 0;
	ActiveEnemyTurnIndex = INDEX_NONE;
	LastShooter.Reset();
	for (AFRBattleCharacter* Enemy : EnemyCharacters)
	{
		if (Enemy && !Enemy->IsDefeated())
		{
			Enemy->BeginTurn();
		}
	}
	SetStatusText(NewStatusText);
}

AFRBattleCharacter* AFRTurnBasedGameState::AdvanceToNextEnemyTurnCharacter(const TArray<AFRBattleCharacter*>& EnemyCharacters)
{
	for (int32 EnemyIndex = ActiveEnemyTurnIndex + 1; EnemyIndex < EnemyCharacters.Num(); ++EnemyIndex)
	{
		AFRBattleCharacter* Candidate = EnemyCharacters[EnemyIndex];
		if (Candidate && !Candidate->IsDefeated())
		{
			ActiveEnemyTurnIndex = EnemyIndex;
			return Candidate;
		}
	}

	return nullptr;
}

AFRBattleCharacter* AFRTurnBasedGameState::GetActiveEnemyTurnCharacter(const TArray<AFRBattleCharacter*>& EnemyCharacters) const
{
	if (EnemyCharacters.IsValidIndex(ActiveEnemyTurnIndex))
	{
		AFRBattleCharacter* ActiveEnemy = EnemyCharacters[ActiveEnemyTurnIndex];
		if (ActiveEnemy && !ActiveEnemy->IsDefeated())
		{
			return ActiveEnemy;
		}
	}

	for (AFRBattleCharacter* Enemy : EnemyCharacters)
	{
		if (Enemy && !Enemy->IsDefeated())
		{
			return Enemy;
		}
	}

	return nullptr;
}

void AFRTurnBasedGameState::EnterShotResolution(AFRBattleCharacter* Shooter, int32 ProjectileCount, const FText& NewStatusText)
{
	PendingProjectiles = FMath::Max(0, ProjectileCount);
	LastShooter = Shooter;
	BattleState = EFRBattleState::ResolvingShot;
	if (Shooter)
	{
		Shooter->EndTurn();
	}
	SetStatusText(NewStatusText);
}

void AFRTurnBasedGameState::EnterEnemyTurnContinuation()
{
	BattleState = EFRBattleState::EnemyTurn;
}

void AFRTurnBasedGameState::EnterRewardState(const FText& NewStatusText)
{
	BattleState = EFRBattleState::Reward;
	PendingProjectiles = 0;
	LastShooter.Reset();
	SetStatusText(NewStatusText);
}

void AFRTurnBasedGameState::EnterWonState(const FText& NewStatusText)
{
	BattleState = EFRBattleState::Won;
	PendingProjectiles = 0;
	LastShooter.Reset();
	SetStatusText(NewStatusText);
}

void AFRTurnBasedGameState::EnterLostState(const FText& NewStatusText)
{
	BattleState = EFRBattleState::Lost;
	PendingProjectiles = 0;
	LastShooter.Reset();
	SetStatusText(NewStatusText);
}

void AFRTurnBasedGameState::RegisterProjectileSpawned(bool bIncreasePendingProjectileCount)
{
	if (bIncreasePendingProjectileCount && BattleState == EFRBattleState::ResolvingShot)
	{
		++PendingProjectiles;
	}
}

int32 AFRTurnBasedGameState::RegisterProjectileSpawnFailed(int32 ProjectileCount)
{
	if (ProjectileCount > 0 && BattleState == EFRBattleState::ResolvingShot)
	{
		PendingProjectiles = FMath::Max(0, PendingProjectiles - ProjectileCount);
	}
	return PendingProjectiles;
}

int32 AFRTurnBasedGameState::RegisterProjectileResolved()
{
	if (BattleState == EFRBattleState::ResolvingShot)
	{
		PendingProjectiles = FMath::Max(0, PendingProjectiles - 1);
	}
	return PendingProjectiles;
}

bool AFRTurnBasedGameState::CanCheckTurnDefeatState() const
{
	return BattleState == EFRBattleState::PlayerTurn || BattleState == EFRBattleState::EnemyTurn;
}

void AFRTurnBasedGameState::SetStatusText(const FText& NewStatusText)
{
	StatusText = NewStatusText;
}

EFRBattleState AFRTurnBasedGameState::GetBattleState() const
{
	return BattleState;
}

float AFRTurnBasedGameState::GetWind() const
{
	return Wind;
}

int32 AFRTurnBasedGameState::GetPendingProjectileCount() const
{
	return PendingProjectiles;
}

int32 AFRTurnBasedGameState::GetActiveEnemyTurnIndex() const
{
	return ActiveEnemyTurnIndex;
}

AFRBattleCharacter* AFRTurnBasedGameState::GetLastShooter() const
{
	return LastShooter.Get();
}

FText AFRTurnBasedGameState::GetStatusText() const
{
	return StatusText;
}