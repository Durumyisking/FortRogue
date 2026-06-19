// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Weapons/FortRogueWeaponDefinition.h"
#include "FortRogueRewardTypes.generated.h"

class UFortRogueItemDefinition;
class UFortRoguePerkDefinition;
class UFortRogueWeaponDefinition;
class UFortRogueAbilitySet;

UENUM(BlueprintType)
enum class EFortRogueRewardType : uint8
{
	Weapon,
	Consumable,
	Trait
};

USTRUCT(BlueprintType)
struct FFortRogueRewardChoice
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward", meta = (ToolTip = "보상 종류입니다. Weapon은 무기, Consumable은 아이템, Trait은 퍽/패시브 보상입니다."))
	EFortRogueRewardType Type = EFortRogueRewardType::Trait;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward", meta = (ToolTip = "보상 선택 화면에 표시할 이름입니다."))
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward", meta = (ToolTip = "보상 선택 화면에 표시할 설명입니다. 플레이 방식이 어떻게 바뀌는지 적어주세요."))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward", meta = (Categories = "Weapon,Item,Trait", ToolTip = "런 안에서 보상 획득 여부를 추적할 태그입니다. Weapon.*, Item.*, Trait.* 태그만 사용하세요."))
	FGameplayTag RewardTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward", meta = (ClampMin = "0.0", ToolTip = "보상 풀에서 선택될 상대 가중치입니다. 0보다 커야 합니다."))
	float RewardWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward", meta = (ToolTip = "켜면 같은 런에서 RewardTag가 이미 선택된 후 다시 등장하지 않습니다. RewardTag가 필요합니다."))
	bool bOfferOncePerRun = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward|Condition", meta = (Categories = "Weapon,Item,Trait", ToolTip = "이 보상이 등장하려면 이미 선택되어 있어야 하는 보상 태그입니다."))
	FGameplayTagContainer RequiredRewardTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward|Condition", meta = (Categories = "Weapon,Item,Trait", ToolTip = "이 태그가 이미 선택되어 있으면 보상이 등장하지 않습니다."))
	FGameplayTagContainer BlockedRewardTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward", meta = (ToolTip = "Type이 Weapon일 때 지급할 무기 데이터입니다."))
	TObjectPtr<UFortRogueWeaponDefinition> WeaponReward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward", meta = (ToolTip = "Type이 Consumable일 때 지급할 아이템 데이터입니다."))
	TObjectPtr<UFortRogueItemDefinition> ItemReward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward", meta = (ToolTip = "Type이 Trait일 때 지급할 퍽 데이터입니다."))
	TObjectPtr<UFortRoguePerkDefinition> PerkReward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward", meta = (ToolTip = "보상 획득 시 직접 부여할 AbilitySet입니다."))
	TObjectPtr<UFortRogueAbilitySet> GrantedAbilitySet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier", meta = (TitleProperty = DisplayName, ToolTip = "보상 획득 후 플레이어에게 부여할 샷 modifier 목록입니다."))
	TArray<FFortRogueShotModifierSpec> ShotModifiers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward", meta = (ToolTip = "캐릭터 기본 피해에 더할 고정값입니다."))
	float DamageBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward", meta = (ToolTip = "최대 체력에 더할 고정값입니다."))
	float MaxHealthBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward", meta = (ToolTip = "턴당 최대 이동 예산에 더할 고정값입니다."))
	float MaxMoveBudgetBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward", meta = (ToolTip = "기본 발사 투사체 수에 더할 값입니다."))
	int32 ProjectileBonus = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward", meta = (ToolTip = "샷 파워 배율에 더할 값입니다. 0이면 변화 없음입니다."))
	float ShotPowerMultiplierBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward", meta = (ToolTip = "ItemReward의 사용 횟수에 추가할 보상 충전 수입니다."))
	int32 RepairCharges = 0;

	FText GetEffectSummary() const;
	FText GetDataValidationSummary() const;
	bool MeetsRewardTagConditions(const FGameplayTagContainer& ChosenRewardTags) const;
	FText GetRewardTagConditionFailureSummary(const FGameplayTagContainer& ChosenRewardTags) const;
};
