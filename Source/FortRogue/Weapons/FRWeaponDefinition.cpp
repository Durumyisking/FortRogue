// Copyright Epic Games, Inc. All Rights Reserved.

#include "Weapons/FRWeaponDefinition.h"

#include "Combat/FRShotSpec.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

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

bool HasWeaponGameplayEffect(const FFRWeaponSpec& Weapon)
{
	return Weapon.HitDamage > 0.0f
		|| Weapon.Damage > 0.0f
		|| Weapon.BlastRadius > 0.0f
		|| Weapon.TerrainDamage > 0.0f
		|| HasShotModifierProjectileEffect(Weapon.ProjectileEffects);
}

FGameplayTagContainer BuildShotConditionTags(const FFRShotSpec& ShotSpec)
{
	FGameplayTagContainer ShotConditionTags = ShotSpec.EffectTags;
	if (ShotSpec.WeaponTag.IsValid())
	{
		ShotConditionTags.AddTag(ShotSpec.WeaponTag);
	}
	return ShotConditionTags;
}
}

bool FFRShotModifierSpec::MeetsShotConditions(const FFRShotSpec& CurrentShotSpec, float CurrentAimAngle, float Wind, bool bShotFacingRight) const
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
	if (bRequireWindOpposed)
	{
		const float ShotDirection = bShotFacingRight ? 1.0f : -1.0f;
		if (FMath::Abs(Wind) < MinWindMagnitude || Wind * ShotDirection >= 0.0f)
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

FText FFRShotModifierSpec::GetShotConditionFailureSummary(const FFRShotSpec& CurrentShotSpec, float CurrentAimAngle, float Wind, bool bShotFacingRight) const
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
	if (bRequireWindOpposed)
	{
		const float ShotDirection = bShotFacingRight ? 1.0f : -1.0f;
		if (FMath::Abs(Wind) < MinWindMagnitude || Wind * ShotDirection >= 0.0f)
		{
			return FText::FromString(MinWindMagnitude > 0.0f ? FString::Printf(TEXT("requires opposed wind %.0f+"), MinWindMagnitude) : FString(TEXT("requires opposed wind")));
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

void FFRShotModifierSpec::ApplyToShotSpec(FFRShotSpec& ShotSpec) const
{
	ShotSpec.EffectTags.AppendTags(EffectTags);
	ShotSpec.Damage = FMath::Max(0.0f, (ShotSpec.Damage + DamageBonus) * DamageMultiplier);
	ShotSpec.BlastRadius = FMath::Max(0.0f, (ShotSpec.BlastRadius + BlastRadiusBonus) * BlastRadiusMultiplier);
	ShotSpec.ExplosionFullDamageRadius = FMath::Clamp(ShotSpec.ExplosionFullDamageRadius, 0.0f, ShotSpec.BlastRadius);
	ShotSpec.LaunchSpeed = FMath::Max(0.0f, ShotSpec.LaunchSpeed * LaunchSpeedMultiplier);
	ShotSpec.Gravity = FMath::Max(0.0f, ShotSpec.Gravity * GravityMultiplier);
	ShotSpec.ProjectileCount = FMath::Max(1, ShotSpec.ProjectileCount + ProjectileCountBonus);
	for (const FFRProjectileEffectSpec& ProjectileEffect : ProjectileEffects)
	{
		ProjectileEffect.ApplyToShotSpec(ShotSpec);
		if (ProjectileEffect.RequiresProjectileRuntime())
		{
			ShotSpec.ProjectileEffects.Add(ProjectileEffect);
		}
	}
}

bool FFRShotModifierSpec::HasGameplayEffect() const
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

FText FFRShotModifierSpec::GetDataValidationSummary() const
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
	if (bRequireWindAligned && bRequireWindOpposed)
	{
		AddShotModifierValidationIssue(Issues, TEXT("wind cannot require aligned and opposed directions together"));
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

FText FFRWeaponSpec::GetDataValidationSummary() const
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

FText UFRWeaponDefinition::GetDataValidationSummary() const
{
	return Weapon.GetDataValidationSummary();
}

#if WITH_EDITOR
EDataValidationResult UFRWeaponDefinition::IsDataValid(FDataValidationContext& Context) const
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
