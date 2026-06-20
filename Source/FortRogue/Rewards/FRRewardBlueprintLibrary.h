// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Combat/FRShotSpec.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Rewards/FRRewardTypes.h"
#include "FRRewardBlueprintLibrary.generated.h"

class UFRAbilitySet;
class UFRDefaultLoadoutDefinition;
class UFRItemDefinition;
class UFRPerkDefinition;
class UFRStageRunDefinition;
class UFRWeaponDefinition;

UCLASS()
class FORTROGUE_API UFRRewardBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	static FText GetRewardEffectSummary(const FFRRewardChoice& RewardChoice);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	static FText GetRewardDataValidationSummary(const FFRRewardChoice& RewardChoice);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	static FText GetStageRunDataValidationSummary(UFRStageRunDefinition* StageRunDefinition);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	static FText GetDefaultLoadoutDataValidationSummary(UFRDefaultLoadoutDefinition* LoadoutDefinition);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	static FText GetWeaponEffectSummary(UFRWeaponDefinition* WeaponDefinition);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	static FText GetWeaponDataValidationSummary(UFRWeaponDefinition* WeaponDefinition);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	static FText GetItemEffectSummary(UFRItemDefinition* ItemDefinition);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	static FText GetItemDataValidationSummary(UFRItemDefinition* ItemDefinition);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	static FText GetPerkEffectSummary(UFRPerkDefinition* PerkDefinition);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	static FText GetPerkDataValidationSummary(UFRPerkDefinition* PerkDefinition);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	static FText GetAbilitySetEffectSummary(UFRAbilitySet* AbilitySet);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	static FText GetAbilitySetDataValidationSummary(UFRAbilitySet* AbilitySet);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	static FText GetShotModifierEffectSummary(const TArray<FFRShotModifierSpec>& ShotModifiers);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	static FText GetShotModifierDataValidationSummary(const FFRShotModifierSpec& ShotModifier);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	static bool DoesShotModifierMeetShotConditions(const FFRShotModifierSpec& ShotModifier, const FFRShotSpec& CurrentShotSpec, float CurrentAimAngle, float Wind, bool bShotFacingRight);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	static FText GetShotModifierConditionFailureSummary(const FFRShotModifierSpec& ShotModifier, const FFRShotSpec& CurrentShotSpec, float CurrentAimAngle, float Wind, bool bShotFacingRight);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	static bool DoesRewardMeetTagConditions(const FFRRewardChoice& RewardChoice, const FGameplayTagContainer& ChosenRewardTags);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	static FText GetRewardTagConditionFailureSummary(const FFRRewardChoice& RewardChoice, const FGameplayTagContainer& ChosenRewardTags);
};
