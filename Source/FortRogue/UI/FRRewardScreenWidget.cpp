// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FRRewardScreenWidget.h"

#include "FRGameMode.h"

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
		}
	}
}
