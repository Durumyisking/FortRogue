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

FText UFortRogueRewardBlueprintLibrary::GetRewardDataValidationSummary(const FFortRogueRewardChoice& RewardChoice)
{
	return RewardChoice.GetDataValidationSummary();
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

FText UFortRogueRewardBlueprintLibrary::GetAbilitySetDataValidationSummary(UFortRogueAbilitySet* AbilitySet)
{
	return AbilitySet ? AbilitySet->GetDataValidationSummary() : FText::FromString(TEXT("missing ability set"));
}

FText UFortRogueRewardBlueprintLibrary::GetShotModifierEffectSummary(const TArray<FFortRogueShotModifierSpec>& ShotModifiers)
{
	FFortRogueRewardChoice RewardChoice;
	RewardChoice.ShotModifiers = ShotModifiers;
	return RewardChoice.GetEffectSummary();
}

bool UFortRogueRewardBlueprintLibrary::DoesShotModifierMeetShotConditions(const FFortRogueShotModifierSpec& ShotModifier, const FFortRogueShotSpec& CurrentShotSpec, float CurrentAimAngle, float Wind, bool bShotFacingRight)
{
	return ShotModifier.MeetsShotConditions(CurrentShotSpec, CurrentAimAngle, Wind, bShotFacingRight);
}

FText UFortRogueRewardBlueprintLibrary::GetShotModifierConditionFailureSummary(const FFortRogueShotModifierSpec& ShotModifier, const FFortRogueShotSpec& CurrentShotSpec, float CurrentAimAngle, float Wind, bool bShotFacingRight)
{
	return ShotModifier.GetShotConditionFailureSummary(CurrentShotSpec, CurrentAimAngle, Wind, bShotFacingRight);
}

bool UFortRogueRewardBlueprintLibrary::DoesRewardMeetTagConditions(const FFortRogueRewardChoice& RewardChoice, const FGameplayTagContainer& ChosenRewardTags)
{
	return RewardChoice.MeetsRewardTagConditions(ChosenRewardTags);
}

FText UFortRogueRewardBlueprintLibrary::GetRewardTagConditionFailureSummary(const FFortRogueRewardChoice& RewardChoice, const FGameplayTagContainer& ChosenRewardTags)
{
	return RewardChoice.GetRewardTagConditionFailureSummary(ChosenRewardTags);
}
