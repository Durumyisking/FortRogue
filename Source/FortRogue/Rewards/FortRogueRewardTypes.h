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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	EFortRogueRewardType Type = EFortRogueRewardType::Trait;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	FGameplayTag RewardTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward", meta = (ClampMin = "0.0"))
	float RewardWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	bool bOfferOncePerRun = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward|Condition")
	FGameplayTagContainer RequiredRewardTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward|Condition")
	FGameplayTagContainer BlockedRewardTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	TObjectPtr<UFortRogueWeaponDefinition> WeaponReward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	TObjectPtr<UFortRogueItemDefinition> ItemReward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	TObjectPtr<UFortRoguePerkDefinition> PerkReward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	TObjectPtr<UFortRogueAbilitySet> GrantedAbilitySet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Modifier", meta = (TitleProperty = EffectTags))
	TArray<FFortRogueShotModifierSpec> ShotModifiers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	float DamageBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	float MaxHealthBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	float MaxMoveBudgetBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 ProjectileBonus = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	float ShotPowerMultiplierBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 RepairCharges = 0;

	FText GetEffectSummary() const;
	bool MeetsRewardTagConditions(const FGameplayTagContainer& ChosenRewardTags) const;
};
