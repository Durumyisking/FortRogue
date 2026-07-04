// Copyright Epic Games, Inc. All Rights Reserved.

#include "Rewards/FRRewardGrant.h"

#include "Combat/FRBattleCharacter.h"
#include "Items/FRItemDefinition.h"
#include "Items/FRItemEffect.h"
#include "Perks/FRPerkDefinition.h"
#include "Rewards/FRRewardTypes.h"
#include "Weapons/FRWeaponDefinition.h"

namespace
{
void AddGrantSummaryPart(TArray<FString>& Parts, const FString& Part)
{
	if (!Part.IsEmpty())
	{
		Parts.Add(Part);
	}
}

int32 CountGrantProjectileEffects(const TArray<FFRProjectileEffectSpec>& ProjectileEffects)
{
	int32 TotalCount = 0;
	for (const FFRProjectileEffectSpec& ProjectileEffect : ProjectileEffects)
	{
		if (ProjectileEffect.EffectClass)
		{
			++TotalCount;
		}
	}
	return TotalCount;
}

FString GetPerkRarityName(EFRPerkRarity Rarity)
{
	switch (Rarity)
	{
	case EFRPerkRarity::Common:
		return TEXT("Common");
	case EFRPerkRarity::Rare:
		return TEXT("Rare");
	case EFRPerkRarity::Epic:
		return TEXT("Epic");
	case EFRPerkRarity::Legendary:
		return TEXT("Legendary");
	default:
		return TEXT("Common");
	}
}
}

void UFRRewardGrant_Weapon::ApplyToCharacter(AFRBattleCharacter& Character) const
{
	if (!WeaponDefinition)
	{
		return;
	}

	// 같은 무기를 보상으로 두 번 받아도 중복 슬롯을 만들지 않습니다.
	const FGameplayTag NewWeaponTag = WeaponDefinition->Weapon.WeaponTag;
	if (NewWeaponTag.IsValid() && Character.GetWeaponIndexByTag(NewWeaponTag) != INDEX_NONE)
	{
		return;
	}

	Character.AddWeaponDefinition(WeaponDefinition);
}

void UFRRewardGrant_Weapon::AppendEffectSummary(TArray<FString>& Parts) const
{
	if (!WeaponDefinition)
	{
		return;
	}

	const FFRWeaponSpec& Weapon = WeaponDefinition->Weapon;
	AddGrantSummaryPart(Parts, FString::Printf(TEXT("weapon %s"), *Weapon.DisplayName.ToString()));
	const FString WeaponDescription = Weapon.Description.ToString();
	if (!WeaponDescription.IsEmpty())
	{
		AddGrantSummaryPart(Parts, WeaponDescription);
	}
	if (Weapon.WeaponTag.IsValid())
	{
		AddGrantSummaryPart(Parts, FString::Printf(TEXT("tag %s"), *Weapon.WeaponTag.ToString()));
	}
	if (Weapon.HitDamage > 0.0f)
	{
		AddGrantSummaryPart(Parts, FString::Printf(TEXT("hit damage %.0f"), Weapon.HitDamage));
	}
	if (Weapon.Damage > 0.0f)
	{
		AddGrantSummaryPart(Parts, FString::Printf(TEXT("explosion damage %.0f"), Weapon.Damage));
	}
	if (Weapon.BlastRadius > 0.0f)
	{
		AddGrantSummaryPart(Parts, FString::Printf(TEXT("blast %.0f"), Weapon.BlastRadius));
	}
	if (Weapon.TerrainDamage > 0.0f)
	{
		AddGrantSummaryPart(Parts, FString::Printf(TEXT("terrain damage %.0f"), Weapon.TerrainDamage));
	}
	if (Weapon.ProjectilesPerShot > 1)
	{
		AddGrantSummaryPart(Parts, FString::Printf(TEXT("projectiles %d"), Weapon.ProjectilesPerShot));
	}
	for (const FFRProjectileEffectSpec& ProjectileEffect : Weapon.ProjectileEffects)
	{
		const FString ProjectileEffectName = ProjectileEffect.GetEffectDisplayName().ToString();
		if (!ProjectileEffectName.IsEmpty())
		{
			AddGrantSummaryPart(Parts, FString::Printf(TEXT("projectile effect %s"), *ProjectileEffectName));
		}
	}
	const int32 ProjectileEffectCount = CountGrantProjectileEffects(Weapon.ProjectileEffects);
	if (ProjectileEffectCount > 0)
	{
		AddGrantSummaryPart(Parts, FString::Printf(TEXT("projectile effects %d"), ProjectileEffectCount));
	}
}

