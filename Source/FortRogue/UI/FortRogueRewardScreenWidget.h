// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UI/FortRogueActivatableWidget.h"
#include "FortRogueRewardScreenWidget.generated.h"

UCLASS(Abstract, Blueprintable)
class FORTROGUE_API UFortRogueRewardScreenWidget : public UFortRogueActivatableWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FortRogue|UI")
	class AFortRogueGameMode* GetFortRogueGameMode() const;

	UFUNCTION(BlueprintCallable, Category = "FortRogue|UI")
	void ChooseReward(int32 ChoiceIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = "FortRogue|UI")
	void RefreshRewardScreen();
};
