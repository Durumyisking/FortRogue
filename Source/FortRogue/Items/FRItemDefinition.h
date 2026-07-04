// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Weapons/FRWeaponDefinition.h"
#include "FRItemDefinition.generated.h"

class UFRAbilitySet;
class UFRItemDefinition;
class UFRItemEffect;

USTRUCT(BlueprintType)
struct FFRItemStack
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ToolTip = "캐릭터가 보유할 아이템 데이터입니다. 비워두면 해당 슬롯은 사용할 수 없습니다."))
	TObjectPtr<UFRItemDefinition> ItemDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ClampMin = "0", ToolTip = "이 아이템을 몇 회 사용할 수 있는지 정합니다. 0이면 보유하지만 사용할 수 없습니다."))
	int32 Charges = 1;
};

UENUM(BlueprintType)
enum class EFRItemType : uint8
{
	AttackMultiplier UMETA(ToolTip = "공격 강화 단축키(J)로 사용되는 분류입니다."),
	Heal UMETA(ToolTip = "회복 단축키(H)로 사용되는 분류입니다."),
	AbilitySet UMETA(ToolTip = "능력 부여형 아이템 분류입니다.")
};

UCLASS(BlueprintType)
class FORTROGUE_API UFRItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|Item")
	FText GetDataValidationSummary() const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ToolTip = "에디터와 UI에 표시할 아이템 이름입니다."))
	FText DisplayName = FText::FromString(TEXT("Item"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (MultiLine = "true", ToolTip = "아이템 설명입니다. 사용 시 어떤 선택지가 생기는지 적어주세요."))
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (Categories = "Item", ToolTip = "아이템을 식별하는 태그입니다. Item.* 태그만 사용하세요."))
	FGameplayTag ItemTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ToolTip = "아이템 단축키/UI 분류입니다. 실제 효과는 UseEffects, UseAbilitySet, UseShotModifiers로 정의합니다."))
	EFRItemType ItemType = EFRItemType::Heal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "1", ToolTip = "런 시작 시 지급되는 기본 사용 횟수입니다. 1 이상이어야 합니다."))
	int32 InitialCharges = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "Item|Effects", meta = (ToolTip = "아이템 사용 시 즉시 실행할 조립식 효과 목록입니다. 회복, 공격 강화 같은 효과 클래스를 추가하세요."))
	TArray<TObjectPtr<UFRItemEffect>> UseEffects;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Effects", meta = (ToolTip = "아이템 사용 시 부여할 AbilitySet입니다."))
	TObjectPtr<UFRAbilitySet> UseAbilitySet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shot Modifier", meta = (TitleProperty = ModifierTag, ToolTip = "아이템 사용 후 다음 샷에 적용할 modifier 목록입니다. 굴착, 지형 생성, 분열 같은 샷 변화를 여기에 조립합니다."))
	TArray<FFRShotModifierSpec> UseShotModifiers;
};
