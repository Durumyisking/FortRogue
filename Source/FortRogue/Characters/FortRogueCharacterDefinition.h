// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "Items/FortRogueItemDefinition.h"
#include "FortRogueCharacterDefinition.generated.h"

class UFortRogueAbilitySet;
class UPaperFlipbook;
class UFortRogueTerrainMapDefinition;
class UFortRogueWeaponDefinition;

USTRUCT(BlueprintType)
struct FFortRogueItemStack
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ToolTip = "캐릭터가 보유할 아이템 데이터입니다. 비워두면 해당 슬롯은 사용할 수 없습니다."))
	TObjectPtr<UFortRogueItemDefinition> ItemDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ClampMin = "0", ToolTip = "이 아이템을 몇 회 사용할 수 있는지 정합니다. 0이면 보유하지만 사용할 수 없습니다."))
	int32 Charges = 1;
};

UCLASS(BlueprintType)
class FORTROGUE_API UFortRogueCharacterDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character", meta = (ToolTip = "에디터와 UI에 표시할 캐릭터 이름입니다."))
	FText DisplayName = FText::FromString(TEXT("Rookie Tank"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character", meta = (ToolTip = "이 캐릭터가 전투에 사용할 전용 맵 데이터입니다. 비워두면 런/스테이지 기본 맵을 사용합니다."))
	TObjectPtr<UFortRogueTerrainMapDefinition> BattleMapDefinition;

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
	TArray<TObjectPtr<UFortRogueAbilitySet>> StartupAbilitySets;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapons", meta = (ToolTip = "이 캐릭터가 기본으로 보유할 무기 목록입니다. 첫 번째 무기가 기본 선택 무기가 됩니다."))
	TArray<TObjectPtr<UFortRogueWeaponDefinition>> WeaponLoadout;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Items", meta = (TitleProperty = ItemDefinition, ToolTip = "이 캐릭터가 기본으로 보유할 아이템과 사용 횟수 목록입니다."))
	TArray<FFortRogueItemStack> ItemLoadout;
};
