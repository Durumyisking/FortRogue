// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/FRAbilitySet.h"

#include "AbilitySystem/FRAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/FRGameplayAbility.h"
#include "GameplayEffect.h"

namespace
{
void AddAbilitySetSummaryPart(TArray<FString>& Parts, const FString& Part)
{
	if (!Part.IsEmpty())
	{
		Parts.Add(Part);
	}
}
}

void FFRAbilitySet_GrantedHandles::AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle)
{
	if (Handle.IsValid())
	{
		AbilitySpecHandles.Add(Handle);
	}
}

void FFRAbilitySet_GrantedHandles::AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle)
{
	if (Handle.IsValid())
	{
		GameplayEffectHandles.Add(Handle);
	}
}

void FFRAbilitySet_GrantedHandles::AddAttributeSet(UAttributeSet* Set)
{
	if (Set)
	{
		GrantedAttributeSets.Add(Set);
	}
}

void FFRAbilitySet_GrantedHandles::TakeFromAbilitySystem(UFRAbilitySystemComponent* AbilitySystemComponent)
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

UFRAbilitySet::UFRAbilitySet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FText UFRAbilitySet::GetEffectSummary() const
{
	TArray<FString> Parts;
	const FString DisplayNameString = DisplayName.ToString();
	const FString AbilitySetName = DisplayNameString.IsEmpty() ? GetName() : DisplayNameString;
	AddAbilitySetSummaryPart(Parts, AbilitySetName);
	const FString DescriptionString = Description.ToString();
	if (!DescriptionString.IsEmpty())
	{
		AddAbilitySetSummaryPart(Parts, DescriptionString);
	}
	if (AbilitySetTag.IsValid())
	{
		AddAbilitySetSummaryPart(Parts, AbilitySetTag.ToString());
	}

	int32 AbilityCount = 0;
	for (const FFRAbilitySet_GameplayAbility& AbilityToGrant : GrantedGameplayAbilities)
	{
		if (AbilityToGrant.Ability)
		{
			++AbilityCount;
		}
	}
	if (AbilityCount > 0)
	{
		AddAbilitySetSummaryPart(Parts, FString::Printf(TEXT("abilities %d"), AbilityCount));
	}

	int32 EffectCount = 0;
	for (const FFRAbilitySet_GameplayEffect& EffectToGrant : GrantedGameplayEffects)
	{
		if (EffectToGrant.GameplayEffect)
		{
			++EffectCount;
		}
	}
	if (EffectCount > 0)
	{
		AddAbilitySetSummaryPart(Parts, FString::Printf(TEXT("effects %d"), EffectCount));
	}

	int32 AttributeSetCount = 0;
	for (const FFRAbilitySet_AttributeSet& SetToGrant : GrantedAttributes)
	{
		if (SetToGrant.AttributeSet)
		{
			++AttributeSetCount;
		}
	}
	if (AttributeSetCount > 0)
	{
		AddAbilitySetSummaryPart(Parts, FString::Printf(TEXT("attribute sets %d"), AttributeSetCount));
	}

	if (Parts.Num() <= 0)
	{
		return Description;
	}

	return FText::FromString(FString::Join(Parts, TEXT(" | ")));
}

FText UFRAbilitySet::GetDataValidationSummary() const
{
	TArray<FString> Issues;
	if (DisplayName.ToString().IsEmpty())
	{
		AddAbilitySetSummaryPart(Issues, TEXT("missing display name"));
	}

	bool bHasGrant = false;
	bool bHasEmptyAbilityEntry = false;
	bool bHasInvalidAbilityLevel = false;
	for (const FFRAbilitySet_GameplayAbility& AbilityToGrant : GrantedGameplayAbilities)
	{
		if (!AbilityToGrant.Ability)
		{
			bHasEmptyAbilityEntry = true;
			continue;
		}

		bHasGrant = true;
		if (AbilityToGrant.AbilityLevel <= 0)
		{
			bHasInvalidAbilityLevel = true;
		}
	}

	bool bHasEmptyEffectEntry = false;
	bool bHasInvalidEffectLevel = false;
	for (const FFRAbilitySet_GameplayEffect& EffectToGrant : GrantedGameplayEffects)
	{
		if (!EffectToGrant.GameplayEffect)
		{
			bHasEmptyEffectEntry = true;
			continue;
		}

		bHasGrant = true;
		if (EffectToGrant.EffectLevel <= 0.0f)
		{
			bHasInvalidEffectLevel = true;
		}
	}

	bool bHasEmptyAttributeEntry = false;
	for (const FFRAbilitySet_AttributeSet& SetToGrant : GrantedAttributes)
	{
		if (!SetToGrant.AttributeSet)
		{
			bHasEmptyAttributeEntry = true;
			continue;
		}

		bHasGrant = true;
	}

	if (!bHasGrant)
	{
		AddAbilitySetSummaryPart(Issues, TEXT("missing granted ability/effect/attribute"));
	}
	if (bHasEmptyAbilityEntry)
	{
		AddAbilitySetSummaryPart(Issues, TEXT("empty ability entry"));
	}
	if (bHasInvalidAbilityLevel)
	{
		AddAbilitySetSummaryPart(Issues, TEXT("ability level must be greater than 0"));
	}
	if (bHasEmptyEffectEntry)
	{
		AddAbilitySetSummaryPart(Issues, TEXT("empty gameplay effect entry"));
	}
	if (bHasInvalidEffectLevel)
	{
		AddAbilitySetSummaryPart(Issues, TEXT("effect level must be greater than 0"));
	}
	if (bHasEmptyAttributeEntry)
	{
		AddAbilitySetSummaryPart(Issues, TEXT("empty attribute set entry"));
	}

	return Issues.Num() > 0 ? FText::FromString(FString::Join(Issues, TEXT(" | "))) : FText::GetEmpty();
}

void UFRAbilitySet::GiveToAbilitySystem(UFRAbilitySystemComponent* AbilitySystemComponent, FFRAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject) const
{
	check(AbilitySystemComponent);

	for (const FFRAbilitySet_AttributeSet& SetToGrant : GrantedAttributes)
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

	for (const FFRAbilitySet_GameplayAbility& AbilityToGrant : GrantedGameplayAbilities)
	{
		if (!AbilityToGrant.Ability)
		{
			continue;
		}

		UFRGameplayAbility* AbilityCDO = AbilityToGrant.Ability->GetDefaultObject<UFRGameplayAbility>();
		FGameplayAbilitySpec AbilitySpec(AbilityCDO, AbilityToGrant.AbilityLevel);
		AbilitySpec.SourceObject = SourceObject;
		AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilityToGrant.InputTag);

		const FGameplayAbilitySpecHandle Handle = AbilitySystemComponent->GiveAbility(AbilitySpec);
		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAbilitySpecHandle(Handle);
		}
	}

	for (const FFRAbilitySet_GameplayEffect& EffectToGrant : GrantedGameplayEffects)
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
