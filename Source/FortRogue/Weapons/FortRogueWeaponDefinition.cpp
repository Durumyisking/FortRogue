// Copyright Epic Games, Inc. All Rights Reserved.

#include "Weapons/FortRogueWeaponDefinition.h"

#include "Combat/FortRogueShotSpec.h"

namespace
{
void AddShotModifierValidationIssue(TArray<FString>& Issues, const FString& Issue)
{
	if (!Issue.IsEmpty())
	{
		Issues.Add(Issue);
	}
}

bool HasShotModifierProjectileEffect(const TArray<FFRProjectileEffectSpec>& ProjectileEffects)
{
	for (const FFRProjectileEffectSpec& ProjectileEffect : ProjectileEffects)
	{
		if (ProjectileEffect.EffectClass)
		{
			return true;
		}
	}
	return false;
}

void AddWeaponValidationIssue(TArray<FString>& Issues, const FString& Issue)
{
	if (!Issue.IsEmpty())
	{
		Issues.Add(Issue);
	}
}

bool HasWeaponGameplayEffect(const FFortRogueWeaponSpec& Weapon)
{
	return Weapon.Damage > 0.0f
		|| Weapon.BlastRadius > 0.0f
		|| HasShotModifierProjectileEffect(Weapon.ProjectileEffects);
}

FGameplayTagContainer BuildShotConditionTags(const FFortRogueShotSpec& ShotSpec)
{
	FGameplayTagContainer ShotConditionTags = ShotSpec.EffectTags;
	if (ShotSpec.WeaponTag.IsValid())
	{
		ShotConditionTags.AddTag(ShotSpec.WeaponTag);
	}
	return ShotConditionTags;
}
}

bool FFortRogueShotModifierSpec::MeetsShotConditions(const FFortRogueShotSpec& CurrentShotSpec, float CurrentAimAngle, float Wind, bool bShotFacingRight) const
{
	if (bUseAimAngleRange)
	{
		const float MinAngle = FMath::Min(MinAimAngle, MaxAimAngle);
		const float MaxAngle = FMath::Max(MinAimAngle, MaxAimAngle);
		if (CurrentAimAngle < MinAngle || CurrentAimAngle > MaxAngle)
		{
			return false;
		}
	}
	if (bRequireWindAligned)
	{
		const float ShotDirection = bShotFacingRight ? 1.0f : -1.0f;
		if (FMath::Abs(Wind) < MinWindMagnitude || Wind * ShotDirection <= 0.0f)
		{
			return false;
		}
	}
	const FGameplayTagContainer ShotConditionTags = BuildShotConditionTags(CurrentShotSpec);
	if (!RequiredShotTags.IsEmpty() && !ShotConditionTags.HasAll(RequiredShotTags))
	{
		return false;
	}
	if (!BlockedShotTags.IsEmpty() && ShotConditionTags.HasAny(BlockedShotTags))
	{
		return false;
	}
	return true;
}

FText FFortRogueShotModifierSpec::GetShotConditionFailureSummary(const FFortRogueShotSpec& CurrentShotSpec, float CurrentAimAngle, float Wind, bool bShotFacingRight) const
{
	if (bUseAimAngleRange)
	{
		const float MinAngle = FMath::Min(MinAimAngle, MaxAimAngle);
		const float MaxAngle = FMath::Max(MinAimAngle, MaxAimAngle);
		if (CurrentAimAngle < MinAngle || CurrentAimAngle > MaxAngle)
		{
			return FText::FromString(FString::Printf(TEXT("requires aim %.0f-%.0f deg"), MinAngle, MaxAngle));
		}
	}
	if (bRequireWindAligned)
	{
		const float ShotDirection = bShotFacingRight ? 1.0f : -1.0f;
		if (FMath::Abs(Wind) < MinWindMagnitude || Wind * ShotDirection <= 0.0f)
		{
			return FText::FromString(MinWindMagnitude > 0.0f ? FString::Printf(TEXT("requires aligned wind %.0f+"), MinWindMagnitude) : FString(TEXT("requires aligned wind")));
		}
	}
	const FGameplayTagContainer ShotConditionTags = BuildShotConditionTags(CurrentShotSpec);
	if (!RequiredShotTags.IsEmpty() && !ShotConditionTags.HasAll(RequiredShotTags))
	{
		return FText::FromString(FString::Printf(TEXT("requires shot tag %s"), *RequiredShotTags.ToStringSimple()));
	}
	if (!BlockedShotTags.IsEmpty() && ShotConditionTags.HasAny(BlockedShotTags))
	{
		return FText::FromString(FString::Printf(TEXT("blocked by shot tag %s"), *BlockedShotTags.ToStringSimple()));
	}
	return FText::GetEmpty();
}

