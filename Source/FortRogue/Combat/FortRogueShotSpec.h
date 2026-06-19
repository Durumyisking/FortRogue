// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ProjectileEffects/FRProjectileEffect.h"
#include "FortRogueShotSpec.generated.h"

class AFortRogueProjectile;

USTRUCT(BlueprintType)
struct FFortRogueShotSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot", meta = (Categories = "Weapon", ToolTip = "현재 샷이 사용하는 무기 태그입니다. Weapon.* 태그만 사용하세요."))
	FGameplayTag WeaponTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot", meta = (Categories = "Weapon,ShotEffect", ToolTip = "현재 샷에 적용된 효과 태그 모음입니다. Weapon.* 또는 ShotEffect.* 태그만 사용하세요."))
	FGameplayTagContainer EffectTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot", meta = (ToolTip = "최종 피해량입니다. 무기, 캐릭터 보너스, modifier가 모두 반영된 값입니다."))
	float Damage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot", meta = (ToolTip = "최종 폭발 반경입니다. 피해 판정과 기본 지형 파괴 기준으로 사용됩니다."))
	float BlastRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot", meta = (ToolTip = "충돌 지점에서 지형을 파내는 최종 반경입니다."))
	float TerrainCarveRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot", meta = (ToolTip = "충돌 지점에서 지형을 생성하는 최종 반경입니다. 0보다 크면 파괴 대신 생성이 우선됩니다."))
	float TerrainFillRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot", meta = (ToolTip = "최종 발사 속도입니다. 무기 속도, 파워, 캐릭터 배율, modifier가 반영된 값입니다."))
	float LaunchSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot", meta = (ToolTip = "최종 투사체 중력입니다. 값이 클수록 빠르게 떨어집니다."))
	float Gravity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot", meta = (ToolTip = "이번 발사에서 생성할 최종 투사체 수입니다. 최소 1개로 보정됩니다."))
	int32 ProjectileCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot", meta = (ToolTip = "이번 샷에서 스폰할 투사체 클래스입니다."))
	TSubclassOf<AFortRogueProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot", meta = (TitleProperty = EffectClass, ToolTip = "이번 샷의 충돌/비행 단계에서 실행할 조립식 투사체 효과 목록입니다. 배열 순서대로 실행됩니다."))
	TArray<FFRProjectileEffectSpec> ProjectileEffects;
};
