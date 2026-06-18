// Copyright Epic Games, Inc. All Rights Reserved.

#include "Rewards/FortRogueRewardTypes.h"

#include "Items/FortRogueItemDefinition.h"
#include "Perks/FortRoguePerkDefinition.h"
#include "Weapons/FortRogueWeaponDefinition.h"

namespace
{
void AddSummaryPart(TArray<FString>& Parts, const FString& Part)
{
	if (!Part.IsEmpty())
	{
		Parts.Add(Part);
	}
}

void AddShotModifierSummary(TArray<FString>& Parts, const TArray<FFortRogueShotModifierSpec>& Modifiers)
{
	if (Modifiers.Num() <= 0)
	{
		return;
	}

	int32 ProjectileBonus = 0;
	float DamageMultiplier = 1.0f;
	float BlastRadiusMultiplier = 1.0f;
	float TerrainCarveBonus = 0.0f;
	float TerrainFillBonus = 0.0f;
	for (const FFortRogueShotModifierSpec& Modifier : Modifiers)
	{
		ProjectileBonus += Modifier.ProjectileCountBonus;
		DamageMultiplier *= Modifier.DamageMultiplier;
		BlastRadiusMultiplier *= Modifier.BlastRadiusMultiplier;
		TerrainCarveBonus += Modifier.TerrainCarveRadiusBonus;
		TerrainFillBonus += Modifier.TerrainFillRadiusBonus;
	}

	if (!FMath::IsNearlyEqual(DamageMultiplier, 1.0f))
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("shot damage x%.2g"), DamageMultiplier));
	}
	if (!FMath::IsNearlyEqual(BlastRadiusMultiplier, 1.0f))
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("blast x%.2g"), BlastRadiusMultiplier));
	}
	if (ProjectileBonus != 0)
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("projectiles %+d"), ProjectileBonus));
	}
	if (TerrainCarveBonus > 0.0f)
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("carve +%.0f"), TerrainCarveBonus));
	}
	if (TerrainFillBonus > 0.0f)
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("fill +%.0f"), TerrainFillBonus));
	}
}
}

FText FFortRogueRewardChoice::GetEffectSummary() const
{
	TArray<FString> Parts;
	if (WeaponReward)
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("weapon %s"), *WeaponReward->Weapon.DisplayName.ToString()));
		AddShotModifierSummary(Parts, WeaponReward->Weapon.ShotModifiers);
		if (WeaponReward->Weapon.ImpactSpawns.Num() > 0)
		{
			AddSummaryPart(Parts, TEXT("impact spawns"));
		}
	}
	if (ItemReward)
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("item %s"), *ItemReward->DisplayName.ToString()));
		AddShotModifierSummary(Parts, ItemReward->UseShotModifiers);
	}
	if (PerkReward)
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("perk %s"), *PerkReward->DisplayName.ToString()));
		AddShotModifierSummary(Parts, PerkReward->ShotModifiers);
	}
	if (GrantedAbilitySet)
	{
		AddSummaryPart(Parts, TEXT("grants AbilitySet"));
	}
	AddShotModifierSummary(Parts, ShotModifiers);

	if (DamageBonus > 0.0f)
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("damage +%.0f"), DamageBonus));
	}
	if (MaxHealthBonus > 0.0f)
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("max HP +%.0f"), MaxHealthBonus));
	}
	if (ProjectileBonus > 0)
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("projectiles +%d"), ProjectileBonus));
	}
	if (RepairCharges > 0)
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("charges +%d"), RepairCharges));
	}

	if (Parts.Num() <= 0)
	{
		return Description;
	}

	return FText::FromString(FString::Join(Parts, TEXT(" | ")));
}
