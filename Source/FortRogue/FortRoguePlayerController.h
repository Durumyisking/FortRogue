// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "Items/FortRogueItemDefinition.h"
#include "FortRoguePlayerController.generated.h"

class UFortRogueBattleHUDWidget;
class UFortRogueRewardScreenWidget;
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

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Items")
	void UsePlayerItemByTag(FGameplayTag ItemTag);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Items")
	void UsePlayerItemByIndex(int32 ItemIndex);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Items")
	bool CanUsePlayerItemByType(EFortRogueItemType ItemType) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Items")
	bool CanUsePlayerItemByTag(FGameplayTag ItemTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Items")
	bool CanUsePlayerItemByIndex(int32 ItemIndex) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Items")
	int32 GetPlayerItemIndexByTag(FGameplayTag ItemTag) const;

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Weapons")
	bool SelectPlayerWeaponByTag(FGameplayTag WeaponTag);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Weapons")
	bool CanSelectPlayerWeapon(int32 WeaponIndex) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Weapons")
	bool CanSelectPlayerWeaponByTag(FGameplayTag WeaponTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Weapons")
	int32 GetPlayerWeaponIndexByTag(FGameplayTag WeaponTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	bool CanFirePlayerWeapon() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Stats")
	bool TryGetPlayerCombatAttributeValueByTag(FGameplayTag AttributeTag, float& OutValue) const;

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Stats")
	bool TryApplyPlayerCombatAttributeDeltaByTag(FGameplayTag AttributeTag, float DeltaValue);

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
