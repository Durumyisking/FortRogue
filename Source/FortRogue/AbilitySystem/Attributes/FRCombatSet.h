// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "FRCombatSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class FORTROGUE_API UFRCombatSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UFRCombatSet();

	ATTRIBUTE_ACCESSORS(UFRCombatSet, Health);
	ATTRIBUTE_ACCESSORS(UFRCombatSet, MaxHealth);
	ATTRIBUTE_ACCESSORS(UFRCombatSet, MoveBudget);
	ATTRIBUTE_ACCESSORS(UFRCombatSet, MaxMoveBudget);
	ATTRIBUTE_ACCESSORS(UFRCombatSet, Damage);
	ATTRIBUTE_ACCESSORS(UFRCombatSet, ShotPowerMultiplier);
	ATTRIBUTE_ACCESSORS(UFRCombatSet, ProjectileCount);
	ATTRIBUTE_ACCESSORS(UFRCombatSet, MinAimAngle);
	ATTRIBUTE_ACCESSORS(UFRCombatSet, MaxAimAngle);

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	void ResetTurnBudget();
	void ApplyDamage(float DamageAmount);
	void Heal(float HealAmount);
	void AddMaxHealth(float BonusHealth);
	void AddMaxMoveBudget(float BonusMoveBudget);
	void AddDamage(float BonusDamage);
	void AddShotPowerMultiplier(float BonusMultiplier);
	void AddProjectileCount(float BonusProjectiles);
	void AddMinAimAngle(float BonusMinAimAngle);
	void AddMaxAimAngle(float BonusMaxAimAngle);

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

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|Combat")
	FGameplayAttributeData MinAimAngle;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|Combat")
	FGameplayAttributeData MaxAimAngle;
};
