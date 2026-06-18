// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Commandlets/Commandlet.h"
#include "FortRogueGenerateSpriteFlipbooksCommandlet.generated.h"

UCLASS()
class UFortRogueGenerateSpriteFlipbooksCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UFortRogueGenerateSpriteFlipbooksCommandlet();

	virtual int32 Main(const FString& Params) override;
};
