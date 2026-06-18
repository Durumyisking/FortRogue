// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FortRogueDefaultLoadoutDefinition.generated.h"

class UFortRogueItemDefinition;
class UFortRogueWeaponDefinition;

USTRUCT(BlueprintType)
struct FFortRogueDefaultItemStack
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UFortRogueItemDefinition> ItemDefinition;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "0"))
	int32 Charges = 1;
};

UCLASS(BlueprintType)
class FORTROGUE_API UFortRogueDefaultLoadoutDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapons")
	TArray<TObjectPtr<UFortRogueWeaponDefinition>> WeaponDefinitions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Items")
	TArray<FFortRogueDefaultItemStack> ItemDefinitions;
};
