// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "FortRogueRewardTypes.generated.h"

class UFortRogueItemDefinition;
class UFortRoguePerkDefinition;
class UFortRogueWeaponDefinition;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	TObjectPtr<UFortRogueWeaponDefinition> WeaponReward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	TObjectPtr<UFortRogueItemDefinition> ItemReward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	TObjectPtr<UFortRoguePerkDefinition> PerkReward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	float DamageBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	float MaxHealthBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 ProjectileBonus = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 RepairCharges = 0;
};
