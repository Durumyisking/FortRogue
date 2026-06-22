// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonActivatableWidget.h"
#include "CommonButtonBase.h"
#include "MVVMViewModelBase.h"
#include "Rewards/FRRewardTypes.h"
#include "FRRewardScreenWidget.generated.h"

class UCommonNumericTextBlock;
class UCommonTextBlock;
class UPanelWidget;
class UFRRewardScreenWidget;

UCLASS(BlueprintType)
class FORTROGUE_API UFRRewardChoiceViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|Reward")
	int32 GetChoiceIndex() const { return ChoiceIndex; }
	void SetChoiceIndex(int32 InChoiceIndex);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|Reward")
	const FText& GetTitleText() const { return TitleText; }
	void SetTitleText(const FText& InTitleText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|Reward")
	const FText& GetSummaryText() const { return SummaryText; }
	void SetSummaryText(const FText& InSummaryText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|Reward")
	const FText& GetConditionText() const { return ConditionText; }
	void SetConditionText(const FText& InConditionText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|Reward")
	bool CanChoose() const { return bCanChoose; }
	void SetCanChoose(bool bInCanChoose);

private:
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|Reward", meta = (AllowPrivateAccess = "true"))
	int32 ChoiceIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|Reward", meta = (AllowPrivateAccess = "true"))
	FText TitleText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|Reward", meta = (AllowPrivateAccess = "true"))
	FText SummaryText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|Reward", meta = (AllowPrivateAccess = "true"))
	FText ConditionText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|Reward", meta = (AllowPrivateAccess = "true"))
	bool bCanChoose = false;
};

UCLASS(BlueprintType)
class FORTROGUE_API UFRRewardScreenViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|Reward")
	const FText& GetTitleText() const { return TitleText; }
	void SetTitleText(const FText& InTitleText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|Reward")
	UFRRewardChoiceViewModel* GetChoiceViewModel(int32 ChoiceIndex) const;

	UFRRewardChoiceViewModel* GetOrCreateChoiceViewModel(int32 ChoiceIndex);
	void SetChoiceCount(int32 ChoiceCount);

private:
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|Reward", meta = (AllowPrivateAccess = "true"))
	FText TitleText;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|UI|Reward", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UFRRewardChoiceViewModel>> ChoiceViewModels;
};

UCLASS(Blueprintable)
class FORTROGUE_API UFRRewardChoiceButtonWidget : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FortRogue|UI|Reward")
	void SetViewModel(UFRRewardChoiceViewModel* InViewModel);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|Reward")
	UFRRewardChoiceViewModel* GetViewModel() const { return RewardChoiceViewModel; }

	void SetRewardScreen(UFRRewardScreenWidget* InRewardScreen);
	void RefreshFromViewModel();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnClicked() override;

private:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonNumericTextBlock> ChoiceIndexText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> TitleText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> SummaryText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> ConditionText;

	UPROPERTY(Transient)
	TObjectPtr<UFRRewardScreenWidget> RewardScreen;

	UPROPERTY(Transient)
	TObjectPtr<UFRRewardChoiceViewModel> RewardChoiceViewModel;
};

UCLASS(Abstract, Blueprintable)
class FORTROGUE_API UFRRewardScreenWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FortRogue|UI")
	class AFRGameMode* GetFortRogueGameMode() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	TArray<FFRRewardChoice> GetRewardChoices() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	int32 GetRewardChoiceCount() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FFRRewardChoice GetRewardChoice(int32 ChoiceIndex) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FText GetRewardChoiceSummary(int32 ChoiceIndex) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FText GetRewardChoiceConditionFailureSummary(int32 ChoiceIndex) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FGameplayTagContainer GetChosenRewardTags() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	bool CanChooseReward(int32 ChoiceIndex) const;

	UFUNCTION(BlueprintCallable, Category = "FortRogue|UI")
	void ChooseReward(int32 ChoiceIndex);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|MVVM")
	UFRRewardScreenViewModel* GetRewardScreenViewModel() const { return RewardScreenViewModel; }

	UFUNCTION(BlueprintNativeEvent, Category = "FortRogue|UI")
	void RefreshRewardScreen();

protected:
	virtual void NativeOnInitialized() override;

private:
	void CreateViewModel();
	void RefreshViewModel();
	void RefreshRewardChoiceButtons();

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> RewardChoicePanel;

	UPROPERTY(Transient)
	TObjectPtr<UFRRewardScreenViewModel> RewardScreenViewModel;
};
