// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "Items/FRItemDefinition.h"
#include "FRCharacterDefinition.generated.h"

class UFRAbilitySet;
class UPaperFlipbook;
class UFRWeaponDefinition;

UCLASS(BlueprintType)
class FORTROGUE_API UFRCharacterDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character", meta = (ToolTip = "에디터와 UI에 표시할 캐릭터 이름입니다."))
	FText DisplayName = FText::FromString(TEXT("Rookie Tank"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual", meta = (ToolTip = "전투에서 캐릭터 몸체로 표시할 Paper2D flipbook입니다."))
	TObjectPtr<UPaperFlipbook> BodyFlipbook;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "1.0", ToolTip = "캐릭터의 최대 체력입니다. 1 이상 값을 사용하세요."))
	float MaxHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0", ToolTip = "모든 샷 피해에 더해지는 기본 피해 보너스입니다."))
	float BonusDamage = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0", ToolTip = "한 턴에 이동할 수 있는 최대 거리 예산입니다. 0이면 이동할 수 없습니다."))
	float MaxMoveBudget = 420.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0", ToolTip = "샷 차지 파워에 곱해지는 배율입니다. 1.0은 변화 없음입니다."))
	float ShotPowerMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities", meta = (ToolTip = "전투 시작 시 이 캐릭터에게 자동으로 부여할 AbilitySet 목록입니다."))
	TArray<TObjectPtr<UFRAbilitySet>> StartupAbilitySets;

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
