// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Weapons/FortRogueWeaponDefinition.h"
#include "FortRogueItemDefinition.generated.h"

class UFortRogueAbilitySet;

UENUM(BlueprintType)
enum class EFortRogueItemType : uint8
{
	AttackMultiplier,
	Heal,
	AbilitySet
};

UCLASS(BlueprintType)
class FORTROGUE_API UFortRogueItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|Item")
	FText GetDataValidationSummary() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ToolTip = "에디터와 UI에 표시할 아이템 이름입니다."))
	FText DisplayName = FText::FromString(TEXT("Item"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (MultiLine = "true", ToolTip = "아이템 설명입니다. 사용 시 어떤 선택지가 생기는지 적어주세요."))
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (Categories = "Item", ToolTip = "아이템을 식별하는 태그입니다. Item.* 태그만 사용하세요."))
	FGameplayTag ItemTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ToolTip = "아이템 사용 방식입니다. Heal은 즉시 회복, AttackMultiplier는 다음 샷 강화, AbilitySet은 GAS 능력 세트를 부여합니다."))
	EFortRogueItemType ItemType = EFortRogueItemType::Heal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "1", ToolTip = "런 시작 시 지급되는 기본 사용 횟수입니다. 1 이상이어야 합니다."))
	int32 InitialCharges = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ToolTip = "아이템 사용 시 부여할 AbilitySet입니다. ItemType이 AbilitySet일 때 주로 사용합니다."))
	TObjectPtr<UFortRogueAbilitySet> UseAbilitySet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shot Modifier", meta = (TitleProperty = DisplayName, ToolTip = "아이템 사용 후 다음 샷에 적용할 modifier 목록입니다. 굴착, 지형 생성, 분열 같은 샷 변화를 여기에 조립합니다."))
	TArray<FFortRogueShotModifierSpec> UseShotModifiers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (EditCondition = "ItemType == EFortRogueItemType::AttackMultiplier", ClampMin = "1.0", ToolTip = "AttackMultiplier 아이템이 다음 샷 피해량에 적용할 배율입니다. 1.0은 변화 없음입니다."))
	float AttackMultiplier = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (EditCondition = "ItemType == EFortRogueItemType::Heal", ClampMin = "0.0", ToolTip = "Heal 아이템 사용 시 회복할 체력입니다."))
	float HealAmount = 35.0f;
};
