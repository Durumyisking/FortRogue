// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "FortRogueHUD.generated.h"

UCLASS()
class FORTROGUE_API AFortRogueHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	void DrawBattleHUD(class AFortRogueGameMode* GameMode, float X, float& Y);
	void DrawPowerGauge(class AFortRogueBattleCharacter* Player, float X, float& Y);
};
