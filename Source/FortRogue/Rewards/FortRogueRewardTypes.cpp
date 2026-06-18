// Copyright Epic Games, Inc. All Rights Reserved.

#include "Rewards/FortRogueRewardTypes.h"

#include "AbilitySystem/FortRogueAbilitySet.h"
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

void AddAbilitySetSummary(TArray<FString>& Parts, const UFortRogueAbilitySet* AbilitySet)
{
	if (!AbilitySet)
	{
		return;
	}

	const FString AbilitySetSummary = AbilitySet->GetEffectSummary().ToString();
	const FString SummaryText = AbilitySetSummary.IsEmpty() ? AbilitySet->GetName() : AbilitySetSummary;
	AddSummaryPart(Parts, FString::Printf(TEXT("ability set %s"), *SummaryText));
}

int32 CountImpactSpawnProjectiles(const TArray<FFortRogueImpactSpawnSpec>& ImpactSpawns)
{
	int32 TotalCount = 0;
	for (const FFortRogueImpactSpawnSpec& ImpactSpawn : ImpactSpawns)
	{
		TotalCount += FMath::Max(0, ImpactSpawn.ProjectileCount);
	}
	return TotalCount;
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
		const FString ModifierDisplayName = Modifier.DisplayName.ToString();
		if (!ModifierDisplayName.IsEmpty())
		{
			AddSummaryPart(Parts, FString::Printf(TEXT("modifier %s"), *ModifierDisplayName));
		}
		const FString ModifierDescription = Modifier.Description.ToString();
		if (!ModifierDescription.IsEmpty())
		{
			AddSummaryPart(Parts, ModifierDescription);
		}
		if (Modifier.ModifierTag.IsValid())
		{
			AddSummaryPart(Parts, FString::Printf(TEXT("modifier tag %s"), *Modifier.ModifierTag.ToString()));
		}
		if (!Modifier.EffectTags.IsEmpty())
		{
			AddSummaryPart(Parts, FString::Printf(TEXT("effect tags %s"), *Modifier.EffectTags.ToStringSimple()));
		}
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
		ImpactSpawnCount += CountImpactSpawnProjectiles(Modifier.ImpactSpawns);
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
		AddSummaryPart(Parts, FString::Printf(TEXT("impact projectiles +%d"), ImpactSpawnCount));
	}
}
}

