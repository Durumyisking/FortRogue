// Copyright Epic Games, Inc. All Rights Reserved.

#include "Rewards/FRRewardTypes.h"

#include "AbilitySystem/FRAbilitySet.h"
#include "Combat/FRBattleCharacter.h"
#include "Rewards/FRRewardGrant.h"

namespace
{
void AddSummaryPart(TArray<FString>& Parts, const FString& Part)
{
	if (!Part.IsEmpty())
	{
		Parts.Add(Part);
	}
}

int32 CountRewardProjectileEffects(const TArray<FFRProjectileEffectSpec>& ProjectileEffects)
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
}

void AppendFortRogueAbilitySetSummary(TArray<FString>& Parts, const UFRAbilitySet* AbilitySet)
{
	if (!AbilitySet)
	{
		return;
	}

	const FString AbilitySetSummary = AbilitySet->GetEffectSummary().ToString();
	const FString SummaryText = AbilitySetSummary.IsEmpty() ? AbilitySet->GetName() : AbilitySetSummary;
	AddSummaryPart(Parts, FString::Printf(TEXT("ability set %s"), *SummaryText));
}

void AppendFortRogueShotModifierSummary(TArray<FString>& Parts, const TArray<FFRShotModifierSpec>& Modifiers)
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
	float LaunchSpeedMultiplier = 1.0f;
	float GravityMultiplier = 1.0f;
	int32 ProjectileEffectCount = 0;
	for (const FFRShotModifierSpec& Modifier : Modifiers)
	{
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
		for (const FFRProjectileEffectSpec& ProjectileEffect : Modifier.ProjectileEffects)
		{
			const FString ProjectileEffectName = ProjectileEffect.GetEffectDisplayName().ToString();
			if (!ProjectileEffectName.IsEmpty())
			{
				AddSummaryPart(Parts, FString::Printf(TEXT("projectile effect %s"), *ProjectileEffectName));
			}
		}

		ProjectileBonus += Modifier.ProjectileCountBonus;
		DamageBonus += Modifier.DamageBonus;
		DamageMultiplier *= Modifier.DamageMultiplier;
		BlastRadiusBonus += Modifier.BlastRadiusBonus;
		BlastRadiusMultiplier *= Modifier.BlastRadiusMultiplier;
		LaunchSpeedMultiplier *= Modifier.LaunchSpeedMultiplier;
		GravityMultiplier *= Modifier.GravityMultiplier;
		ProjectileEffectCount += CountRewardProjectileEffects(Modifier.ProjectileEffects);
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
	if (!FMath::IsNearlyEqual(LaunchSpeedMultiplier, 1.0f))
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("speed x%.2g"), LaunchSpeedMultiplier));
	}
	if (!FMath::IsNearlyEqual(GravityMultiplier, 1.0f))
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("gravity x%.2g"), GravityMultiplier));
	}
	if (ProjectileEffectCount > 0)
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("projectile effects %d"), ProjectileEffectCount));
	}
}

FText GetFortRogueShotModifierEffectSummary(const TArray<FFRShotModifierSpec>& ShotModifiers)
{
	TArray<FString> Parts;
	AppendFortRogueShotModifierSummary(Parts, ShotModifiers);
	return Parts.Num() > 0 ? FText::FromString(FString::Join(Parts, TEXT(" | "))) : FText::GetEmpty();
}

void FFRRewardChoice::ApplyToCharacter(AFRBattleCharacter& Character) const
{
	for (const UFRRewardGrant* Grant : Grants)
	{
		if (Grant)
		{
			Grant->ApplyToCharacter(Character);
		}
	}
}

FText FFRRewardChoice::GetResolvedDisplayName() const
{
	if (!DisplayName.IsEmpty())
	{
		return DisplayName;
	}

	for (const UFRRewardGrant* Grant : Grants)
	{
		if (Grant && !Grant->GetDefaultDisplayName().IsEmpty())
		{
			return Grant->GetDefaultDisplayName();
		}
	}
	return FText::GetEmpty();
}

FGameplayTag FFRRewardChoice::GetResolvedRewardTag() const
{
	if (RewardTag.IsValid())
	{
		return RewardTag;
	}

	for (const UFRRewardGrant* Grant : Grants)
	{
		if (Grant && Grant->GetDefaultRewardTag().IsValid())
		{
			return Grant->GetDefaultRewardTag();
		}
	}
	return FGameplayTag();
}

FText FFRRewardChoice::GetEffectSummary() const
{
	TArray<FString> Parts;
	const FString RewardDisplayName = GetResolvedDisplayName().ToString();
	if (!RewardDisplayName.IsEmpty())
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("reward %s"), *RewardDisplayName));
	}
	const FString RewardDescription = Description.ToString();
	if (!RewardDescription.IsEmpty())
	{
		AddSummaryPart(Parts, RewardDescription);
	}
	for (const UFRRewardGrant* Grant : Grants)
	{
		if (Grant)
		{
			Grant->AppendEffectSummary(Parts);
		}
	}
	if (RewardTag.IsValid())
	{
		AddSummaryPart(Parts, FString::Printf(TEXT("reward tag %s"), *RewardTag.ToString()));
	}
	if (bOfferOncePerRun && GetResolvedRewardTag().IsValid())
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

FText FFRRewardChoice::GetDataValidationSummary() const
{
	TArray<FString> Issues;
	if (GetResolvedDisplayName().IsEmpty())
	{
		AddSummaryPart(Issues, TEXT("missing display name"));
	}
	if (RewardWeight <= 0.0f)
	{
		AddSummaryPart(Issues, TEXT("reward weight must be greater than 0"));
	}
	if (bOfferOncePerRun && !GetResolvedRewardTag().IsValid())
	{
		AddSummaryPart(Issues, TEXT("once per run rewards need a RewardTag"));
	}

	bool bHasGameplayEffect = false;
	for (const UFRRewardGrant* Grant : Grants)
	{
		if (!Grant)
		{
			AddSummaryPart(Issues, TEXT("grants contain an empty entry"));
			continue;
		}
		Grant->AppendValidationIssues(Issues);
		bHasGameplayEffect |= Grant->HasGameplayEffect();
	}
	if (!bHasGameplayEffect)
	{
		AddSummaryPart(Issues, TEXT("missing gameplay effect"));
	}
	if (!RequiredRewardTags.IsEmpty() && !BlockedRewardTags.IsEmpty() && RequiredRewardTags.HasAny(BlockedRewardTags))
	{
		AddSummaryPart(Issues, TEXT("required and blocked reward tags overlap"));
	}

	return Issues.Num() > 0 ? FText::FromString(FString::Join(Issues, TEXT(" | "))) : FText::GetEmpty();
}

bool FFRRewardChoice::MeetsRewardTagConditions(const FGameplayTagContainer& ChosenRewardTags) const
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

FText FFRRewardChoice::GetRewardTagConditionFailureSummary(const FGameplayTagContainer& ChosenRewardTags) const
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

bool FFRRewardChoice::MatchesPerkCategoryFilter(const FGameplayTagContainer& RequiredCategoryTags, const FGameplayTagContainer& BlockedCategoryTags) const
{
	for (const UFRRewardGrant* Grant : Grants)
	{
		if (Grant && !Grant->MatchesPerkCategoryFilter(RequiredCategoryTags, BlockedCategoryTags))
		{
			return false;
		}
	}
	return true;
}
