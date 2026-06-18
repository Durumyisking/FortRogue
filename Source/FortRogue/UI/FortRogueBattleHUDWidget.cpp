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

TArray<FFortRogueWeaponSpec> UFortRogueBattleHUDWidget::GetPlayerWeaponLoadout() const
{
	if (AFortRogueBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetWeaponLoadoutForBlueprint();
	}
	return TArray<FFortRogueWeaponSpec>();
}

FFortRogueWeaponSpec UFortRogueBattleHUDWidget::GetPlayerCurrentWeaponSpec() const
{
	if (AFortRogueBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetCurrentWeaponSpec();
	}
	return FFortRogueWeaponSpec();
}

int32 UFortRogueBattleHUDWidget::GetPlayerSelectedWeaponIndex() const
{
	if (AFortRogueBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetSelectedWeaponIndex();
	}
	return INDEX_NONE;
}

TArray<FFortRogueItemStack> UFortRogueBattleHUDWidget::GetPlayerItemLoadout() const
{
	if (AFortRogueBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetItemLoadoutForBlueprint();
	}
	return TArray<FFortRogueItemStack>();
}

float UFortRogueBattleHUDWidget::GetPlayerAimAngle() const
{
	if (AFortRogueBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetAimAngle();
	}
	return 0.0f;
}

float UFortRogueBattleHUDWidget::GetPlayerShotPower() const
{
	if (AFortRogueBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetShotPower();
	}
	return 0.0f;
}

float UFortRogueBattleHUDWidget::GetPlayerShotChargeAlpha() const
{
	if (AFortRogueBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetShotChargeAlpha();
	}
	return 0.0f;
}

bool UFortRogueBattleHUDWidget::IsPlayerChargingShot() const
{
	if (AFortRogueBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->IsChargingShot();
	}
	return false;
}

bool UFortRogueBattleHUDWidget::CanFirePlayerWeapon() const
{
	if (AFortRogueBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->CanFireSelectedWeapon();
	}
	return false;
}
