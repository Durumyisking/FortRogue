// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Items/FRItemDefinition.h"
#include "FRCharacterDefinition.generated.h"

class UFRAbilitySet;
class UPaperFlipbook;
class UFRWeaponDefinition;

/** 전투 상태별 캐릭터 스프라이트 플립북 모음입니다. 비어 있는 항목은 Idle(없으면 BodyFlipbook)로 대체됩니다. */
USTRUCT(BlueprintType)
struct FFRCharacterAnimationSet
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (ToolTip = "대기 상태에서 반복 재생할 플립북입니다."))
	TObjectPtr<UPaperFlipbook> Idle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (ToolTip = "지형 이동 중 반복 재생할 플립북입니다."))
	TObjectPtr<UPaperFlipbook> Move;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (ToolTip = "기본 공격 발사 시 한 번 재생할 플립북입니다."))
	TObjectPtr<UPaperFlipbook> Shoot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (ToolTip = "특수 공격 발사 시 한 번 재생할 플립북입니다. 비어 있으면 Shoot을 사용합니다."))
	TObjectPtr<UPaperFlipbook> Special;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (ToolTip = "피격 시 한 번 재생할 플립북입니다."))
	TObjectPtr<UPaperFlipbook> Hurt;

	bool HasAnyAnimation() const
	{
		return Idle || Move || Shoot || Special || Hurt;
	}
};

UCLASS(BlueprintType)
class FORTROGUE_API UFRCharacterDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character", meta = (ToolTip = "에디터와 UI에 표시할 캐릭터 이름입니다."))
	FText DisplayName = FText::FromString(TEXT("Rookie Tank"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual", meta = (ToolTip = "전투에서 캐릭터 몸체로 표시할 Paper2D flipbook입니다. AnimationSet.Idle이 있으면 그쪽이 우선합니다."))
	TObjectPtr<UPaperFlipbook> BodyFlipbook;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual", meta = (ToolTip = "전투 상태(대기/이동/발사/특수/피격)별 스프라이트 플립북 모음입니다."))
	FFRCharacterAnimationSet AnimationSet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "1.0", ToolTip = "캐릭터의 최대 체력입니다. 1 이상 값을 사용하세요."))
	float MaxHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0", ToolTip = "모든 샷 피해에 더해지는 기본 피해 보너스입니다."))
	float BonusDamage = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0", ToolTip = "한 턴에 이동할 수 있는 최대 거리 예산입니다. 0이면 이동할 수 없습니다."))
	float MaxMoveBudget = 420.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0", ToolTip = "샷 차지 파워에 곱해지는 배율입니다. 1.0은 변화 없음입니다."))
	float ShotPowerMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "-90.0", ClampMax = "180.0", ToolTip = "캐릭터가 조준할 수 있는 최소 발사 각도입니다."))
	float MinAimAngle = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "-90.0", ClampMax = "180.0", ToolTip = "캐릭터가 조준할 수 있는 최대 발사 각도입니다."))
	float MaxAimAngle = 90.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities", meta = (ToolTip = "전투 시작 시 이 캐릭터에게 자동으로 부여할 AbilitySet 목록입니다."))
	TArray<TObjectPtr<UFRAbilitySet>> StartupAbilitySets;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perks", meta = (Categories = "Trait.Category", ToolTip = "비어 있지 않으면 모든 태그를 가진 Perk만 이 캐릭터의 보상 후보가 됩니다. 무기와 아이템 보상에는 적용되지 않습니다."))
	FGameplayTagContainer RequiredPerkCategoryTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perks", meta = (Categories = "Trait.Category", ToolTip = "하나라도 가진 Perk을 이 캐릭터의 보상 후보에서 제외합니다. 무기와 아이템 보상에는 적용되지 않습니다."))
	FGameplayTagContainer BlockedPerkCategoryTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attacks", meta = (ToolTip = "모든 직업이 공유하는 기본 공격입니다. WeaponDefinition과 같은 샷 데이터 구조를 사용합니다."))
	TObjectPtr<UFRWeaponDefinition> BasicAttackDefinition;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attacks", meta = (ToolTip = "직업 정체성을 만드는 특수 공격입니다. 기본 공격과 같은 WeaponDefinition 데이터 구조를 사용합니다."))
	TObjectPtr<UFRWeaponDefinition> SpecialAttackDefinition;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attacks", meta = (ToolTip = "이 캐릭터가 특수 공격 슬롯을 사용할 수 있는지 정합니다. 적 배치에서는 별도로 끄거나 켤 수 있습니다."))
	bool bCanUseSpecialAttack = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapons", meta = (ToolTip = "호환용 추가 무기 목록입니다. 기본/특수 공격 뒤에 붙는 추가 슬롯으로 사용됩니다."))
	TArray<TObjectPtr<UFRWeaponDefinition>> WeaponLoadout;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Items", meta = (TitleProperty = ItemDefinition, ToolTip = "이 캐릭터가 기본으로 보유할 아이템과 사용 횟수 목록입니다."))
	TArray<FFRItemStack> ItemLoadout;
};
