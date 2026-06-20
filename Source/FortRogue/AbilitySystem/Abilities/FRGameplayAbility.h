// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Abilities/GameplayAbility.h"
#include "FRGameplayAbility.generated.h"

UCLASS(Abstract, Blueprintable)
class FORTROGUE_API UFRGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UFRGameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
