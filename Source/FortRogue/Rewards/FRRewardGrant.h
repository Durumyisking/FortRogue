// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "FRRewardGrant.generated.h"

class AFRBattleCharacter;
class UFRItemDefinition;
class UFRPerkDefinition;
class UFRWeaponDefinition;

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced, CollapseCategories)
class FORTROGUE_API UFRRewardGrant : public UObject
{
	GENERATED_BODY()

public:
	virtual void ApplyToCharacter(AFRBattleCharacter& Character) const {}
	virtual void AppendEffectSummary(TArray<FString>& Parts) const {}
	virtual void AppendValidationIssues(TArray<FString>& Issues) const {}
	virtual bool HasGameplayEffect() const { return false; }
	virtual bool MatchesPerkCategoryFilter(const FGameplayTagContainer& RequiredCategoryTags, const FGameplayTagContainer& BlockedCategoryTags) const { return true; }
	virtual FGameplayTag GetDefaultRewardTag() const { return FGameplayTag(); }
	virtual FText GetDefaultDisplayName() const { return FText::GetEmpty(); }
};

UCLASS(meta = (DisplayName = "FR Reward Grant Weapon"))
class FORTROGUE_API UFRRewardGrant_Weapon : public UFRRewardGrant
{
	GENERATED_BODY()

public:
	virtual void ApplyToCharacter(AFRBattleCharacter& Character) const override;
	virtual void AppendEffectSummary(TArray<FString>& Parts) const override;
	virtual void AppendValidationIssues(TArray<FString>& Issues) const override;
	virtual bool HasGameplayEffect() const override;
	virtual FGameplayTag GetDefaultRewardTag() const override;
	virtual FText GetDefaultDisplayName() const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reward", meta = (ToolTip = "이 보상이 지급할 무기 데이터입니다."))
	TObjectPtr<UFRWeaponDefinition> WeaponDefinition;
};

UCLASS(meta = (DisplayName = "FR Reward Grant Item"))
class FORTROGUE_API UFRRewardGrant_Item : public UFRRewardGrant
{
	GENERATED_BODY()

public:
	virtual void ApplyToCharacter(AFRBattleCharacter& Character) const override;
	virtual void AppendEffectSummary(TArray<FString>& Parts) const override;
	virtual void AppendValidationIssues(TArray<FString>& Issues) const override;
	virtual bool HasGameplayEffect() const override;
	virtual FGameplayTag GetDefaultRewardTag() const override;
	virtual FText GetDefaultDisplayName() const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reward", meta = (ToolTip = "이 보상이 지급할 아이템 데이터입니다."))
	TObjectPtr<UFRItemDefinition> ItemDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reward", meta = (ClampMin = "-1", ToolTip = "지급할 사용 횟수입니다. -1이면 아이템의 InitialCharges를 사용합니다."))
	int32 ChargesOverride = -1;
};

UCLASS(meta = (DisplayName = "FR Reward Grant Perk"))
class FORTROGUE_API UFRRewardGrant_Perk : public UFRRewardGrant
{
	GENERATED_BODY()

public:
	virtual void ApplyToCharacter(AFRBattleCharacter& Character) const override;
	virtual void AppendEffectSummary(TArray<FString>& Parts) const override;
	virtual void AppendValidationIssues(TArray<FString>& Issues) const override;
	virtual bool HasGameplayEffect() const override;
	virtual bool MatchesPerkCategoryFilter(const FGameplayTagContainer& RequiredCategoryTags, const FGameplayTagContainer& BlockedCategoryTags) const override;
	virtual FGameplayTag GetDefaultRewardTag() const override;
	virtual FText GetDefaultDisplayName() const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reward", meta = (ToolTip = "이 보상이 지급할 퍽 데이터입니다."))
	TObjectPtr<UFRPerkDefinition> PerkDefinition;
};
