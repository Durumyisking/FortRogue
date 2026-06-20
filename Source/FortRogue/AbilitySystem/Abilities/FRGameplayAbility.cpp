// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/FRGameplayAbility.h"

UFRGameplayAbility::UFRGameplayAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}
