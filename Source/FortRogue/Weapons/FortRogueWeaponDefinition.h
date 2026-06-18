// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Combat/FortRogueImpactSpawnSpec.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "FortRogueWeaponDefinition.generated.h"

class AFortRogueProjectile;

USTRUCT(BlueprintType)
struct FFortRogueShotModifierSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier", meta = (MultiLine = "true"))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier")
	FGameplayTagContainer EffectTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier")
	FGameplayTag ModifierTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier|Condition")
	bool bUseAimAngleRange = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier|Condition", meta = (EditCondition = "bUseAimAngleRange", ClampMin = "0.0", ClampMax = "90.0"))
	float MinAimAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier|Condition", meta = (EditCondition = "bUseAimAngleRange", ClampMin = "0.0", ClampMax = "90.0"))
	float MaxAimAngle = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier|Condition")
	bool bRequireWindAligned = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier|Condition", meta = (EditCondition = "bRequireWindAligned", ClampMin = "0.0"))
	float MinWindMagnitude = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier|Condition")
	FGameplayTagContainer RequiredShotTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier|Condition")
	FGameplayTagContainer BlockedShotTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier")
	float DamageBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier", meta = (ClampMin = "0.0"))
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier")
	float BlastRadiusBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier", meta = (ClampMin = "0.0"))
	float BlastRadiusMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier")
	float TerrainCarveRadiusBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier", meta = (ClampMin = "0.0"))
	float TerrainCarveRadiusMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier")
	float TerrainFillRadiusBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier", meta = (ClampMin = "0.0"))
	float TerrainFillRadiusMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier", meta = (ClampMin = "0.0"))
	float LaunchSpeedMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier", meta = (ClampMin = "0.0"))
	float GravityMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier")
	int32 ProjectileCountBonus = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier|Impact", meta = (TitleProperty = ProjectileCount))
	TArray<FFortRogueImpactSpawnSpec> ImpactSpawns;
};

USTRUCT(BlueprintType)
struct FFortRogueWeaponSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FText DisplayName = FText::FromString(TEXT("Shell"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FText Description = FText::FromString(TEXT("A reliable arcing shell."));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FGameplayTag WeaponTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FGameplayTagContainer ShotEffectTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (TitleProperty = DisplayName))
	TArray<FFortRogueShotModifierSpec> ShotModifiers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (TitleProperty = ProjectileCount))
	TArray<FFortRogueImpactSpawnSpec> ImpactSpawns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float Damage = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float BlastRadius = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float ProjectileSpeed = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float Gravity = 980.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int32 ProjectilesPerShot = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<AFortRogueProjectile> ProjectileClass;
};

UCLASS(BlueprintType)
class FORTROGUE_API UFortRogueWeaponDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FFortRogueWeaponSpec Weapon;
};
