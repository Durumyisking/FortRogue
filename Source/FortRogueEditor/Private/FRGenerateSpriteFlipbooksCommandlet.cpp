// Copyright Epic Games, Inc. All Rights Reserved.

#include "FRGenerateSpriteFlipbooksCommandlet.h"

#include "FRSpriteFlipbookGenerator.h"

UFRGenerateSpriteFlipbooksCommandlet::UFRGenerateSpriteFlipbooksCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

int32 UFRGenerateSpriteFlipbooksCommandlet::Main(const FString& Params)
{
	FREditor::GenerateSpriteFlipbooks();
	return 0;
}
