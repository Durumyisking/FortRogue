// Copyright Epic Games, Inc. All Rights Reserved.

#include "Items/FortRogueItemDefinition.h"

#include "AbilitySystem/FortRogueAbilitySet.h"

namespace
{
void AddItemValidationIssue(TArray<FString>& Issues, const FString& Issue)
{
	if (!Issue.IsEmpty())
	{
		Issues.Add(Issue);
	}
}

bool HasItemGameplayEffect(const UFortRogueItemDefinition& ItemDefinition)
{
	if (ItemDefinition.UseAbilitySet || ItemDefinition.UseShotModifiers.Num() > 0)
	{
		return true;
	}
	if (ItemDefinition.ItemType == EFortRogueItemType::Heal)
	{
		return ItemDefinition.HealAmount > 0.0f;
	}
	if (ItemDefinition.ItemType == EFortRogueItemType::AttackMultiplier)
	{
		return ItemDefinition.AttackMultiplier > 1.0f;
	}
	return false;
}
}

FText UFortRogueItemDefinition::GetDataValidationSummary() const
{
	TArray<FString> Issues;
	if (DisplayName.ToString().IsEmpty())
	{
		AddItemValidationIssue(Issues, TEXT("missing display name"));
	}
	if (!ItemTag.IsValid())
	{
		AddItemValidationIssue(Issues, TEXT("missing ItemTag"));
	}
	if (InitialCharges <= 0)
	{
		AddItemValidationIssue(Issues, TEXT("initial charges must be greater than 0"));
	}
	if (!HasItemGameplayEffect(*this))
	{
		AddItemValidationIssue(Issues, TEXT("missing item effect"));
	}
	if (ItemType == EFortRogueItemType::Heal && HealAmount <= 0.0f)
	{
		AddItemValidationIssue(Issues, TEXT("heal amount must be greater than 0"));
	}
	if (ItemType == EFortRogueItemType::AttackMultiplier && AttackMultiplier <= 1.0f)
	{
		AddItemValidationIssue(Issues, TEXT("attack multiplier must be greater than 1"));
	}
	if (UseAbilitySet && !UseAbilitySet->GetDataValidationSummary().IsEmpty())
	{
		AddItemValidationIssue(Issues, TEXT("ability set data has warnings"));
	}
	for (const FFortRogueShotModifierSpec& ShotModifier : UseShotModifiers)
	{
		if (!ShotModifier.GetDataValidationSummary().IsEmpty())
		{
			AddItemValidationIssue(Issues, TEXT("shot modifier data has warnings"));
			break;
		}
	}

	return Issues.Num() > 0 ? FText::FromString(FString::Join(Issues, TEXT(" | "))) : FText::GetEmpty();
}
