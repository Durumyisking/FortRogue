// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Items/FRItemDefinition.h"
#include "FRPlayerController.generated.h"

class UFRAbilitySystemComponent;
class UInputAction;
class UInputMappingContext;
struct FGameplayTag;
struct FInputActionValue;

UCLASS()
class FORTROGUE_API AFRPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AFRPlayerController();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	void TickBattleInput(float DeltaSeconds);
	void TickKeyboardFireInput();
	void TickKeyboardWeaponInput();
	void TickKeyboardItemInput();
	void TickRewardInput();
	UFRAbilitySystemComponent* GetPlayerAbilitySystemComponent() const;
	void ProcessPlayerAbilityInput(float DeltaSeconds);
	void SetPlayerAbilityInputTag(const FGameplayTag& InputTag, bool bPressed);
	void SetPlayerFireAbilityInput(bool bPressed);
	void UpdatePlayerAbilityAxisInput(float Axis, const FGameplayTag& NegativeTag, const FGameplayTag& PositiveTag, bool& bNegativePressed, bool& bPositivePressed);
	void ReleasePlayerAbilityAxisInput();
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
	void UsePlayerItem(EFRItemType ItemType);
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


	float EnhancedMoveAxis = 0.0f;
	float EnhancedAimAxis = 0.0f;
	bool bMoveLeftAbilityInputPressed = false;
	bool bMoveRightAbilityInputPressed = false;
	bool bAimUpAbilityInputPressed = false;
	bool bAimDownAbilityInputPressed = false;
	bool bFireAbilityInputPressed = false;
};
