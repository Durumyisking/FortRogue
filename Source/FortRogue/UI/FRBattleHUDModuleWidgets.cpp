// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FRBattleHUDModuleWidgets.h"

#include "CommonNumericTextBlock.h"
#include "CommonTextBlock.h"
#include "Components/ProgressBar.h"
#include "UI/FRBattleHUDModuleViewModels.h"

namespace
{
	void SetText(UCommonTextBlock* TextBlock, const FText& Text)
	{
		if (TextBlock)
		{
			TextBlock->SetText(Text);
		}
	}

	void SetNumericText(UCommonNumericTextBlock* TextBlock, float Value)
	{
		if (TextBlock)
		{
			TextBlock->SetCurrentValue(Value);
		}
	}

	void SetBar(UProgressBar* ProgressBar, float Percent)
	{
		if (ProgressBar)
		{
			ProgressBar->SetPercent(FMath::Clamp(Percent, 0.0f, 1.0f));
		}
	}
}

void UFRBattleHUDModuleWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	RefreshFromViewModel();
}

void UFRBattleHUDModuleWidgetBase::SetRawViewModel(UMVVMViewModelBase* InViewModel)
{
	if (RawViewModel == InViewModel)
	{
		return;
	}

	RawViewModel = InViewModel;
	RefreshFromViewModel();
}

void UFRBattleHUDModuleWidgetBase::RefreshFromViewModel()
{
	NativeRefreshFromViewModel();
}

void UFRBattleHUDModuleWidgetBase::NativeRefreshFromViewModel()
{
}

void UFRBattleStatePanelWidget::SetViewModel(UFRBattleStateViewModel* InViewModel)
{
	SetRawViewModel(InViewModel);
}

UFRBattleStateViewModel* UFRBattleStatePanelWidget::GetViewModel() const
{
	return Cast<UFRBattleStateViewModel>(GetRawViewModel());
}

void UFRBattleStatePanelWidget::NativeRefreshFromViewModel()
{
	const UFRBattleStateViewModel* ViewModel = GetViewModel();
	if (!ViewModel)
	{
		return;
	}

	SetText(TurnText, ViewModel->GetTurnText());
	SetText(RunProgressText, ViewModel->GetRunProgressText());
	SetText(StatusText, ViewModel->GetStatusText());
}

void UFRCombatantStatusPanelWidget::SetViewModel(UFRCombatantStatusViewModel* InViewModel)
{
	SetRawViewModel(InViewModel);
}

UFRCombatantStatusViewModel* UFRCombatantStatusPanelWidget::GetViewModel() const
{
	return Cast<UFRCombatantStatusViewModel>(GetRawViewModel());
}

void UFRCombatantStatusPanelWidget::NativeRefreshFromViewModel()
{
	const UFRCombatantStatusViewModel* ViewModel = GetViewModel();
	if (!ViewModel)
	{
		return;
	}

	SetText(TitleText, ViewModel->GetTitleText());
	SetText(HealthText, ViewModel->GetHealthText());
	SetNumericText(CurrentHealthValueText, ViewModel->GetCurrentHealthValue());
	SetNumericText(MaxHealthValueText, ViewModel->GetMaxHealthValue());
	SetBar(HealthBar, ViewModel->GetHealthPercent());
	SetText(MoveBudgetText, ViewModel->GetMoveBudgetText());
	SetNumericText(MoveBudgetValueText, ViewModel->GetMoveBudgetValue());
	SetNumericText(MaxMoveBudgetValueText, ViewModel->GetMaxMoveBudgetValue());
	SetBar(MoveBudgetBar, ViewModel->GetMoveBudgetPercent());
}

void UFRAimWindIndicatorWidget::SetViewModel(UFRAimWindViewModel* InViewModel)
{
	SetRawViewModel(InViewModel);
}

UFRAimWindViewModel* UFRAimWindIndicatorWidget::GetViewModel() const
{
	return Cast<UFRAimWindViewModel>(GetRawViewModel());
}

