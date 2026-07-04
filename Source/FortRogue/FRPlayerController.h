// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "Items/FRItemDefinition.h"
#include "FRPlayerController.generated.h"

class UFRAbilitySystemComponent;
class UInputAction;
class UInputMappingContext;
enum class EInputActionValueType : uint8;
struct FGameplayTag;

/**
 * 전투 입력을 Enhanced Input 단일 경로로 처리합니다.
 * Input Action 에셋이 지정되지 않은 항목은 C++에서 기본 액션과 키 매핑을 생성해 사용하므로,
 * 에셋 없이도 동작하고 에셋을 지정하면 그대로 대체됩니다.
 */
UCLASS()
class FORTROGUE_API AFRPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AFRPlayerController();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(Exec)
	void StartMainGame();

	UFUNCTION(BlueprintCallable, Category = "FortRogue|HUD")
	void BeginHUDShotCharge();

	UFUNCTION(BlueprintCallable, Category = "FortRogue|HUD")
	void ReleaseHUDShotCharge();

	UFUNCTION(BlueprintCallable, Category = "FortRogue|HUD")
	void SelectHUDWeapon(int32 WeaponIndex);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|HUD")
	void UseHUDItem(int32 ItemIndex);

private:
	void TickCameraInput(float DeltaSeconds);
	void ApplyCurrentGameFlowMode();
	UFRAbilitySystemComponent* GetPlayerAbilitySystemComponent() const;
	void ProcessPlayerAbilityInput(float DeltaSeconds);
	void SetPlayerAbilityInputTag(const FGameplayTag& InputTag, bool bPressed);
	void SetPlayerFireAbilityInput(bool bPressed);
	void UpdatePlayerAbilityAxisInput(float Axis, const FGameplayTag& NegativeTag, const FGameplayTag& PositiveTag, bool& bNegativePressed, bool& bPositivePressed);
	void ReleasePlayerAbilityAxisInput();

	void BuildDefaultInputBindings();
	UInputAction* CreateDefaultAction(const TCHAR* ActionName, EInputActionValueType ValueType);
	void MapDefaultKey(UInputAction* Action, const FKey& Key, bool bNegate = false, bool bSwizzleToY = false);

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
	void HandleCameraPan(const FInputActionValue& Value);
	void HandleCameraPanReleased(const FInputActionValue& Value);
	void ApplyMoveAxis(float Axis, float DeltaSeconds);
	void ApplyAimAxis(float Axis, float DeltaSeconds);
	void BeginPlayerWeaponCharge();
	void TickPlayerWeaponCharge(float DeltaSeconds);
	void ReleasePlayerWeaponCharge();
	void SelectPlayerWeapon(int32 WeaponIndex);
	void UsePlayerItem(EFRItemType ItemType);
	void ChooseReward(int32 ChoiceIndex);

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "전투 중 사용할 Enhanced Input Mapping Context입니다. 비워두면 C++ 기본 매핑만 사용합니다."))
	TObjectPtr<UInputMappingContext> BattleInputMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "BattleInputMappingContext를 LocalPlayer Subsystem에 추가할 때 사용하는 우선순위입니다. 값이 높을수록 먼저 처리됩니다."))
	int32 BattleInputMappingPriority = 1;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "좌우 이동 입력에 연결할 Input Action입니다. 비워두면 A/D 키 기본 매핑이 생성됩니다."))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "조준 각도 조절 입력에 연결할 Input Action입니다. 비워두면 W/S 키 기본 매핑이 생성됩니다."))
	TObjectPtr<UInputAction> AimAction;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "샷 차지 시작/발사 입력에 연결할 Input Action입니다. 비워두면 SpaceBar 기본 매핑이 생성됩니다."))
	TObjectPtr<UInputAction> FireAction;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "로드아웃의 1번 무기를 선택할 Input Action입니다. 비워두면 1 키 기본 매핑이 생성됩니다."))
	TObjectPtr<UInputAction> Weapon1Action;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "로드아웃의 2번 무기를 선택할 Input Action입니다. 비워두면 2 키 기본 매핑이 생성됩니다."))
	TObjectPtr<UInputAction> Weapon2Action;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "로드아웃의 3번 무기를 선택할 Input Action입니다. 비워두면 3 키 기본 매핑이 생성됩니다."))
	TObjectPtr<UInputAction> Weapon3Action;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "로드아웃의 4번 무기를 선택할 Input Action입니다. 비워두면 4 키 기본 매핑이 생성됩니다."))
	TObjectPtr<UInputAction> Weapon4Action;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "로드아웃의 5번 무기를 선택할 Input Action입니다. 비워두면 5 키 기본 매핑이 생성됩니다."))
	TObjectPtr<UInputAction> Weapon5Action;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "공격 강화 아이템을 사용하는 Input Action입니다. 비워두면 J 키 기본 매핑이 생성됩니다."))
	TObjectPtr<UInputAction> AttackItemAction;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "회복 아이템을 사용하는 Input Action입니다. 비워두면 H 키 기본 매핑이 생성됩니다."))
	TObjectPtr<UInputAction> HealItemAction;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "보상 화면에서 1번 선택지를 고르는 Input Action입니다. 비워두면 1 키 기본 매핑이 생성됩니다."))
	TObjectPtr<UInputAction> Reward1Action;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "보상 화면에서 2번 선택지를 고르는 Input Action입니다. 비워두면 2 키 기본 매핑이 생성됩니다."))
	TObjectPtr<UInputAction> Reward2Action;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "보상 화면에서 3번 선택지를 고르는 Input Action입니다. 비워두면 3 키 기본 매핑이 생성됩니다."))
	TObjectPtr<UInputAction> Reward3Action;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "보상 화면에서 4번 선택지를 고르는 Input Action입니다. 비워두면 4 키 기본 매핑이 생성됩니다."))
	TObjectPtr<UInputAction> Reward4Action;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "보상 화면에서 5번 선택지를 고르는 Input Action입니다. 비워두면 5 키 기본 매핑이 생성됩니다."))
	TObjectPtr<UInputAction> Reward5Action;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Input", meta = (ToolTip = "카메라 수동 이동(2D 축) Input Action입니다. 비워두면 방향키 기본 매핑이 생성됩니다."))
	TObjectPtr<UInputAction> CameraPanAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputMappingContext> DefaultInputMappingContext;

	float EnhancedMoveAxis = 0.0f;
	float EnhancedAimAxis = 0.0f;
	FVector2D EnhancedCameraAxis = FVector2D::ZeroVector;
	bool bCameraPanHeld = false;
	bool bMoveLeftAbilityInputPressed = false;
	bool bMoveRightAbilityInputPressed = false;
	bool bAimUpAbilityInputPressed = false;
	bool bAimDownAbilityInputPressed = false;
	bool bFireAbilityInputPressed = false;
	bool bHUDShotChargePressed = false;
};
