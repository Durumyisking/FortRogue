// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "FortRogueImpactSpawnSpec.generated.h"

class AFortRogueProjectile;

USTRUCT(BlueprintType)
struct FFortRogueImpactSpawnSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Spawn", meta = (ClampMin = "0"))
	int32 ProjectileCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Spawn", meta = (ClampMin = "0.0"))
	float SpreadDegrees = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Spawn", meta = (ClampMin = "0.0"))
	float LaunchSpeed = 750.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Spawn", meta = (ClampMin = "0.0"))
	float DamageMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Spawn", meta = (ClampMin = "0.0"))
	float BlastRadiusMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Spawn", meta = (ClampMin = "0.0"))
	float TerrainCarveRadiusMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Spawn", meta = (ClampMin = "0.0"))
	float GravityMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Spawn")
	FGameplayTagContainer ChildEffectTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Spawn")
	TSubclassOf<AFortRogueProjectile> ProjectileClass;
};
