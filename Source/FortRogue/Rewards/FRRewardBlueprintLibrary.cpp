// Copyright Epic Games, Inc. All Rights Reserved.

#include "Rewards/FRRewardBlueprintLibrary.h"

#include "AbilitySystem/FRAbilitySet.h"
#include "Items/FRItemDefinition.h"
#include "Perks/FRPerkDefinition.h"
#include "Run/FRDefaultLoadoutDefinition.h"
#include "Run/FRStageRunDefinition.h"
#include "Weapons/FRWeaponDefinition.h"

FText UFRRewardBlueprintLibrary::GetRewardEffectSummary(const FFRRewardChoice& RewardChoice)
{
	return RewardChoice.GetEffectSummary();
}

FText UFRRewardBlueprintLibrary::GetRewardDataValidationSummary(const FFRRewardChoice& RewardChoice)
{
	return RewardChoice.GetDataValidationSummary();
}

FText UFRRewardBlueprintLibrary::GetStageRunDataValidationSummary(UFRStageRunDefinition* StageRunDefinition)
{
	return StageRunDefinition ? StageRunDefinition->GetDataValidationSummary() : FText::FromString(TEXT("missing stage run"));
}

FText UFRRewardBlueprintLibrary::GetDefaultLoadoutDataValidationSummary(UFRDefaultLoadoutDefinition* LoadoutDefinition)
{
	return LoadoutDefinition ? LoadoutDefinition->GetDataValidationSummary() : FText::FromString(TEXT("missing loadout"));
}

FText UFRRewardBlueprintLibrary::GetWeaponEffectSummary(UFRWeaponDefinition* WeaponDefinition)
{
	FFRRewardChoice RewardChoice;
	RewardChoice.WeaponReward = WeaponDefinition;
	return RewardChoice.GetEffectSummary();
}

FText UFRRewardBlueprintLibrary::GetWeaponDataValidationSummary(UFRWeaponDefinition* WeaponDefinition)
{
	return WeaponDefinition ? WeaponDefinition->GetDataValidationSummary() : FText::FromString(TEXT("missing weapon"));
}

FText UFRRewardBlueprintLibrary::GetItemEffectSummary(UFRItemDefinition* ItemDefinition)
{
	FFRRewardChoice RewardChoice;
	RewardChoice.ItemReward = ItemDefinition;
	return RewardChoice.GetEffectSummary();
}

FText UFRRewardBlueprintLibrary::GetItemDataValidationSummary(UFRItemDefinition* ItemDefinition)
{
	return ItemDefinition ? ItemDefinition->GetDataValidationSummary() : FText::FromString(TEXT("missing item"));
}

FText UFRRewardBlueprintLibrary::GetPerkEffectSummary(UFRPerkDefinition* PerkDefinition)
{
	FFRRewardChoice RewardChoice;
	RewardChoice.PerkReward = PerkDefinition;
	return RewardChoice.GetEffectSummary();
}

FText UFRRewardBlueprintLibrary::GetPerkDataValidationSummary(UFRPerkDefinition* PerkDefinition)
{
	return PerkDefinition ? PerkDefinition->GetDataValidationSummary() : FText::FromString(TEXT("missing perk"));
}

FText UFRRewardBlueprintLibrary::GetAbilitySetEffectSummary(UFRAbilitySet* AbilitySet)
{
	return AbilitySet ? AbilitySet->GetEffectSummary() : FText::GetEmpty();
}

FText UFRRewardBlueprintLibrary::GetAbilitySetDataValidationSummary(UFRAbilitySet* AbilitySet)
{
	return AbilitySet ? AbilitySet->GetDataValidationSummary() : FText::FromString(TEXT("missing ability set"));
}

FText UFRRewardBlueprintLibrary::GetShotModifierEffectSummary(const TArray<FFRShotModifierSpec>& ShotModifiers)
{
	return GetFortRogueShotModifierEffectSummary(ShotModifiers);
}

FText UFRRewardBlueprintLibrary::GetShotModifierDataValidationSummary(const FFRShotModifierSpec& ShotModifier)
{
	return ShotModifier.GetDataValidationSummary();
}

bool UFRRewardBlueprintLibrary::DoesShotModifierMeetShotConditions(const FFRShotModifierSpec& ShotModifier, const FFRShotSpec& CurrentShotSpec, float CurrentAimAngle, float Wind, bool bShotFacingRight)
{
	return ShotModifier.MeetsShotConditions(CurrentShotSpec, CurrentAimAngle, Wind, bShotFacingRight);
}

FText UFRRewardBlueprintLibrary::GetShotModifierConditionFailureSummary(const FFRShotModifierSpec& ShotModifier, const FFRShotSpec& CurrentShotSpec, float CurrentAimAngle, float Wind, bool bShotFacingRight)
{
	return ShotModifier.GetShotConditionFailureSummary(CurrentShotSpec, CurrentAimAngle, Wind, bShotFacingRight);
}

bool UFRRewardBlueprintLibrary::DoesRewardMeetTagConditions(const FFRRewardChoice& RewardChoice, const FGameplayTagContainer& ChosenRewardTags)
{
	return RewardChoice.MeetsRewardTagConditions(ChosenRewardTags);
}

FText UFRRewardBlueprintLibrary::GetRewardTagConditionFailureSummary(const FFRRewardChoice& RewardChoice, const FGameplayTagContainer& ChosenRewardTags)
{
	return RewardChoice.GetRewardTagConditionFailureSummary(ChosenRewardTags);
}
