// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "UObject/Object.h"
#include "FRProjectileEffect.generated.h"

class AFortRogueBattleCharacter;
class AFortRogueDestructibleTerrain;
class AFortRogueProjectile;
struct FFortRogueShotSpec;
struct FFRProjectileEffectSpec;

struct FFRProjectileEffectImpactContext
{
	UWorld* World = nullptr;
	AFortRogueProjectile* Projectile = nullptr;
	AFortRogueBattleCharacter* OwnerCharacter = nullptr;
	AFortRogueDestructibleTerrain* AssignedTerrain = nullptr;
	FVector ImpactLocation = FVector::ZeroVector;
	FVector Velocity = FVector::ZeroVector;
	FGameplayTag WeaponTag;
	FGameplayTagContainer EffectTags;
	float Damage = 0.0f;
	float BlastRadius = 0.0f;
	float TerrainCarveRadius = 0.0f;
	float TerrainFillRadius = 0.0f;
	float Gravity = 0.0f;
};

USTRUCT(BlueprintType)
struct FORTROGUE_API FFRProjectileEffectParameters
{
	GENERATED_BODY()
};

UCLASS(Abstract, BlueprintType, Const)
class FORTROGUE_API UFRProjectileEffectBase : public UObject
{
	GENERATED_BODY()

public:
	virtual const UScriptStruct* GetParameterStruct() const;
	virtual void ApplyToShotSpec(const FFRProjectileEffectSpec& EffectSpec, FFortRogueShotSpec& ShotSpec) const;
	virtual void HandleImpact(const FFRProjectileEffectSpec& EffectSpec, const FFRProjectileEffectImpactContext& Context) const;
	virtual bool UsesCustomTerrainImpact(const FFRProjectileEffectSpec& EffectSpec) const;
	virtual void AddDataValidationIssues(const FFRProjectileEffectSpec& EffectSpec, TArray<FString>& Issues) const;
};

USTRUCT(BlueprintType)
struct FORTROGUE_API FFRProjectileEffectSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Effect", meta = (AllowAbstract = "false", ToolTip = "실행할 투사체 효과 클래스입니다. Drill, TerrainCreate 같은 UFRProjectileEffectBase 상속 클래스를 선택하세요."))
	TSubclassOf<UFRProjectileEffectBase> EffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Effect", meta = (BaseStruct = "/Script/FortRogue.FRProjectileEffectParameters", ToolTip = "EffectClass가 요구하는 파라미터 구조체입니다. DetailCustomization이 EffectClass에 맞는 구조체로 자동 보정합니다."))
	FInstancedStruct Parameters;

	const UFRProjectileEffectBase* GetEffectCDO() const;
	const UScriptStruct* GetExpectedParameterStruct() const;
	FText GetEffectDisplayName() const;
	bool HasValidParameters() const;
	bool EnsureParametersMatchEffectClass();
	void ApplyToShotSpec(FFortRogueShotSpec& ShotSpec) const;
	void HandleImpact(const FFRProjectileEffectImpactContext& Context) const;
	bool UsesCustomTerrainImpact() const;
	FText GetDataValidationSummary() const;

	template <typename ParametersType>
	const ParametersType& GetParametersOrDefault() const
	{
		if (const ParametersType* TypedParameters = Parameters.GetPtr<ParametersType>())
		{
			return *TypedParameters;
		}

		static const ParametersType DefaultParameters;
		return DefaultParameters;
	}
};

USTRUCT(BlueprintType)
struct FORTROGUE_API FFRProjectileEffectDrillParams : public FFRProjectileEffectParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drill", meta = (ClampMin = "0.0", ToolTip = "기본 지형 파괴 반경에 더할 값입니다. 0이면 현재 ShotSpec의 TerrainCarveRadius를 그대로 사용합니다."))
	float RadiusBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drill", meta = (ClampMin = "0.0", ToolTip = "RadiusBonus 적용 후 지형 파괴 반경에 곱할 배율입니다. 1.0은 변화 없음입니다."))
	float RadiusMultiplier = 1.0f;
};

UCLASS(BlueprintType, Const, meta = (DisplayName = "FR Projectile Effect Drill"))
class FORTROGUE_API UFRProjectileEffectDrill : public UFRProjectileEffectBase
{
	GENERATED_BODY()

public:
	virtual const UScriptStruct* GetParameterStruct() const override;
	virtual void ApplyToShotSpec(const FFRProjectileEffectSpec& EffectSpec, FFortRogueShotSpec& ShotSpec) const override;
	virtual void HandleImpact(const FFRProjectileEffectSpec& EffectSpec, const FFRProjectileEffectImpactContext& Context) const override;
	virtual bool UsesCustomTerrainImpact(const FFRProjectileEffectSpec& EffectSpec) const override;
	virtual void AddDataValidationIssues(const FFRProjectileEffectSpec& EffectSpec, TArray<FString>& Issues) const override;
};

USTRUCT(BlueprintType)
struct FORTROGUE_API FFRProjectileEffectTerrainCreateParams : public FFRProjectileEffectParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Create", meta = (ClampMin = "0.0", ToolTip = "생성할 지형 반경에 더할 값입니다. 기본값 120은 일반 탄보다 작은 발판/벽을 만드는 기준값입니다."))
	float RadiusBonus = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Create", meta = (ClampMin = "0.0", ToolTip = "RadiusBonus 적용 후 지형 생성 반경에 곱할 배율입니다. 1.0은 변화 없음입니다."))
	float RadiusMultiplier = 1.0f;
};

UCLASS(BlueprintType, Const, meta = (DisplayName = "FR Projectile Effect Terrain Create"))
class FORTROGUE_API UFRProjectileEffectTerrainCreate : public UFRProjectileEffectBase
{
	GENERATED_BODY()

public:
	virtual const UScriptStruct* GetParameterStruct() const override;
	virtual void ApplyToShotSpec(const FFRProjectileEffectSpec& EffectSpec, FFortRogueShotSpec& ShotSpec) const override;
	virtual void HandleImpact(const FFRProjectileEffectSpec& EffectSpec, const FFRProjectileEffectImpactContext& Context) const override;
	virtual bool UsesCustomTerrainImpact(const FFRProjectileEffectSpec& EffectSpec) const override;
	virtual void AddDataValidationIssues(const FFRProjectileEffectSpec& EffectSpec, TArray<FString>& Issues) const override;
};
