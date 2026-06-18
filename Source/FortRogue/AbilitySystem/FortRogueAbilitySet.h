// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ActiveGameplayEffectHandle.h"
#include "AttributeSet.h"
#include "Engine/DataAsset.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "FortRogueAbilitySet.generated.h"

class UFortRogueAbilitySystemComponent;
class UFortRogueGameplayAbility;
class UGameplayEffect;

USTRUCT(BlueprintType)
struct FFortRogueAbilitySet_GameplayAbility
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TSubclassOf<UFortRogueGameplayAbility> Ability;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	int32 AbilityLevel = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Ability", meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

USTRUCT(BlueprintType)
struct FFortRogueAbilitySet_GameplayEffect
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TSubclassOf<UGameplayEffect> GameplayEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	float EffectLevel = 1.0f;
};

USTRUCT(BlueprintType)
struct FFortRogueAbilitySet_AttributeSet
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Attributes")
	TSubclassOf<UAttributeSet> AttributeSet;
};

USTRUCT(BlueprintType)
struct FFortRogueAbilitySet_GrantedHandles
{
	GENERATED_BODY()

	void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);
	void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle);
	void AddAttributeSet(UAttributeSet* Set);
	void TakeFromAbilitySystem(UFortRogueAbilitySystemComponent* AbilitySystemComponent);

private:
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> GameplayEffectHandles;

	UPROPERTY()
	TArray<TObjectPtr<UAttributeSet>> GrantedAttributeSets;
};

UCLASS(BlueprintType, Const)
class FORTROGUE_API UFortRogueAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFortRogueAbilitySet(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void GiveToAbilitySystem(UFortRogueAbilitySystemComponent* AbilitySystemComponent, FFortRogueAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject = nullptr) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Ability Set")
	FText GetEffectSummary() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability Set")
	FText DisplayName = FText::FromString(TEXT("Ability Set"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability Set", meta = (MultiLine = "true"))
	FText Description;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities", meta = (TitleProperty = Ability))
	TArray<FFortRogueAbilitySet_GameplayAbility> GrantedGameplayAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects", meta = (TitleProperty = GameplayEffect))
	TArray<FFortRogueAbilitySet_GameplayEffect> GrantedGameplayEffects;

	UPROPERTY(EditDefaultsOnly, Category = "Attribute Sets", meta = (TitleProperty = AttributeSet))
	TArray<FFortRogueAbilitySet_AttributeSet> GrantedAttributes;
};
