// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Commandlets/Commandlet.h"
#include "FRGenerateSpriteFlipbooksCommandlet.generated.h"

UCLASS()
class UFRGenerateSpriteFlipbooksCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UFRGenerateSpriteFlipbooksCommandlet();

	virtual int32 Main(const FString& Params) override;
};
