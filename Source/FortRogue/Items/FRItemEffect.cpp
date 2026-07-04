// Copyright Epic Games, Inc. All Rights Reserved.

#include "Items/FRItemEffect.h"

#include "Combat/FRBattleCharacter.h"

void UFRItemEffect_Heal::ApplyToCharacter(AFRBattleCharacter& Character) const
{
	Character.ApplyHeal(HealAmount);
}

void UFRItemEffect_Heal::AppendEffectSummary(TArray<FString>& Parts) const
{
	if (HealAmount > 0.0f)
	{
		Parts.Add(FString::Printf(TEXT("heal +%.0f"), HealAmount));
	}
}

void UFRItemEffect_Heal::AppendValidationIssues(TArray<FString>& Issues) const
{
	if (HealAmount <= 0.0f)
	{
		Issues.Add(TEXT("heal amount must be greater than 0"));
	}
}

bool UFRItemEffect_Heal::HasGameplayEffect() const
{
	return HealAmount > 0.0f;
}

void UFRItemEffect_AttackMultiplier::ApplyToCharacter(AFRBattleCharacter& Character) const
{
	Character.ApplyPendingAttackMultiplier(AttackMultiplier);
}

void UFRItemEffect_AttackMultiplier::AppendEffectSummary(TArray<FString>& Parts) const
{
	if (!FMath::IsNearlyEqual(AttackMultiplier, 1.0f))
	{
		Parts.Add(FString::Printf(TEXT("next shot attack x%.2g"), AttackMultiplier));
	}
}

void UFRItemEffect_AttackMultiplier::AppendValidationIssues(TArray<FString>& Issues) const
{
	if (AttackMultiplier <= 1.0f)
	{
		Issues.Add(TEXT("attack multiplier must be greater than 1"));
	}
}

bool UFRItemEffect_AttackMultiplier::HasGameplayEffect() const
{
	return AttackMultiplier > 1.0f;
}
