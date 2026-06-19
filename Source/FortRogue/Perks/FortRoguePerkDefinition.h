// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Weapons/FortRogueWeaponDefinition.h"
#include "FortRoguePerkDefinition.generated.h"

class UFortRogueAbilitySet;

UCLASS(BlueprintType)
class FORTROGUE_API UFortRoguePerkDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|Perk")
	FText GetDataValidationSummary() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perk", meta = (ToolTip = "에디터와 UI에 표시할 퍽 이름입니다."))
	FText DisplayName = FText::FromString(TEXT("Perk"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perk", meta = (ToolTip = "퍽 설명입니다. 런에서 어떤 플레이 스타일을 만드는지 적어주세요."))
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perk", meta = (Categories = "Trait", ToolTip = "퍽을 식별하는 태그입니다. Trait.* 태그만 사용하세요."))
	FGameplayTag PerkTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perk", meta = (ToolTip = "퍽 획득 시 부여할 AbilitySet입니다."))
	TObjectPtr<UFortRogueAbilitySet> GrantedAbilitySet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shot Modifier", meta = (TitleProperty = DisplayName, ToolTip = "퍽 획득 후 계속 적용될 샷 modifier 목록입니다. 빌드의 핵심 샷 변화를 여기에 조립합니다."))
	TArray<FFortRogueShotModifierSpec> ShotModifiers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0", ToolTip = "캐릭터 기본 피해에 더할 고정값입니다."))
	float DamageBonus = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0", ToolTip = "최대 체력에 더할 고정값입니다."))
	float MaxHealthBonus = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0", ToolTip = "턴당 최대 이동 예산에 더할 고정값입니다."))
	float MaxMoveBudgetBonus = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0", ToolTip = "기본 발사 투사체 수에 더할 값입니다."))
	int32 ProjectileBonus = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0", ToolTip = "샷 파워 배율에 더할 값입니다. 0이면 변화 없음입니다."))
	float ShotPowerMultiplierBonus = 0.0f;
};
