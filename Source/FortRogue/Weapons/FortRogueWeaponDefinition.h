// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Combat/FortRogueImpactSpawnSpec.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ProjectileEffects/FRProjectileEffect.h"
#include "FortRogueWeaponDefinition.generated.h"

class AFortRogueProjectile;
struct FFortRogueShotSpec;

USTRUCT(BlueprintType)
struct FFortRogueShotModifierSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier", meta = (ToolTip = "에디터와 UI에 표시할 샷 modifier 이름입니다. 비워두면 데이터 검수 경고가 발생합니다."))
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier", meta = (MultiLine = "true", ToolTip = "플레이어에게 보여줄 설명입니다. 어떤 조건에서 어떤 플레이가 바뀌는지 적어주세요."))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier", meta = (Categories = "ShotEffect", ToolTip = "이 modifier가 최종 ShotSpec에 추가할 효과 태그입니다. ShotEffect.* 태그만 사용하세요."))
	FGameplayTagContainer EffectTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier", meta = (Categories = "ShotEffect", ToolTip = "이 modifier 자체를 찾거나 제거할 때 쓰는 대표 태그입니다. ShotEffect.* 태그만 사용하세요."))
	FGameplayTag ModifierTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier|Condition", meta = (ToolTip = "켜면 현재 조준 각도가 Min/Max 범위 안일 때만 modifier가 적용됩니다."))
	bool bUseAimAngleRange = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier|Condition", meta = (EditCondition = "bUseAimAngleRange", ClampMin = "0.0", ClampMax = "90.0", ToolTip = "modifier가 적용되는 최소 조준 각도입니다. 0~90도 값을 사용하세요."))
	float MinAimAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier|Condition", meta = (EditCondition = "bUseAimAngleRange", ClampMin = "0.0", ClampMax = "90.0", ToolTip = "modifier가 적용되는 최대 조준 각도입니다. Min보다 작으면 데이터 검수 경고가 발생합니다."))
	float MaxAimAngle = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier|Condition", meta = (ToolTip = "켜면 발사 방향과 바람 방향이 같을 때만 modifier가 적용됩니다."))
	bool bRequireWindAligned = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier|Condition", meta = (EditCondition = "bRequireWindAligned", ClampMin = "0.0", ToolTip = "바람 조건에 필요한 최소 바람 세기입니다. 0이면 방향만 확인합니다."))
	float MinWindMagnitude = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier|Condition", meta = (Categories = "Weapon,ShotEffect", ToolTip = "현재 ShotSpec에 반드시 있어야 하는 태그입니다. Weapon.* 또는 ShotEffect.* 태그만 사용하세요."))
	FGameplayTagContainer RequiredShotTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier|Condition", meta = (Categories = "Weapon,ShotEffect", ToolTip = "현재 ShotSpec에 있으면 modifier 적용을 막는 태그입니다. Weapon.* 또는 ShotEffect.* 태그만 사용하세요."))
	FGameplayTagContainer BlockedShotTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier|Effects", meta = (TitleProperty = EffectClass, ToolTip = "조건을 만족했을 때 ShotSpec과 투사체 충돌에 적용할 조립식 효과 목록입니다. Drill과 TerrainCreate를 함께 넣으면 한 발이 파괴와 생성을 모두 수행할 수 있습니다."))
	TArray<FFRProjectileEffectSpec> ProjectileEffects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier", meta = (ToolTip = "최종 피해량에 더할 고정값입니다. 음수도 가능하지만 최종 피해는 0 아래로 내려가지 않습니다."))
	float DamageBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier", meta = (ClampMin = "0.0", ToolTip = "고정 피해 보너스 적용 후 곱할 배율입니다. 1.0은 변화 없음입니다."))
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier", meta = (ToolTip = "폭발 반경에 더할 고정값입니다. 월드 단위 기준입니다."))
	float BlastRadiusBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier", meta = (ClampMin = "0.0", ToolTip = "폭발 반경 보너스 적용 후 곱할 배율입니다. 1.0은 변화 없음입니다."))
	float BlastRadiusMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier", meta = (ToolTip = "지형을 파내는 반경에 더할 고정값입니다. 월드 단위 기준입니다."))
	float TerrainCarveRadiusBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier", meta = (ClampMin = "0.0", ToolTip = "지형 파괴 반경 보너스 적용 후 곱할 배율입니다. 1.0은 변화 없음입니다."))
	float TerrainCarveRadiusMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier", meta = (ToolTip = "지형을 생성하는 반경에 더할 고정값입니다. 0보다 크면 충돌 지점에 지형을 채웁니다."))
	float TerrainFillRadiusBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier", meta = (ClampMin = "0.0", ToolTip = "지형 생성 반경 보너스 적용 후 곱할 배율입니다. 1.0은 변화 없음입니다."))
	float TerrainFillRadiusMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier", meta = (ClampMin = "0.0", ToolTip = "발사 속도에 곱할 배율입니다. 1.0은 변화 없음, 0이면 발사 속도가 0이 됩니다."))
	float LaunchSpeedMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier", meta = (ClampMin = "0.0", ToolTip = "투사체 중력에 곱할 배율입니다. 1.0은 변화 없음, 낮을수록 더 완만하게 날아갑니다."))
	float GravityMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier", meta = (ToolTip = "이번 발사에서 추가로 생성할 투사체 수입니다. 음수 입력 시 최종 투사체 수는 최소 1개로 보정됩니다."))
	int32 ProjectileCountBonus = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier|Impact", meta = (TitleProperty = ProjectileCount, ToolTip = "충돌 후 추가 투사체를 생성하는 기존 분열 설정입니다. 새 효과 구조에서는 Split effect로 옮기는 것이 목표입니다."))
	TArray<FFortRogueImpactSpawnSpec> ImpactSpawns;

	bool MeetsShotConditions(const FFortRogueShotSpec& CurrentShotSpec, float CurrentAimAngle, float Wind, bool bShotFacingRight) const;
	FText GetShotConditionFailureSummary(const FFortRogueShotSpec& CurrentShotSpec, float CurrentAimAngle, float Wind, bool bShotFacingRight) const;
	FText GetDataValidationSummary() const;
};

