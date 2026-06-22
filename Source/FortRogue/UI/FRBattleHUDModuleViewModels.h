// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "FRBattleHUDModuleViewModels.generated.h"

UCLASS(BlueprintType)
class FORTROGUE_API UFRBattleStateViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Battle State")
	const FText& GetTurnText() const { return TurnText; }
	void SetTurnText(const FText& InTurnText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Battle State")
	const FText& GetRunProgressText() const { return RunProgressText; }
	void SetRunProgressText(const FText& InRunProgressText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Battle State")
	const FText& GetStatusText() const { return StatusText; }
	void SetStatusText(const FText& InStatusText);

private:
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Battle State", meta = (AllowPrivateAccess = "true"))
	FText TurnText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Battle State", meta = (AllowPrivateAccess = "true"))
	FText RunProgressText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Battle State", meta = (AllowPrivateAccess = "true"))
	FText StatusText;
};

UCLASS(BlueprintType)
class FORTROGUE_API UFRCombatantStatusViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Combatant")
	const FText& GetTitleText() const { return TitleText; }
	void SetTitleText(const FText& InTitleText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Combatant")
	const FText& GetHealthText() const { return HealthText; }
	void SetHealthText(const FText& InHealthText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Combatant")
	float GetCurrentHealthValue() const { return CurrentHealthValue; }
	void SetCurrentHealthValue(float InCurrentHealthValue);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Combatant")
	float GetMaxHealthValue() const { return MaxHealthValue; }
	void SetMaxHealthValue(float InMaxHealthValue);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Combatant")
	float GetHealthPercent() const { return HealthPercent; }
	void SetHealthPercent(float InHealthPercent);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Combatant")
	const FText& GetMoveBudgetText() const { return MoveBudgetText; }
	void SetMoveBudgetText(const FText& InMoveBudgetText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Combatant")
	float GetMoveBudgetValue() const { return MoveBudgetValue; }
	void SetMoveBudgetValue(float InMoveBudgetValue);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Combatant")
	float GetMaxMoveBudgetValue() const { return MaxMoveBudgetValue; }
	void SetMaxMoveBudgetValue(float InMaxMoveBudgetValue);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Combatant")
	float GetMoveBudgetPercent() const { return MoveBudgetPercent; }
	void SetMoveBudgetPercent(float InMoveBudgetPercent);

private:
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Combatant", meta = (AllowPrivateAccess = "true"))
	FText TitleText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Combatant", meta = (AllowPrivateAccess = "true"))
	FText HealthText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Combatant", meta = (AllowPrivateAccess = "true"))
	float CurrentHealthValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Combatant", meta = (AllowPrivateAccess = "true"))
	float MaxHealthValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Combatant", meta = (AllowPrivateAccess = "true"))
	float HealthPercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Combatant", meta = (AllowPrivateAccess = "true"))
	FText MoveBudgetText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Combatant", meta = (AllowPrivateAccess = "true"))
	float MoveBudgetValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Combatant", meta = (AllowPrivateAccess = "true"))
	float MaxMoveBudgetValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Combatant", meta = (AllowPrivateAccess = "true"))
	float MoveBudgetPercent = 0.0f;
};

UCLASS(BlueprintType)
class FORTROGUE_API UFRAimWindViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Aim")
	const FText& GetWindText() const { return WindText; }
	void SetWindText(const FText& InWindText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Aim")
	float GetWindValue() const { return WindValue; }
	void SetWindValue(float InWindValue);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Aim")
	float GetAbsoluteWindValue() const { return AbsoluteWindValue; }
	void SetAbsoluteWindValue(float InAbsoluteWindValue);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Aim")
	const FText& GetAimText() const { return AimText; }
	void SetAimText(const FText& InAimText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Aim")
	float GetAimAngleValue() const { return AimAngleValue; }
	void SetAimAngleValue(float InAimAngleValue);

private:
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Aim", meta = (AllowPrivateAccess = "true"))
	FText WindText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Aim", meta = (AllowPrivateAccess = "true"))
	float WindValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Aim", meta = (AllowPrivateAccess = "true"))
	float AbsoluteWindValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Aim", meta = (AllowPrivateAccess = "true"))
	FText AimText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Aim", meta = (AllowPrivateAccess = "true"))
	float AimAngleValue = 0.0f;
};

UCLASS(BlueprintType)
class FORTROGUE_API UFRShotPowerViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Shot Power")
	const FText& GetShotPowerText() const { return ShotPowerText; }
	void SetShotPowerText(const FText& InShotPowerText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Shot Power")
	float GetShotPowerPercent() const { return ShotPowerPercent; }
	void SetShotPowerPercent(float InShotPowerPercent);

private:
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Shot Power", meta = (AllowPrivateAccess = "true"))
	FText ShotPowerText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Shot Power", meta = (AllowPrivateAccess = "true"))
	float ShotPowerPercent = 0.0f;
};

UCLASS(BlueprintType)
class FORTROGUE_API UFRLoadoutSlotViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Loadout Slot")
	const FText& GetSlotLabelText() const { return SlotLabelText; }
	void SetSlotLabelText(const FText& InSlotLabelText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Loadout Slot")
	const FText& GetDisplayText() const { return DisplayText; }
	void SetDisplayText(const FText& InDisplayText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Loadout Slot")
	float GetCountValue() const { return CountValue; }
	void SetCountValue(float InCountValue);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Loadout Slot")
	bool ShouldShowCount() const { return bShowCount; }
	void SetShowCount(bool bInShowCount);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Loadout Slot")
	bool IsOccupied() const { return bOccupied; }
	void SetOccupied(bool bInOccupied);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Loadout Slot")
	bool IsSelected() const { return bSelected; }
	void SetSelected(bool bInSelected);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Loadout Slot")
	bool IsEnabled() const { return bEnabled; }
	void SetEnabled(bool bInEnabled);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Loadout Slot")
	const FText& GetStatusText() const { return StatusText; }
	void SetStatusText(const FText& InStatusText);

private:
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Loadout Slot", meta = (AllowPrivateAccess = "true"))
	FText SlotLabelText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Loadout Slot", meta = (AllowPrivateAccess = "true"))
	FText DisplayText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Loadout Slot", meta = (AllowPrivateAccess = "true"))
	float CountValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Loadout Slot", meta = (AllowPrivateAccess = "true"))
	bool bShowCount = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Loadout Slot", meta = (AllowPrivateAccess = "true"))
	bool bOccupied = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Loadout Slot", meta = (AllowPrivateAccess = "true"))
	bool bSelected = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Loadout Slot", meta = (AllowPrivateAccess = "true"))
	bool bEnabled = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Loadout Slot", meta = (AllowPrivateAccess = "true"))
	FText StatusText;
};

UCLASS(BlueprintType)
class FORTROGUE_API UFRLoadoutViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Loadout")
	const FText& GetCurrentWeaponText() const { return CurrentWeaponText; }
	void SetCurrentWeaponText(const FText& InCurrentWeaponText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Loadout")
	UFRLoadoutSlotViewModel* GetWeaponSlotViewModel(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Loadout")
	UFRLoadoutSlotViewModel* GetItemSlotViewModel(int32 SlotIndex) const;

	UFRLoadoutSlotViewModel* GetOrCreateWeaponSlotViewModel(int32 SlotIndex);
	UFRLoadoutSlotViewModel* GetOrCreateItemSlotViewModel(int32 SlotIndex);
	void SetWeaponSlotCount(int32 SlotCount);
	void SetItemSlotCount(int32 SlotCount);

private:
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Loadout", meta = (AllowPrivateAccess = "true"))
	FText CurrentWeaponText;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|UI|HUD|Loadout", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UFRLoadoutSlotViewModel>> WeaponSlotViewModels;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|UI|HUD|Loadout", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UFRLoadoutSlotViewModel>> ItemSlotViewModels;
};

UCLASS(BlueprintType)
class FORTROGUE_API UFRShotPreviewViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Shot Preview")
	const FText& GetPrimaryText() const { return PrimaryText; }
	void SetPrimaryText(const FText& InPrimaryText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Shot Preview")
	const FText& GetSecondaryText() const { return SecondaryText; }
	void SetSecondaryText(const FText& InSecondaryText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Shot Preview")
	float GetDamageValue() const { return DamageValue; }
	void SetDamageValue(float InDamageValue);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Shot Preview")
	float GetBlastRadiusValue() const { return BlastRadiusValue; }
	void SetBlastRadiusValue(float InBlastRadiusValue);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Shot Preview")
	int32 GetProjectileCountValue() const { return ProjectileCountValue; }
	void SetProjectileCountValue(int32 InProjectileCountValue);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Shot Preview")
	float GetLaunchSpeedValue() const { return LaunchSpeedValue; }
	void SetLaunchSpeedValue(float InLaunchSpeedValue);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Shot Preview")
	float GetGravityValue() const { return GravityValue; }
	void SetGravityValue(float InGravityValue);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Shot Preview")
	float GetTerrainDamageValue() const { return TerrainDamageValue; }
	void SetTerrainDamageValue(float InTerrainDamageValue);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Shot Preview")
	float GetTerrainFillRadiusValue() const { return TerrainFillRadiusValue; }
	void SetTerrainFillRadiusValue(float InTerrainFillRadiusValue);

private:
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Shot Preview", meta = (AllowPrivateAccess = "true"))
	FText PrimaryText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Shot Preview", meta = (AllowPrivateAccess = "true"))
	FText SecondaryText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Shot Preview", meta = (AllowPrivateAccess = "true"))
	float DamageValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Shot Preview", meta = (AllowPrivateAccess = "true"))
	float BlastRadiusValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Shot Preview", meta = (AllowPrivateAccess = "true"))
	int32 ProjectileCountValue = 0;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Shot Preview", meta = (AllowPrivateAccess = "true"))
	float LaunchSpeedValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Shot Preview", meta = (AllowPrivateAccess = "true"))
	float GravityValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Shot Preview", meta = (AllowPrivateAccess = "true"))
	float TerrainDamageValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Shot Preview", meta = (AllowPrivateAccess = "true"))
	float TerrainFillRadiusValue = 0.0f;
};

UCLASS(BlueprintType)
class FORTROGUE_API UFRModifierSummaryViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Modifiers")
	const FText& GetGrantedModifierText() const { return GrantedModifierText; }
	void SetGrantedModifierText(const FText& InGrantedModifierText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Modifiers")
	const FText& GetPendingModifierText() const { return PendingModifierText; }
	void SetPendingModifierText(const FText& InPendingModifierText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Modifiers")
	const FText& GetAbilitySetText() const { return AbilitySetText; }
	void SetAbilitySetText(const FText& InAbilitySetText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD|Modifiers")
	const FText& GetSummaryText() const { return SummaryText; }
	void SetSummaryText(const FText& InSummaryText);

private:
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Modifiers", meta = (AllowPrivateAccess = "true"))
	FText GrantedModifierText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Modifiers", meta = (AllowPrivateAccess = "true"))
	FText PendingModifierText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Modifiers", meta = (AllowPrivateAccess = "true"))
	FText AbilitySetText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD|Modifiers", meta = (AllowPrivateAccess = "true"))
	FText SummaryText;
};
