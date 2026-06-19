// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Weapons/FortRogueWeaponDefinition.h"
#include "FortRogueItemDefinition.generated.h"

class UFortRogueAbilitySet;

UENUM(BlueprintType)
enum class EFortRogueItemType : uint8
{
	AttackMultiplier,
	Heal,
	AbilitySet
};

UCLASS(BlueprintType)
class FORTROGUE_API UFortRogueItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|Item")
	FText GetDataValidationSummary() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText DisplayName = FText::FromString(TEXT("Item"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (MultiLine = "true"))
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FGameplayTag ItemTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	EFortRogueItemType ItemType = EFortRogueItemType::Heal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "1"))
	int32 InitialCharges = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UFortRogueAbilitySet> UseAbilitySet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shot Modifier", meta = (TitleProperty = DisplayName))
	TArray<FFortRogueShotModifierSpec> UseShotModifiers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (EditCondition = "ItemType == EFortRogueItemType::AttackMultiplier", ClampMin = "1.0"))
	float AttackMultiplier = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (EditCondition = "ItemType == EFortRogueItemType::Heal", ClampMin = "0.0"))
	float HealAmount = 35.0f;
};
