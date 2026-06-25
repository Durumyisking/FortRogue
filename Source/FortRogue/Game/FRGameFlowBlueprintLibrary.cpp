// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/FRGameFlowBlueprintLibrary.h"

#include "Game/FRGameFlowSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

namespace
{
UFRGameFlowSubsystem* GetGameFlowSubsystem(const UObject* WorldContextObject)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<UFRGameFlowSubsystem>() : nullptr;
}
}

bool UFRGameFlowBlueprintLibrary::StartMainMenu(const UObject* WorldContextObject)
{
	if (UFRGameFlowSubsystem* GameFlow = GetGameFlowSubsystem(WorldContextObject))
	{
		return GameFlow->StartMainMenu();
	}
	return false;
}

bool UFRGameFlowBlueprintLibrary::StartMainGame(const UObject* WorldContextObject)
{
	if (UFRGameFlowSubsystem* GameFlow = GetGameFlowSubsystem(WorldContextObject))
	{
		return GameFlow->StartMainGame();
	}
	return false;
}

bool UFRGameFlowBlueprintLibrary::StartGameMode(const UObject* WorldContextObject, UFRGameModeDataAsset* ModeData)
{
	if (UFRGameFlowSubsystem* GameFlow = GetGameFlowSubsystem(WorldContextObject))
	{
		return GameFlow->StartMode(ModeData);
	}
	return false;
}