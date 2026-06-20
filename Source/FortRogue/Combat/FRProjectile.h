// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Combat/FRShotSpec.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "FRProjectile.generated.h"

class AFRBattleCharacter;
class AFRDestructibleTerrain;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class FORTROGUE_API AFRProjectile : public AActor
{
	GENERATED_BODY()

public:
	AFRProjectile();

	virtual void Tick(float DeltaSeconds) override;

	void InitializeProjectile(AFRBattleCharacter* InOwnerCharacter, AFRDestructibleTerrain* InTerrain, const FVector& InVelocity, float InHitDamage, float InExplosionDamage, float InBlastRadius, float InExplosionFullDamageRadius, float InGravity, float InTerrainDamage = 0.0f, float InTerrainFillRadius = 0.0f, FGameplayTag InWeaponTag = FGameplayTag(), FGameplayTagContainer InEffectTags = FGameplayTagContainer(), TArray<FFRProjectileEffectSpec> InProjectileEffects = TArray<FFRProjectileEffectSpec>());
	void InitializeProjectileFromShotSpec(AFRBattleCharacter* InOwnerCharacter, AFRDestructibleTerrain* InTerrain, const FVector& InVelocity, const FFRShotSpec& ShotSpec);
	int32 GetProjectileEffectCount() const;
	bool HasProjectileEffectClass(TSubclassOf<UFRProjectileEffectBase> EffectClass) const;

private:
	AFRBattleCharacter* FindHomingTarget() const;
	void ApplyHoming(float DeltaSeconds);
	void ResolveImpact(const FVector& ImpactLocation, AFRBattleCharacter* DirectHitCharacter = nullptr);
	void ApplyDefaultTerrainImpact(const FVector& ImpactLocation);
	void ApplyProjectileEffects(const FVector& ImpactLocation);
	float CalculateExplosionDamage(float Distance) const;
	bool UsesCustomTerrainImpact() const;

	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<UStaticMeshComponent> Visual;

	UPROPERTY()
	TObjectPtr<AFRBattleCharacter> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<AFRDestructibleTerrain> AssignedTerrain;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Lifetime", meta = (ClampMin = "0.1", ToolTip = "충돌하지 않은 투사체가 자동으로 사라지기까지의 최대 시간입니다. 너무 길면 턴 종료가 늦어질 수 있습니다."))
	float MaxLifeSeconds = 8.0f;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Homing", meta = (ToolTip = "켜면 가장 가까운 적 캐릭터를 향해 투사체 속도 방향을 조금씩 보정합니다."))
	bool bHoming = false;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Homing", meta = (ClampMin = "0.0", ToolTip = "유도탄이 초당 방향을 보정하는 정도입니다. 값이 클수록 목표를 빠르게 따라갑니다."))
	float HomingTurnRate = 2.8f;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Homing", meta = (ClampMin = "0.0", ToolTip = "유도 대상을 찾는 최대 거리입니다."))
	float HomingRange = 2400.0f;

private:
	FVector Velocity = FVector::ZeroVector;
	FGameplayTag WeaponTag;
	FGameplayTagContainer EffectTags;
	TArray<FFRProjectileEffectSpec> ProjectileEffects;
	float HitDamage = 0.0f;
	float Damage = 35.0f;
	float BlastRadius = 150.0f;
	float ExplosionFullDamageRadius = 0.0f;
	float TerrainDamage = 150.0f;
	float TerrainFillRadius = 0.0f;
	float Gravity = 980.0f;
	float LifeSeconds = 0.0f;
	bool bResolved = false;
};
