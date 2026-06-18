// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FortRogueBattleHUDWidget.h"

#include "Combat/FortRogueBattleCharacter.h"
#include "FortRogueGameMode.h"

AFortRogueGameMode* UFortRogueBattleHUDWidget::GetFortRogueGameMode() const
{
	return GetWorld() ? GetWorld()->GetAuthGameMode<AFortRogueGameMode>() : nullptr;
}

AFortRogueBattleCharacter* UFortRogueBattleHUDWidget::GetPlayerCharacter() const
{
	if (AFortRogueGameMode* GameMode = GetFortRogueGameMode())
	{
		return GameMode->GetPlayerCharacter();
	}
	return nullptr;
}

AFortRogueBattleCharacter* UFortRogueBattleHUDWidget::GetEnemyCharacter() const
{
	if (AFortRogueGameMode* GameMode = GetFortRogueGameMode())
	{
		return GameMode->GetEnemyCharacter();
	}
	return nullptr;
}

FText UFortRogueBattleHUDWidget::GetRunProgressSummary() const
{
	if (AFortRogueGameMode* GameMode = GetFortRogueGameMode())
	{
		return GameMode->GetRunProgressSummary();
	}
	return FText::GetEmpty();
}

FText UFortRogueBattleHUDWidget::GetWindSummary() const
{
	if (AFortRogueGameMode* GameMode = GetFortRogueGameMode())
	{
		return GameMode->GetWindSummary();
	}
	return FText::GetEmpty();
}

FText UFortRogueBattleHUDWidget::GetPlayerCombatStatsSummary() const
{
	if (AFortRogueBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetCombatStatsSummary();
	}
	return FText::GetEmpty();
}

FText UFortRogueBattleHUDWidget::GetPlayerShotSummary() const
{
	if (AFortRogueBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetCurrentShotSummary();
	}
	return FText::GetEmpty();
}

bool UFortRogueBattleHUDWidget::CanFirePlayerWeapon() const
{
	if (AFortRogueBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->CanFireSelectedWeapon();
	}
	return false;
}
