// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Commandlets/Commandlet.h"
#include "FRGenerateMainGameHUDCommandlet.generated.h"

UCLASS()
class UFRGenerateMainGameHUDCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UFRGenerateMainGameHUDCommandlet();

	virtual int32 Main(const FString& Params) override;
};
