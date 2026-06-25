// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FRPlayerHUDViewModel.generated.h"

class AFRBattleCharacter;

USTRUCT(BlueprintType)
struct FFRHUDLoadoutSlotViewData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|HUD")
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|HUD")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|HUD")
	FText InputText;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|HUD")
	bool bOccupied = false;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|HUD")
	bool bSelected = false;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|HUD")
	bool bEnabled = false;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|HUD")
	int32 Charges = 0;
};

UCLASS(BlueprintType)
class FORTROGUE_API UFRPlayerHUDViewModel : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FortRogue|HUD")
	void RefreshFromCharacter(AFRBattleCharacter* Character);

	UFUNCTION(BlueprintPure, Category = "FortRogue|HUD")
	AFRBattleCharacter* GetObservedCharacter() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|HUD")
	FFRHUDLoadoutSlotViewData GetWeaponSlot(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|HUD")
	FFRHUDLoadoutSlotViewData GetItemSlot(int32 SlotIndex) const;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|HUD")
	FText PlayerName;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|HUD")
	float Health = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|HUD")
	float MaxHealth = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|HUD")
	float HealthPercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|HUD")
	FText HealthText;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|HUD")
	float MoveBudget = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|HUD")
	float MaxMoveBudget = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|HUD")
	float MoveBudgetPercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|HUD")
	FText MoveBudgetText;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|HUD")
	float ShotPowerPercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|HUD")
	FText ShotPowerText;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|HUD")
	float AimAngle = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|HUD")
	FText AimText;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|HUD")
	FText CurrentWeaponName;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|HUD")
	FText CurrentShotSummary;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|HUD")
	bool bCanFire = false;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|HUD")
	int32 SelectedWeaponIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|HUD")
	TArray<FFRHUDLoadoutSlotViewData> WeaponSlots;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|HUD")
	TArray<FFRHUDLoadoutSlotViewData> ItemSlots;

private:
	void Reset();

	UPROPERTY()
	TWeakObjectPtr<AFRBattleCharacter> ObservedCharacter;
};
