// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ProjectileEffects/FRProjectileEffect.h"
#include "FRProjectileSpecEffects.generated.h"

USTRUCT(BlueprintType)
struct FORTROGUE_API FFRProjectileEffectDirectHitDamageParams : public FFRProjectileEffectParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Hit Damage", meta = (ToolTip = "직격 피해에 먼저 더할 고정값입니다."))
	float BonusDamage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Hit Damage", meta = (ClampMin = "0.0", ToolTip = "고정값 적용 후 직격 피해에 곱할 배율입니다."))
	float DamageMultiplier = 1.0f;
};

UCLASS(BlueprintType, Const, meta = (DisplayName = "FR Shell Effect Direct Hit Damage"))
class FORTROGUE_API UFRProjectileEffectDirectHitDamage : public UFRProjectileEffectBase
{
	GENERATED_BODY()

public:
	virtual const UScriptStruct* GetParameterStruct() const override;
	virtual void ApplyToShotSpec(const FFRProjectileEffectSpec& EffectSpec, FFRShotSpec& ShotSpec) const override;
	virtual void AddDataValidationIssues(const FFRProjectileEffectSpec& EffectSpec, TArray<FString>& Issues) const override;
};

USTRUCT(BlueprintType)
struct FORTROGUE_API FFRProjectileEffectExplosionPayloadParams : public FFRProjectileEffectParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion Payload", meta = (ToolTip = "폭발 피해에 먼저 더할 고정값입니다."))
	float DamageBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion Payload", meta = (ClampMin = "0.0", ToolTip = "고정값 적용 후 폭발 피해에 곱할 배율입니다."))
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion Payload", meta = (ToolTip = "폭발 반경에 먼저 더할 고정값입니다."))
	float RadiusBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion Payload", meta = (ClampMin = "0.0", ToolTip = "고정값 적용 후 폭발 반경에 곱할 배율입니다."))
	float RadiusMultiplier = 1.0f;
};

UCLASS(BlueprintType, Const, meta = (DisplayName = "FR Shell Effect Explosion Payload"))
class FORTROGUE_API UFRProjectileEffectExplosionPayload : public UFRProjectileEffectBase
{
	GENERATED_BODY()

public:
	virtual const UScriptStruct* GetParameterStruct() const override;
	virtual void ApplyToShotSpec(const FFRProjectileEffectSpec& EffectSpec, FFRShotSpec& ShotSpec) const override;
	virtual void AddDataValidationIssues(const FFRProjectileEffectSpec& EffectSpec, TArray<FString>& Issues) const override;
};

USTRUCT(BlueprintType)
struct FORTROGUE_API FFRProjectileEffectTerrainCarvePayloadParams : public FFRProjectileEffectParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Carve", meta = (ToolTip = "지형 파괴 반경에 먼저 더할 고정값입니다."))
	float RadiusBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Carve", meta = (ClampMin = "0.0", ToolTip = "고정값 적용 후 지형 파괴 반경에 곱할 배율입니다."))
	float RadiusMultiplier = 1.0f;
};

UCLASS(BlueprintType, Const, meta = (DisplayName = "FR Shell Effect Terrain Carve Payload"))
class FORTROGUE_API UFRProjectileEffectTerrainCarvePayload : public UFRProjectileEffectBase
{
	GENERATED_BODY()

public:
	virtual const UScriptStruct* GetParameterStruct() const override;
	virtual void ApplyToShotSpec(const FFRProjectileEffectSpec& EffectSpec, FFRShotSpec& ShotSpec) const override;
	virtual void AddDataValidationIssues(const FFRProjectileEffectSpec& EffectSpec, TArray<FString>& Issues) const override;
};

USTRUCT(BlueprintType)
struct FORTROGUE_API FFRProjectileEffectTerrainFillPayloadParams : public FFRProjectileEffectParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Fill", meta = (ToolTip = "지형 생성 반경에 먼저 더할 고정값입니다."))
	float RadiusBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Fill", meta = (ClampMin = "0.0", ToolTip = "고정값 적용 후 지형 생성 반경에 곱할 배율입니다."))
	float RadiusMultiplier = 1.0f;
};

UCLASS(BlueprintType, Const, meta = (DisplayName = "FR Shell Effect Terrain Fill Payload"))
class FORTROGUE_API UFRProjectileEffectTerrainFillPayload : public UFRProjectileEffectBase
{
	GENERATED_BODY()

public:
	virtual const UScriptStruct* GetParameterStruct() const override;
	virtual void ApplyToShotSpec(const FFRProjectileEffectSpec& EffectSpec, FFRShotSpec& ShotSpec) const override;
	virtual void AddDataValidationIssues(const FFRProjectileEffectSpec& EffectSpec, TArray<FString>& Issues) const override;
};

