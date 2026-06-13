// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "Items/FortRogueItemDefinition.h"
#include "FortRogueCharacterDefinition.generated.h"

class UFortRogueAbilitySet;
class UFortRogueWeaponDefinition;

USTRUCT(BlueprintType)
struct FFortRogueItemStack
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<UFortRogueItemDefinition> ItemDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ClampMin = "0"))
	int32 Charges = 1;
};

UCLASS(BlueprintType)
class FORTROGUE_API UFortRogueCharacterDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character")
	FText DisplayName = FText::FromString(TEXT("Rookie Tank"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character")
	FLinearColor BodyTint = FLinearColor(0.1f, 0.7f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0"))
	float BonusDamage = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0"))
	float MaxMoveBudget = 420.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<TObjectPtr<UFortRogueAbilitySet>> StartupAbilitySets;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapons")
	TArray<TObjectPtr<UFortRogueWeaponDefinition>> WeaponLoadout;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Items")
	TArray<FFortRogueItemStack> ItemLoadout;
};
