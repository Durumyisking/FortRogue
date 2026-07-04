// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Weapons/FRWeaponDefinition.h"
#include "FRRewardTypes.generated.h"

class AFRBattleCharacter;
class UFRAbilitySet;
class UFRRewardGrant;

USTRUCT(BlueprintType)
struct FFRRewardChoice
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward", meta = (ToolTip = "보상 선택 화면에 표시할 이름입니다. 비워두면 첫 번째 그랜트의 기본 이름을 사용합니다."))
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward", meta = (ToolTip = "보상 선택 화면에 표시할 설명입니다. 플레이 방식이 어떻게 바뀌는지 적어주세요."))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Reward", meta = (ToolTip = "이 보상이 지급할 그랜트 목록입니다. 무기, 아이템, 퍽 그랜트를 조합할 수 있습니다."))
	TArray<TObjectPtr<UFRRewardGrant>> Grants;

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

	void ApplyToCharacter(AFRBattleCharacter& Character) const;
	FText GetResolvedDisplayName() const;
	FGameplayTag GetResolvedRewardTag() const;
	FText GetEffectSummary() const;
	FText GetDataValidationSummary() const;
	bool MeetsRewardTagConditions(const FGameplayTagContainer& ChosenRewardTags) const;
	FText GetRewardTagConditionFailureSummary(const FGameplayTagContainer& ChosenRewardTags) const;
	bool MatchesPerkCategoryFilter(const FGameplayTagContainer& RequiredCategoryTags, const FGameplayTagContainer& BlockedCategoryTags) const;
};

FORTROGUE_API FText GetFortRogueShotModifierEffectSummary(const TArray<FFRShotModifierSpec>& ShotModifiers);
FORTROGUE_API void AppendFortRogueShotModifierSummary(TArray<FString>& Parts, const TArray<FFRShotModifierSpec>& ShotModifiers);
FORTROGUE_API void AppendFortRogueAbilitySetSummary(TArray<FString>& Parts, const UFRAbilitySet* AbilitySet);