USTRUCT(BlueprintType)
struct FORTROGUE_API FFRProjectileEffectFlightProfileParams : public FFRProjectileEffectParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight Profile", meta = (ToolTip = "발사 속도에 먼저 더할 고정값입니다."))
	float LaunchSpeedBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight Profile", meta = (ClampMin = "0.0", ToolTip = "고정값 적용 후 발사 속도에 곱할 배율입니다."))
	float LaunchSpeedMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight Profile", meta = (ToolTip = "투사체 중력에 먼저 더할 고정값입니다."))
	float GravityBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight Profile", meta = (ClampMin = "0.0", ToolTip = "고정값 적용 후 투사체 중력에 곱할 배율입니다."))
	float GravityMultiplier = 1.0f;
};

UCLASS(BlueprintType, Const, meta = (DisplayName = "FR Shell Effect Flight Profile"))
class FORTROGUE_API UFRProjectileEffectFlightProfile : public UFRProjectileEffectBase
{
	GENERATED_BODY()

public:
	virtual const UScriptStruct* GetParameterStruct() const override;
	virtual void ApplyToShotSpec(const FFRProjectileEffectSpec& EffectSpec, FFRShotSpec& ShotSpec) const override;
	virtual void AddDataValidationIssues(const FFRProjectileEffectSpec& EffectSpec, TArray<FString>& Issues) const override;
};

USTRUCT(BlueprintType)
struct FORTROGUE_API FFRProjectileEffectProjectileCountParams : public FFRProjectileEffectParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Count", meta = (ToolTip = "한 번의 발사에 추가할 투사체 수입니다. 최종 수는 최소 1개로 보정됩니다."))
	int32 CountBonus = 0;
};

UCLASS(BlueprintType, Const, meta = (DisplayName = "FR Shell Effect Projectile Count"))
class FORTROGUE_API UFRProjectileEffectProjectileCount : public UFRProjectileEffectBase
{
	GENERATED_BODY()

public:
	virtual const UScriptStruct* GetParameterStruct() const override;
	virtual void ApplyToShotSpec(const FFRProjectileEffectSpec& EffectSpec, FFRShotSpec& ShotSpec) const override;
};

USTRUCT(BlueprintType)
struct FORTROGUE_API FFRProjectileEffectSalvoParams : public FFRProjectileEffectParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Salvo", meta = (ToolTip = "한 번의 발사 입력에 추가할 반복 발사 횟수입니다."))
	int32 CountBonus = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Salvo", meta = (ClampMin = "0.0", ToolTip = "0보다 크면 반복 발사 간격을 이 값으로 설정합니다."))
	float SalvoInterval = 0.15f;
};

UCLASS(BlueprintType, Const, meta = (DisplayName = "FR Shell Effect Salvo"))
class FORTROGUE_API UFRProjectileEffectSalvo : public UFRProjectileEffectBase
{
	GENERATED_BODY()

public:
	virtual const UScriptStruct* GetParameterStruct() const override;
	virtual void ApplyToShotSpec(const FFRProjectileEffectSpec& EffectSpec, FFRShotSpec& ShotSpec) const override;
	virtual void AddDataValidationIssues(const FFRProjectileEffectSpec& EffectSpec, TArray<FString>& Issues) const override;
};

USTRUCT(BlueprintType)
struct FORTROGUE_API FFRProjectileEffectKnockbackParams : public FFRProjectileEffectParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knockback", meta = (ClampMin = "0.0", ToolTip = "직격한 캐릭터를 충돌 지점 반대 방향으로 밀어낼 거리입니다."))
	float HorizontalDistance = 80.0f;
};

UCLASS(BlueprintType, Const, meta = (DisplayName = "FR Projectile Effect Knockback"))
class FORTROGUE_API UFRProjectileEffectKnockback : public UFRProjectileEffectBase
{
	GENERATED_BODY()

public:
	virtual const UScriptStruct* GetParameterStruct() const override;
	virtual void ApplyToShotSpec(const FFRProjectileEffectSpec& EffectSpec, FFRShotSpec& ShotSpec) const override;
	virtual void HandlePostImpact(const FFRProjectileEffectSpec& EffectSpec, const FFRProjectileEffectImpactContext& Context) const override;
	virtual bool RequiresProjectileRuntime(const FFRProjectileEffectSpec& EffectSpec) const override;
	virtual void AddDataValidationIssues(const FFRProjectileEffectSpec& EffectSpec, TArray<FString>& Issues) const override;
};
