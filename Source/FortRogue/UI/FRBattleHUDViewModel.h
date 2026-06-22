// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "FRBattleHUDViewModel.generated.h"

UCLASS(BlueprintType)
class FORTROGUE_API UFRBattleHUDViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD")
	const FText& GetTurnText() const { return TurnText; }
	void SetTurnText(const FText& InTurnText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD")
	const FText& GetRunProgressText() const { return RunProgressText; }
	void SetRunProgressText(const FText& InRunProgressText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD")
	const FText& GetStatusText() const { return StatusText; }
	void SetStatusText(const FText& InStatusText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD")
	const FText& GetPlayerHealthText() const { return PlayerHealthText; }
	void SetPlayerHealthText(const FText& InPlayerHealthText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD")
	float GetPlayerHealthPercent() const { return PlayerHealthPercent; }
	void SetPlayerHealthPercent(float InPlayerHealthPercent);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD")
	const FText& GetEnemyHealthText() const { return EnemyHealthText; }
	void SetEnemyHealthText(const FText& InEnemyHealthText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD")
	float GetEnemyHealthPercent() const { return EnemyHealthPercent; }
	void SetEnemyHealthPercent(float InEnemyHealthPercent);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD")
	const FText& GetWindText() const { return WindText; }
	void SetWindText(const FText& InWindText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD")
	const FText& GetAimText() const { return AimText; }
	void SetAimText(const FText& InAimText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD")
	const FText& GetShotPowerText() const { return ShotPowerText; }
	void SetShotPowerText(const FText& InShotPowerText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD")
	float GetShotPowerPercent() const { return ShotPowerPercent; }
	void SetShotPowerPercent(float InShotPowerPercent);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD")
	const FText& GetMoveBudgetText() const { return MoveBudgetText; }
	void SetMoveBudgetText(const FText& InMoveBudgetText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD")
	float GetMoveBudgetPercent() const { return MoveBudgetPercent; }
	void SetMoveBudgetPercent(float InMoveBudgetPercent);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD")
	const FText& GetCurrentWeaponText() const { return CurrentWeaponText; }
	void SetCurrentWeaponText(const FText& InCurrentWeaponText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD")
	const FText& GetShotInfoText() const { return ShotInfoText; }
	void SetShotInfoText(const FText& InShotInfoText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD")
	const FText& GetGrantedModifierText() const { return GrantedModifierText; }
	void SetGrantedModifierText(const FText& InGrantedModifierText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD")
	const FText& GetPendingModifierText() const { return PendingModifierText; }
	void SetPendingModifierText(const FText& InPendingModifierText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|HUD")
	const FText& GetAbilitySetText() const { return AbilitySetText; }
	void SetAbilitySetText(const FText& InAbilitySetText);

private:
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD", meta = (AllowPrivateAccess = "true"))
	FText TurnText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD", meta = (AllowPrivateAccess = "true"))
	FText RunProgressText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD", meta = (AllowPrivateAccess = "true"))
	FText StatusText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD", meta = (AllowPrivateAccess = "true"))
	FText PlayerHealthText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD", meta = (AllowPrivateAccess = "true"))
	float PlayerHealthPercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD", meta = (AllowPrivateAccess = "true"))
	FText EnemyHealthText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD", meta = (AllowPrivateAccess = "true"))
	float EnemyHealthPercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD", meta = (AllowPrivateAccess = "true"))
	FText WindText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD", meta = (AllowPrivateAccess = "true"))
	FText AimText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD", meta = (AllowPrivateAccess = "true"))
	FText ShotPowerText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD", meta = (AllowPrivateAccess = "true"))
	float ShotPowerPercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD", meta = (AllowPrivateAccess = "true"))
	FText MoveBudgetText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD", meta = (AllowPrivateAccess = "true"))
	float MoveBudgetPercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD", meta = (AllowPrivateAccess = "true"))
	FText CurrentWeaponText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD", meta = (AllowPrivateAccess = "true"))
	FText ShotInfoText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD", meta = (AllowPrivateAccess = "true"))
	FText GrantedModifierText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD", meta = (AllowPrivateAccess = "true"))
	FText PendingModifierText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|HUD", meta = (AllowPrivateAccess = "true"))
	FText AbilitySetText;
};
