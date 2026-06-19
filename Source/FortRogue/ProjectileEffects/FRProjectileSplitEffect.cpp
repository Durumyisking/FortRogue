// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectileEffects/FRProjectileSplitEffect.h"

#include "Combat/FortRogueBattleCharacter.h"
#include "Combat/FortRogueProjectile.h"
#include "Combat/FortRogueShotSpec.h"
#include "FortRogueGameMode.h"
#include "FortRogueGameplayTags.h"

const UScriptStruct* UFRProjectileEffectSplit::GetParameterStruct() const
{
	return FFRProjectileEffectSplitParams::StaticStruct();
}

void UFRProjectileEffectSplit::ApplyToShotSpec(const FFRProjectileEffectSpec& EffectSpec, FFortRogueShotSpec& ShotSpec) const
{
	ShotSpec.EffectTags.AddTag(FortRogueGameplayTags::ShotEffect_SplitOnImpact);
}

void UFRProjectileEffectSplit::HandleImpact(const FFRProjectileEffectSpec& EffectSpec, const FFRProjectileEffectImpactContext& Context) const
{
	if (!Context.World)
	{
		return;
	}

	const FFRProjectileEffectSplitParams& Params = EffectSpec.GetParametersOrDefault<FFRProjectileEffectSplitParams>();
	const int32 ChildCount = FMath::Max(0, Params.ProjectileCount);
	if (ChildCount <= 0)
	{
		return;
	}

	FFortRogueShotSpec BaseChildShotSpec;
	BaseChildShotSpec.WeaponTag = Context.WeaponTag;
	BaseChildShotSpec.EffectTags = Context.EffectTags;
	BaseChildShotSpec.EffectTags.AppendTags(Params.ChildEffectTags);
	BaseChildShotSpec.Damage = FMath::Max(0.0f, Context.Damage * Params.DamageMultiplier);
	BaseChildShotSpec.BlastRadius = FMath::Max(0.0f, Context.BlastRadius * Params.BlastRadiusMultiplier);
	BaseChildShotSpec.TerrainCarveRadius = FMath::Max(0.0f, Context.TerrainCarveRadius * Params.TerrainCarveRadiusMultiplier);
	BaseChildShotSpec.TerrainFillRadius = FMath::Max(0.0f, Params.TerrainFillRadius);
	BaseChildShotSpec.LaunchSpeed = FMath::Max(0.0f, Params.LaunchSpeed);
	BaseChildShotSpec.Gravity = FMath::Max(0.0f, Context.Gravity * Params.GravityMultiplier);
	BaseChildShotSpec.ProjectileCount = 1;
	BaseChildShotSpec.ProjectileClass = Params.ProjectileClass ? Params.ProjectileClass : TSubclassOf<AFortRogueProjectile>(Context.Projectile ? Context.Projectile->GetClass() : AFortRogueProjectile::StaticClass());
	const AFortRogueGameMode* WindGameMode = Context.World->GetAuthGameMode<AFortRogueGameMode>();
	const float Wind = WindGameMode ? WindGameMode->GetWind() : 0.0f;
	const FVector FallbackDirection = Context.OwnerCharacter && Context.OwnerCharacter->IsEnemy()
		? FVector(-1.0f, 0.0f, 1.0f).GetSafeNormal()
		: FVector(1.0f, 0.0f, 1.0f).GetSafeNormal();
	const FVector BaseDirection = Context.Velocity.GetSafeNormal(SMALL_NUMBER, FallbackDirection);
	const float BaseAngleDegrees = FMath::RadiansToDegrees(FMath::Atan2(BaseDirection.Z, BaseDirection.X));
	for (int32 Index = 0; Index < ChildCount; ++Index)
	{
		const float SpreadAlpha = ChildCount > 1 ? static_cast<float>(Index) / static_cast<float>(ChildCount - 1) - 0.5f : 0.0f;
		const float ChildAngleDegrees = BaseAngleDegrees + SpreadAlpha * Params.SpreadDegrees;
		const float ChildAngleRadians = FMath::DegreesToRadians(ChildAngleDegrees);
		const FVector ChildDirection = FVector(FMath::Cos(ChildAngleRadians), 0.0f, FMath::Sin(ChildAngleRadians)).GetSafeNormal();
		const float ChildAimAngle = FMath::Clamp(FMath::RadiansToDegrees(FMath::Atan2(FMath::Abs(ChildDirection.Z), FMath::Abs(ChildDirection.X))), 0.0f, 90.0f);
		const bool bChildFacingRight = ChildDirection.X >= 0.0f;
		FFortRogueShotSpec ChildShotSpec = BaseChildShotSpec;
		for (const FFortRogueShotModifierSpec& ChildModifier : Params.ChildShotModifiers)
		{
			if (ChildModifier.MeetsShotConditions(ChildShotSpec, ChildAimAngle, Wind, bChildFacingRight))
			{
				ChildModifier.ApplyToShotSpec(ChildShotSpec);
			}
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Context.OwnerCharacter ? Cast<AActor>(Context.OwnerCharacter) : (Context.Projectile ? Context.Projectile->GetOwner() : nullptr);
		SpawnParams.Instigator = Context.OwnerCharacter;
		AFortRogueProjectile* ChildProjectile = Context.World->SpawnActor<AFortRogueProjectile>(ChildShotSpec.ProjectileClass, Context.ImpactLocation + ChildDirection * 18.0f, FRotator::ZeroRotator, SpawnParams);
		if (!ChildProjectile)
		{
			continue;
		}

		ChildProjectile->InitializeProjectileFromShotSpec(
			Context.OwnerCharacter,
			Context.AssignedTerrain,
			ChildDirection * ChildShotSpec.LaunchSpeed,
			ChildShotSpec);

		if (AFortRogueGameMode* GameMode = Context.World->GetAuthGameMode<AFortRogueGameMode>())
		{
			GameMode->NotifyProjectileSpawned(ChildProjectile);
		}
	}
}

void UFRProjectileEffectSplit::AddDataValidationIssues(const FFRProjectileEffectSpec& EffectSpec, TArray<FString>& Issues) const
{
	const FFRProjectileEffectSplitParams& Params = EffectSpec.GetParametersOrDefault<FFRProjectileEffectSplitParams>();
	if (Params.ProjectileCount <= 0)
	{
		Issues.Add(TEXT("split projectile count must be greater than 0"));
	}
	if (Params.SpreadDegrees < 0.0f)
	{
		Issues.Add(TEXT("split spread degrees must be non-negative"));
	}
	if (Params.LaunchSpeed <= 0.0f)
	{
		Issues.Add(TEXT("split launch speed must be greater than 0"));
	}
	if (Params.DamageMultiplier < 0.0f)
	{
		Issues.Add(TEXT("split damage multiplier must be non-negative"));
	}
	if (Params.BlastRadiusMultiplier < 0.0f)
	{
		Issues.Add(TEXT("split blast radius multiplier must be non-negative"));
	}
	if (Params.TerrainCarveRadiusMultiplier < 0.0f)
	{
		Issues.Add(TEXT("split terrain carve radius multiplier must be non-negative"));
	}
	if (Params.TerrainFillRadius < 0.0f)
	{
		Issues.Add(TEXT("split terrain fill radius must be non-negative"));
	}
	if (Params.GravityMultiplier < 0.0f)
	{
		Issues.Add(TEXT("split gravity multiplier must be non-negative"));
	}

	bool bHasInvalidChildModifier = false;
	bool bHasIgnoredProjectileCountBonus = false;
	bool bHasLegacyImpactSpawns = false;
	for (const FFortRogueShotModifierSpec& ChildModifier : Params.ChildShotModifiers)
	{
		if (ChildModifier.ProjectileCountBonus != 0)
		{
			bHasIgnoredProjectileCountBonus = true;
		}
		if (ChildModifier.ImpactSpawns.Num() > 0)
		{
			bHasLegacyImpactSpawns = true;
		}
		if (!ChildModifier.GetDataValidationSummary().IsEmpty())
		{
			bHasInvalidChildModifier = true;
		}
	}
	if (bHasIgnoredProjectileCountBonus)
	{
		Issues.Add(TEXT("split child shot modifier projectile count bonus is ignored"));
	}
	if (bHasLegacyImpactSpawns)
	{
		Issues.Add(TEXT("split child shot modifier legacy impact spawns should use split projectile effects"));
	}
	if (bHasInvalidChildModifier)
	{
		Issues.Add(TEXT("split child shot modifier data has warnings"));
	}
}
