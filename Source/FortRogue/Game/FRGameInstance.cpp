// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/FRGameInstance.h"

#include "Game/FRGameFlowSubsystem.h"

void UFRGameInstance::OnStart()
{
	Super::OnStart();

	if (UFRGameFlowSubsystem* GameFlow = GetSubsystem<UFRGameFlowSubsystem>())
	{
		GameFlow->StartStartupMode();
	}
}