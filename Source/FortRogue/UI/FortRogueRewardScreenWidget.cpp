// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FortRogueRewardScreenWidget.h"

#include "FortRogueGameMode.h"

AFortRogueGameMode* UFortRogueRewardScreenWidget::GetFortRogueGameMode() const
{
	return GetWorld() ? GetWorld()->GetAuthGameMode<AFortRogueGameMode>() : nullptr;
}

TArray<FFortRogueRewardChoice> UFortRogueRewardScreenWidget::GetRewardChoices() const
{
	if (AFortRogueGameMode* GameMode = GetFortRogueGameMode())
	{
		return GameMode->GetRewardChoices();
	}
	return TArray<FFortRogueRewardChoice>();
}

int32 UFortRogueRewardScreenWidget::GetRewardChoiceCount() const
{
	if (AFortRogueGameMode* GameMode = GetFortRogueGameMode())
	{
		return GameMode->GetRewardChoiceCount();
	}
	return 0;
}

FFortRogueRewardChoice UFortRogueRewardScreenWidget::GetRewardChoice(int32 ChoiceIndex) const
{
	if (AFortRogueGameMode* GameMode = GetFortRogueGameMode())
	{
		return GameMode->GetRewardChoice(ChoiceIndex);
	}
	return FFortRogueRewardChoice();
}

FText UFortRogueRewardScreenWidget::GetRewardChoiceSummary(int32 ChoiceIndex) const
{
	if (AFortRogueGameMode* GameMode = GetFortRogueGameMode())
	{
		return GameMode->GetRewardChoiceSummary(ChoiceIndex);
	}
	return FText::GetEmpty();
}

FGameplayTagContainer UFortRogueRewardScreenWidget::GetChosenRewardTags() const
{
	if (AFortRogueGameMode* GameMode = GetFortRogueGameMode())
	{
		return GameMode->GetChosenRewardTags();
	}
	return FGameplayTagContainer();
}

bool UFortRogueRewardScreenWidget::CanChooseReward(int32 ChoiceIndex) const
{
	if (AFortRogueGameMode* GameMode = GetFortRogueGameMode())
	{
		return GameMode->CanApplyRewardChoice(ChoiceIndex);
	}
	return false;
}

void UFortRogueRewardScreenWidget::ChooseReward(int32 ChoiceIndex)
{
	if (AFortRogueGameMode* GameMode = GetFortRogueGameMode())
	{
		if (GameMode->CanApplyRewardChoice(ChoiceIndex))
		{
			GameMode->ApplyRewardChoice(ChoiceIndex);
		}
	}
}
