// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Abilities/GameplayAbility.h"
#include "FortRogueGameplayAbility.generated.h"

UCLASS(Abstract, Blueprintable)
class FORTROGUE_API UFortRogueGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UFortRogueGameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
