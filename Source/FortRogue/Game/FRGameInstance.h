// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "FRGameInstance.generated.h"

UCLASS()
class FORTROGUE_API UFRGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;

	bool ShouldDeferBattleStartForStartupMenu() const;
	void NotifyStartupMainMenuDisplayed();

private:
	void HandleWorldBeginTearDown(UWorld* World);

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|MainMenu")
	bool bUseStartupMainMenu = true;

	bool bStartupMainMenuDisplayed = false;
	bool bStartupMainMenuCompleted = false;
};