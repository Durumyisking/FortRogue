// Copyright Epic Games, Inc. All Rights Reserved.

#include "Perks/FRPerkDefinition.h"

#include "AbilitySystem/FRAbilitySet.h"

namespace
{
void AddPerkValidationIssue(TArray<FString>& Issues, const FString& Issue)
{
	if (!Issue.IsEmpty())
	{
		Issues.Add(Issue);
	}
}

bool HasPerkGameplayEffect(const UFRPerkDefinition& PerkDefinition)
{
	if (PerkDefinition.GrantedAbilitySet
		|| !FMath::IsNearlyZero(PerkDefinition.DamageBonus)
		|| !FMath::IsNearlyZero(PerkDefinition.MaxHealthBonus)
		|| !FMath::IsNearlyZero(PerkDefinition.MaxMoveBudgetBonus)
		|| PerkDefinition.ProjectileBonus != 0
		|| !FMath::IsNearlyZero(PerkDefinition.ShotPowerMultiplierBonus))
	{
		return true;
	}

	for (const FFRShotModifierSpec& ShotModifier : PerkDefinition.ShotModifiers)
	{
		if (ShotModifier.HasGameplayEffect())
		{
			return true;
		}
	}
	return false;
}
}

FText UFRPerkDefinition::GetDataValidationSummary() const
{
	TArray<FString> Issues;
	if (DisplayName.ToString().IsEmpty())
	{
		AddPerkValidationIssue(Issues, TEXT("missing display name"));
	}
	if (!PerkTag.IsValid())
	{
		AddPerkValidationIssue(Issues, TEXT("missing PerkTag"));
	}
	if (!HasPerkGameplayEffect(*this))
	{
		AddPerkValidationIssue(Issues, TEXT("missing perk effect"));
	}
	if (GrantedAbilitySet && !GrantedAbilitySet->GetDataValidationSummary().IsEmpty())
	{
		AddPerkValidationIssue(Issues, TEXT("ability set data has warnings"));
	}
	for (const FFRShotModifierSpec& ShotModifier : ShotModifiers)
	{
		if (!ShotModifier.GetDataValidationSummary().IsEmpty())
		{
			AddPerkValidationIssue(Issues, TEXT("shot modifier data has warnings"));
			break;
		}
	}

	return Issues.Num() > 0 ? FText::FromString(FString::Join(Issues, TEXT(" | "))) : FText::GetEmpty();
}
