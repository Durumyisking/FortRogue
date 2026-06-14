// Copyright Epic Games, Inc. All Rights Reserved.

#include "FortRogueTerrainMapDefinitionFactory.h"

#include "Combat/FortRogueTerrainMapDefinition.h"

#define LOCTEXT_NAMESPACE "FortRogueTerrainMapDefinitionFactory"

UFortRogueTerrainMapDefinitionFactory::UFortRogueTerrainMapDefinitionFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UFortRogueTerrainMapDefinition::StaticClass();
}

UObject* UFortRogueTerrainMapDefinitionFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UFortRogueTerrainMapDefinition>(InParent, Class, Name, Flags);
}

FText UFortRogueTerrainMapDefinitionFactory::GetDisplayName() const
{
	return LOCTEXT("DisplayName", "FortRogue Terrain Map Definition");
}

#undef LOCTEXT_NAMESPACE
