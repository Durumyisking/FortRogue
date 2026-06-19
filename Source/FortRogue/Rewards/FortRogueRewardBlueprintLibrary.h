// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Combat/FortRogueShotSpec.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Rewards/FortRogueRewardTypes.h"
#include "FortRogueRewardBlueprintLibrary.generated.h"

class UFortRogueAbilitySet;
class UFortRogueItemDefinition;
class UFortRoguePerkDefinition;
class UFortRogueWeaponDefinition;

UCLASS()
class FORTROGUE_API UFortRogueRewardBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	static FText GetRewardEffectSummary(const FFortRogueRewardChoice& RewardChoice);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	static FText GetRewardDataValidationSummary(const FFortRogueRewardChoice& RewardChoice);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	static FText GetWeaponEffectSummary(UFortRogueWeaponDefinition* WeaponDefinition);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	static FText GetItemEffectSummary(UFortRogueItemDefinition* ItemDefinition);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	static FText GetPerkEffectSummary(UFortRoguePerkDefinition* PerkDefinition);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	static FText GetAbilitySetEffectSummary(UFortRogueAbilitySet* AbilitySet);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	static FText GetAbilitySetDataValidationSummary(UFortRogueAbilitySet* AbilitySet);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	static FText GetShotModifierEffectSummary(const TArray<FFortRogueShotModifierSpec>& ShotModifiers);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	static bool DoesShotModifierMeetShotConditions(const FFortRogueShotModifierSpec& ShotModifier, const FFortRogueShotSpec& CurrentShotSpec, float CurrentAimAngle, float Wind, bool bShotFacingRight);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	static FText GetShotModifierConditionFailureSummary(const FFortRogueShotModifierSpec& ShotModifier, const FFortRogueShotSpec& CurrentShotSpec, float CurrentAimAngle, float Wind, bool bShotFacingRight);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	static bool DoesRewardMeetTagConditions(const FFortRogueRewardChoice& RewardChoice, const FGameplayTagContainer& ChosenRewardTags);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	static FText GetRewardTagConditionFailureSummary(const FFortRogueRewardChoice& RewardChoice, const FGameplayTagContainer& ChosenRewardTags);
};
