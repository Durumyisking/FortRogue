// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "FortRogueTerrainMapDefinitionFactory.generated.h"

UCLASS()
class UFortRogueTerrainMapDefinitionFactory : public UFactory
{
	GENERATED_BODY()

public:
	UFortRogueTerrainMapDefinitionFactory();

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual FText GetDisplayName() const override;
};
