// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FRGameFlowSubsystem.generated.h"

class APlayerController;
class UButton;
class UFRGameModeDataAsset;
class UUserWidget;

UCLASS()
class FORTROGUE_API UFRGameFlowSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FortRogue|Game Flow")
	bool StartStartupMode();

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Game Flow")
	bool StartMainMenu();

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Game Flow")
	bool StartMainGame();

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Game Flow")
	bool StartMode(UFRGameModeDataAsset* ModeData);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Game Flow")
	UFRGameModeDataAsset* GetCurrentModeData() const { return CurrentModeData; }

	bool EnsureStartupMode();
	void ApplyCurrentModeToWorld();
	void ApplyCurrentModeToPlayerController(APlayerController* PlayerController);

private:
	UFRGameModeDataAsset* LoadModeData(const TSoftObjectPtr<UFRGameModeDataAsset>& ModeData) const;
	bool StartModeInternal(UFRGameModeDataAsset* ModeData);
	bool ShouldTravelToModeLevel(const UFRGameModeDataAsset* ModeData, FString& OutLevelPackageName) const;
	void BindModeWidgetActions();
	void ClearActiveHUD();

	UFUNCTION()
	void HandleStartMainGameClicked();

	UPROPERTY(Transient)
	TObjectPtr<UFRGameModeDataAsset> CurrentModeData;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> ActiveHUDWidget;
};