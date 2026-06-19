// Copyright Epic Games, Inc. All Rights Reserved.

#include "Rewards/FortRogueRewardBlueprintLibrary.h"

#include "AbilitySystem/FortRogueAbilitySet.h"
#include "Items/FortRogueItemDefinition.h"
#include "Perks/FortRoguePerkDefinition.h"
#include "Run/FortRogueDefaultLoadoutDefinition.h"
#include "Run/FortRogueStageRunDefinition.h"
#include "Weapons/FortRogueWeaponDefinition.h"

FText UFortRogueRewardBlueprintLibrary::GetRewardEffectSummary(const FFortRogueRewardChoice& RewardChoice)
{
	return RewardChoice.GetEffectSummary();
}

FText UFortRogueRewardBlueprintLibrary::GetRewardDataValidationSummary(const FFortRogueRewardChoice& RewardChoice)
{
	return RewardChoice.GetDataValidationSummary();
}

FText UFortRogueRewardBlueprintLibrary::GetStageRunDataValidationSummary(UFortRogueStageRunDefinition* StageRunDefinition)
{
	return StageRunDefinition ? StageRunDefinition->GetDataValidationSummary() : FText::FromString(TEXT("missing stage run"));
}

FText UFortRogueRewardBlueprintLibrary::GetDefaultLoadoutDataValidationSummary(UFortRogueDefaultLoadoutDefinition* LoadoutDefinition)
{
	return LoadoutDefinition ? LoadoutDefinition->GetDataValidationSummary() : FText::FromString(TEXT("missing loadout"));
}

FText UFortRogueRewardBlueprintLibrary::GetWeaponEffectSummary(UFortRogueWeaponDefinition* WeaponDefinition)
{
	FFortRogueRewardChoice RewardChoice;
	RewardChoice.WeaponReward = WeaponDefinition;
	return RewardChoice.GetEffectSummary();
}

FText UFortRogueRewardBlueprintLibrary::GetWeaponDataValidationSummary(UFortRogueWeaponDefinition* WeaponDefinition)
{
	return WeaponDefinition ? WeaponDefinition->GetDataValidationSummary() : FText::FromString(TEXT("missing weapon"));
}

FText UFortRogueRewardBlueprintLibrary::GetItemEffectSummary(UFortRogueItemDefinition* ItemDefinition)
{
	FFortRogueRewardChoice RewardChoice;
	RewardChoice.ItemReward = ItemDefinition;
	return RewardChoice.GetEffectSummary();
}

FText UFortRogueRewardBlueprintLibrary::GetItemDataValidationSummary(UFortRogueItemDefinition* ItemDefinition)
{
	return ItemDefinition ? ItemDefinition->GetDataValidationSummary() : FText::FromString(TEXT("missing item"));
}

FText UFortRogueRewardBlueprintLibrary::GetPerkEffectSummary(UFortRoguePerkDefinition* PerkDefinition)
{
	FFortRogueRewardChoice RewardChoice;
	RewardChoice.PerkReward = PerkDefinition;
	return RewardChoice.GetEffectSummary();
}

FText UFortRogueRewardBlueprintLibrary::GetPerkDataValidationSummary(UFortRoguePerkDefinition* PerkDefinition)
{
	return PerkDefinition ? PerkDefinition->GetDataValidationSummary() : FText::FromString(TEXT("missing perk"));
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
	return GetFortRogueShotModifierEffectSummary(ShotModifiers);
}

FText UFortRogueRewardBlueprintLibrary::GetShotModifierDataValidationSummary(const FFortRogueShotModifierSpec& ShotModifier)
{
	return ShotModifier.GetDataValidationSummary();
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