void UFRRewardGrant_Weapon::AppendValidationIssues(TArray<FString>& Issues) const
{
	if (!WeaponDefinition)
	{
		Issues.Add(TEXT("weapon grant is missing a weapon definition"));
		return;
	}
	if (!WeaponDefinition->GetDataValidationSummary().IsEmpty())
	{
		Issues.Add(TEXT("weapon data has warnings"));
	}
}

bool UFRRewardGrant_Weapon::HasGameplayEffect() const
{
	return WeaponDefinition != nullptr;
}

FGameplayTag UFRRewardGrant_Weapon::GetDefaultRewardTag() const
{
	return WeaponDefinition ? WeaponDefinition->Weapon.WeaponTag : FGameplayTag();
}

FText UFRRewardGrant_Weapon::GetDefaultDisplayName() const
{
	return WeaponDefinition ? WeaponDefinition->Weapon.DisplayName : FText::GetEmpty();
}

void UFRRewardGrant_Item::ApplyToCharacter(AFRBattleCharacter& Character) const
{
	Character.AddItemDefinition(ItemDefinition, ChargesOverride);
}

void UFRRewardGrant_Item::AppendEffectSummary(TArray<FString>& Parts) const
{
	if (!ItemDefinition)
	{
		return;
	}

	AddGrantSummaryPart(Parts, FString::Printf(TEXT("item %s"), *ItemDefinition->DisplayName.ToString()));
	const FString ItemDescription = ItemDefinition->Description.ToString();
	if (!ItemDescription.IsEmpty())
	{
		AddGrantSummaryPart(Parts, ItemDescription);
	}
	if (ItemDefinition->ItemTag.IsValid())
	{
		AddGrantSummaryPart(Parts, FString::Printf(TEXT("tag %s"), *ItemDefinition->ItemTag.ToString()));
	}
	const int32 GrantedCharges = ChargesOverride >= 0 ? ChargesOverride : ItemDefinition->InitialCharges;
	if (GrantedCharges > 1)
	{
		AddGrantSummaryPart(Parts, FString::Printf(TEXT("charges %d"), GrantedCharges));
	}
	for (const UFRItemEffect* UseEffect : ItemDefinition->UseEffects)
	{
		if (UseEffect)
		{
			UseEffect->AppendEffectSummary(Parts);
		}
	}
	AppendFortRogueShotModifierSummary(Parts, ItemDefinition->UseShotModifiers);
	AppendFortRogueAbilitySetSummary(Parts, ItemDefinition->UseAbilitySet);
}

void UFRRewardGrant_Item::AppendValidationIssues(TArray<FString>& Issues) const
{
	if (!ItemDefinition)
	{
		Issues.Add(TEXT("item grant is missing an item definition"));
		return;
	}
	if (!ItemDefinition->GetDataValidationSummary().IsEmpty())
	{
		Issues.Add(TEXT("item data has warnings"));
	}
}

bool UFRRewardGrant_Item::HasGameplayEffect() const
{
	return ItemDefinition != nullptr;
}

FGameplayTag UFRRewardGrant_Item::GetDefaultRewardTag() const
{
	return ItemDefinition ? ItemDefinition->ItemTag : FGameplayTag();
}

FText UFRRewardGrant_Item::GetDefaultDisplayName() const
{
	return ItemDefinition ? ItemDefinition->DisplayName : FText::GetEmpty();
}

