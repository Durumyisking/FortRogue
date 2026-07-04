// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FRItemEffect.generated.h"

class AFRBattleCharacter;

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced, CollapseCategories)
class FORTROGUE_API UFRItemEffect : public UObject
{
	GENERATED_BODY()

public:
	virtual void ApplyToCharacter(AFRBattleCharacter& Character) const {}
	virtual void AppendEffectSummary(TArray<FString>& Parts) const {}
	virtual void AppendValidationIssues(TArray<FString>& Issues) const {}
	virtual bool HasGameplayEffect() const { return false; }
};

UCLASS(meta = (DisplayName = "FR Item Effect Heal"))
class FORTROGUE_API UFRItemEffect_Heal : public UFRItemEffect
{
	GENERATED_BODY()

public:
	virtual void ApplyToCharacter(AFRBattleCharacter& Character) const override;
	virtual void AppendEffectSummary(TArray<FString>& Parts) const override;
	virtual void AppendValidationIssues(TArray<FString>& Issues) const override;
	virtual bool HasGameplayEffect() const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heal", meta = (ClampMin = "0.0", ToolTip = "사용 시 즉시 회복할 체력입니다."))
	float HealAmount = 35.0f;
};

UCLASS(meta = (DisplayName = "FR Item Effect Attack Multiplier"))
class FORTROGUE_API UFRItemEffect_AttackMultiplier : public UFRItemEffect
{
	GENERATED_BODY()

public:
	virtual void ApplyToCharacter(AFRBattleCharacter& Character) const override;
	virtual void AppendEffectSummary(TArray<FString>& Parts) const override;
	virtual void AppendValidationIssues(TArray<FString>& Issues) const override;
	virtual bool HasGameplayEffect() const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Multiplier", meta = (ClampMin = "1.0", ToolTip = "다음 샷 피해량에 곱할 배율입니다. 이미 더 큰 배율이 대기 중이면 큰 값이 유지됩니다."))
	float AttackMultiplier = 1.5f;
};
