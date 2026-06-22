// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FRBattleHUDViewModel.h"

void UFRBattleHUDViewModel::SetTurnText(const FText& InTurnText)
{
	UE_MVVM_SET_PROPERTY_VALUE(TurnText, InTurnText);
}

void UFRBattleHUDViewModel::SetRunProgressText(const FText& InRunProgressText)
{
	UE_MVVM_SET_PROPERTY_VALUE(RunProgressText, InRunProgressText);
}

void UFRBattleHUDViewModel::SetStatusText(const FText& InStatusText)
{
	UE_MVVM_SET_PROPERTY_VALUE(StatusText, InStatusText);
}

void UFRBattleHUDViewModel::SetPlayerHealthText(const FText& InPlayerHealthText)
{
	UE_MVVM_SET_PROPERTY_VALUE(PlayerHealthText, InPlayerHealthText);
}

void UFRBattleHUDViewModel::SetPlayerHealthPercent(float InPlayerHealthPercent)
{
	UE_MVVM_SET_PROPERTY_VALUE(PlayerHealthPercent, InPlayerHealthPercent);
}

void UFRBattleHUDViewModel::SetEnemyHealthText(const FText& InEnemyHealthText)
{
	UE_MVVM_SET_PROPERTY_VALUE(EnemyHealthText, InEnemyHealthText);
}

void UFRBattleHUDViewModel::SetEnemyHealthPercent(float InEnemyHealthPercent)
{
	UE_MVVM_SET_PROPERTY_VALUE(EnemyHealthPercent, InEnemyHealthPercent);
}

void UFRBattleHUDViewModel::SetWindText(const FText& InWindText)
{
	UE_MVVM_SET_PROPERTY_VALUE(WindText, InWindText);
}

void UFRBattleHUDViewModel::SetAimText(const FText& InAimText)
{
	UE_MVVM_SET_PROPERTY_VALUE(AimText, InAimText);
}

void UFRBattleHUDViewModel::SetShotPowerText(const FText& InShotPowerText)
{
	UE_MVVM_SET_PROPERTY_VALUE(ShotPowerText, InShotPowerText);
}

void UFRBattleHUDViewModel::SetShotPowerPercent(float InShotPowerPercent)
{
	UE_MVVM_SET_PROPERTY_VALUE(ShotPowerPercent, InShotPowerPercent);
}

void UFRBattleHUDViewModel::SetMoveBudgetText(const FText& InMoveBudgetText)
{
	UE_MVVM_SET_PROPERTY_VALUE(MoveBudgetText, InMoveBudgetText);
}

void UFRBattleHUDViewModel::SetMoveBudgetPercent(float InMoveBudgetPercent)
{
	UE_MVVM_SET_PROPERTY_VALUE(MoveBudgetPercent, InMoveBudgetPercent);
}

void UFRBattleHUDViewModel::SetCurrentWeaponText(const FText& InCurrentWeaponText)
{
	UE_MVVM_SET_PROPERTY_VALUE(CurrentWeaponText, InCurrentWeaponText);
}

void UFRBattleHUDViewModel::SetShotInfoText(const FText& InShotInfoText)
{
	UE_MVVM_SET_PROPERTY_VALUE(ShotInfoText, InShotInfoText);
}

void UFRBattleHUDViewModel::SetGrantedModifierText(const FText& InGrantedModifierText)
{
	UE_MVVM_SET_PROPERTY_VALUE(GrantedModifierText, InGrantedModifierText);
}

void UFRBattleHUDViewModel::SetPendingModifierText(const FText& InPendingModifierText)
{
	UE_MVVM_SET_PROPERTY_VALUE(PendingModifierText, InPendingModifierText);
}

void UFRBattleHUDViewModel::SetAbilitySetText(const FText& InAbilitySetText)
{
	UE_MVVM_SET_PROPERTY_VALUE(AbilitySetText, InAbilitySetText);
}
