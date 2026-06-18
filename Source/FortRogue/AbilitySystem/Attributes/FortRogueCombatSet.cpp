// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Attributes/FortRogueCombatSet.h"

UFortRogueCombatSet::UFortRogueCombatSet()
{
	InitMaxHealth(100.0f);
	InitHealth(100.0f);
	InitMaxMoveBudget(420.0f);
	InitMoveBudget(420.0f);
	InitDamage(0.0f);
	InitShotPowerMultiplier(1.0f);
	InitProjectileCount(1.0f);
}

void UFortRogueCombatSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(1.0f, NewValue);
	}
	else if (Attribute == GetMoveBudgetAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMoveBudget());
	}
	else if (Attribute == GetMaxMoveBudgetAttribute())
	{
		NewValue = FMath::Max(0.0f, NewValue);
	}
	else if (Attribute == GetDamageAttribute())
	{
		NewValue = FMath::Max(0.0f, NewValue);
	}
	else if (Attribute == GetShotPowerMultiplierAttribute())
	{
		NewValue = FMath::Max(0.0f, NewValue);
	}
	else if (Attribute == GetProjectileCountAttribute())
	{
		NewValue = FMath::Max(1.0f, NewValue);
	}
}

void UFortRogueCombatSet::ResetTurnBudget()
{
	SetMoveBudget(GetMaxMoveBudget());
}

void UFortRogueCombatSet::ApplyDamage(float DamageAmount)
{
	SetHealth(FMath::Max(0.0f, GetHealth() - FMath::Max(0.0f, DamageAmount)));
}

void UFortRogueCombatSet::Heal(float HealAmount)
{
	SetHealth(FMath::Min(GetMaxHealth(), GetHealth() + FMath::Max(0.0f, HealAmount)));
}

void UFortRogueCombatSet::AddMaxHealth(float BonusHealth)
{
	const float ClampedBonus = FMath::Max(0.0f, BonusHealth);
	SetMaxHealth(GetMaxHealth() + ClampedBonus);
	Heal(ClampedBonus);
}

void UFortRogueCombatSet::AddMaxMoveBudget(float BonusMoveBudget)
{
	const float ClampedBonus = FMath::Max(0.0f, BonusMoveBudget);
	SetMaxMoveBudget(GetMaxMoveBudget() + ClampedBonus);
	SetMoveBudget(GetMoveBudget() + ClampedBonus);
}

void UFortRogueCombatSet::AddDamage(float BonusDamage)
{
	SetDamage(GetDamage() + FMath::Max(0.0f, BonusDamage));
}

void UFortRogueCombatSet::AddShotPowerMultiplier(float BonusMultiplier)
{
	SetShotPowerMultiplier(GetShotPowerMultiplier() + FMath::Max(0.0f, BonusMultiplier));
}

void UFortRogueCombatSet::AddProjectileCount(float BonusProjectiles)
{
	SetProjectileCount(GetProjectileCount() + FMath::Max(0.0f, BonusProjectiles));
}
