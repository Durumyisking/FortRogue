// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FRRewardScreenWidget.h"

#include "CommonNumericTextBlock.h"
#include "CommonTextBlock.h"
#include "Components/PanelWidget.h"
#include "FRGameMode.h"
#include "MVVMBlueprintLibrary.h"

namespace
{
	void SetRewardText(UCommonTextBlock* TextBlock, const FText& Text)
	{
		if (TextBlock)
		{
			TextBlock->SetText(Text);
		}
	}

	void ApplyRewardTextStyle(UCommonTextBlock* TextBlock, TSubclassOf<UCommonTextStyle> TextStyle)
	{
		if (TextBlock && TextStyle)
		{
			TextBlock->SetStyle(TextStyle);
		}
	}

	TArray<UFRRewardChoiceButtonWidget*> GetRewardChoiceButtons(UPanelWidget* Panel)
	{
		TArray<UFRRewardChoiceButtonWidget*> Buttons;
		if (!Panel)
		{
			return Buttons;
		}

		for (int32 Index = 0; Index < Panel->GetChildrenCount(); ++Index)
		{
			if (UFRRewardChoiceButtonWidget* Button = Cast<UFRRewardChoiceButtonWidget>(Panel->GetChildAt(Index)))
			{
				Buttons.Add(Button);
			}
		}
		return Buttons;
	}
}

void UFRRewardChoiceViewModel::SetChoiceIndex(int32 InChoiceIndex)
{
	UE_MVVM_SET_PROPERTY_VALUE(ChoiceIndex, InChoiceIndex);
}

void UFRRewardChoiceViewModel::SetTitleText(const FText& InTitleText)
{
	UE_MVVM_SET_PROPERTY_VALUE(TitleText, InTitleText);
}

void UFRRewardChoiceViewModel::SetSummaryText(const FText& InSummaryText)
{
	UE_MVVM_SET_PROPERTY_VALUE(SummaryText, InSummaryText);
}

void UFRRewardChoiceViewModel::SetConditionText(const FText& InConditionText)
{
	UE_MVVM_SET_PROPERTY_VALUE(ConditionText, InConditionText);
}

void UFRRewardChoiceViewModel::SetCanChoose(bool bInCanChoose)
{
	UE_MVVM_SET_PROPERTY_VALUE(bCanChoose, bInCanChoose);
}

void UFRRewardScreenViewModel::SetTitleText(const FText& InTitleText)
{
	UE_MVVM_SET_PROPERTY_VALUE(TitleText, InTitleText);
}

UFRRewardChoiceViewModel* UFRRewardScreenViewModel::GetChoiceViewModel(int32 ChoiceIndex) const
{
	return ChoiceViewModels.IsValidIndex(ChoiceIndex) ? ChoiceViewModels[ChoiceIndex] : nullptr;
}

UFRRewardChoiceViewModel* UFRRewardScreenViewModel::GetOrCreateChoiceViewModel(int32 ChoiceIndex)
{
	if (ChoiceIndex < 0)
	{
		return nullptr;
	}

	SetChoiceCount(ChoiceIndex + 1);
	return ChoiceViewModels[ChoiceIndex];
}

void UFRRewardScreenViewModel::SetChoiceCount(int32 ChoiceCount)
{
	const int32 TargetCount = FMath::Max(0, ChoiceCount);
	while (ChoiceViewModels.Num() < TargetCount)
	{
		ChoiceViewModels.Add(NewObject<UFRRewardChoiceViewModel>(this));
	}
	if (ChoiceViewModels.Num() > TargetCount)
	{
		ChoiceViewModels.SetNum(TargetCount);
	}
}

void UFRRewardChoiceButtonWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ApplyRewardStyleSet();
	RefreshFromViewModel();
}

void UFRRewardChoiceButtonWidget::ApplyRewardStyleSet()
{
	ApplyRewardTextStyle(ChoiceIndexText, RewardStyleSet.ChoiceIndexTextStyle);
	ApplyRewardTextStyle(TitleText, RewardStyleSet.ChoiceTitleTextStyle);
	ApplyRewardTextStyle(SummaryText, RewardStyleSet.ChoiceSummaryTextStyle);
	ApplyRewardTextStyle(ConditionText, RewardStyleSet.ChoiceConditionTextStyle);
	if (RewardStyleSet.ChoiceButtonStyle)
	{
		SetStyle(RewardStyleSet.ChoiceButtonStyle);
	}
}

