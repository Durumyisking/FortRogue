// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FRPlayerHUDViewModel.h"

#include "Combat/FRBattleCharacter.h"
#include "Items/FRItemDefinition.h"
#include "Weapons/FRWeaponDefinition.h"

namespace
{
constexpr int32 MaxHUDWeaponSlots = 5;
constexpr int32 MaxHUDItemSlots = 5;

float GetPercent(float Value, float MaxValue)
{
	return MaxValue > KINDA_SMALL_NUMBER ? FMath::Clamp(Value / MaxValue, 0.0f, 1.0f) : 0.0f;
}

FText FormatIntegerPair(float Value, float MaxValue)
{
	return FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), Value, MaxValue));
}
}

void UFRPlayerHUDViewModel::RefreshFromCharacter(AFRBattleCharacter* Character)
{
	ObservedCharacter = Character;
	if (!Character)
	{
		Reset();
		return;
	}

	PlayerName = Character->GetCharacterDisplayName();

	Health = Character->GetHealth();
	MaxHealth = Character->GetMaxHealth();
	HealthPercent = GetPercent(Health, MaxHealth);
	HealthText = FormatIntegerPair(Health, MaxHealth);

	MoveBudget = Character->GetMoveBudget();
	MaxMoveBudget = Character->GetMaxMoveBudget();
	MoveBudgetPercent = GetPercent(MoveBudget, MaxMoveBudget);
	MoveBudgetText = FormatIntegerPair(MoveBudget, MaxMoveBudget);

	ShotPowerPercent = Character->GetShotChargeAlpha();
	ShotPowerText = FText::FromString(FString::Printf(TEXT("%.0f%%"), ShotPowerPercent * 100.0f));

	AimAngle = Character->GetAimAngle();
	AimText = FText::FromString(FString::Printf(TEXT("%.0f deg"), AimAngle));

	const FFRWeaponSpec CurrentWeapon = Character->GetCurrentWeaponSpec();
	CurrentWeaponName = CurrentWeapon.DisplayName;
	CurrentShotSummary = Character->GetCurrentShotSummary();
	bCanFire = Character->CanFireSelectedWeapon();
	SelectedWeaponIndex = Character->GetSelectedWeaponIndex();

	WeaponSlots.Reset(MaxHUDWeaponSlots);
	const TArray<FFRWeaponSpec> WeaponLoadout = Character->GetWeaponLoadoutForBlueprint();
	for (int32 SlotIndex = 0; SlotIndex < MaxHUDWeaponSlots; ++SlotIndex)
	{
		FFRHUDLoadoutSlotViewData SlotData;
		SlotData.SlotIndex = SlotIndex;
		SlotData.InputText = FText::AsNumber(SlotIndex + 1);
		SlotData.bOccupied = WeaponLoadout.IsValidIndex(SlotIndex);
		SlotData.bSelected = SlotIndex == SelectedWeaponIndex;
		SlotData.bEnabled = Character->CanSelectWeapon(SlotIndex);
		SlotData.DisplayName = SlotData.bOccupied ? WeaponLoadout[SlotIndex].DisplayName : FText::GetEmpty();
		WeaponSlots.Add(SlotData);
	}

	ItemSlots.Reset(MaxHUDItemSlots);
	const TArray<FFRItemStack> ItemLoadout = Character->GetItemLoadoutForBlueprint();
	for (int32 SlotIndex = 0; SlotIndex < MaxHUDItemSlots; ++SlotIndex)
	{
		FFRHUDLoadoutSlotViewData SlotData;
		SlotData.SlotIndex = SlotIndex;
		SlotData.InputText = FText::AsNumber(SlotIndex + 1);
		SlotData.bOccupied = ItemLoadout.IsValidIndex(SlotIndex) && ItemLoadout[SlotIndex].ItemDefinition;
		SlotData.bEnabled = Character->CanUseItemByIndex(SlotIndex);
		if (SlotData.bOccupied)
		{
			const FFRItemStack& ItemStack = ItemLoadout[SlotIndex];
			SlotData.DisplayName = ItemStack.ItemDefinition->DisplayName;
			SlotData.Charges = ItemStack.Charges;
		}
		ItemSlots.Add(SlotData);
	}
}

AFRBattleCharacter* UFRPlayerHUDViewModel::GetObservedCharacter() const
{
	return ObservedCharacter.Get();
}

FFRHUDLoadoutSlotViewData UFRPlayerHUDViewModel::GetWeaponSlot(int32 SlotIndex) const
{
	return WeaponSlots.IsValidIndex(SlotIndex) ? WeaponSlots[SlotIndex] : FFRHUDLoadoutSlotViewData();
}

FFRHUDLoadoutSlotViewData UFRPlayerHUDViewModel::GetItemSlot(int32 SlotIndex) const
{
	return ItemSlots.IsValidIndex(SlotIndex) ? ItemSlots[SlotIndex] : FFRHUDLoadoutSlotViewData();
}

void UFRPlayerHUDViewModel::Reset()
{
	PlayerName = FText::GetEmpty();
	Health = 0.0f;
	MaxHealth = 0.0f;
	HealthPercent = 0.0f;
	HealthText = FText::GetEmpty();
	MoveBudget = 0.0f;
	MaxMoveBudget = 0.0f;
	MoveBudgetPercent = 0.0f;
	MoveBudgetText = FText::GetEmpty();
	ShotPowerPercent = 0.0f;
	ShotPowerText = FText::GetEmpty();
	AimAngle = 0.0f;
	AimText = FText::GetEmpty();
	CurrentWeaponName = FText::GetEmpty();
	CurrentShotSummary = FText::GetEmpty();
	bCanFire = false;
	SelectedWeaponIndex = INDEX_NONE;
	WeaponSlots.Reset();
	ItemSlots.Reset();
}
