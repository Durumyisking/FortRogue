// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/FortRogueAbilitySet.h"

#include "AbilitySystem/FortRogueAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/FortRogueGameplayAbility.h"
#include "GameplayEffect.h"

void FFortRogueAbilitySet_GrantedHandles::AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle)
{
	if (Handle.IsValid())
	{
		AbilitySpecHandles.Add(Handle);
	}
}

void FFortRogueAbilitySet_GrantedHandles::AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle)
{
	if (Handle.IsValid())
	{
		GameplayEffectHandles.Add(Handle);
	}
}

void FFortRogueAbilitySet_GrantedHandles::AddAttributeSet(UAttributeSet* Set)
{
	if (Set)
	{
		GrantedAttributeSets.Add(Set);
	}
}

void FFortRogueAbilitySet_GrantedHandles::TakeFromAbilitySystem(UFortRogueAbilitySystemComponent* AbilitySystemComponent)
{
	check(AbilitySystemComponent);

	for (const FGameplayAbilitySpecHandle& Handle : AbilitySpecHandles)
	{
		if (Handle.IsValid())
		{
			AbilitySystemComponent->ClearAbility(Handle);
		}
	}

	for (const FActiveGameplayEffectHandle& Handle : GameplayEffectHandles)
	{
		if (Handle.IsValid())
		{
			AbilitySystemComponent->RemoveActiveGameplayEffect(Handle);
		}
	}

	for (UAttributeSet* Set : GrantedAttributeSets)
	{
		AbilitySystemComponent->RemoveSpawnedAttribute(Set);
	}

	AbilitySpecHandles.Reset();
	GameplayEffectHandles.Reset();
	GrantedAttributeSets.Reset();
}

UFortRogueAbilitySet::UFortRogueAbilitySet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFortRogueAbilitySet::GiveToAbilitySystem(UFortRogueAbilitySystemComponent* AbilitySystemComponent, FFortRogueAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject) const
{
	check(AbilitySystemComponent);

	for (const FFortRogueAbilitySet_AttributeSet& SetToGrant : GrantedAttributes)
	{
		if (!SetToGrant.AttributeSet)
		{
			continue;
		}

		UAttributeSet* NewSet = NewObject<UAttributeSet>(AbilitySystemComponent->GetOwner(), SetToGrant.AttributeSet);
		AbilitySystemComponent->AddAttributeSetSubobject(NewSet);

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAttributeSet(NewSet);
		}
	}

	for (const FFortRogueAbilitySet_GameplayAbility& AbilityToGrant : GrantedGameplayAbilities)
	{
		if (!AbilityToGrant.Ability)
		{
			continue;
		}

		UFortRogueGameplayAbility* AbilityCDO = AbilityToGrant.Ability->GetDefaultObject<UFortRogueGameplayAbility>();
		FGameplayAbilitySpec AbilitySpec(AbilityCDO, AbilityToGrant.AbilityLevel);
		AbilitySpec.SourceObject = SourceObject;
		AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilityToGrant.InputTag);

		const FGameplayAbilitySpecHandle Handle = AbilitySystemComponent->GiveAbility(AbilitySpec);
		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAbilitySpecHandle(Handle);
		}
	}

	for (const FFortRogueAbilitySet_GameplayEffect& EffectToGrant : GrantedGameplayEffects)
	{
		if (!EffectToGrant.GameplayEffect)
		{
			continue;
		}

		const UGameplayEffect* GameplayEffect = EffectToGrant.GameplayEffect->GetDefaultObject<UGameplayEffect>();
		const FActiveGameplayEffectHandle Handle = AbilitySystemComponent->ApplyGameplayEffectToSelf(GameplayEffect, EffectToGrant.EffectLevel, AbilitySystemComponent->MakeEffectContext());
		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddGameplayEffectHandle(Handle);
		}
	}
}
