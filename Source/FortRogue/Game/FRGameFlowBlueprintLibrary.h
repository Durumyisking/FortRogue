// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FRGameFlowBlueprintLibrary.generated.h"

class UFRGameModeDataAsset;

UCLASS()
class FORTROGUE_API UFRGameFlowBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FortRogue|Game Flow", meta = (WorldContext = "WorldContextObject"))
	static bool StartMainMenu(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Game Flow", meta = (WorldContext = "WorldContextObject"))
	static bool StartMainGame(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Game Flow", meta = (WorldContext = "WorldContextObject"))
	static bool StartGameMode(const UObject* WorldContextObject, UFRGameModeDataAsset* ModeData);
};