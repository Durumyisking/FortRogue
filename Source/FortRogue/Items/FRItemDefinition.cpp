// Copyright Epic Games, Inc. All Rights Reserved.

#include "Items/FRItemDefinition.h"

#include "AbilitySystem/FRAbilitySet.h"
#include "Items/FRItemEffect.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

namespace
{
void AddItemValidationIssue(TArray<FString>& Issues, const FString& Issue)
{
	if (!Issue.IsEmpty())
	{
		Issues.Add(Issue);
	}
}

bool HasItemGameplayEffect(const UFRItemDefinition& ItemDefinition)
{
	if (ItemDefinition.UseAbilitySet)
	{
		return true;
	}
	for (const FFRShotModifierSpec& ShotModifier : ItemDefinition.UseShotModifiers)
	{
		if (ShotModifier.HasGameplayEffect())
		{
			return true;
		}
	}
	for (const UFRItemEffect* UseEffect : ItemDefinition.UseEffects)
	{
		if (UseEffect && UseEffect->HasGameplayEffect())
		{
			return true;
		}
	}
	return false;
}
}

FText UFRItemDefinition::GetDataValidationSummary() const
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
	for (const UFRItemEffect* UseEffect : UseEffects)
	{
		if (!UseEffect)
		{
			AddItemValidationIssue(Issues, TEXT("use effects contain an empty entry"));
			continue;
		}
		UseEffect->AppendValidationIssues(Issues);
	}
	if (UseAbilitySet && !UseAbilitySet->GetDataValidationSummary().IsEmpty())
	{
		AddItemValidationIssue(Issues, TEXT("ability set data has warnings"));
	}
	for (const FFRShotModifierSpec& ShotModifier : UseShotModifiers)
	{
		if (!ShotModifier.GetDataValidationSummary().IsEmpty())
		{
			AddItemValidationIssue(Issues, TEXT("shot modifier data has warnings"));
			break;
		}
	}

	return Issues.Num() > 0 ? FText::FromString(FString::Join(Issues, TEXT(" | "))) : FText::GetEmpty();
}

#if WITH_EDITOR
EDataValidationResult UFRItemDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	const FText ValidationSummary = GetDataValidationSummary();
	if (!ValidationSummary.IsEmpty())
	{
		Context.AddWarning(ValidationSummary);
	}
	return Result;
}
#endif
