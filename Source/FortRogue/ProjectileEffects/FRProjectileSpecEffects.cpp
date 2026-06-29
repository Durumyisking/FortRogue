// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectileEffects/FRProjectileSpecEffects.h"

#include "Combat/FRBattleCharacter.h"
#include "Combat/FRShotSpec.h"
#include "FRGameplayTags.h"

const UScriptStruct* UFRProjectileEffectDirectHitDamage::GetParameterStruct() const
{
	return FFRProjectileEffectDirectHitDamageParams::StaticStruct();
}

void UFRProjectileEffectDirectHitDamage::ApplyToShotSpec(const FFRProjectileEffectSpec& EffectSpec, FFRShotSpec& ShotSpec) const
{
	const FFRProjectileEffectDirectHitDamageParams& Params = EffectSpec.GetParametersOrDefault<FFRProjectileEffectDirectHitDamageParams>();
	ShotSpec.EffectTags.AddTag(FRGameplayTags::ShotEffect_Damage);
	ShotSpec.HitDamage = FMath::Max(0.0f, (ShotSpec.HitDamage + Params.BonusDamage) * Params.DamageMultiplier);
}

void UFRProjectileEffectDirectHitDamage::AddDataValidationIssues(const FFRProjectileEffectSpec& EffectSpec, TArray<FString>& Issues) const
{
	if (EffectSpec.GetParametersOrDefault<FFRProjectileEffectDirectHitDamageParams>().DamageMultiplier < 0.0f)
	{
		Issues.Add(TEXT("direct hit damage multiplier must be non-negative"));
	}
}

const UScriptStruct* UFRProjectileEffectExplosionPayload::GetParameterStruct() const
{
	return FFRProjectileEffectExplosionPayloadParams::StaticStruct();
}

void UFRProjectileEffectExplosionPayload::ApplyToShotSpec(const FFRProjectileEffectSpec& EffectSpec, FFRShotSpec& ShotSpec) const
{
	const FFRProjectileEffectExplosionPayloadParams& Params = EffectSpec.GetParametersOrDefault<FFRProjectileEffectExplosionPayloadParams>();
	if (!FMath::IsNearlyZero(Params.DamageBonus) || !FMath::IsNearlyEqual(Params.DamageMultiplier, 1.0f))
	{
		ShotSpec.EffectTags.AddTag(FRGameplayTags::ShotEffect_Damage);
	}
	if (!FMath::IsNearlyZero(Params.RadiusBonus) || !FMath::IsNearlyEqual(Params.RadiusMultiplier, 1.0f))
	{
		ShotSpec.EffectTags.AddTag(FRGameplayTags::ShotEffect_BlastRadius);
	}
	ShotSpec.Damage = FMath::Max(0.0f, (ShotSpec.Damage + Params.DamageBonus) * Params.DamageMultiplier);
	ShotSpec.BlastRadius = FMath::Max(0.0f, (ShotSpec.BlastRadius + Params.RadiusBonus) * Params.RadiusMultiplier);
	ShotSpec.ExplosionFullDamageRadius = FMath::Clamp(ShotSpec.ExplosionFullDamageRadius, 0.0f, ShotSpec.BlastRadius);
}

void UFRProjectileEffectExplosionPayload::AddDataValidationIssues(const FFRProjectileEffectSpec& EffectSpec, TArray<FString>& Issues) const
{
	const FFRProjectileEffectExplosionPayloadParams& Params = EffectSpec.GetParametersOrDefault<FFRProjectileEffectExplosionPayloadParams>();
	if (Params.DamageMultiplier < 0.0f)
	{
		Issues.Add(TEXT("explosion damage multiplier must be non-negative"));
	}
	if (Params.RadiusMultiplier < 0.0f)
	{
		Issues.Add(TEXT("explosion radius multiplier must be non-negative"));
	}
}

const UScriptStruct* UFRProjectileEffectTerrainCarvePayload::GetParameterStruct() const
{
	return FFRProjectileEffectTerrainCarvePayloadParams::StaticStruct();
}

void UFRProjectileEffectTerrainCarvePayload::ApplyToShotSpec(const FFRProjectileEffectSpec& EffectSpec, FFRShotSpec& ShotSpec) const
{
	const FFRProjectileEffectTerrainCarvePayloadParams& Params = EffectSpec.GetParametersOrDefault<FFRProjectileEffectTerrainCarvePayloadParams>();
	ShotSpec.TerrainDamage = FMath::Max(0.0f, (ShotSpec.TerrainDamage + Params.RadiusBonus) * Params.RadiusMultiplier);
}

void UFRProjectileEffectTerrainCarvePayload::AddDataValidationIssues(const FFRProjectileEffectSpec& EffectSpec, TArray<FString>& Issues) const
{
	if (EffectSpec.GetParametersOrDefault<FFRProjectileEffectTerrainCarvePayloadParams>().RadiusMultiplier < 0.0f)
	{
		Issues.Add(TEXT("terrain carve radius multiplier must be non-negative"));
	}
}

const UScriptStruct* UFRProjectileEffectTerrainFillPayload::GetParameterStruct() const
{
	return FFRProjectileEffectTerrainFillPayloadParams::StaticStruct();
}

void UFRProjectileEffectTerrainFillPayload::ApplyToShotSpec(const FFRProjectileEffectSpec& EffectSpec, FFRShotSpec& ShotSpec) const
{
	const FFRProjectileEffectTerrainFillPayloadParams& Params = EffectSpec.GetParametersOrDefault<FFRProjectileEffectTerrainFillPayloadParams>();
	ShotSpec.TerrainFillRadius = FMath::Max(0.0f, (ShotSpec.TerrainFillRadius + Params.RadiusBonus) * Params.RadiusMultiplier);
}

