// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FortRogueDefaultLoadoutDefinition.generated.h"

class UFortRogueItemDefinition;
class UFortRogueWeaponDefinition;

USTRUCT(BlueprintType)
struct FFortRogueDefaultItemStack
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ToolTip = "런 시작 시 지급할 아이템 데이터입니다. 비워두면 데이터 검수 경고가 발생합니다."))
	TObjectPtr<UFortRogueItemDefinition> ItemDefinition;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "0", ToolTip = "이 아이템을 몇 회 사용할 수 있게 시작할지 정합니다. 0이면 지급은 되지만 사용할 수 없습니다."))
	int32 Charges = 1;
};

UCLASS(BlueprintType)
class FORTROGUE_API UFortRogueDefaultLoadoutDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|Loadout")
	FText GetDataValidationSummary() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapons", meta = (ToolTip = "플레이어가 런 시작 시 장착할 기본 무기 목록입니다. 첫 번째 무기가 기본 선택 무기가 됩니다."))
	TArray<TObjectPtr<UFortRogueWeaponDefinition>> WeaponDefinitions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Items", meta = (TitleProperty = ItemDefinition, ToolTip = "플레이어가 런 시작 시 보유할 아이템과 사용 횟수 목록입니다."))
	TArray<FFortRogueDefaultItemStack> ItemDefinitions;
};