USTRUCT(BlueprintType)
struct FFortRogueWeaponSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (ToolTip = "에디터와 UI에 표시할 무기 이름입니다."))
	FText DisplayName = FText::FromString(TEXT("Shell"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (ToolTip = "무기 설명입니다. 탄의 플레이 방식과 강점을 적어주세요."))
	FText Description = FText::FromString(TEXT("A reliable arcing shell."));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (Categories = "Weapon", ToolTip = "무기를 식별하는 태그입니다. Weapon.* 태그만 사용하세요."))
	FGameplayTag WeaponTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (Categories = "ShotEffect", ToolTip = "이 무기가 기본으로 가진 샷 효과 태그입니다. ShotEffect.* 태그만 사용하세요."))
	FGameplayTagContainer ShotEffectTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (TitleProperty = DisplayName, ToolTip = "이 무기에 항상 적용되는 modifier 목록입니다. 조건을 만족한 항목만 최종 ShotSpec에 반영됩니다."))
	TArray<FFortRogueShotModifierSpec> ShotModifiers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (TitleProperty = ProjectileCount, ToolTip = "무기 자체가 충돌 후 추가 투사체를 만들 때 사용하는 기존 분열 설정입니다."))
	TArray<FFortRogueImpactSpawnSpec> ImpactSpawns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (ToolTip = "기본 피해량입니다. 캐릭터 피해 보너스와 modifier가 이후 반영됩니다."))
	float Damage = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (ToolTip = "기본 폭발 반경입니다. 지형 파괴 반경의 기본값으로도 사용됩니다."))
	float BlastRadius = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (ToolTip = "기본 발사 속도입니다. 실제 발사 속도는 파워와 샷 파워 배율을 곱해 결정됩니다."))
	float ProjectileSpeed = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (ToolTip = "투사체에 적용되는 기본 중력입니다. 값이 클수록 빠르게 떨어집니다."))
	float Gravity = 980.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (ToolTip = "한 번 발사할 때 생성되는 기본 투사체 수입니다. 최소 1개여야 합니다."))
	int32 ProjectilesPerShot = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (ToolTip = "스폰할 투사체 클래스입니다. 비워두면 기본 AFortRogueProjectile을 사용합니다."))
	TSubclassOf<AFortRogueProjectile> ProjectileClass;

	FText GetDataValidationSummary() const;
};

UCLASS(BlueprintType)
class FORTROGUE_API UFortRogueWeaponDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|Weapon")
	FText GetDataValidationSummary() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FFortRogueWeaponSpec Weapon;
};
