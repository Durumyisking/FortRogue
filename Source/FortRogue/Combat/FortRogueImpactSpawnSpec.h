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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Spawn", meta = (ClampMin = "0", ToolTip = "충돌 후 생성할 child 투사체 수입니다. 0이면 아무것도 생성하지 않습니다."))
	int32 ProjectileCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Spawn", meta = (ClampMin = "0.0", ToolTip = "child 투사체들이 퍼지는 전체 각도입니다. 0이면 모두 같은 방향으로 나갑니다."))
	float SpreadDegrees = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Spawn", meta = (ClampMin = "0.0", ToolTip = "child 투사체의 발사 속도입니다. 0이면 생성되지만 움직이지 않습니다."))
	float LaunchSpeed = 750.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Spawn", meta = (ClampMin = "0.0", ToolTip = "부모 투사체 피해량에 곱할 child 피해 배율입니다. 1.0이면 부모와 같습니다."))
	float DamageMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Spawn", meta = (ClampMin = "0.0", ToolTip = "부모 폭발 반경에 곱할 child 폭발 반경 배율입니다."))
	float BlastRadiusMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Spawn", meta = (ClampMin = "0.0", ToolTip = "부모 지형 파괴 반경에 곱할 child 파괴 반경 배율입니다."))
	float TerrainCarveRadiusMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Spawn", meta = (ClampMin = "0.0", ToolTip = "child 투사체가 충돌 시 생성할 지형 반경입니다. 0이면 지형을 생성하지 않고 파괴합니다."))
	float TerrainFillRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Spawn", meta = (ClampMin = "0.0", ToolTip = "부모 중력에 곱할 child 중력 배율입니다. 1.0이면 부모와 같습니다."))
	float GravityMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Spawn", meta = (Categories = "ShotEffect", ToolTip = "child 투사체에 추가할 효과 태그입니다. ShotEffect.* 태그만 사용하세요."))
	FGameplayTagContainer ChildEffectTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Spawn", meta = (ToolTip = "child 투사체 클래스입니다. 비워두면 부모 투사체 클래스를 그대로 사용합니다."))
	TSubclassOf<AFortRogueProjectile> ProjectileClass;
};
