// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Weapons/FortRogueWeaponDefinition.h"
#include "FortRoguePerkDefinition.generated.h"

class UFortRogueAbilitySet;

UCLASS(BlueprintType)
class FORTROGUE_API UFortRoguePerkDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|Perk")
	FText GetDataValidationSummary() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perk")
	FText DisplayName = FText::FromString(TEXT("Perk"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perk")
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perk")
	FGameplayTag PerkTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perk")
	TObjectPtr<UFortRogueAbilitySet> GrantedAbilitySet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shot Modifier", meta = (TitleProperty = DisplayName))
	TArray<FFortRogueShotModifierSpec> ShotModifiers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0"))
	float DamageBonus = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0"))
	float MaxHealthBonus = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0"))
	float MaxMoveBudgetBonus = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0"))
	int32 ProjectileBonus = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0"))
	float ShotPowerMultiplierBonus = 0.0f;
};