void UFRAimWindIndicatorWidget::NativeRefreshFromViewModel()
{
	const UFRAimWindViewModel* ViewModel = GetViewModel();
	if (!ViewModel)
	{
		return;
	}

	SetText(WindText, ViewModel->GetWindText());
	SetNumericText(WindValueText, ViewModel->GetWindValue());
	SetNumericText(AbsoluteWindValueText, ViewModel->GetAbsoluteWindValue());
	SetText(AimText, ViewModel->GetAimText());
	SetNumericText(AimAngleValueText, ViewModel->GetAimAngleValue());
}

void UFRShotPowerMeterWidget::SetViewModel(UFRShotPowerViewModel* InViewModel)
{
	SetRawViewModel(InViewModel);
}

UFRShotPowerViewModel* UFRShotPowerMeterWidget::GetViewModel() const
{
	return Cast<UFRShotPowerViewModel>(GetRawViewModel());
}

void UFRShotPowerMeterWidget::NativeRefreshFromViewModel()
{
	const UFRShotPowerViewModel* ViewModel = GetViewModel();
	if (!ViewModel)
	{
		return;
	}

	SetText(ShotPowerText, ViewModel->GetShotPowerText());
	SetNumericText(ShotPowerValueText, ViewModel->GetShotPowerPercent());
	SetBar(ShotPowerBar, ViewModel->GetShotPowerPercent());
}

void UFRLoadoutBarWidget::SetViewModel(UFRLoadoutViewModel* InViewModel)
{
	SetRawViewModel(InViewModel);
}

UFRLoadoutViewModel* UFRLoadoutBarWidget::GetViewModel() const
{
	return Cast<UFRLoadoutViewModel>(GetRawViewModel());
}

void UFRLoadoutBarWidget::NativeRefreshFromViewModel()
{
	const UFRLoadoutViewModel* ViewModel = GetViewModel();
	if (!ViewModel)
	{
		return;
	}

	SetText(CurrentWeaponText, ViewModel->GetCurrentWeaponText());
}

void UFRShotInfoPanelWidget::SetViewModel(UFRShotPreviewViewModel* InViewModel)
{
	SetRawViewModel(InViewModel);
}

UFRShotPreviewViewModel* UFRShotInfoPanelWidget::GetViewModel() const
{
	return Cast<UFRShotPreviewViewModel>(GetRawViewModel());
}

void UFRShotInfoPanelWidget::NativeRefreshFromViewModel()
{
	const UFRShotPreviewViewModel* ViewModel = GetViewModel();
	if (!ViewModel)
	{
		return;
	}

	SetText(PrimaryText, ViewModel->GetPrimaryText());
	SetText(SecondaryText, ViewModel->GetSecondaryText());
	SetNumericText(DamageValueText, ViewModel->GetDamageValue());
	SetNumericText(BlastRadiusValueText, ViewModel->GetBlastRadiusValue());
	SetNumericText(ProjectileCountValueText, static_cast<float>(ViewModel->GetProjectileCountValue()));
	SetNumericText(LaunchSpeedValueText, ViewModel->GetLaunchSpeedValue());
	SetNumericText(GravityValueText, ViewModel->GetGravityValue());
	SetNumericText(TerrainDamageValueText, ViewModel->GetTerrainDamageValue());
	SetNumericText(TerrainFillRadiusValueText, ViewModel->GetTerrainFillRadiusValue());
}

void UFRModifierSummaryWidget::SetViewModel(UFRModifierSummaryViewModel* InViewModel)
{
	SetRawViewModel(InViewModel);
}

UFRModifierSummaryViewModel* UFRModifierSummaryWidget::GetViewModel() const
{
	return Cast<UFRModifierSummaryViewModel>(GetRawViewModel());
}

void UFRModifierSummaryWidget::NativeRefreshFromViewModel()
{
	const UFRModifierSummaryViewModel* ViewModel = GetViewModel();
	if (!ViewModel)
	{
		return;
	}

	SetText(GrantedModifierText, ViewModel->GetGrantedModifierText());
	SetText(PendingModifierText, ViewModel->GetPendingModifierText());
	SetText(AbilitySetText, ViewModel->GetAbilitySetText());
	SetText(SummaryText, ViewModel->GetSummaryText());
}
