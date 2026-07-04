// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Items/FRItemDefinition.h"
#include "FRDefaultLoadoutDefinition.generated.h"

class UFRWeaponDefinition;

UCLASS(BlueprintType)
class FORTROGUE_API UFRDefaultLoadoutDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|Loadout")
	FText GetDataValidationSummary() const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapons", meta = (ToolTip = "플레이어가 런 시작 시 장착할 기본 무기 목록입니다. 첫 번째 무기가 기본 선택 무기가 됩니다."))
	TArray<TObjectPtr<UFRWeaponDefinition>> WeaponDefinitions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Items", meta = (TitleProperty = ItemDefinition, ToolTip = "플레이어가 런 시작 시 보유할 아이템과 사용 횟수 목록입니다."))
	TArray<FFRItemStack> ItemDefinitions;
};