FText FFortRogueRewardChoice::GetEffectSummary() const
{
	TArray<FString> Parts;
	const FString RewardDisplayName = DisplayName.ToString();
	if (!RewardDisplayName.IsEmpty())
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("reward %s"), *RewardDisplayName));
	}
	const FString RewardDescription = Description.ToString();
	if (!RewardDescription.IsEmpty())
	{
		AddSummaryPart(Parts, RewardDescription);
	}
	if (WeaponReward)
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("weapon %s"), *WeaponReward->Weapon.DisplayName.ToString()));
		const FString WeaponDescription = WeaponReward->Weapon.Description.ToString();
		if (!WeaponDescription.IsEmpty())
		{
			AddSummaryPart(Parts, WeaponDescription);
		}
		if (WeaponReward->Weapon.WeaponTag.IsValid())
		{
			AddSummaryPart(Parts, FString::Printf(TEXT("tag %s"), *WeaponReward->Weapon.WeaponTag.ToString()));
		}
		if (WeaponReward->Weapon.Damage > 0.0f)
		{
			AddSummaryPart(Parts, FString::Printf(TEXT("damage %.0f"), WeaponReward->Weapon.Damage));
		}
		if (WeaponReward->Weapon.BlastRadius > 0.0f)
		{
			AddSummaryPart(Parts, FString::Printf(TEXT("blast %.0f"), WeaponReward->Weapon.BlastRadius));
		}
		if (WeaponReward->Weapon.ProjectilesPerShot > 1)
		{
			AddSummaryPart(Parts, FString::Printf(TEXT("projectiles %d"), WeaponReward->Weapon.ProjectilesPerShot));
		}
		AddShotModifierSummary(Parts, WeaponReward->Weapon.ShotModifiers);
		const int32 ImpactSpawnCount = CountImpactSpawnProjectiles(WeaponReward->Weapon.ImpactSpawns);
		if (ImpactSpawnCount > 0)
		{
			AddSummaryPart(Parts, FString::Printf(TEXT("impact projectiles %d"), ImpactSpawnCount));
		}
	}
	if (ItemReward)
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("item %s"), *ItemReward->DisplayName.ToString()));
		const FString ItemDescription = ItemReward->Description.ToString();
		if (!ItemDescription.IsEmpty())
		{
			AddSummaryPart(Parts, ItemDescription);
		}
		if (ItemReward->ItemTag.IsValid())
		{
			AddSummaryPart(Parts, FString::Printf(TEXT("tag %s"), *ItemReward->ItemTag.ToString()));
		}
		if (RepairCharges <= 0 && ItemReward->InitialCharges > 1)
		{
			AddSummaryPart(Parts, FString::Printf(TEXT("charges %d"), ItemReward->InitialCharges));
		}
		if (ItemReward->ItemType == EFortRogueItemType::Heal && ItemReward->HealAmount > 0.0f)
		{
			AddSummaryPart(Parts, FString::Printf(TEXT("heal +%.0f"), ItemReward->HealAmount));
		}
		if (ItemReward->ItemType == EFortRogueItemType::AttackMultiplier && !FMath::IsNearlyEqual(ItemReward->AttackMultiplier, 1.0f))
		{
			AddSummaryPart(Parts, FString::Printf(TEXT("next shot attack x%.2g"), ItemReward->AttackMultiplier));
		}
		AddShotModifierSummary(Parts, ItemReward->UseShotModifiers);
		AddAbilitySetSummary(Parts, ItemReward->UseAbilitySet);
	}
	if (PerkReward)
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("perk %s"), *PerkReward->DisplayName.ToString()));
		const FString PerkDescription = PerkReward->Description.ToString();
		if (!PerkDescription.IsEmpty())
		{
			AddSummaryPart(Parts, PerkDescription);
		}
		if (PerkReward->PerkTag.IsValid())
		{
			AddSummaryPart(Parts, FString::Printf(TEXT("tag %s"), *PerkReward->PerkTag.ToString()));
		}
		AddShotModifierSummary(Parts, PerkReward->ShotModifiers);
		AddAbilitySetSummary(Parts, PerkReward->GrantedAbilitySet);
		if (!FMath::IsNearlyZero(PerkReward->DamageBonus))
		{
			AddSummaryPart(Parts, FString::Printf(TEXT("damage %+.0f"), PerkReward->DamageBonus));
		}
		if (!FMath::IsNearlyZero(PerkReward->MaxHealthBonus))
		{
			AddSummaryPart(Parts, FString::Printf(TEXT("max HP %+.0f"), PerkReward->MaxHealthBonus));
		}
		if (!FMath::IsNearlyZero(PerkReward->MaxMoveBudgetBonus))
		{
			AddSummaryPart(Parts, FString::Printf(TEXT("move %+.0f"), PerkReward->MaxMoveBudgetBonus));
		}
		if (PerkReward->ProjectileBonus != 0)
		{
			AddSummaryPart(Parts, FString::Printf(TEXT("projectiles %+d"), PerkReward->ProjectileBonus));
		}
		if (!FMath::IsNearlyZero(PerkReward->ShotPowerMultiplierBonus))
		{
			AddSummaryPart(Parts, FString::Printf(TEXT("shot power %+.2g"), PerkReward->ShotPowerMultiplierBonus));
		}
	}
	if (GrantedAbilitySet)
	{
		AddAbilitySetSummary(Parts, GrantedAbilitySet);
	}
	AddShotModifierSummary(Parts, ShotModifiers);

	if (!FMath::IsNearlyZero(DamageBonus))
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("damage %+.0f"), DamageBonus));
	}
	if (!FMath::IsNearlyZero(MaxHealthBonus))
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("max HP %+.0f"), MaxHealthBonus));
	}
	if (!FMath::IsNearlyZero(MaxMoveBudgetBonus))
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("move %+.0f"), MaxMoveBudgetBonus));
	}
	if (ProjectileBonus != 0)
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("projectiles %+d"), ProjectileBonus));
	}
	if (!FMath::IsNearlyZero(ShotPowerMultiplierBonus))
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("shot power %+.2g"), ShotPowerMultiplierBonus));
	}
	if (RepairCharges > 0)
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("charges +%d"), RepairCharges));
	}
	if (RewardTag.IsValid())
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("reward tag %s"), *RewardTag.ToString()));
	}
	if (bOfferOncePerRun && RewardTag.IsValid())
	{
		AddSummaryPart(Parts, TEXT("once per run"));
	}
	if (!RequiredRewardTags.IsEmpty())
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("requires reward %s"), *RequiredRewardTags.ToStringSimple()));
	}
	if (!BlockedRewardTags.IsEmpty())
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("blocks reward %s"), *BlockedRewardTags.ToStringSimple()));
	}

	if (Parts.Num() <= 0)
	{
		return Description;
	}

	return FText::FromString(FString::Join(Parts, TEXT(" | ")));
}

bool FFortRogueRewardChoice::MeetsRewardTagConditions(const FGameplayTagContainer& ChosenRewardTags) const
{
	if (!RequiredRewardTags.IsEmpty() && !ChosenRewardTags.HasAll(RequiredRewardTags))
	{
		return false;
	}
	if (!BlockedRewardTags.IsEmpty() && ChosenRewardTags.HasAny(BlockedRewardTags))
	{
		return false;
	}
	return true;
}

FText FFortRogueRewardChoice::GetRewardTagConditionFailureSummary(const FGameplayTagContainer& ChosenRewardTags) const
{
	if (!RequiredRewardTags.IsEmpty() && !ChosenRewardTags.HasAll(RequiredRewardTags))
	{
		return FText::FromString(FString::Printf(TEXT("requires reward %s"), *RequiredRewardTags.ToStringSimple()));
	}
	if (!BlockedRewardTags.IsEmpty() && ChosenRewardTags.HasAny(BlockedRewardTags))
	{
		return FText::FromString(FString::Printf(TEXT("blocked by reward %s"), *BlockedRewardTags.ToStringSimple()));
	}
	return FText::GetEmpty();
}
