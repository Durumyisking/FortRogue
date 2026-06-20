// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Commandlets/Commandlet.h"
#include "FRGenerateCombatDataCommandlet.generated.h"

UCLASS()
class UFRGenerateCombatDataCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UFRGenerateCombatDataCommandlet();

	virtual int32 Main(const FString& Params) override;
};
