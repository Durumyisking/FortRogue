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

void UFRCombatantStatusViewModel::SetCurrentHealthValue(float InCurrentHealthValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(CurrentHealthValue, InCurrentHealthValue);
}

void UFRCombatantStatusViewModel::SetMaxHealthValue(float InMaxHealthValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(MaxHealthValue, InMaxHealthValue);
}

void UFRCombatantStatusViewModel::SetHealthPercent(float InHealthPercent)
{
	UE_MVVM_SET_PROPERTY_VALUE(HealthPercent, InHealthPercent);
}

void UFRCombatantStatusViewModel::SetMoveBudgetText(const FText& InMoveBudgetText)
{
	UE_MVVM_SET_PROPERTY_VALUE(MoveBudgetText, InMoveBudgetText);
}

void UFRCombatantStatusViewModel::SetMoveBudgetValue(float InMoveBudgetValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(MoveBudgetValue, InMoveBudgetValue);
}

void UFRCombatantStatusViewModel::SetMaxMoveBudgetValue(float InMaxMoveBudgetValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(MaxMoveBudgetValue, InMaxMoveBudgetValue);
}

void UFRCombatantStatusViewModel::SetMoveBudgetPercent(float InMoveBudgetPercent)
{
	UE_MVVM_SET_PROPERTY_VALUE(MoveBudgetPercent, InMoveBudgetPercent);
}

void UFRAimWindViewModel::SetWindText(const FText& InWindText)
{
	UE_MVVM_SET_PROPERTY_VALUE(WindText, InWindText);
}

void UFRAimWindViewModel::SetWindValue(float InWindValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(WindValue, InWindValue);
}

void UFRAimWindViewModel::SetAbsoluteWindValue(float InAbsoluteWindValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(AbsoluteWindValue, InAbsoluteWindValue);
}

void UFRAimWindViewModel::SetAimText(const FText& InAimText)
{
	UE_MVVM_SET_PROPERTY_VALUE(AimText, InAimText);
}

void UFRAimWindViewModel::SetAimAngleValue(float InAimAngleValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(AimAngleValue, InAimAngleValue);
}

void UFRShotPowerViewModel::SetShotPowerText(const FText& InShotPowerText)
{
	UE_MVVM_SET_PROPERTY_VALUE(ShotPowerText, InShotPowerText);
}

void UFRShotPowerViewModel::SetShotPowerPercent(float InShotPowerPercent)
{
	UE_MVVM_SET_PROPERTY_VALUE(ShotPowerPercent, InShotPowerPercent);
}

void UFRLoadoutSlotViewModel::SetSlotLabelText(const FText& InSlotLabelText)
{
	UE_MVVM_SET_PROPERTY_VALUE(SlotLabelText, InSlotLabelText);
}

void UFRLoadoutSlotViewModel::SetDisplayText(const FText& InDisplayText)
{
	UE_MVVM_SET_PROPERTY_VALUE(DisplayText, InDisplayText);
}

void UFRLoadoutSlotViewModel::SetCountValue(float InCountValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(CountValue, InCountValue);
}

void UFRLoadoutSlotViewModel::SetShowCount(bool bInShowCount)
{
	UE_MVVM_SET_PROPERTY_VALUE(bShowCount, bInShowCount);
}

void UFRLoadoutSlotViewModel::SetOccupied(bool bInOccupied)
{
	UE_MVVM_SET_PROPERTY_VALUE(bOccupied, bInOccupied);
}

void UFRLoadoutSlotViewModel::SetSelected(bool bInSelected)
{
	UE_MVVM_SET_PROPERTY_VALUE(bSelected, bInSelected);
}

void UFRLoadoutSlotViewModel::SetEnabled(bool bInEnabled)
{
	UE_MVVM_SET_PROPERTY_VALUE(bEnabled, bInEnabled);
}

void UFRLoadoutSlotViewModel::SetStatusText(const FText& InStatusText)
{
	UE_MVVM_SET_PROPERTY_VALUE(StatusText, InStatusText);
}

