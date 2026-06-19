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

bool HasShotModifierImpactSpawnEffect(const TArray<FFortRogueImpactSpawnSpec>& ImpactSpawns)
{
	for (const FFortRogueImpactSpawnSpec& ImpactSpawn : ImpactSpawns)
	{
		if (ImpactSpawn.ProjectileCount > 0)
		{
			return true;
		}
	}
	return false;
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

bool HasWeaponImpactSpawnEffect(const TArray<FFortRogueImpactSpawnSpec>& ImpactSpawns)
{
	for (const FFortRogueImpactSpawnSpec& ImpactSpawn : ImpactSpawns)
	{
		if (ImpactSpawn.ProjectileCount > 0)
		{
			return true;
		}
	}
	return false;
}

bool HasWeaponGameplayEffect(const FFortRogueWeaponSpec& Weapon)
{
	if (Weapon.Damage > 0.0f || Weapon.BlastRadius > 0.0f || HasWeaponImpactSpawnEffect(Weapon.ImpactSpawns))
	{
		return true;
	}

	for (const FFortRogueShotModifierSpec& ShotModifier : Weapon.ShotModifiers)
	{
		if (ShotModifier.HasGameplayEffect())
		{
			return true;
		}
	}
	return false;
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
	if (!RequiredShotTags.IsEmpty() && !CurrentShotSpec.EffectTags.HasAny(RequiredShotTags))
	{
		return false;
	}
	if (!BlockedShotTags.IsEmpty() && CurrentShotSpec.EffectTags.HasAny(BlockedShotTags))
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
	if (!RequiredShotTags.IsEmpty() && !CurrentShotSpec.EffectTags.HasAny(RequiredShotTags))
	{
		return FText::FromString(FString::Printf(TEXT("requires shot tag %s"), *RequiredShotTags.ToStringSimple()));
	}
	if (!BlockedShotTags.IsEmpty() && CurrentShotSpec.EffectTags.HasAny(BlockedShotTags))
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
	ShotSpec.TerrainCarveRadius = FMath::Max(0.0f, (ShotSpec.TerrainCarveRadius + TerrainCarveRadiusBonus) * TerrainCarveRadiusMultiplier);
	ShotSpec.TerrainFillRadius = FMath::Max(0.0f, (ShotSpec.TerrainFillRadius + TerrainFillRadiusBonus) * TerrainFillRadiusMultiplier);
	ShotSpec.LaunchSpeed = FMath::Max(0.0f, ShotSpec.LaunchSpeed * LaunchSpeedMultiplier);
	ShotSpec.Gravity = FMath::Max(0.0f, ShotSpec.Gravity * GravityMultiplier);
	ShotSpec.ProjectileCount = FMath::Max(1, ShotSpec.ProjectileCount + ProjectileCountBonus);
	for (const FFRProjectileEffectSpec& ProjectileEffect : ProjectileEffects)
	{
		ProjectileEffect.ApplyToShotSpec(ShotSpec);
		ShotSpec.ProjectileEffects.Add(ProjectileEffect);
	}
	ShotSpec.ImpactSpawns.Append(ImpactSpawns);
}

bool FFortRogueShotModifierSpec::HasGameplayEffect() const
{
	return !EffectTags.IsEmpty()
		|| HasShotModifierProjectileEffect(ProjectileEffects)
		|| !FMath::IsNearlyZero(DamageBonus)
		|| !FMath::IsNearlyEqual(DamageMultiplier, 1.0f)
		|| !FMath::IsNearlyZero(BlastRadiusBonus)
		|| !FMath::IsNearlyEqual(BlastRadiusMultiplier, 1.0f)
		|| !FMath::IsNearlyZero(TerrainCarveRadiusBonus)
		|| !FMath::IsNearlyEqual(TerrainCarveRadiusMultiplier, 1.0f)
		|| !FMath::IsNearlyZero(TerrainFillRadiusBonus)
		|| !FMath::IsNearlyEqual(TerrainFillRadiusMultiplier, 1.0f)
		|| !FMath::IsNearlyEqual(LaunchSpeedMultiplier, 1.0f)
		|| !FMath::IsNearlyEqual(GravityMultiplier, 1.0f)
		|| ProjectileCountBonus != 0
		|| HasShotModifierImpactSpawnEffect(ImpactSpawns);
}

FText FFortRogueShotModifierSpec::GetDataValidationSummary() const
{
	TArray<FString> Issues;
	if (DisplayName.ToString().IsEmpty())
	{
		AddShotModifierValidationIssue(Issues, TEXT("missing display name"));
	}
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

	bool bHasEmptyImpactSpawn = false;
	for (const FFortRogueImpactSpawnSpec& ImpactSpawn : ImpactSpawns)
	{
		if (ImpactSpawn.ProjectileCount <= 0)
		{
			bHasEmptyImpactSpawn = true;
			break;
		}
	}
	if (bHasEmptyImpactSpawn)
	{
		AddShotModifierValidationIssue(Issues, TEXT("impact spawn projectile count must be greater than 0"));
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

	bool bHasEmptyImpactSpawn = false;
	for (const FFortRogueImpactSpawnSpec& ImpactSpawn : ImpactSpawns)
	{
		if (ImpactSpawn.ProjectileCount <= 0)
		{
			bHasEmptyImpactSpawn = true;
			break;
		}
	}
	if (bHasEmptyImpactSpawn)
	{
		AddWeaponValidationIssue(Issues, TEXT("impact spawn projectile count must be greater than 0"));
	}

	for (const FFortRogueShotModifierSpec& ShotModifier : ShotModifiers)
	{
		if (!ShotModifier.GetDataValidationSummary().IsEmpty())
		{
			AddWeaponValidationIssue(Issues, TEXT("shot modifier data has warnings"));
			break;
		}
	}

	return Issues.Num() > 0 ? FText::FromString(FString::Join(Issues, TEXT(" | "))) : FText::GetEmpty();
}

FText UFortRogueWeaponDefinition::GetDataValidationSummary() const
{
	return Weapon.GetDataValidationSummary();
}
