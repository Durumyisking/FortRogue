// Copyright Epic Games, Inc. All Rights Reserved.

#include "Run/FortRogueDefaultLoadoutDefinition.h"

#include "Items/FortRogueItemDefinition.h"
#include "Weapons/FortRogueWeaponDefinition.h"

namespace
{
void AddDefaultLoadoutValidationIssue(TArray<FString>& Issues, const FString& Issue)
{
	if (!Issue.IsEmpty())
	{
		Issues.Add(Issue);
	}
}
}

FText UFortRogueDefaultLoadoutDefinition::GetDataValidationSummary() const
{
	TArray<FString> Issues;
	if (WeaponDefinitions.Num() <= 0)
	{
		AddDefaultLoadoutValidationIssue(Issues, TEXT("weapon definitions are empty"));
	}

	bool bHasMissingWeapon = false;
	bool bHasWeaponDataWarning = false;
	for (const UFortRogueWeaponDefinition* WeaponDefinition : WeaponDefinitions)
	{
		if (!WeaponDefinition)
		{
			bHasMissingWeapon = true;
			continue;
		}
		if (!WeaponDefinition->GetDataValidationSummary().IsEmpty())
		{
			bHasWeaponDataWarning = true;
		}
	}
	if (bHasMissingWeapon)
	{
		AddDefaultLoadoutValidationIssue(Issues, TEXT("missing weapon entry"));
	}
	if (bHasWeaponDataWarning)
	{
		AddDefaultLoadoutValidationIssue(Issues, TEXT("weapon data has warnings"));
	}

	bool bHasMissingItem = false;
	bool bHasInvalidItemCharges = false;
	bool bHasItemDataWarning = false;
	for (const FFortRogueDefaultItemStack& ItemStack : ItemDefinitions)
	{
		if (!ItemStack.ItemDefinition)
		{
			bHasMissingItem = true;
		}
		else if (!ItemStack.ItemDefinition->GetDataValidationSummary().IsEmpty())
		{
			bHasItemDataWarning = true;
		}
		if (ItemStack.Charges <= 0)
		{
			bHasInvalidItemCharges = true;
		}
	}
	if (bHasMissingItem)
	{
		AddDefaultLoadoutValidationIssue(Issues, TEXT("missing item entry"));
	}
	if (bHasInvalidItemCharges)
	{
		AddDefaultLoadoutValidationIssue(Issues, TEXT("item charges must be greater than 0"));
	}
	if (bHasItemDataWarning)
	{
		AddDefaultLoadoutValidationIssue(Issues, TEXT("item data has warnings"));
	}

	return Issues.Num() > 0 ? FText::FromString(FString::Join(Issues, TEXT(" | "))) : FText::GetEmpty();
}
