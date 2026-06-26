// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Attributes/FRCombatSet.h"

UFRCombatSet::UFRCombatSet()
{
	InitMaxHealth(100.0f);
	InitHealth(100.0f);
	InitMaxMoveBudget(420.0f);
	InitMoveBudget(420.0f);
	InitDamage(0.0f);
	InitShotPowerMultiplier(1.0f);
	InitProjectileCount(1.0f);
	InitMinAimAngle(0.0f);
	InitMaxAimAngle(90.0f);
}

void UFRCombatSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
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
	else if (Attribute == GetMinAimAngleAttribute())
	{
		NewValue = FMath::Clamp(NewValue, -90.0f, 180.0f);
	}
	else if (Attribute == GetMaxAimAngleAttribute())
	{
		NewValue = FMath::Clamp(NewValue, -90.0f, 180.0f);
	}
}

void UFRCombatSet::ResetTurnBudget()
{
	SetMoveBudget(GetMaxMoveBudget());
}

void UFRCombatSet::ApplyDamage(float DamageAmount)
{
	SetHealth(FMath::Max(0.0f, GetHealth() - FMath::Max(0.0f, DamageAmount)));
}

void UFRCombatSet::Heal(float HealAmount)
{
	SetHealth(FMath::Min(GetMaxHealth(), GetHealth() + FMath::Max(0.0f, HealAmount)));
}

void UFRCombatSet::AddMaxHealth(float BonusHealth)
{
	const float PreviousMaxHealth = GetMaxHealth();
	SetMaxHealth(FMath::Max(1.0f, GetMaxHealth() + BonusHealth));

	const float AppliedBonus = GetMaxHealth() - PreviousMaxHealth;
	if (AppliedBonus > 0.0f)
	{
		Heal(AppliedBonus);
	}
	else
	{
		SetHealth(FMath::Min(GetHealth(), GetMaxHealth()));
	}
}

void UFRCombatSet::AddMaxMoveBudget(float BonusMoveBudget)
{
	const float PreviousMaxMoveBudget = GetMaxMoveBudget();
	SetMaxMoveBudget(FMath::Max(0.0f, GetMaxMoveBudget() + BonusMoveBudget));

	const float AppliedBonus = GetMaxMoveBudget() - PreviousMaxMoveBudget;
	SetMoveBudget(FMath::Clamp(GetMoveBudget() + AppliedBonus, 0.0f, GetMaxMoveBudget()));
}

void UFRCombatSet::AddDamage(float BonusDamage)
{
	SetDamage(FMath::Max(0.0f, GetDamage() + BonusDamage));
}

void UFRCombatSet::AddShotPowerMultiplier(float BonusMultiplier)
{
	SetShotPowerMultiplier(FMath::Max(0.0f, GetShotPowerMultiplier() + BonusMultiplier));
}

void UFRCombatSet::AddProjectileCount(float BonusProjectiles)
{
	SetProjectileCount(FMath::Max(1.0f, GetProjectileCount() + BonusProjectiles));
}

void UFRCombatSet::AddMinAimAngle(float BonusMinAimAngle)
{
	SetMinAimAngle(FMath::Clamp(GetMinAimAngle() + BonusMinAimAngle, -90.0f, 180.0f));
}

void UFRCombatSet::AddMaxAimAngle(float BonusMaxAimAngle)
{
	SetMaxAimAngle(FMath::Clamp(GetMaxAimAngle() + BonusMaxAimAngle, -90.0f, 180.0f));
}