void UFRRewardChoiceButtonWidget::SetRewardScreen(UFRRewardScreenWidget* InRewardScreen)
{
	RewardScreen = InRewardScreen;
}

void UFRRewardChoiceButtonWidget::SetViewModel(UFRRewardChoiceViewModel* InViewModel)
{
	if (RewardChoiceViewModel == InViewModel)
	{
		return;
	}

	RewardChoiceViewModel = InViewModel;
	RefreshFromViewModel();
}

void UFRRewardChoiceButtonWidget::RefreshFromViewModel()
{
	if (!RewardChoiceViewModel)
	{
		SetRewardText(TitleText, FText::FromString(TEXT("-")));
		SetRewardText(SummaryText, FText::GetEmpty());
		SetRewardText(ConditionText, FText::FromString(TEXT("UNAVAILABLE")));
		if (ChoiceIndexText)
		{
			ChoiceIndexText->SetCurrentValue(0.0f);
		}
		SetIsEnabled(false);
		return;
	}

	SetRewardText(TitleText, RewardChoiceViewModel->GetTitleText());
	SetRewardText(SummaryText, RewardChoiceViewModel->GetSummaryText());
	SetRewardText(ConditionText, RewardChoiceViewModel->GetConditionText());
	if (ChoiceIndexText)
	{
		ChoiceIndexText->SetCurrentValue(static_cast<float>(RewardChoiceViewModel->GetChoiceIndex() + 1));
	}
	if (ConditionText)
	{
		ConditionText->SetVisibility(RewardChoiceViewModel->GetConditionText().IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
	}
	SetIsEnabled(RewardChoiceViewModel->CanChoose());
}

void UFRRewardChoiceButtonWidget::NativeOnClicked()
{
	Super::NativeOnClicked();

	if (RewardScreen && RewardChoiceViewModel && RewardChoiceViewModel->CanChoose())
	{
		RewardScreen->ChooseReward(RewardChoiceViewModel->GetChoiceIndex());
	}
}

void UFRRewardScreenWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	CreateViewModel();
	if (RewardScreenViewModel)
	{
		TScriptInterface<INotifyFieldValueChanged> ViewModelInterface;
		ViewModelInterface.SetObject(RewardScreenViewModel);
		ViewModelInterface.SetInterface(Cast<INotifyFieldValueChanged>(RewardScreenViewModel));
		UMVVMBlueprintLibrary::SetViewModelByClass(this, ViewModelInterface);
	}
	RefreshRewardScreen();
}

void UFRRewardScreenWidget::CreateViewModel()
{
	if (!RewardScreenViewModel)
	{
		RewardScreenViewModel = NewObject<UFRRewardScreenViewModel>(this);
	}
}

void UFRRewardScreenWidget::RefreshRewardScreen_Implementation()
{
	RefreshViewModel();
	RefreshFromViewModel();
	RefreshRewardChoiceButtons();
	ApplyRewardScreenStyleSet();
}

void UFRRewardScreenWidget::RefreshViewModel()
{
	CreateViewModel();
	if (!RewardScreenViewModel)
	{
		return;
	}

	RewardScreenViewModel->SetTitleText(FText::FromString(TEXT("Choose Reward")));

	AFRGameMode* GameMode = GetFortRogueGameMode();
	const int32 ChoiceCount = GameMode ? GameMode->GetRewardChoiceCount() : 0;
	RewardScreenViewModel->SetChoiceCount(ChoiceCount);
	for (int32 Index = 0; Index < ChoiceCount; ++Index)
	{
		UFRRewardChoiceViewModel* ChoiceViewModel = RewardScreenViewModel->GetOrCreateChoiceViewModel(Index);
		if (!ChoiceViewModel || !GameMode)
		{
			continue;
		}

		const FFRRewardChoice RewardChoice = GameMode->GetRewardChoice(Index);
		const bool bCanChoose = GameMode->CanApplyRewardChoice(Index);
		ChoiceViewModel->SetChoiceIndex(Index);
		ChoiceViewModel->SetTitleText(RewardChoice.DisplayName.IsEmpty()
			? FText::FromString(FString::Printf(TEXT("Reward %d"), Index + 1))
			: RewardChoice.DisplayName);
		ChoiceViewModel->SetSummaryText(GameMode->GetRewardChoiceSummary(Index));
		ChoiceViewModel->SetConditionText(bCanChoose ? FText::GetEmpty() : GameMode->GetRewardChoiceConditionFailureSummary(Index));
		ChoiceViewModel->SetCanChoose(bCanChoose);
	}
}

