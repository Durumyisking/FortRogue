// Copyright Epic Games, Inc. All Rights Reserved.

#include "Rewards/FortRogueRewardBlueprintLibrary.h"

FText UFortRogueRewardBlueprintLibrary::GetRewardEffectSummary(const FFortRogueRewardChoice& RewardChoice)
{
	return RewardChoice.GetEffectSummary();
}
