// Copyright Epic Games, Inc. All Rights Reserved.

#include "FRGenerateCommonUIWidgetsCommandlet.h"

#include "FRCommonUIWidgetGenerator.h"

UFRGenerateCommonUIWidgetsCommandlet::UFRGenerateCommonUIWidgetsCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

int32 UFRGenerateCommonUIWidgetsCommandlet::Main(const FString& Params)
{
	return FREditor::GenerateCommonUIWidgets();
}