void UFRRewardScreenWidget::RefreshFromViewModel()
{
	if (!RewardScreenViewModel)
	{
		return;
	}

	SetRewardText(TitleText, RewardScreenViewModel->GetTitleText());
}

void UFRRewardScreenWidget::ApplyRewardScreenStyleSet()
{
	ApplyRewardTextStyle(TitleText, RewardStyleSet.ScreenTitleTextStyle);
}

void UFRRewardScreenWidget::RefreshRewardChoiceButtons()
{
	TArray<UFRRewardChoiceButtonWidget*> RewardChoiceButtons = GetRewardChoiceButtons(RewardChoicePanel);
	for (int32 Index = 0; Index < RewardChoiceButtons.Num(); ++Index)
	{
		RewardChoiceButtons[Index]->SetRewardScreen(this);
		RewardChoiceButtons[Index]->SetViewModel(RewardScreenViewModel ? RewardScreenViewModel->GetChoiceViewModel(Index) : nullptr);
	}
}

AFRGameMode* UFRRewardScreenWidget::GetFortRogueGameMode() const
{
	return GetWorld() ? GetWorld()->GetAuthGameMode<AFRGameMode>() : nullptr;
}

TArray<FFRRewardChoice> UFRRewardScreenWidget::GetRewardChoices() const
{
	if (AFRGameMode* GameMode = GetFortRogueGameMode())
	{
		return GameMode->GetRewardChoices();
	}
	return TArray<FFRRewardChoice>();
}

int32 UFRRewardScreenWidget::GetRewardChoiceCount() const
{
	if (AFRGameMode* GameMode = GetFortRogueGameMode())
	{
		return GameMode->GetRewardChoiceCount();
	}
	return 0;
}

FFRRewardChoice UFRRewardScreenWidget::GetRewardChoice(int32 ChoiceIndex) const
{
	if (AFRGameMode* GameMode = GetFortRogueGameMode())
	{
		return GameMode->GetRewardChoice(ChoiceIndex);
	}
	return FFRRewardChoice();
}

FText UFRRewardScreenWidget::GetRewardChoiceSummary(int32 ChoiceIndex) const
{
	if (AFRGameMode* GameMode = GetFortRogueGameMode())
	{
		return GameMode->GetRewardChoiceSummary(ChoiceIndex);
	}
	return FText::GetEmpty();
}

FText UFRRewardScreenWidget::GetRewardChoiceConditionFailureSummary(int32 ChoiceIndex) const
{
	if (AFRGameMode* GameMode = GetFortRogueGameMode())
	{
		return GameMode->GetRewardChoiceConditionFailureSummary(ChoiceIndex);
	}
	return FText::GetEmpty();
}

FGameplayTagContainer UFRRewardScreenWidget::GetChosenRewardTags() const
{
	if (AFRGameMode* GameMode = GetFortRogueGameMode())
	{
		return GameMode->GetChosenRewardTags();
	}
	return FGameplayTagContainer();
}

bool UFRRewardScreenWidget::CanChooseReward(int32 ChoiceIndex) const
{
	if (AFRGameMode* GameMode = GetFortRogueGameMode())
	{
		return GameMode->CanApplyRewardChoice(ChoiceIndex);
	}
	return false;
}

void UFRRewardScreenWidget::ChooseReward(int32 ChoiceIndex)
{
	if (AFRGameMode* GameMode = GetFortRogueGameMode())
	{
		if (GameMode->CanApplyRewardChoice(ChoiceIndex))
		{
			GameMode->ApplyRewardChoice(ChoiceIndex);
			RefreshRewardScreen();
		}
	}
}
