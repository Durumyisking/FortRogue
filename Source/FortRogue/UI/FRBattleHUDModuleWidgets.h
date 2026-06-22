// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonUserWidget.h"
#include "CoreMinimal.h"
#include "FRBattleHUDModuleWidgets.generated.h"

class UCommonNumericTextBlock;
class UCommonTextBlock;
class UFRAimWindViewModel;
class UFRBattleStateViewModel;
class UFRCombatantStatusViewModel;
class UFRLoadoutViewModel;
class UFRModifierSummaryViewModel;
class UFRShotPowerViewModel;
class UFRShotPreviewViewModel;
class UMVVMViewModelBase;
class UProgressBar;

UCLASS(Abstract, Blueprintable)
class FORTROGUE_API UFRBattleHUDModuleWidgetBase : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	void SetRawViewModel(UMVVMViewModelBase* InViewModel);
	void RefreshFromViewModel();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeRefreshFromViewModel();

	UMVVMViewModelBase* GetRawViewModel() const { return RawViewModel; }

private:
	UPROPERTY(Transient)
	TObjectPtr<UMVVMViewModelBase> RawViewModel;
};

UCLASS(Blueprintable)
class FORTROGUE_API UFRBattleStatePanelWidget : public UFRBattleHUDModuleWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FortRogue|UI|HUD")
	void SetViewModel(UFRBattleStateViewModel* InViewModel);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD")
	UFRBattleStateViewModel* GetViewModel() const;

protected:
	virtual void NativeRefreshFromViewModel() override;

private:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> TurnText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> RunProgressText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> StatusText;
};

UCLASS(Blueprintable)
class FORTROGUE_API UFRCombatantStatusPanelWidget : public UFRBattleHUDModuleWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FortRogue|UI|HUD")
	void SetViewModel(UFRCombatantStatusViewModel* InViewModel);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD")
	UFRCombatantStatusViewModel* GetViewModel() const;

protected:
	virtual void NativeRefreshFromViewModel() override;

private:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> TitleText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> HealthText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonNumericTextBlock> CurrentHealthValueText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonNumericTextBlock> MaxHealthValueText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> MoveBudgetText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonNumericTextBlock> MoveBudgetValueText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonNumericTextBlock> MaxMoveBudgetValueText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> MoveBudgetBar;
};

UCLASS(Blueprintable)
class FORTROGUE_API UFRAimWindIndicatorWidget : public UFRBattleHUDModuleWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FortRogue|UI|HUD")
	void SetViewModel(UFRAimWindViewModel* InViewModel);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD")
	UFRAimWindViewModel* GetViewModel() const;

protected:
	virtual void NativeRefreshFromViewModel() override;

private:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> WindText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonNumericTextBlock> WindValueText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonNumericTextBlock> AbsoluteWindValueText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> AimText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonNumericTextBlock> AimAngleValueText;
};

UCLASS(Blueprintable)
class FORTROGUE_API UFRShotPowerMeterWidget : public UFRBattleHUDModuleWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FortRogue|UI|HUD")
	void SetViewModel(UFRShotPowerViewModel* InViewModel);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD")
	UFRShotPowerViewModel* GetViewModel() const;

protected:
	virtual void NativeRefreshFromViewModel() override;

private:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> ShotPowerText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonNumericTextBlock> ShotPowerValueText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> ShotPowerBar;
};

UCLASS(Blueprintable)
class FORTROGUE_API UFRLoadoutBarWidget : public UFRBattleHUDModuleWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FortRogue|UI|HUD")
	void SetViewModel(UFRLoadoutViewModel* InViewModel);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD")
	UFRLoadoutViewModel* GetViewModel() const;

protected:
	virtual void NativeRefreshFromViewModel() override;

private:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> CurrentWeaponText;
};

UCLASS(Blueprintable)
class FORTROGUE_API UFRShotInfoPanelWidget : public UFRBattleHUDModuleWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FortRogue|UI|HUD")
	void SetViewModel(UFRShotPreviewViewModel* InViewModel);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD")
	UFRShotPreviewViewModel* GetViewModel() const;

protected:
	virtual void NativeRefreshFromViewModel() override;

private:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> PrimaryText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> SecondaryText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonNumericTextBlock> DamageValueText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonNumericTextBlock> BlastRadiusValueText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonNumericTextBlock> ProjectileCountValueText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonNumericTextBlock> LaunchSpeedValueText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonNumericTextBlock> GravityValueText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonNumericTextBlock> TerrainDamageValueText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonNumericTextBlock> TerrainFillRadiusValueText;
};

UCLASS(Blueprintable)
class FORTROGUE_API UFRModifierSummaryWidget : public UFRBattleHUDModuleWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FortRogue|UI|HUD")
	void SetViewModel(UFRModifierSummaryViewModel* InViewModel);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD")
	UFRModifierSummaryViewModel* GetViewModel() const;

protected:
	virtual void NativeRefreshFromViewModel() override;

private:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> GrantedModifierText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> PendingModifierText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> AbilitySetText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> SummaryText;
};