void UFRRewardGrant_Perk::ApplyToCharacter(AFRBattleCharacter& Character) const
{
	Character.ApplyPerkDefinition(PerkDefinition);
}

void UFRRewardGrant_Perk::AppendEffectSummary(TArray<FString>& Parts) const
{
	if (!PerkDefinition)
	{
		return;
	}

	AddGrantSummaryPart(Parts, FString::Printf(TEXT("perk %s"), *PerkDefinition->DisplayName.ToString()));
	const FString PerkDescription = PerkDefinition->Description.ToString();
	if (!PerkDescription.IsEmpty())
	{
		AddGrantSummaryPart(Parts, PerkDescription);
	}
	if (PerkDefinition->PerkTag.IsValid())
	{
		AddGrantSummaryPart(Parts, FString::Printf(TEXT("tag %s"), *PerkDefinition->PerkTag.ToString()));
	}
	AddGrantSummaryPart(Parts, FString::Printf(TEXT("rarity %s"), *GetPerkRarityName(PerkDefinition->Rarity)));
	AppendFortRogueShotModifierSummary(Parts, PerkDefinition->ShotModifiers);
	AppendFortRogueAbilitySetSummary(Parts, PerkDefinition->GrantedAbilitySet);
	if (!FMath::IsNearlyZero(PerkDefinition->DamageBonus))
	{
		AddGrantSummaryPart(Parts, FString::Printf(TEXT("damage %+.0f"), PerkDefinition->DamageBonus));
	}
	if (!FMath::IsNearlyZero(PerkDefinition->MaxHealthBonus))
	{
		AddGrantSummaryPart(Parts, FString::Printf(TEXT("max HP %+.0f"), PerkDefinition->MaxHealthBonus));
	}
	if (!FMath::IsNearlyZero(PerkDefinition->MaxMoveBudgetBonus))
	{
		AddGrantSummaryPart(Parts, FString::Printf(TEXT("move %+.0f"), PerkDefinition->MaxMoveBudgetBonus));
	}
	if (PerkDefinition->ProjectileBonus != 0)
	{
		AddGrantSummaryPart(Parts, FString::Printf(TEXT("projectiles %+d"), PerkDefinition->ProjectileBonus));
	}
	if (!FMath::IsNearlyZero(PerkDefinition->ShotPowerMultiplierBonus))
	{
		AddGrantSummaryPart(Parts, FString::Printf(TEXT("shot power %+.2g"), PerkDefinition->ShotPowerMultiplierBonus));
	}
}

void UFRRewardGrant_Perk::AppendValidationIssues(TArray<FString>& Issues) const
{
	if (!PerkDefinition)
	{
		Issues.Add(TEXT("perk grant is missing a perk definition"));
		return;
	}
	if (!PerkDefinition->GetDataValidationSummary().IsEmpty())
	{
		Issues.Add(TEXT("perk data has warnings"));
	}
}

bool UFRRewardGrant_Perk::HasGameplayEffect() const
{
	return PerkDefinition != nullptr;
}

bool UFRRewardGrant_Perk::MatchesPerkCategoryFilter(const FGameplayTagContainer& RequiredCategoryTags, const FGameplayTagContainer& BlockedCategoryTags) const
{
	if (!PerkDefinition)
	{
		return RequiredCategoryTags.IsEmpty();
	}
	if (!PerkDefinition->HasAllCategoryTags(RequiredCategoryTags))
	{
		return false;
	}
	return BlockedCategoryTags.IsEmpty() || !PerkDefinition->HasAnyCategoryTags(BlockedCategoryTags);
}

FGameplayTag UFRRewardGrant_Perk::GetDefaultRewardTag() const
{
	return PerkDefinition ? PerkDefinition->PerkTag : FGameplayTag();
}

FText UFRRewardGrant_Perk::GetDefaultDisplayName() const
{
	return PerkDefinition ? PerkDefinition->DisplayName : FText::GetEmpty();
}
