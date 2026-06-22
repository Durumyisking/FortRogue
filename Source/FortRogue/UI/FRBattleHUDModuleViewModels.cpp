// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FRBattleHUDModuleViewModels.h"

void UFRBattleStateViewModel::SetTurnText(const FText& InTurnText)
{
	UE_MVVM_SET_PROPERTY_VALUE(TurnText, InTurnText);
}

void UFRBattleStateViewModel::SetRunProgressText(const FText& InRunProgressText)
{
	UE_MVVM_SET_PROPERTY_VALUE(RunProgressText, InRunProgressText);
}

void UFRBattleStateViewModel::SetStatusText(const FText& InStatusText)
{
	UE_MVVM_SET_PROPERTY_VALUE(StatusText, InStatusText);
}

void UFRCombatantStatusViewModel::SetTitleText(const FText& InTitleText)
{
	UE_MVVM_SET_PROPERTY_VALUE(TitleText, InTitleText);
}

void UFRCombatantStatusViewModel::SetHealthText(const FText& InHealthText)
{
	UE_MVVM_SET_PROPERTY_VALUE(HealthText, InHealthText);
}

void UFRCombatantStatusViewModel::SetHealthPercent(float InHealthPercent)
{
	UE_MVVM_SET_PROPERTY_VALUE(HealthPercent, InHealthPercent);
}

void UFRCombatantStatusViewModel::SetMoveBudgetText(const FText& InMoveBudgetText)
{
	UE_MVVM_SET_PROPERTY_VALUE(MoveBudgetText, InMoveBudgetText);
}

void UFRCombatantStatusViewModel::SetMoveBudgetPercent(float InMoveBudgetPercent)
{
	UE_MVVM_SET_PROPERTY_VALUE(MoveBudgetPercent, InMoveBudgetPercent);
}

void UFRAimWindViewModel::SetWindText(const FText& InWindText)
{
	UE_MVVM_SET_PROPERTY_VALUE(WindText, InWindText);
}

void UFRAimWindViewModel::SetAimText(const FText& InAimText)
{
	UE_MVVM_SET_PROPERTY_VALUE(AimText, InAimText);
}

void UFRShotPowerViewModel::SetShotPowerText(const FText& InShotPowerText)
{
	UE_MVVM_SET_PROPERTY_VALUE(ShotPowerText, InShotPowerText);
}

void UFRShotPowerViewModel::SetShotPowerPercent(float InShotPowerPercent)
{
	UE_MVVM_SET_PROPERTY_VALUE(ShotPowerPercent, InShotPowerPercent);
}

void UFRLoadoutViewModel::SetCurrentWeaponText(const FText& InCurrentWeaponText)
{
	UE_MVVM_SET_PROPERTY_VALUE(CurrentWeaponText, InCurrentWeaponText);
}

void UFRShotPreviewViewModel::SetPrimaryText(const FText& InPrimaryText)
{
	UE_MVVM_SET_PROPERTY_VALUE(PrimaryText, InPrimaryText);
}

void UFRShotPreviewViewModel::SetSecondaryText(const FText& InSecondaryText)
{
	UE_MVVM_SET_PROPERTY_VALUE(SecondaryText, InSecondaryText);
}

void UFRModifierSummaryViewModel::SetGrantedModifierText(const FText& InGrantedModifierText)
{
	UE_MVVM_SET_PROPERTY_VALUE(GrantedModifierText, InGrantedModifierText);
}

void UFRModifierSummaryViewModel::SetPendingModifierText(const FText& InPendingModifierText)
{
	UE_MVVM_SET_PROPERTY_VALUE(PendingModifierText, InPendingModifierText);
}

void UFRModifierSummaryViewModel::SetAbilitySetText(const FText& InAbilitySetText)
{
	UE_MVVM_SET_PROPERTY_VALUE(AbilitySetText, InAbilitySetText);
}

void UFRModifierSummaryViewModel::SetSummaryText(const FText& InSummaryText)
{
	UE_MVVM_SET_PROPERTY_VALUE(SummaryText, InSummaryText);
}
