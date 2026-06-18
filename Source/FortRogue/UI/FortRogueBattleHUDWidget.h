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

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	class AFortRogueBattleCharacter* GetPlayerCharacter() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	class AFortRogueBattleCharacter* GetEnemyCharacter() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FText GetRunProgressSummary() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FText GetWindSummary() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FText GetPlayerCombatStatsSummary() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FText GetPlayerShotSummary() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	float GetPlayerAimAngle() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	float GetPlayerShotPower() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	float GetPlayerShotChargeAlpha() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	bool IsPlayerChargingShot() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	bool CanFirePlayerWeapon() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "FortRogue|UI")
	void RefreshBattleHUD();
};
