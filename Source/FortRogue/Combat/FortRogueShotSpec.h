// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "FortRogueShotSpec.generated.h"

class AFortRogueProjectile;

USTRUCT(BlueprintType)
struct FFortRogueShotSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	FGameplayTag WeaponTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	FGameplayTagContainer EffectTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	float Damage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	float BlastRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	float TerrainCarveRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	float LaunchSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	float Gravity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	int32 ProjectileCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	TSubclassOf<AFortRogueProjectile> ProjectileClass;
};
