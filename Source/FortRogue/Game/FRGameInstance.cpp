// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/FRGameInstance.h"

#include "Engine/World.h"

void UFRGameInstance::Init()
{
	Super::Init();
	FWorldDelegates::OnWorldBeginTearDown.AddUObject(this, &UFRGameInstance::HandleWorldBeginTearDown);
}

void UFRGameInstance::Shutdown()
{
	FWorldDelegates::OnWorldBeginTearDown.RemoveAll(this);
	Super::Shutdown();
}

bool UFRGameInstance::ShouldDeferBattleStartForStartupMenu() const
{
	return bUseStartupMainMenu && !bStartupMainMenuCompleted;
}

void UFRGameInstance::NotifyStartupMainMenuDisplayed()
{
	bStartupMainMenuDisplayed = true;
}

void UFRGameInstance::HandleWorldBeginTearDown(UWorld* World)
{
	if (bStartupMainMenuDisplayed && !bStartupMainMenuCompleted)
	{
		bStartupMainMenuCompleted = true;
	}
}