void UFRLoadoutViewModel::SetCurrentWeaponText(const FText& InCurrentWeaponText)
{
	UE_MVVM_SET_PROPERTY_VALUE(CurrentWeaponText, InCurrentWeaponText);
}

UFRLoadoutSlotViewModel* UFRLoadoutViewModel::GetWeaponSlotViewModel(int32 SlotIndex) const
{
	return WeaponSlotViewModels.IsValidIndex(SlotIndex) ? WeaponSlotViewModels[SlotIndex] : nullptr;
}

UFRLoadoutSlotViewModel* UFRLoadoutViewModel::GetItemSlotViewModel(int32 SlotIndex) const
{
	return ItemSlotViewModels.IsValidIndex(SlotIndex) ? ItemSlotViewModels[SlotIndex] : nullptr;
}

UFRLoadoutSlotViewModel* UFRLoadoutViewModel::GetOrCreateWeaponSlotViewModel(int32 SlotIndex)
{
	if (SlotIndex < 0)
	{
		return nullptr;
	}

	SetWeaponSlotCount(SlotIndex + 1);
	return WeaponSlotViewModels[SlotIndex];
}

UFRLoadoutSlotViewModel* UFRLoadoutViewModel::GetOrCreateItemSlotViewModel(int32 SlotIndex)
{
	if (SlotIndex < 0)
	{
		return nullptr;
	}

	SetItemSlotCount(SlotIndex + 1);
	return ItemSlotViewModels[SlotIndex];
}

void UFRLoadoutViewModel::SetWeaponSlotCount(int32 SlotCount)
{
	const int32 TargetCount = FMath::Max(0, SlotCount);
	while (WeaponSlotViewModels.Num() < TargetCount)
	{
		WeaponSlotViewModels.Add(NewObject<UFRLoadoutSlotViewModel>(this));
	}
	if (WeaponSlotViewModels.Num() > TargetCount)
	{
		WeaponSlotViewModels.SetNum(TargetCount);
	}
}

void UFRLoadoutViewModel::SetItemSlotCount(int32 SlotCount)
{
	const int32 TargetCount = FMath::Max(0, SlotCount);
	while (ItemSlotViewModels.Num() < TargetCount)
	{
		ItemSlotViewModels.Add(NewObject<UFRLoadoutSlotViewModel>(this));
	}
	if (ItemSlotViewModels.Num() > TargetCount)
	{
		ItemSlotViewModels.SetNum(TargetCount);
	}
}

void UFRShotPreviewViewModel::SetPrimaryText(const FText& InPrimaryText)
{
	UE_MVVM_SET_PROPERTY_VALUE(PrimaryText, InPrimaryText);
}

void UFRShotPreviewViewModel::SetSecondaryText(const FText& InSecondaryText)
{
	UE_MVVM_SET_PROPERTY_VALUE(SecondaryText, InSecondaryText);
}

void UFRShotPreviewViewModel::SetDamageValue(float InDamageValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(DamageValue, InDamageValue);
}

void UFRShotPreviewViewModel::SetBlastRadiusValue(float InBlastRadiusValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(BlastRadiusValue, InBlastRadiusValue);
}

void UFRShotPreviewViewModel::SetProjectileCountValue(int32 InProjectileCountValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(ProjectileCountValue, InProjectileCountValue);
}

void UFRShotPreviewViewModel::SetLaunchSpeedValue(float InLaunchSpeedValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(LaunchSpeedValue, InLaunchSpeedValue);
}

void UFRShotPreviewViewModel::SetGravityValue(float InGravityValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(GravityValue, InGravityValue);
}

void UFRShotPreviewViewModel::SetTerrainDamageValue(float InTerrainDamageValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(TerrainDamageValue, InTerrainDamageValue);
}

void UFRShotPreviewViewModel::SetTerrainFillRadiusValue(float InTerrainFillRadiusValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(TerrainFillRadiusValue, InTerrainFillRadiusValue);
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
