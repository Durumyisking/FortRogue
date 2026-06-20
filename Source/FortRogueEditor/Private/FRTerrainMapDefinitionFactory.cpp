// Copyright Epic Games, Inc. All Rights Reserved.

#include "FRTerrainMapDefinitionFactory.h"

#include "Combat/FRTerrainMapDefinition.h"

#define LOCTEXT_NAMESPACE "FRTerrainMapDefinitionFactory"

UFRTerrainMapDefinitionFactory::UFRTerrainMapDefinitionFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UFRTerrainMapDefinition::StaticClass();
}

UObject* UFRTerrainMapDefinitionFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UFRTerrainMapDefinition>(InParent, Class, Name, Flags);
}

FText UFRTerrainMapDefinitionFactory::GetDisplayName() const
{
	return LOCTEXT("DisplayName", "FortRogue Terrain Map Definition");
}

#undef LOCTEXT_NAMESPACE
