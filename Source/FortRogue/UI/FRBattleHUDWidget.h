// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Combat/FRShotSpec.h"
#include "FRGameMode.h"
#include "GameplayTagContainer.h"
#include "Items/FRItemDefinition.h"
#include "CommonActivatableWidget.h"
#include "Weapons/FRWeaponDefinition.h"
#include "FRBattleHUDWidget.generated.h"

class UFRAbilitySet;
class UBorder;
class UHorizontalBox;
class UOverlay;
class UProgressBar;
class UTextBlock;
class UUserWidget;
class UVerticalBox;
class UWidget;
class UMVVMViewModelBase;
class UFRAimWindViewModel;
class UFRBattleHUDViewModel;
class UFRBattleStateViewModel;
class UFRCombatantStatusViewModel;
class UFRLoadoutViewModel;
class UFRModifierSummaryViewModel;
class UFRShotPowerViewModel;
class UFRShotPreviewViewModel;

UCLASS(Blueprintable)
class FORTROGUE_API UFRBattleHUDWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FortRogue|UI")
	class AFRGameMode* GetFortRogueGameMode() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	class AFRBattleCharacter* GetPlayerCharacter() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	class AFRBattleCharacter* GetEnemyCharacter() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FText GetRunProgressSummary() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	EFRBattleState GetBattleState() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FText GetStatusText() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FText GetWindSummary() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FText GetPlayerCombatStatsSummary() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	bool TryGetPlayerCombatAttributeValueByTag(UPARAM(meta = (Categories = "Attribute")) FGameplayTag AttributeTag, float& OutValue) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FText GetPlayerShotSummary() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FFRShotSpec GetPlayerCurrentShotSpec() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	bool DoesPlayerShotModifierMeetCurrentShotConditions(const FFRShotModifierSpec& ShotModifier) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FText GetPlayerShotModifierCurrentConditionFailureSummary(const FFRShotModifierSpec& ShotModifier) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	TArray<FFRWeaponSpec> GetPlayerWeaponLoadout() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FFRWeaponSpec GetPlayerCurrentWeaponSpec() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	int32 GetPlayerSelectedWeaponIndex() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	bool CanSelectPlayerWeapon(int32 WeaponIndex) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	bool CanSelectPlayerWeaponByTag(UPARAM(meta = (Categories = "Weapon")) FGameplayTag WeaponTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	int32 GetPlayerWeaponIndexByTag(UPARAM(meta = (Categories = "Weapon")) FGameplayTag WeaponTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	TArray<FFRItemStack> GetPlayerItemLoadout() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	int32 GetPlayerItemCharges(EFRItemType ItemType) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	int32 GetPlayerItemChargesByTag(UPARAM(meta = (Categories = "Item")) FGameplayTag ItemTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	bool CanUsePlayerItemByType(EFRItemType ItemType) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	bool CanUsePlayerItemByTag(UPARAM(meta = (Categories = "Item")) FGameplayTag ItemTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	bool CanUsePlayerItemByIndex(int32 ItemIndex) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	int32 GetPlayerItemIndexByTag(UPARAM(meta = (Categories = "Item")) FGameplayTag ItemTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	TArray<FFRShotModifierSpec> GetPlayerGrantedShotModifiers() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	TArray<FFRShotModifierSpec> GetPlayerPendingShotModifiers() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FText GetPlayerGrantedShotModifiersSummary() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FText GetPlayerPendingShotModifiersSummary() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	int32 GetPlayerGrantedShotModifierCountByTag(UPARAM(meta = (Categories = "ShotEffect")) FGameplayTag ModifierTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	int32 GetPlayerPendingShotModifierCountByTag(UPARAM(meta = (Categories = "ShotEffect")) FGameplayTag ModifierTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	bool HasPlayerGrantedShotModifierByTag(UPARAM(meta = (Categories = "ShotEffect")) FGameplayTag ModifierTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	bool HasPlayerPendingShotModifierByTag(UPARAM(meta = (Categories = "ShotEffect")) FGameplayTag ModifierTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	TArray<UFRAbilitySet*> GetPlayerGrantedAbilitySets() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FText GetPlayerGrantedAbilitySetsSummary() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	int32 GetPlayerGrantedAbilitySetCount(UFRAbilitySet* AbilitySet) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	int32 GetPlayerGrantedAbilitySetCountByTag(UPARAM(meta = (Categories = "Trait")) FGameplayTag AbilitySetTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	bool HasPlayerGrantedAbilitySetByTag(UPARAM(meta = (Categories = "Trait")) FGameplayTag AbilitySetTag) const;

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

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|MVVM")
	UFRBattleHUDViewModel* GetBattleHUDViewModel() const { return BattleHUDViewModel; }

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|MVVM")
	UFRBattleStateViewModel* GetBattleStateViewModel() const { return BattleStateViewModel; }

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|MVVM")
	UFRCombatantStatusViewModel* GetPlayerStatusViewModel() const { return PlayerStatusViewModel; }

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|MVVM")
	UFRAimWindViewModel* GetAimWindViewModel() const { return AimWindViewModel; }

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|MVVM")
	UFRShotPowerViewModel* GetShotPowerViewModel() const { return ShotPowerViewModel; }

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|MVVM")
	UFRLoadoutViewModel* GetLoadoutViewModel() const { return LoadoutViewModel; }

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|MVVM")
	UFRShotPreviewViewModel* GetShotPreviewViewModel() const { return ShotPreviewViewModel; }

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|MVVM")
	UFRModifierSummaryViewModel* GetModifierSummaryViewModel() const { return ModifierSummaryViewModel; }

	UFUNCTION(BlueprintNativeEvent, Category = "FortRogue|UI")
	void RefreshBattleHUD();

protected:
	virtual void NativeOnInitialized() override;

private:
	void BuildDefaultHUD();
	void CreateViewModels();
	void ApplyViewModel(UUserWidget* Widget, UMVVMViewModelBase* ViewModel) const;
	void ApplyViewModelToChild(FName WidgetName, UMVVMViewModelBase* ViewModel) const;
	void ApplyBattleHUDViewModel(UUserWidget* Widget) const;
	void RefreshViewModel();
	void RefreshDefaultHUD();
	void RefreshCharacterBars(class AFRBattleCharacter* PlayerCharacter, class AFRBattleCharacter* EnemyCharacter);
	void RefreshWeaponSlots(class AFRBattleCharacter* PlayerCharacter);
	void RefreshItemSlots(class AFRBattleCharacter* PlayerCharacter);
	void RefreshModifierSummaries(class AFRBattleCharacter* PlayerCharacter);
	void RefreshAimIndicator(float AimAngle);

	UTextBlock* AddText(UWidget* Parent, FName WidgetName, const FText& InitialText, float FontSize, const FLinearColor& Color);
	UProgressBar* AddLabeledBar(UVerticalBox* Parent, FName WidgetName, const FText& LabelText, UTextBlock*& OutValueText);
	UBorder* AddSlot(UHorizontalBox* Parent, FName WidgetName, const FVector2D& Size);

	UPROPERTY(Transient)
	TObjectPtr<UFRBattleHUDViewModel> BattleHUDViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UFRBattleStateViewModel> BattleStateViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UFRCombatantStatusViewModel> PlayerStatusViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UFRAimWindViewModel> AimWindViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UFRShotPowerViewModel> ShotPowerViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UFRLoadoutViewModel> LoadoutViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UFRShotPreviewViewModel> ShotPreviewViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UFRModifierSummaryViewModel> ModifierSummaryViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> RunProgressText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TurnText;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> TurnBadge;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> PlayerHealthBar;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PlayerHealthText;

	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> EnemyHealthBar;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> EnemyHealthText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> WindText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> AimText;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> AimBarrel;

	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> ShotPowerBar;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ShotPowerText;

	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> MoveBudgetBar;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> MoveBudgetText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CurrentWeaponText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> AttackTypeText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ShotInfoText;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UBorder>> WeaponSlots;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> WeaponSlotTexts;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UBorder>> ItemSlots;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> ItemSlotTexts;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> GrantedModifierText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PendingModifierText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> AbilitySetText;
};
