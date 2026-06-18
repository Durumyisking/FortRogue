// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Combat/FortRogueShotSpec.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "Items/FortRogueItemDefinition.h"
#include "Rewards/FortRogueRewardTypes.h"
#include "FortRoguePlayerController.generated.h"

class UFortRogueBattleHUDWidget;
class UFortRogueRewardScreenWidget;
class UFortRogueAbilitySet;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

UCLASS()
class FORTROGUE_API AFortRoguePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AFortRoguePlayerController();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	TArray<FFortRogueRewardChoice> GetCurrentRewardChoices() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	int32 GetCurrentRewardChoiceCount() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	FFortRogueRewardChoice GetCurrentRewardChoice(int32 ChoiceIndex) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	FText GetCurrentRewardChoiceSummary(int32 ChoiceIndex) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	FText GetCurrentRewardChoiceConditionFailureSummary(int32 ChoiceIndex) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	FGameplayTagContainer GetChosenRewardTags() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Rewards")
	bool CanChooseReward(int32 ChoiceIndex) const;

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Rewards")
	bool ChooseRewardByIndex(int32 ChoiceIndex);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Items")
	void UsePlayerItemByTag(FGameplayTag ItemTag);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Items")
	void UsePlayerItemByIndex(int32 ItemIndex);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Items")
	TArray<FFortRogueItemStack> GetPlayerItemLoadout() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Items")
	int32 GetPlayerItemCharges(EFortRogueItemType ItemType) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Items")
	int32 GetPlayerItemChargesByTag(FGameplayTag ItemTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Items")
	bool CanUsePlayerItemByType(EFortRogueItemType ItemType) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Items")
	bool CanUsePlayerItemByTag(FGameplayTag ItemTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Items")
	bool CanUsePlayerItemByIndex(int32 ItemIndex) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Items")
	int32 GetPlayerItemIndexByTag(FGameplayTag ItemTag) const;

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Weapons")
	bool SelectPlayerWeaponByIndex(int32 WeaponIndex);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Weapons")
	bool SelectPlayerWeaponByTag(FGameplayTag WeaponTag);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Weapons")
	bool CanSelectPlayerWeapon(int32 WeaponIndex) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Weapons")
	bool CanSelectPlayerWeaponByTag(FGameplayTag WeaponTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Weapons")
	int32 GetPlayerWeaponIndexByTag(FGameplayTag WeaponTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Weapons")
	TArray<FFortRogueWeaponSpec> GetPlayerWeaponLoadout() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Weapons")
	FFortRogueWeaponSpec GetPlayerCurrentWeaponSpec() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Weapons")
	int32 GetPlayerSelectedWeaponIndex() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	bool CanFirePlayerWeapon() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	float GetPlayerAimAngle() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	float GetPlayerShotPower() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	float GetPlayerShotChargeAlpha() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	bool IsPlayerChargingShot() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	FFortRogueShotSpec GetPlayerCurrentShotSpec() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	FText GetPlayerCurrentShotSummary() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	bool DoesPlayerShotModifierMeetCurrentShotConditions(const FFortRogueShotModifierSpec& ShotModifier) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	FText GetPlayerShotModifierCurrentConditionFailureSummary(const FFortRogueShotModifierSpec& ShotModifier) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Stats")
	bool TryGetPlayerCombatAttributeValueByTag(FGameplayTag AttributeTag, float& OutValue) const;

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Stats")
	bool TryApplyPlayerCombatAttributeDeltaByTag(FGameplayTag AttributeTag, float DeltaValue);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Abilities")
	void GrantPlayerAbilitySet(UFortRogueAbilitySet* AbilitySet);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Abilities")
	bool RemovePlayerAbilitySet(UFortRogueAbilitySet* AbilitySet);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Abilities")
	int32 RemovePlayerAbilitySetsByTag(FGameplayTag AbilitySetTag);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Abilities")
	int32 GetPlayerGrantedAbilitySetCount(UFortRogueAbilitySet* AbilitySet) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Abilities")
	int32 GetPlayerGrantedAbilitySetCountByTag(FGameplayTag AbilitySetTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Abilities")
	bool HasPlayerGrantedAbilitySetByTag(FGameplayTag AbilitySetTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Abilities")
	TArray<UFortRogueAbilitySet*> GetPlayerGrantedAbilitySets() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Abilities")
	FText GetPlayerGrantedAbilitySetsSummary() const;

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Combat")
	void GrantPlayerShotModifiers(const TArray<FFortRogueShotModifierSpec>& ShotModifiers);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Combat")
	void GrantPlayerPendingShotModifiers(const TArray<FFortRogueShotModifierSpec>& ShotModifiers);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	int32 GetPlayerGrantedShotModifierCountByTag(FGameplayTag ModifierTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	int32 GetPlayerPendingShotModifierCountByTag(FGameplayTag ModifierTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	bool HasPlayerGrantedShotModifierByTag(FGameplayTag ModifierTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	bool HasPlayerPendingShotModifierByTag(FGameplayTag ModifierTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	TArray<FFortRogueShotModifierSpec> GetPlayerGrantedShotModifiers() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	TArray<FFortRogueShotModifierSpec> GetPlayerPendingShotModifiers() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	FText GetPlayerGrantedShotModifiersSummary() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	FText GetPlayerPendingShotModifiersSummary() const;

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Combat")
	int32 RemovePlayerGrantedShotModifiersByTag(FGameplayTag ModifierTag);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Combat")
	int32 RemovePlayerPendingShotModifiersByTag(FGameplayTag ModifierTag);

private:
	void TickBattleInput(float DeltaSeconds);
	void TickKeyboardFireInput();
	void TickKeyboardWeaponInput();
	void TickKeyboardItemInput();
	void TickRewardInput();
	void UpdateOptionalWidgets();
	bool HasEnhancedInputBindings() const;

	void HandleMove(const FInputActionValue& Value);
	void HandleAim(const FInputActionValue& Value);
	void HandleFirePressed();
	void HandleFireReleased();
	void HandleWeapon1();
	void HandleWeapon2();
	void HandleWeapon3();
	void HandleWeapon4();
	void HandleWeapon5();
	void HandleAttackItem();
	void HandleHealItem();
	void HandleReward1();
	void HandleReward2();
	void HandleReward3();
	void HandleReward4();
	void HandleReward5();
	void ApplyMoveAxis(float Axis, float DeltaSeconds);
	void ApplyAimAxis(float Axis, float DeltaSeconds);
	void BeginPlayerWeaponCharge();
	void TickPlayerWeaponCharge(float DeltaSeconds);
	void ReleasePlayerWeaponCharge();
	void SelectPlayerWeapon(int32 WeaponIndex);
	void UsePlayerItem(EFortRogueItemType ItemType);
	void ChooseReward(int32 ChoiceIndex);

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input")
	TObjectPtr<UInputMappingContext> BattleInputMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input")
	int32 BattleInputMappingPriority = 0;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input")
	TObjectPtr<UInputAction> AimAction;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input")
	TObjectPtr<UInputAction> FireAction;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input")
	TObjectPtr<UInputAction> Weapon1Action;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input")
	TObjectPtr<UInputAction> Weapon2Action;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input")
	TObjectPtr<UInputAction> Weapon3Action;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input")
	TObjectPtr<UInputAction> Weapon4Action;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input")
	TObjectPtr<UInputAction> Weapon5Action;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input")
	TObjectPtr<UInputAction> AttackItemAction;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input")
	TObjectPtr<UInputAction> HealItemAction;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input")
	TObjectPtr<UInputAction> Reward1Action;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input")
	TObjectPtr<UInputAction> Reward2Action;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input")
	TObjectPtr<UInputAction> Reward3Action;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input")
	TObjectPtr<UInputAction> Reward4Action;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input")
	TObjectPtr<UInputAction> Reward5Action;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI")
	TSubclassOf<UFortRogueBattleHUDWidget> BattleHUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI")
	TSubclassOf<UFortRogueRewardScreenWidget> RewardScreenWidgetClass;

	UPROPERTY()
	TObjectPtr<UFortRogueBattleHUDWidget> BattleHUDWidget;

	UPROPERTY()
	TObjectPtr<UFortRogueRewardScreenWidget> RewardScreenWidget;

	float EnhancedMoveAxis = 0.0f;
	float EnhancedAimAxis = 0.0f;
};
