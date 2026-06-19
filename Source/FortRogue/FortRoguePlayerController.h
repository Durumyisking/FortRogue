// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Combat/FortRogueShotSpec.h"
#include "FortRogueGameMode.h"
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

	UFUNCTION(BlueprintPure, Category = "FortRogue|Battle")
	EFortRogueBattleState GetCurrentBattleState() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Battle")
	FText GetCurrentStatusText() const;

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
	bool CanBeginPlayerShotCharge() const;

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

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "전투 중 사용할 Enhanced Input Mapping Context입니다. 비워두면 키보드 fallback 입력만 동작합니다."))
	TObjectPtr<UInputMappingContext> BattleInputMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "BattleInputMappingContext를 LocalPlayer Subsystem에 추가할 때 사용하는 우선순위입니다. 값이 높을수록 먼저 처리됩니다."))
	int32 BattleInputMappingPriority = 0;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "좌우 이동 입력에 연결할 Input Action입니다. Axis 값으로 캐릭터 이동을 처리합니다."))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "조준 각도 조절 입력에 연결할 Input Action입니다. Axis 값으로 포신 각도를 올리거나 내립니다."))
	TObjectPtr<UInputAction> AimAction;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "샷 차지 시작/발사 입력에 연결할 Input Action입니다. 누르고 있는 동안 파워를 모으고, 떼면 발사합니다."))
	TObjectPtr<UInputAction> FireAction;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "로드아웃의 1번 무기를 선택할 Input Action입니다."))
	TObjectPtr<UInputAction> Weapon1Action;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "로드아웃의 2번 무기를 선택할 Input Action입니다."))
	TObjectPtr<UInputAction> Weapon2Action;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "로드아웃의 3번 무기를 선택할 Input Action입니다."))
	TObjectPtr<UInputAction> Weapon3Action;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "로드아웃의 4번 무기를 선택할 Input Action입니다."))
	TObjectPtr<UInputAction> Weapon4Action;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "로드아웃의 5번 무기를 선택할 Input Action입니다."))
	TObjectPtr<UInputAction> Weapon5Action;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "공격 강화 아이템을 사용하는 Input Action입니다. 사용 가능 횟수가 있을 때 다음 샷 modifier를 부여합니다."))
	TObjectPtr<UInputAction> AttackItemAction;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "회복 아이템을 사용하는 Input Action입니다. 사용 가능 횟수가 있을 때 플레이어 체력을 회복합니다."))
	TObjectPtr<UInputAction> HealItemAction;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "보상 화면에서 1번 선택지를 고르는 Input Action입니다."))
	TObjectPtr<UInputAction> Reward1Action;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "보상 화면에서 2번 선택지를 고르는 Input Action입니다."))
	TObjectPtr<UInputAction> Reward2Action;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "보상 화면에서 3번 선택지를 고르는 Input Action입니다."))
	TObjectPtr<UInputAction> Reward3Action;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "보상 화면에서 4번 선택지를 고르는 Input Action입니다."))
	TObjectPtr<UInputAction> Reward4Action;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "보상 화면에서 5번 선택지를 고르는 Input Action입니다."))
	TObjectPtr<UInputAction> Reward5Action;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI", meta = (ToolTip = "전투 중 표시할 HUD 위젯 클래스입니다. 체력, 무기, 조준, 파워, 바람 정보를 보여줍니다."))
	TSubclassOf<UFortRogueBattleHUDWidget> BattleHUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI", meta = (ToolTip = "스테이지 클리어 후 보상 선택에 사용할 위젯 클래스입니다."))
	TSubclassOf<UFortRogueRewardScreenWidget> RewardScreenWidgetClass;

	UPROPERTY()
	TObjectPtr<UFortRogueBattleHUDWidget> BattleHUDWidget;

	UPROPERTY()
	TObjectPtr<UFortRogueRewardScreenWidget> RewardScreenWidget;

	float EnhancedMoveAxis = 0.0f;
	float EnhancedAimAxis = 0.0f;
};
