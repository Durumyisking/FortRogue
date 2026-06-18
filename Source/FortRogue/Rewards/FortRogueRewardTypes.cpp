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
	float DamageBonus = 0.0f;
	float DamageMultiplier = 1.0f;
	float BlastRadiusBonus = 0.0f;
	float BlastRadiusMultiplier = 1.0f;
	float TerrainCarveBonus = 0.0f;
	float TerrainCarveMultiplier = 1.0f;
	float TerrainFillBonus = 0.0f;
	float TerrainFillMultiplier = 1.0f;
	float LaunchSpeedMultiplier = 1.0f;
	float GravityMultiplier = 1.0f;
	int32 ImpactSpawnCount = 0;
	for (const FFortRogueShotModifierSpec& Modifier : Modifiers)
	{
		if (Modifier.bUseAimAngleRange)
		{
			AddSummaryPart(Parts, FString::Printf(TEXT("aim %.0f-%.0f deg"), Modifier.MinAimAngle, Modifier.MaxAimAngle));
		}
		if (Modifier.bRequireWindAligned)
		{
			AddSummaryPart(Parts, Modifier.MinWindMagnitude > 0.0f ? FString::Printf(TEXT("with wind %.0f+"), Modifier.MinWindMagnitude) : FString(TEXT("with wind")));
		}
		if (!Modifier.RequiredShotTags.IsEmpty())
		{
			AddSummaryPart(Parts, FString::Printf(TEXT("requires %s"), *Modifier.RequiredShotTags.ToStringSimple()));
		}
		if (!Modifier.BlockedShotTags.IsEmpty())
		{
			AddSummaryPart(Parts, FString::Printf(TEXT("blocks %s"), *Modifier.BlockedShotTags.ToStringSimple()));
		}

		ProjectileBonus += Modifier.ProjectileCountBonus;
		DamageBonus += Modifier.DamageBonus;
		DamageMultiplier *= Modifier.DamageMultiplier;
		BlastRadiusBonus += Modifier.BlastRadiusBonus;
		BlastRadiusMultiplier *= Modifier.BlastRadiusMultiplier;
		TerrainCarveBonus += Modifier.TerrainCarveRadiusBonus;
		TerrainCarveMultiplier *= Modifier.TerrainCarveRadiusMultiplier;
		TerrainFillBonus += Modifier.TerrainFillRadiusBonus;
		TerrainFillMultiplier *= Modifier.TerrainFillRadiusMultiplier;
		LaunchSpeedMultiplier *= Modifier.LaunchSpeedMultiplier;
		GravityMultiplier *= Modifier.GravityMultiplier;
		ImpactSpawnCount += Modifier.ImpactSpawns.Num();
	}

	if (!FMath::IsNearlyZero(DamageBonus))
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("shot damage %+.0f"), DamageBonus));
	}
	if (!FMath::IsNearlyEqual(DamageMultiplier, 1.0f))
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("shot damage x%.2g"), DamageMultiplier));
	}
	if (!FMath::IsNearlyZero(BlastRadiusBonus))
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("blast %+.0f"), BlastRadiusBonus));
	}
	if (!FMath::IsNearlyEqual(BlastRadiusMultiplier, 1.0f))
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("blast x%.2g"), BlastRadiusMultiplier));
	}
	if (ProjectileBonus != 0)
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("projectiles %+d"), ProjectileBonus));
	}
	if (!FMath::IsNearlyZero(TerrainCarveBonus))
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("carve %+.0f"), TerrainCarveBonus));
	}
	if (!FMath::IsNearlyEqual(TerrainCarveMultiplier, 1.0f))
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("carve x%.2g"), TerrainCarveMultiplier));
	}
	if (!FMath::IsNearlyZero(TerrainFillBonus))
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("fill %+.0f"), TerrainFillBonus));
	}
	if (!FMath::IsNearlyEqual(TerrainFillMultiplier, 1.0f))
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("fill x%.2g"), TerrainFillMultiplier));
	}
	if (!FMath::IsNearlyEqual(LaunchSpeedMultiplier, 1.0f))
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("speed x%.2g"), LaunchSpeedMultiplier));
	}
	if (!FMath::IsNearlyEqual(GravityMultiplier, 1.0f))
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("gravity x%.2g"), GravityMultiplier));
	}
	if (ImpactSpawnCount > 0)
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("impact spawns +%d"), ImpactSpawnCount));
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
		if (PerkReward->DamageBonus > 0.0f)
		{
			AddSummaryPart(Parts, FString::Printf(TEXT("damage +%.0f"), PerkReward->DamageBonus));
		}
		if (PerkReward->MaxHealthBonus > 0.0f)
		{
			AddSummaryPart(Parts, FString::Printf(TEXT("max HP +%.0f"), PerkReward->MaxHealthBonus));
		}
		if (PerkReward->MaxMoveBudgetBonus > 0.0f)
		{
			AddSummaryPart(Parts, FString::Printf(TEXT("move +%.0f"), PerkReward->MaxMoveBudgetBonus));
		}
		if (PerkReward->ProjectileBonus > 0)
		{
			AddSummaryPart(Parts, FString::Printf(TEXT("projectiles +%d"), PerkReward->ProjectileBonus));
		}
		if (PerkReward->ShotPowerMultiplierBonus > 0.0f)
		{
			AddSummaryPart(Parts, FString::Printf(TEXT("shot power +%.2g"), PerkReward->ShotPowerMultiplierBonus));
		}
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
	if (MaxMoveBudgetBonus > 0.0f)
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("move +%.0f"), MaxMoveBudgetBonus));
	}
	if (ProjectileBonus > 0)
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("projectiles +%d"), ProjectileBonus));
	}
	if (ShotPowerMultiplierBonus > 0.0f)
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("shot power +%.2g"), ShotPowerMultiplierBonus));
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
