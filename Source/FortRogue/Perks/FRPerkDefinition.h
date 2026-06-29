// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Weapons/FRWeaponDefinition.h"
#include "FRPerkDefinition.generated.h"

class UFRAbilitySet;

UENUM(BlueprintType)
enum class EFRPerkRarity : uint8
{
	Common UMETA(DisplayName = "Common", ToolTip = "직관적이고 바로 이해되는 기본형 퍽입니다."),
	Rare UMETA(DisplayName = "Rare", ToolTip = "기본형과 시너지형 사이의 퍽입니다."),
	Epic UMETA(DisplayName = "Epic", ToolTip = "새 기믹이나 트레이드오프를 추가하는 퍽입니다."),
	Legendary UMETA(DisplayName = "Legendary", ToolTip = "특이한 룰 변경이나 강한 시너지 축을 여는 퍽입니다.")
};

UCLASS(BlueprintType)
class FORTROGUE_API UFRPerkDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|Perk")
	FText GetDataValidationSummary() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Perk")
	bool HasAllCategoryTags(const FGameplayTagContainer& RequiredTags) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Perk")
	bool HasAnyCategoryTags(const FGameplayTagContainer& Tags) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perk", meta = (ToolTip = "에디터와 UI에 표시할 퍽 이름입니다."))
	FText DisplayName = FText::FromString(TEXT("Perk"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perk", meta = (ToolTip = "퍽 설명입니다. 런에서 어떤 플레이 스타일을 만드는지 적어주세요."))
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perk", meta = (Categories = "Trait", ToolTip = "퍽을 식별하는 태그입니다. Trait.* 태그만 사용하세요."))
	FGameplayTag PerkTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perk", meta = (Categories = "Trait.Category", ToolTip = "보상 필터와 시너지 검색에 사용할 복수 분류 태그입니다. Trait.Category.* 태그만 사용하세요."))
	FGameplayTagContainer PerkCategoryTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perk", meta = (ToolTip = "퍽 희귀도입니다. 강함의 절대 등급이 아니라 이해 난이도, 시너지, 룰 변경 정도를 뜻합니다."))
	EFRPerkRarity Rarity = EFRPerkRarity::Common;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perk", meta = (ToolTip = "퍽 획득 시 부여할 AbilitySet입니다."))
	TObjectPtr<UFRAbilitySet> GrantedAbilitySet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shot Modifier", meta = (TitleProperty = ModifierTag, ToolTip = "퍽 획득 후 계속 적용될 샷 modifier 목록입니다. 빌드의 핵심 샷 변화를 여기에 조립합니다."))
	TArray<FFRShotModifierSpec> ShotModifiers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ToolTip = "캐릭터 기본 피해에 더할 고정값입니다. 음수 값은 리스크/보상형 퍽에 사용할 수 있습니다."))
	float DamageBonus = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ToolTip = "최대 체력에 더할 고정값입니다. 음수 값은 리스크/보상형 퍽에 사용할 수 있습니다."))
	float MaxHealthBonus = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ToolTip = "턴당 최대 이동 예산에 더할 고정값입니다. 음수 값은 리스크/보상형 퍽에 사용할 수 있습니다."))
	float MaxMoveBudgetBonus = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ToolTip = "기본 발사 투사체 수에 더할 값입니다. 음수 값은 리스크/보상형 퍽에 사용할 수 있습니다."))
	int32 ProjectileBonus = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ToolTip = "샷 파워 배율에 더할 값입니다. 0이면 변화 없음이며, 음수 값은 리스크/보상형 퍽에 사용할 수 있습니다."))
	float ShotPowerMultiplierBonus = 0.0f;
};
