// Copyright Epic Games, Inc. All Rights Reserved.

#include "FortRogueGenerateSpriteFlipbooksCommandlet.h"

#include "FortRogueSpriteFlipbookGenerator.h"

UFortRogueGenerateSpriteFlipbooksCommandlet::UFortRogueGenerateSpriteFlipbooksCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

int32 UFortRogueGenerateSpriteFlipbooksCommandlet::Main(const FString& Params)
{
	FortRogueEditor::GenerateSpriteFlipbooks();
	return 0;
}