void FFortRogueShotModifierSpec::ApplyToShotSpec(FFortRogueShotSpec& ShotSpec) const
{
	ShotSpec.EffectTags.AppendTags(EffectTags);
	ShotSpec.Damage = FMath::Max(0.0f, (ShotSpec.Damage + DamageBonus) * DamageMultiplier);
	ShotSpec.BlastRadius = FMath::Max(0.0f, (ShotSpec.BlastRadius + BlastRadiusBonus) * BlastRadiusMultiplier);
	ShotSpec.LaunchSpeed = FMath::Max(0.0f, ShotSpec.LaunchSpeed * LaunchSpeedMultiplier);
	ShotSpec.Gravity = FMath::Max(0.0f, ShotSpec.Gravity * GravityMultiplier);
	ShotSpec.ProjectileCount = FMath::Max(1, ShotSpec.ProjectileCount + ProjectileCountBonus);
	for (const FFRProjectileEffectSpec& ProjectileEffect : ProjectileEffects)
	{
		ProjectileEffect.ApplyToShotSpec(ShotSpec);
		ShotSpec.ProjectileEffects.Add(ProjectileEffect);
	}
}

bool FFortRogueShotModifierSpec::HasGameplayEffect() const
{
	return !EffectTags.IsEmpty()
		|| HasShotModifierProjectileEffect(ProjectileEffects)
		|| !FMath::IsNearlyZero(DamageBonus)
		|| !FMath::IsNearlyEqual(DamageMultiplier, 1.0f)
		|| !FMath::IsNearlyZero(BlastRadiusBonus)
		|| !FMath::IsNearlyEqual(BlastRadiusMultiplier, 1.0f)
		|| !FMath::IsNearlyEqual(LaunchSpeedMultiplier, 1.0f)
		|| !FMath::IsNearlyEqual(GravityMultiplier, 1.0f)
		|| ProjectileCountBonus != 0;
}

FText FFortRogueShotModifierSpec::GetDataValidationSummary() const
{
	TArray<FString> Issues;
	if (!HasGameplayEffect())
	{
		AddShotModifierValidationIssue(Issues, TEXT("missing shot effect"));
	}
	if (bUseAimAngleRange && MinAimAngle > MaxAimAngle)
	{
		AddShotModifierValidationIssue(Issues, TEXT("aim range min is greater than max"));
	}
	if (!RequiredShotTags.IsEmpty() && !BlockedShotTags.IsEmpty() && RequiredShotTags.HasAny(BlockedShotTags))
	{
		AddShotModifierValidationIssue(Issues, TEXT("required and blocked shot tags overlap"));
	}

	bool bHasInvalidProjectileEffect = false;
	for (const FFRProjectileEffectSpec& ProjectileEffect : ProjectileEffects)
	{
		if (!ProjectileEffect.GetDataValidationSummary().IsEmpty())
		{
			bHasInvalidProjectileEffect = true;
			break;
		}
	}
	if (bHasInvalidProjectileEffect)
	{
		AddShotModifierValidationIssue(Issues, TEXT("projectile effect data has warnings"));
	}

	return Issues.Num() > 0 ? FText::FromString(FString::Join(Issues, TEXT(" | "))) : FText::GetEmpty();
}

FText FFortRogueWeaponSpec::GetDataValidationSummary() const
{
	TArray<FString> Issues;
	if (DisplayName.ToString().IsEmpty())
	{
		AddWeaponValidationIssue(Issues, TEXT("missing display name"));
	}
	if (!WeaponTag.IsValid())
	{
		AddWeaponValidationIssue(Issues, TEXT("missing WeaponTag"));
	}
	if (!HasWeaponGameplayEffect(*this))
	{
		AddWeaponValidationIssue(Issues, TEXT("missing weapon effect"));
	}
	if (ProjectileSpeed <= 0.0f)
	{
		AddWeaponValidationIssue(Issues, TEXT("projectile speed must be greater than 0"));
	}
	if (ProjectilesPerShot <= 0)
	{
		AddWeaponValidationIssue(Issues, TEXT("projectiles per shot must be greater than 0"));
	}
	if (SalvoCount <= 0)
	{
		AddWeaponValidationIssue(Issues, TEXT("salvo count must be greater than 0"));
	}
	if (SalvoCount > 1 && SalvoInterval <= 0.0f)
	{
		AddWeaponValidationIssue(Issues, TEXT("salvo interval must be greater than 0"));
	}

	bool bHasInvalidProjectileEffect = false;
	for (const FFRProjectileEffectSpec& ProjectileEffect : ProjectileEffects)
	{
		if (!ProjectileEffect.GetDataValidationSummary().IsEmpty())
		{
			bHasInvalidProjectileEffect = true;
			break;
		}
	}
	if (bHasInvalidProjectileEffect)
	{
		AddWeaponValidationIssue(Issues, TEXT("projectile effect data has warnings"));
	}

	return Issues.Num() > 0 ? FText::FromString(FString::Join(Issues, TEXT(" | "))) : FText::GetEmpty();
}

FText UFortRogueWeaponDefinition::GetDataValidationSummary() const
{
	return Weapon.GetDataValidationSummary();
}
