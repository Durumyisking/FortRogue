// Copyright Epic Games, Inc. All Rights Reserved.

#include "Rewards/FortRogueRewardBlueprintLibrary.h"

#include "AbilitySystem/FortRogueAbilitySet.h"
#include "Items/FortRogueItemDefinition.h"
#include "Perks/FortRoguePerkDefinition.h"
#include "Weapons/FortRogueWeaponDefinition.h"

FText UFortRogueRewardBlueprintLibrary::GetRewardEffectSummary(const FFortRogueRewardChoice& RewardChoice)
{
	return RewardChoice.GetEffectSummary();
}

FText UFortRogueRewardBlueprintLibrary::GetWeaponEffectSummary(UFortRogueWeaponDefinition* WeaponDefinition)
{
	FFortRogueRewardChoice RewardChoice;
	RewardChoice.WeaponReward = WeaponDefinition;
	return RewardChoice.GetEffectSummary();
}

FText UFortRogueRewardBlueprintLibrary::GetItemEffectSummary(UFortRogueItemDefinition* ItemDefinition)
{
	FFortRogueRewardChoice RewardChoice;
	RewardChoice.ItemReward = ItemDefinition;
	return RewardChoice.GetEffectSummary();
}

FText UFortRogueRewardBlueprintLibrary::GetPerkEffectSummary(UFortRoguePerkDefinition* PerkDefinition)
{
	FFortRogueRewardChoice RewardChoice;
	RewardChoice.PerkReward = PerkDefinition;
	return RewardChoice.GetEffectSummary();
}

FText UFortRogueRewardBlueprintLibrary::GetAbilitySetEffectSummary(UFortRogueAbilitySet* AbilitySet)
{
	return AbilitySet ? AbilitySet->GetEffectSummary() : FText::GetEmpty();
}
