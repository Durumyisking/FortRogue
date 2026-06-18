// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Rewards/FortRogueRewardTypes.h"
#include "FortRogueRewardBlueprintLibrary.generated.h"

UCLASS()
class FORTROGUE_API UFortRogueRewardBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	static FText GetRewardEffectSummary(const FFortRogueRewardChoice& RewardChoice);
};
