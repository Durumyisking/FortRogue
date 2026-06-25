// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "FRGameFlowSettings.generated.h"

class UFRGameModeDataAsset;

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "FortRogue Game Flow"))
class FORTROGUE_API UFRGameFlowSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game Flow")
	TSoftObjectPtr<UFRGameModeDataAsset> StartupModeData;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game Flow")
	TSoftObjectPtr<UFRGameModeDataAsset> MainMenuModeData;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game Flow")
	TSoftObjectPtr<UFRGameModeDataAsset> MainGameModeData;
};