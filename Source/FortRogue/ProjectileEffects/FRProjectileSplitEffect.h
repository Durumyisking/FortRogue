// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ProjectileEffects/FRProjectileEffect.h"
#include "Weapons/FortRogueWeaponDefinition.h"
#include "FRProjectileSplitEffect.generated.h"

USTRUCT(BlueprintType)
struct FORTROGUE_API FFRProjectileEffectSplitParams : public FFRProjectileEffectParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Split", meta = (ClampMin = "0", ToolTip = "충돌 후 생성할 child 투사체 수입니다. 0이면 분열하지 않습니다."))
	int32 ProjectileCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Split", meta = (ClampMin = "0.0", ToolTip = "child 투사체들이 퍼지는 전체 각도입니다. 0이면 모두 같은 방향으로 나갑니다."))
	float SpreadDegrees = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Split", meta = (ClampMin = "0.0", ToolTip = "child 투사체의 기본 발사 속도입니다. child ShotModifier가 LaunchSpeedMultiplier로 추가 조정할 수 있습니다."))
	float LaunchSpeed = 750.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Split", meta = (ClampMin = "0.0", ToolTip = "부모 투사체 피해량에 곱할 child 피해 배율입니다."))
	float DamageMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Split", meta = (ClampMin = "0.0", ToolTip = "부모 폭발 반경에 곱할 child 폭발 반경 배율입니다."))
	float BlastRadiusMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Split", meta = (ClampMin = "0.0", ToolTip = "부모 지형 파괴 반경에 곱할 child 지형 파괴 반경 배율입니다."))
	float TerrainCarveRadiusMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Split", meta = (ClampMin = "0.0", ToolTip = "child 투사체의 기본 지형 생성 반경입니다. 0이면 기본적으로 지형을 생성하지 않습니다."))
	float TerrainFillRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Split", meta = (ClampMin = "0.0", ToolTip = "부모 중력에 곱할 child 중력 배율입니다."))
	float GravityMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Split", meta = (Categories = "ShotEffect", ToolTip = "child 투사체에 기본으로 추가할 효과 태그입니다. ShotEffect.* 태그만 사용하세요."))
	FGameplayTagContainer ChildEffectTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Split", meta = (ToolTip = "child 투사체 클래스입니다. 비워두면 부모 투사체 클래스를 사용합니다."))
	TSubclassOf<AFortRogueProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Split", meta = (TitleProperty = DisplayName, ToolTip = "child 투사체 ShotSpec에 추가로 적용할 modifier 목록입니다. 여기서 Drill, TerrainCreate 같은 ProjectileEffects를 다시 조립할 수 있습니다."))
	TArray<FFortRogueShotModifierSpec> ChildShotModifiers;
};

UCLASS(BlueprintType, Const, meta = (DisplayName = "FR Projectile Effect Split"))
class FORTROGUE_API UFRProjectileEffectSplit : public UFRProjectileEffectBase
{
	GENERATED_BODY()

public:
	virtual const UScriptStruct* GetParameterStruct() const override;
	virtual void ApplyToShotSpec(const FFRProjectileEffectSpec& EffectSpec, FFortRogueShotSpec& ShotSpec) const override;
	virtual void HandleImpact(const FFRProjectileEffectSpec& EffectSpec, const FFRProjectileEffectImpactContext& Context) const override;
};
