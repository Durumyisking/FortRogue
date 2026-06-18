// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Combat/FortRogueShotSpec.h"
#include "FortRogueGameMode.h"
#include "GameplayTagContainer.h"
#include "Items/FortRogueItemDefinition.h"
#include "UI/FortRogueActivatableWidget.h"
#include "Weapons/FortRogueWeaponDefinition.h"
#include "FortRogueBattleHUDWidget.generated.h"

class UFortRogueAbilitySet;

UCLASS(Abstract, Blueprintable)
class FORTROGUE_API UFortRogueBattleHUDWidget : public UFortRogueActivatableWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FortRogue|UI")
	class AFortRogueGameMode* GetFortRogueGameMode() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	class AFortRogueBattleCharacter* GetPlayerCharacter() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	class AFortRogueBattleCharacter* GetEnemyCharacter() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FText GetRunProgressSummary() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	EFortRogueBattleState GetBattleState() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FText GetStatusText() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FText GetWindSummary() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FText GetPlayerCombatStatsSummary() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	bool TryGetPlayerCombatAttributeValueByTag(FGameplayTag AttributeTag, float& OutValue) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FText GetPlayerShotSummary() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FFortRogueShotSpec GetPlayerCurrentShotSpec() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	bool DoesPlayerShotModifierMeetCurrentShotConditions(const FFortRogueShotModifierSpec& ShotModifier) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FText GetPlayerShotModifierCurrentConditionFailureSummary(const FFortRogueShotModifierSpec& ShotModifier) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	TArray<FFortRogueWeaponSpec> GetPlayerWeaponLoadout() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FFortRogueWeaponSpec GetPlayerCurrentWeaponSpec() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	int32 GetPlayerSelectedWeaponIndex() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	bool CanSelectPlayerWeapon(int32 WeaponIndex) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	bool CanSelectPlayerWeaponByTag(FGameplayTag WeaponTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	int32 GetPlayerWeaponIndexByTag(FGameplayTag WeaponTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	TArray<FFortRogueItemStack> GetPlayerItemLoadout() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	int32 GetPlayerItemCharges(EFortRogueItemType ItemType) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	int32 GetPlayerItemChargesByTag(FGameplayTag ItemTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	bool CanUsePlayerItemByType(EFortRogueItemType ItemType) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	bool CanUsePlayerItemByTag(FGameplayTag ItemTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	bool CanUsePlayerItemByIndex(int32 ItemIndex) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	int32 GetPlayerItemIndexByTag(FGameplayTag ItemTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	TArray<FFortRogueShotModifierSpec> GetPlayerGrantedShotModifiers() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	TArray<FFortRogueShotModifierSpec> GetPlayerPendingShotModifiers() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FText GetPlayerGrantedShotModifiersSummary() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FText GetPlayerPendingShotModifiersSummary() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	int32 GetPlayerGrantedShotModifierCountByTag(FGameplayTag ModifierTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	int32 GetPlayerPendingShotModifierCountByTag(FGameplayTag ModifierTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	bool HasPlayerGrantedShotModifierByTag(FGameplayTag ModifierTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	bool HasPlayerPendingShotModifierByTag(FGameplayTag ModifierTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	TArray<UFortRogueAbilitySet*> GetPlayerGrantedAbilitySets() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FText GetPlayerGrantedAbilitySetsSummary() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	int32 GetPlayerGrantedAbilitySetCount(UFortRogueAbilitySet* AbilitySet) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	int32 GetPlayerGrantedAbilitySetCountByTag(FGameplayTag AbilitySetTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	bool HasPlayerGrantedAbilitySetByTag(FGameplayTag AbilitySetTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	float GetPlayerAimAngle() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	float GetPlayerShotPower() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	float GetPlayerShotChargeAlpha() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	bool IsPlayerChargingShot() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	bool CanFirePlayerWeapon() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	bool CanBeginPlayerShotCharge() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "FortRogue|UI")
	void RefreshBattleHUD();
};
