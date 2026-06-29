// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ProjectileEffects/FRProjectileEffect.h"
#include "Weapons/FRWeaponDefinition.h"
#include "FRProjectileSplitEffect.generated.h"

USTRUCT(BlueprintType)
struct FORTROGUE_API FFRProjectileEffectSplitParams : public FFRProjectileEffectParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Split", meta = (ClampMin = "1", ToolTip = "충돌 후 생성할 child 투사체 수입니다. Split 효과는 1 이상이어야 하며 0 이하는 데이터 검수 경고가 발생합니다."))
	int32 ProjectileCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Split", meta = (ClampMin = "0.0", ToolTip = "child 투사체들이 퍼지는 전체 각도입니다. 0이면 모두 같은 방향으로 나갑니다."))
	float SpreadDegrees = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Split", meta = (ClampMin = "0.001", ToolTip = "child 투사체의 기본 발사 속도입니다. 0보다 커야 하며, child ShotModifier가 LaunchSpeedMultiplier로 추가 조정할 수 있습니다."))
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
	TSubclassOf<AFRProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Split", meta = (ToolTip = "켜면 부모 투사체의 비행/충돌 효과를 child가 상속합니다. Split 자체는 재귀 분열을 막기 위해 제외됩니다."))
	bool bInheritParentRuntimeEffects = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Split", meta = (TitleProperty = EffectClass, ToolTip = "모든 child ShotSpec에 추가로 적용할 효과입니다."))
	TArray<FFRProjectileEffectSpec> ChildProjectileEffects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Split", meta = (TitleProperty = ModifierTag, ToolTip = "child 투사체 ShotSpec에 추가로 적용할 modifier 목록입니다. 여기서 Drill, TerrainCreate 같은 ProjectileEffects를 다시 조립할 수 있습니다."))
	TArray<FFRShotModifierSpec> ChildShotModifiers;
};

UCLASS(BlueprintType, Const, meta = (DisplayName = "FR Projectile Effect Split"))
class FORTROGUE_API UFRProjectileEffectSplit : public UFRProjectileEffectBase
{
	GENERATED_BODY()

public:
	virtual const UScriptStruct* GetParameterStruct() const override;
	virtual void ApplyToShotSpec(const FFRProjectileEffectSpec& EffectSpec, FFRShotSpec& ShotSpec) const override;
	virtual void HandleImpact(const FFRProjectileEffectSpec& EffectSpec, const FFRProjectileEffectImpactContext& Context) const override;
	virtual bool RequiresProjectileRuntime(const FFRProjectileEffectSpec& EffectSpec) const override;
	virtual void AddDataValidationIssues(const FFRProjectileEffectSpec& EffectSpec, TArray<FString>& Issues) const override;
};

USTRUCT(BlueprintType)
struct FORTROGUE_API FFRProjectileEffectSplitModifierParams : public FFRProjectileEffectParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Split Modifier", meta = (ClampMin = "0.0", ToolTip = "기존 Split child 피해 배율에 추가로 곱할 값입니다."))
	float ChildDamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Split Modifier", meta = (ClampMin = "0.0", ToolTip = "기존 Split child 폭발 반경 배율에 추가로 곱할 값입니다."))
	float ChildBlastRadiusMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Split Modifier", meta = (ClampMin = "0.0", ToolTip = "기존 Split child 지형 파괴 반경 배율에 추가로 곱할 값입니다."))
	float ChildTerrainCarveMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Split Modifier", meta = (ToolTip = "켜면 부모의 비행/충돌 효과를 child가 상속하도록 기존 Split을 변경합니다."))
	bool bInheritParentRuntimeEffects = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Split Modifier", meta = (TitleProperty = EffectClass, ToolTip = "기존 Split의 모든 child에 추가할 효과입니다."))
	TArray<FFRProjectileEffectSpec> ChildProjectileEffects;
};

UCLASS(BlueprintType, Const, meta = (DisplayName = "FR Shell Effect Split Modifier"))
class FORTROGUE_API UFRProjectileEffectSplitModifier : public UFRProjectileEffectBase
{
	GENERATED_BODY()

public:
	virtual const UScriptStruct* GetParameterStruct() const override;
	virtual void ApplyToShotSpec(const FFRProjectileEffectSpec& EffectSpec, FFRShotSpec& ShotSpec) const override;
	virtual void AddDataValidationIssues(const FFRProjectileEffectSpec& EffectSpec, TArray<FString>& Issues) const override;
};
