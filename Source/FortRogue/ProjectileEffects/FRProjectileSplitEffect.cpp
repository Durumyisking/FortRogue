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
	FFortRogueShotSpec ChildShotSpec;
	ChildShotSpec.WeaponTag = Context.WeaponTag;
	ChildShotSpec.EffectTags = Context.EffectTags;
	ChildShotSpec.EffectTags.AppendTags(Params.ChildEffectTags);
	ChildShotSpec.Damage = FMath::Max(0.0f, Context.Damage * Params.DamageMultiplier);
	ChildShotSpec.BlastRadius = FMath::Max(0.0f, Context.BlastRadius * Params.BlastRadiusMultiplier);
	ChildShotSpec.TerrainCarveRadius = FMath::Max(0.0f, Context.TerrainCarveRadius * Params.TerrainCarveRadiusMultiplier);
	ChildShotSpec.TerrainFillRadius = FMath::Max(0.0f, Params.TerrainFillRadius);
	ChildShotSpec.LaunchSpeed = FMath::Max(0.0f, Params.LaunchSpeed);
	ChildShotSpec.Gravity = FMath::Max(0.0f, Context.Gravity * Params.GravityMultiplier);
	ChildShotSpec.ProjectileCount = FMath::Max(0, Params.ProjectileCount);
	ChildShotSpec.ProjectileClass = Params.ProjectileClass ? Params.ProjectileClass : TSubclassOf<AFortRogueProjectile>(Context.Projectile ? Context.Projectile->GetClass() : AFortRogueProjectile::StaticClass());
	for (const FFortRogueShotModifierSpec& ChildModifier : Params.ChildShotModifiers)
	{
		ChildModifier.ApplyToShotSpec(ChildShotSpec);
	}

	const int32 ChildCount = FMath::Max(0, ChildShotSpec.ProjectileCount);
	if (ChildCount <= 0)
	{
		return;
	}

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
