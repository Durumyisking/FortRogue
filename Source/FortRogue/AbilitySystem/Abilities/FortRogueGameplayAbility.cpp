// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/FortRogueGameplayAbility.h"

UFortRogueGameplayAbility::UFortRogueGameplayAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}
