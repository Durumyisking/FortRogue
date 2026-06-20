// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "FRTerrainMapDefinitionFactory.generated.h"

UCLASS()
class UFRTerrainMapDefinitionFactory : public UFactory
{
	GENERATED_BODY()

public:
	UFRTerrainMapDefinitionFactory();

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual FText GetDisplayName() const override;
};
