// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FortRogueRewardScreenWidget.h"

#include "FortRogueGameMode.h"

AFortRogueGameMode* UFortRogueRewardScreenWidget::GetFortRogueGameMode() const
{
	return GetWorld() ? GetWorld()->GetAuthGameMode<AFortRogueGameMode>() : nullptr;
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
