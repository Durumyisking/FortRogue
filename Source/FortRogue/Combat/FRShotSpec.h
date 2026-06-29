// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ProjectileEffects/FRProjectileEffect.h"
#include "FRShotSpec.generated.h"

class AFRProjectile;

USTRUCT(BlueprintType)
struct FFRShotSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot", meta = (Categories = "Weapon", ToolTip = "현재 샷이 사용하는 무기 태그입니다. Weapon.* 태그만 사용하세요."))
	FGameplayTag WeaponTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot", meta = (Categories = "Weapon,ShotEffect", ToolTip = "현재 샷에 적용된 효과 태그 모음입니다. Weapon.* 또는 ShotEffect.* 태그만 사용하세요."))
	FGameplayTagContainer EffectTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot", meta = (ToolTip = "투사체가 캐릭터에게 직접 닿았을 때 주는 최종 명중 피해량입니다."))
	float HitDamage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot", meta = (DisplayName = "Explosion Damage", ToolTip = "폭발 범위 안 캐릭터에게 주는 최종 폭발 피해량입니다. 무기, 캐릭터 보너스, modifier가 모두 반영된 값입니다."))
	float Damage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot", meta = (ToolTip = "폭발 피해가 닿는 최종 바깥 반경입니다."))
	float BlastRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot", meta = (ClampMin = "0.0", ToolTip = "충돌 지점부터 이 반경까지는 폭발 피해가 100% 적용되고, 여기서 BlastRadius까지 0으로 선형 감소합니다."))
	float ExplosionFullDamageRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot", meta = (ClampMin = "0.0", ToolTip = "충돌 지점에서 지형을 파내는 최종 반경입니다. 0이면 기본 지형 파괴가 일어나지 않습니다."))
	float TerrainDamage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot", meta = (ToolTip = "충돌 지점에서 지형을 생성하는 최종 반경입니다. 0보다 크면 파괴 대신 생성이 우선됩니다."))
	float TerrainFillRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot", meta = (ToolTip = "최종 발사 속도입니다. 무기 속도, 파워, 캐릭터 배율, modifier가 반영된 값입니다."))
	float LaunchSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot", meta = (ToolTip = "최종 투사체 중력입니다. 값이 클수록 빠르게 떨어집니다."))
	float Gravity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot", meta = (ToolTip = "이번 발사에서 생성할 최종 투사체 수입니다. 최소 1개로 보정됩니다."))
	int32 ProjectileCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot", meta = (ToolTip = "한 번의 발사 입력으로 반복 발사할 최종 횟수입니다. 최소 1회로 보정됩니다."))
	int32 SalvoCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot", meta = (ClampMin = "0.0", ToolTip = "반복 발사 사이의 최종 간격입니다. 초 단위입니다."))
	float SalvoInterval = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot", meta = (ToolTip = "이번 샷에서 스폰할 투사체 클래스입니다."))
	TSubclassOf<AFRProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot", meta = (TitleProperty = EffectClass, ToolTip = "이번 샷의 충돌/비행 단계에서 실행할 조립식 투사체 효과 목록입니다. 배열 순서대로 실행됩니다."))
	TArray<FFRProjectileEffectSpec> ProjectileEffects;
};
