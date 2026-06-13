// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "FortRogueCombatSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class FORTROGUE_API UFortRogueCombatSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UFortRogueCombatSet();

	ATTRIBUTE_ACCESSORS(UFortRogueCombatSet, Health);
	ATTRIBUTE_ACCESSORS(UFortRogueCombatSet, MaxHealth);
	ATTRIBUTE_ACCESSORS(UFortRogueCombatSet, MoveBudget);
	ATTRIBUTE_ACCESSORS(UFortRogueCombatSet, MaxMoveBudget);
	ATTRIBUTE_ACCESSORS(UFortRogueCombatSet, Damage);
	ATTRIBUTE_ACCESSORS(UFortRogueCombatSet, ShotPowerMultiplier);
	ATTRIBUTE_ACCESSORS(UFortRogueCombatSet, ProjectileCount);

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	void ResetTurnBudget();
	void ApplyDamage(float DamageAmount);
	void Heal(float HealAmount);
	void AddMaxHealth(float BonusHealth);
	void AddDamage(float BonusDamage);
	void AddProjectileCount(float BonusProjectiles);

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|Combat")
	FGameplayAttributeData Health;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|Combat")
	FGameplayAttributeData MaxHealth;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|Combat")
	FGameplayAttributeData MoveBudget;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|Combat")
	FGameplayAttributeData MaxMoveBudget;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|Combat")
	FGameplayAttributeData Damage;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|Combat")
	FGameplayAttributeData ShotPowerMultiplier;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|Combat")
	FGameplayAttributeData ProjectileCount;
};