void UFRProjectileEffectTerrainFillPayload::AddDataValidationIssues(const FFRProjectileEffectSpec& EffectSpec, TArray<FString>& Issues) const
{
	if (EffectSpec.GetParametersOrDefault<FFRProjectileEffectTerrainFillPayloadParams>().RadiusMultiplier < 0.0f)
	{
		Issues.Add(TEXT("terrain fill radius multiplier must be non-negative"));
	}
}

const UScriptStruct* UFRProjectileEffectFlightProfile::GetParameterStruct() const
{
	return FFRProjectileEffectFlightProfileParams::StaticStruct();
}

void UFRProjectileEffectFlightProfile::ApplyToShotSpec(const FFRProjectileEffectSpec& EffectSpec, FFRShotSpec& ShotSpec) const
{
	const FFRProjectileEffectFlightProfileParams& Params = EffectSpec.GetParametersOrDefault<FFRProjectileEffectFlightProfileParams>();
	ShotSpec.LaunchSpeed = FMath::Max(0.0f, (ShotSpec.LaunchSpeed + Params.LaunchSpeedBonus) * Params.LaunchSpeedMultiplier);
	ShotSpec.Gravity = FMath::Max(0.0f, (ShotSpec.Gravity + Params.GravityBonus) * Params.GravityMultiplier);
	if (Params.GravityBonus < 0.0f || Params.GravityMultiplier < 1.0f)
	{
		ShotSpec.EffectTags.AddTag(FRGameplayTags::ShotEffect_LowGravity);
	}
}

void UFRProjectileEffectFlightProfile::AddDataValidationIssues(const FFRProjectileEffectSpec& EffectSpec, TArray<FString>& Issues) const
{
	const FFRProjectileEffectFlightProfileParams& Params = EffectSpec.GetParametersOrDefault<FFRProjectileEffectFlightProfileParams>();
	if (Params.LaunchSpeedMultiplier < 0.0f)
	{
		Issues.Add(TEXT("launch speed multiplier must be non-negative"));
	}
	if (Params.GravityMultiplier < 0.0f)
	{
		Issues.Add(TEXT("gravity multiplier must be non-negative"));
	}
}

const UScriptStruct* UFRProjectileEffectProjectileCount::GetParameterStruct() const
{
	return FFRProjectileEffectProjectileCountParams::StaticStruct();
}

void UFRProjectileEffectProjectileCount::ApplyToShotSpec(const FFRProjectileEffectSpec& EffectSpec, FFRShotSpec& ShotSpec) const
{
	const FFRProjectileEffectProjectileCountParams& Params = EffectSpec.GetParametersOrDefault<FFRProjectileEffectProjectileCountParams>();
	ShotSpec.EffectTags.AddTag(FRGameplayTags::ShotEffect_Projectiles);
	ShotSpec.ProjectileCount = FMath::Max(1, ShotSpec.ProjectileCount + Params.CountBonus);
}

const UScriptStruct* UFRProjectileEffectSalvo::GetParameterStruct() const
{
	return FFRProjectileEffectSalvoParams::StaticStruct();
}

void UFRProjectileEffectSalvo::ApplyToShotSpec(const FFRProjectileEffectSpec& EffectSpec, FFRShotSpec& ShotSpec) const
{
	const FFRProjectileEffectSalvoParams& Params = EffectSpec.GetParametersOrDefault<FFRProjectileEffectSalvoParams>();
	ShotSpec.EffectTags.AddTag(FRGameplayTags::ShotEffect_Salvo);
	ShotSpec.SalvoCount = FMath::Max(1, ShotSpec.SalvoCount + Params.CountBonus);
	if (ShotSpec.SalvoCount > 1 && Params.SalvoInterval > 0.0f)
	{
		ShotSpec.SalvoInterval = Params.SalvoInterval;
	}
}

void UFRProjectileEffectSalvo::AddDataValidationIssues(const FFRProjectileEffectSpec& EffectSpec, TArray<FString>& Issues) const
{
	if (EffectSpec.GetParametersOrDefault<FFRProjectileEffectSalvoParams>().SalvoInterval < 0.0f)
	{
		Issues.Add(TEXT("salvo interval must be non-negative"));
	}
}

const UScriptStruct* UFRProjectileEffectKnockback::GetParameterStruct() const
{
	return FFRProjectileEffectKnockbackParams::StaticStruct();
}

void UFRProjectileEffectKnockback::ApplyToShotSpec(const FFRProjectileEffectSpec& EffectSpec, FFRShotSpec& ShotSpec) const
{
	ShotSpec.EffectTags.AddTag(FRGameplayTags::ShotEffect_Knockback);
}

void UFRProjectileEffectKnockback::HandlePostImpact(const FFRProjectileEffectSpec& EffectSpec, const FFRProjectileEffectImpactContext& Context) const
{
	if (Context.DirectHitCharacter)
	{
		Context.DirectHitCharacter->ApplyImpactKnockback(
			EffectSpec.GetParametersOrDefault<FFRProjectileEffectKnockbackParams>().HorizontalDistance,
			Context.ImpactLocation,
			Context.Velocity);
	}
}

bool UFRProjectileEffectKnockback::RequiresProjectileRuntime(const FFRProjectileEffectSpec& EffectSpec) const
{
	return true;
}

void UFRProjectileEffectKnockback::AddDataValidationIssues(const FFRProjectileEffectSpec& EffectSpec, TArray<FString>& Issues) const
{
	if (EffectSpec.GetParametersOrDefault<FFRProjectileEffectKnockbackParams>().HorizontalDistance < 0.0f)
	{
		Issues.Add(TEXT("knockback distance must be non-negative"));
	}
}
