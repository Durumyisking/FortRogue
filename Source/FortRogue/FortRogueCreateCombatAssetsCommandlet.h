// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Commandlets/Commandlet.h"
#include "FortRogueCreateCombatAssetsCommandlet.generated.h"

UCLASS()
class FORTROGUE_API UFortRogueCreateCombatAssetsCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UFortRogueCreateCombatAssetsCommandlet();

	virtual int32 Main(const FString& Params) override;
};
