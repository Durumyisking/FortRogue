// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UI/FortRogueActivatableWidget.h"
#include "FortRogueBattleHUDWidget.generated.h"

UCLASS(Abstract, Blueprintable)
class FORTROGUE_API UFortRogueBattleHUDWidget : public UFortRogueActivatableWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FortRogue|UI")
	class AFortRogueGameMode* GetFortRogueGameMode() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "FortRogue|UI")
	void RefreshBattleHUD();
};
