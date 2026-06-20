// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ActiveGameplayEffectHandle.h"
#include "AttributeSet.h"
#include "Engine/DataAsset.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "FRAbilitySet.generated.h"

class UFRAbilitySystemComponent;
class UFRGameplayAbility;
class UGameplayEffect;

USTRUCT(BlueprintType)
struct FFRAbilitySet_GameplayAbility
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Ability", meta = (ToolTip = "부여할 Gameplay Ability 클래스입니다."))
	TSubclassOf<UFRGameplayAbility> Ability;

	UPROPERTY(EditDefaultsOnly, Category = "Ability", meta = (ToolTip = "부여할 ability 레벨입니다. 1 이상 값을 권장합니다."))
	int32 AbilityLevel = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Ability", meta = (Categories = "InputTag", ToolTip = "ability를 입력에 묶을 때 사용하는 태그입니다. InputTag.* 태그만 사용하세요."))
	FGameplayTag InputTag;
};

USTRUCT(BlueprintType)
struct FFRAbilitySet_GameplayEffect
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Effect", meta = (ToolTip = "부여할 Gameplay Effect 클래스입니다."))
	TSubclassOf<UGameplayEffect> GameplayEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Effect", meta = (ToolTip = "적용할 Gameplay Effect 레벨입니다."))
	float EffectLevel = 1.0f;
};

USTRUCT(BlueprintType)
struct FFRAbilitySet_AttributeSet
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Attributes", meta = (ToolTip = "AbilitySystemComponent에 추가할 AttributeSet 클래스입니다."))
	TSubclassOf<UAttributeSet> AttributeSet;
};

USTRUCT(BlueprintType)
struct FFRAbilitySet_GrantedHandles
{
	GENERATED_BODY()

	void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);
	void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle);
	void AddAttributeSet(UAttributeSet* Set);
	void TakeFromAbilitySystem(UFRAbilitySystemComponent* AbilitySystemComponent);

private:
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> GameplayEffectHandles;

	UPROPERTY()
	TArray<TObjectPtr<UAttributeSet>> GrantedAttributeSets;
};

UCLASS(BlueprintType, Const)
class FORTROGUE_API UFRAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFRAbilitySet(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void GiveToAbilitySystem(UFRAbilitySystemComponent* AbilitySystemComponent, FFRAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject = nullptr) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Ability Set")
	FText GetEffectSummary() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Ability Set")
	FText GetDataValidationSummary() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability Set", meta = (ToolTip = "에디터와 UI에 표시할 AbilitySet 이름입니다."))
	FText DisplayName = FText::FromString(TEXT("Ability Set"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability Set", meta = (MultiLine = "true", ToolTip = "AbilitySet 설명입니다. 어떤 능력/효과 묶음인지 적어주세요."))
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability Set", meta = (Categories = "Trait", ToolTip = "AbilitySet을 찾거나 제거할 때 쓰는 태그입니다. Trait.* 태그만 사용하세요."))
	FGameplayTag AbilitySetTag;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities", meta = (TitleProperty = Ability, ToolTip = "이 AbilitySet이 부여할 Gameplay Ability 목록입니다."))
	TArray<FFRAbilitySet_GameplayAbility> GrantedGameplayAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects", meta = (TitleProperty = GameplayEffect, ToolTip = "이 AbilitySet이 즉시 적용할 Gameplay Effect 목록입니다."))
	TArray<FFRAbilitySet_GameplayEffect> GrantedGameplayEffects;

	UPROPERTY(EditDefaultsOnly, Category = "Attribute Sets", meta = (TitleProperty = AttributeSet, ToolTip = "이 AbilitySet이 ASC에 추가할 AttributeSet 목록입니다."))
	TArray<FFRAbilitySet_AttributeSet> GrantedAttributes;
};
