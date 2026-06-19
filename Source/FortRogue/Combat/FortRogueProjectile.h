// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Combat/FortRogueImpactSpawnSpec.h"
#include "Combat/FortRogueShotSpec.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "FortRogueProjectile.generated.h"

class AFortRogueBattleCharacter;
class AFortRogueDestructibleTerrain;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class FORTROGUE_API AFortRogueProjectile : public AActor
{
	GENERATED_BODY()

public:
	AFortRogueProjectile();

	virtual void Tick(float DeltaSeconds) override;

	void InitializeProjectile(AFortRogueBattleCharacter* InOwnerCharacter, AFortRogueDestructibleTerrain* InTerrain, const FVector& InVelocity, float InDamage, float InBlastRadius, float InGravity, float InTerrainCarveRadius = -1.0f, float InTerrainFillRadius = 0.0f, FGameplayTag InWeaponTag = FGameplayTag(), FGameplayTagContainer InEffectTags = FGameplayTagContainer(), TArray<FFortRogueImpactSpawnSpec> InImpactSpawns = TArray<FFortRogueImpactSpawnSpec>(), TArray<FFRProjectileEffectSpec> InProjectileEffects = TArray<FFRProjectileEffectSpec>());
	void InitializeProjectileFromShotSpec(AFortRogueBattleCharacter* InOwnerCharacter, AFortRogueDestructibleTerrain* InTerrain, const FVector& InVelocity, const FFortRogueShotSpec& ShotSpec);
	int32 GetProjectileEffectCount() const;
	bool HasProjectileEffectClass(TSubclassOf<UFRProjectileEffectBase> EffectClass) const;

private:
	void ResolveImpact(const FVector& ImpactLocation);
	void ApplyDefaultTerrainImpact(const FVector& ImpactLocation);
	void ApplyProjectileEffects(const FVector& ImpactLocation);
	void SpawnImpactProjectiles(const FVector& ImpactLocation);
	bool UsesCustomTerrainImpact() const;

	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<UStaticMeshComponent> Visual;

	UPROPERTY()
	TObjectPtr<AFortRogueBattleCharacter> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<AFortRogueDestructibleTerrain> AssignedTerrain;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Lifetime", meta = (ClampMin = "0.1"))
	float MaxLifeSeconds = 8.0f;

	FVector Velocity = FVector::ZeroVector;
	FGameplayTag WeaponTag;
	FGameplayTagContainer EffectTags;
	TArray<FFortRogueImpactSpawnSpec> ImpactSpawns;
	TArray<FFRProjectileEffectSpec> ProjectileEffects;
	float Damage = 35.0f;
	float BlastRadius = 150.0f;
	float TerrainCarveRadius = 150.0f;
	float TerrainFillRadius = 0.0f;
	float Gravity = 980.0f;
	float LifeSeconds = 0.0f;
	bool bResolved = false;
};
