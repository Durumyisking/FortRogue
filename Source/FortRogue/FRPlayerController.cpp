// Copyright Epic Games, Inc. All Rights Reserved.

#include "FRPlayerController.h"

#include "AbilitySystem/FRAbilitySystemComponent.h"
#include "Combat/FRBattleCharacter.h"
#include "FRGameMode.h"
#include "FRGameplayTags.h"
#include "Game/FRGameFlowSubsystem.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "InputCoreTypes.h"
#include "Items/FRItemDefinition.h"

AFRPlayerController::AFRPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	bShowMouseCursor = false;
}

void AFRPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (DefaultInputMappingContext)
			{
				Subsystem->AddMappingContext(DefaultInputMappingContext, 0);
			}
			if (BattleInputMappingContext)
			{
				Subsystem->AddMappingContext(BattleInputMappingContext, BattleInputMappingPriority);
			}
		}
	}

	ApplyCurrentGameFlowMode();
}

UInputAction* AFRPlayerController::CreateDefaultAction(const TCHAR* ActionName, EInputActionValueType ValueType)
{
	UInputAction* Action = NewObject<UInputAction>(this, ActionName);
	Action->ValueType = ValueType;
	return Action;
}

void AFRPlayerController::MapDefaultKey(UInputAction* Action, const FKey& Key, bool bNegate, bool bSwizzleToY)
{
	if (!Action || !DefaultInputMappingContext)
	{
		return;
	}

	FEnhancedActionKeyMapping& Mapping = DefaultInputMappingContext->MapKey(Action, Key);
	if (bSwizzleToY)
	{
		Mapping.Modifiers.Add(NewObject<UInputModifierSwizzleAxis>(this));
	}
	if (bNegate)
	{
		Mapping.Modifiers.Add(NewObject<UInputModifierNegate>(this));
	}
}

void AFRPlayerController::BuildDefaultInputBindings()
{
	DefaultInputMappingContext = NewObject<UInputMappingContext>(this, TEXT("FRDefaultBattleInputContext"));

	if (!MoveAction)
	{
		MoveAction = CreateDefaultAction(TEXT("FRDefaultMoveAction"), EInputActionValueType::Axis1D);
		MapDefaultKey(MoveAction, EKeys::D);
		MapDefaultKey(MoveAction, EKeys::A, true);
	}
	if (!AimAction)
	{
		AimAction = CreateDefaultAction(TEXT("FRDefaultAimAction"), EInputActionValueType::Axis1D);
		MapDefaultKey(AimAction, EKeys::W);
		MapDefaultKey(AimAction, EKeys::S, true);
	}
	if (!FireAction)
	{
		FireAction = CreateDefaultAction(TEXT("FRDefaultFireAction"), EInputActionValueType::Boolean);
		MapDefaultKey(FireAction, EKeys::SpaceBar);
	}

	const FKey SlotKeys[] = { EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four, EKeys::Five };
	TObjectPtr<UInputAction>* WeaponActions[] = { &Weapon1Action, &Weapon2Action, &Weapon3Action, &Weapon4Action, &Weapon5Action };
	TObjectPtr<UInputAction>* RewardActions[] = { &Reward1Action, &Reward2Action, &Reward3Action, &Reward4Action, &Reward5Action };
	for (int32 SlotIndex = 0; SlotIndex < 5; ++SlotIndex)
	{
		if (!*WeaponActions[SlotIndex])
		{
			*WeaponActions[SlotIndex] = CreateDefaultAction(*FString::Printf(TEXT("FRDefaultWeapon%dAction"), SlotIndex + 1), EInputActionValueType::Boolean);
			MapDefaultKey(*WeaponActions[SlotIndex], SlotKeys[SlotIndex]);
		}
		if (!*RewardActions[SlotIndex])
		{
			// 무기 선택과 같은 숫자 키를 공유합니다. 두 핸들러 모두 전투 상태를 확인하므로 동시에 반응하지 않습니다.
			*RewardActions[SlotIndex] = CreateDefaultAction(*FString::Printf(TEXT("FRDefaultReward%dAction"), SlotIndex + 1), EInputActionValueType::Boolean);
			MapDefaultKey(*RewardActions[SlotIndex], SlotKeys[SlotIndex]);
		}
	}

	if (!AttackItemAction)
	{
		AttackItemAction = CreateDefaultAction(TEXT("FRDefaultAttackItemAction"), EInputActionValueType::Boolean);
		MapDefaultKey(AttackItemAction, EKeys::J);
	}
	if (!HealItemAction)
	{
		HealItemAction = CreateDefaultAction(TEXT("FRDefaultHealItemAction"), EInputActionValueType::Boolean);
		MapDefaultKey(HealItemAction, EKeys::H);
	}
	if (!CameraPanAction)
	{
		CameraPanAction = CreateDefaultAction(TEXT("FRDefaultCameraPanAction"), EInputActionValueType::Axis2D);
		MapDefaultKey(CameraPanAction, EKeys::Right);
		MapDefaultKey(CameraPanAction, EKeys::Left, true);
		MapDefaultKey(CameraPanAction, EKeys::Up, false, true);
		MapDefaultKey(CameraPanAction, EKeys::Down, true, true);
	}
}

void AFRPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInput)
	{
		return;
	}

	BuildDefaultInputBindings();

	EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFRPlayerController::HandleMove);
	EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this, &AFRPlayerController::HandleMove);
	EnhancedInput->BindAction(MoveAction, ETriggerEvent::Canceled, this, &AFRPlayerController::HandleMove);
	EnhancedInput->BindAction(AimAction, ETriggerEvent::Triggered, this, &AFRPlayerController::HandleAim);
	EnhancedInput->BindAction(AimAction, ETriggerEvent::Completed, this, &AFRPlayerController::HandleAim);
	EnhancedInput->BindAction(AimAction, ETriggerEvent::Canceled, this, &AFRPlayerController::HandleAim);
	EnhancedInput->BindAction(FireAction, ETriggerEvent::Started, this, &AFRPlayerController::HandleFirePressed);
	EnhancedInput->BindAction(FireAction, ETriggerEvent::Completed, this, &AFRPlayerController::HandleFireReleased);
	EnhancedInput->BindAction(FireAction, ETriggerEvent::Canceled, this, &AFRPlayerController::HandleFireReleased);
	EnhancedInput->BindAction(Weapon1Action, ETriggerEvent::Started, this, &AFRPlayerController::HandleWeapon1);
	EnhancedInput->BindAction(Weapon2Action, ETriggerEvent::Started, this, &AFRPlayerController::HandleWeapon2);
	EnhancedInput->BindAction(Weapon3Action, ETriggerEvent::Started, this, &AFRPlayerController::HandleWeapon3);
	EnhancedInput->BindAction(Weapon4Action, ETriggerEvent::Started, this, &AFRPlayerController::HandleWeapon4);
	EnhancedInput->BindAction(Weapon5Action, ETriggerEvent::Started, this, &AFRPlayerController::HandleWeapon5);
	EnhancedInput->BindAction(AttackItemAction, ETriggerEvent::Started, this, &AFRPlayerController::HandleAttackItem);
	EnhancedInput->BindAction(HealItemAction, ETriggerEvent::Started, this, &AFRPlayerController::HandleHealItem);
	EnhancedInput->BindAction(Reward1Action, ETriggerEvent::Started, this, &AFRPlayerController::HandleReward1);
	EnhancedInput->BindAction(Reward2Action, ETriggerEvent::Started, this, &AFRPlayerController::HandleReward2);
	EnhancedInput->BindAction(Reward3Action, ETriggerEvent::Started, this, &AFRPlayerController::HandleReward3);
	EnhancedInput->BindAction(Reward4Action, ETriggerEvent::Started, this, &AFRPlayerController::HandleReward4);
	EnhancedInput->BindAction(Reward5Action, ETriggerEvent::Started, this, &AFRPlayerController::HandleReward5);
	EnhancedInput->BindAction(CameraPanAction, ETriggerEvent::Triggered, this, &AFRPlayerController::HandleCameraPan);
	EnhancedInput->BindAction(CameraPanAction, ETriggerEvent::Completed, this, &AFRPlayerController::HandleCameraPanReleased);
	EnhancedInput->BindAction(CameraPanAction, ETriggerEvent::Canceled, this, &AFRPlayerController::HandleCameraPanReleased);
}

void AFRPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	TickCameraInput(DeltaSeconds);
	ApplyMoveAxis(EnhancedMoveAxis, DeltaSeconds);
	ApplyAimAxis(EnhancedAimAxis, DeltaSeconds);
	TickPlayerWeaponCharge(DeltaSeconds);
	ProcessPlayerAbilityInput(DeltaSeconds);
}

void AFRPlayerController::TickCameraInput(float DeltaSeconds)
{
	AFRGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFRGameMode>() : nullptr;
	if (!GameMode)
	{
		return;
	}

	GameMode->HandleManualCameraInput(EnhancedCameraAxis, bCameraPanHeld, DeltaSeconds);
}

void AFRPlayerController::StartMainGame()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UFRGameFlowSubsystem* GameFlow = GameInstance->GetSubsystem<UFRGameFlowSubsystem>())
		{
			GameFlow->StartMainGame();
		}
	}
}

void AFRPlayerController::BeginHUDShotCharge()
{
	AFRGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFRGameMode>() : nullptr;
	if (!GameMode || GameMode->GetBattleState() != EFRBattleState::PlayerTurn || !GameMode->GetPlayerCharacter())
	{
		return;
	}

	bHUDShotChargePressed = true;
	BeginPlayerWeaponCharge();
}

void AFRPlayerController::ReleaseHUDShotCharge()
{
	if (!bHUDShotChargePressed)
	{
		return;
	}

	bHUDShotChargePressed = false;
	ReleasePlayerWeaponCharge();
}

void AFRPlayerController::SelectHUDWeapon(int32 WeaponIndex)
{
	SelectPlayerWeapon(WeaponIndex);
}

void AFRPlayerController::UseHUDItem(int32 ItemIndex)
{
	AFRGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFRGameMode>() : nullptr;
	if (GameMode && GameMode->GetBattleState() == EFRBattleState::PlayerTurn && GameMode->GetPlayerCharacter())
	{
		SetPlayerAbilityInputTag(FRGameplayTags::InputTag_UseItem, true);
		GameMode->GetPlayerCharacter()->UseItemByIndex(ItemIndex);
		SetPlayerAbilityInputTag(FRGameplayTags::InputTag_UseItem, false);
	}
}

void AFRPlayerController::ApplyCurrentGameFlowMode()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UFRGameFlowSubsystem* GameFlow = GameInstance->GetSubsystem<UFRGameFlowSubsystem>())
		{
			GameFlow->EnsureStartupMode();
			GameFlow->ApplyCurrentModeToPlayerController(this);
			return;
		}
	}

	bShowMouseCursor = false;
	SetInputMode(FInputModeGameOnly());
}

UFRAbilitySystemComponent* AFRPlayerController::GetPlayerAbilitySystemComponent() const
{
	const AFRGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFRGameMode>() : nullptr;
	const AFRBattleCharacter* PlayerCharacter = GameMode ? GameMode->GetPlayerCharacter() : nullptr;
	return PlayerCharacter ? Cast<UFRAbilitySystemComponent>(PlayerCharacter->GetAbilitySystemComponent()) : nullptr;
}

void AFRPlayerController::ProcessPlayerAbilityInput(float DeltaSeconds)
{
	const AFRGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFRGameMode>() : nullptr;
	UFRAbilitySystemComponent* AbilitySystem = GetPlayerAbilitySystemComponent();
	if (!GameMode || GameMode->GetBattleState() != EFRBattleState::PlayerTurn || !GameMode->GetPlayerCharacter() || !AbilitySystem)
	{
		ReleasePlayerAbilityAxisInput();
		SetPlayerFireAbilityInput(false);
		if (AbilitySystem)
		{
			AbilitySystem->ProcessAbilityInput(DeltaSeconds, GetWorld() && GetWorld()->IsPaused());
			AbilitySystem->ClearAbilityInput();
		}
		return;
	}

	AbilitySystem->ProcessAbilityInput(DeltaSeconds, GetWorld() && GetWorld()->IsPaused());
}

void AFRPlayerController::SetPlayerAbilityInputTag(const FGameplayTag& InputTag, bool bPressed)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	if (UFRAbilitySystemComponent* AbilitySystem = GetPlayerAbilitySystemComponent())
	{
		if (bPressed)
		{
			AbilitySystem->AbilityInputTagPressed(InputTag);
		}
		else
		{
			AbilitySystem->AbilityInputTagReleased(InputTag);
		}
	}
}

void AFRPlayerController::SetPlayerFireAbilityInput(bool bPressed)
{
	if (bFireAbilityInputPressed == bPressed)
	{
		return;
	}

	bFireAbilityInputPressed = bPressed;
	SetPlayerAbilityInputTag(FRGameplayTags::InputTag_Fire, bFireAbilityInputPressed);
}

void AFRPlayerController::UpdatePlayerAbilityAxisInput(float Axis, const FGameplayTag& NegativeTag, const FGameplayTag& PositiveTag, bool& bNegativePressed, bool& bPositivePressed)
{
	const bool bShouldPressNegative = Axis < -KINDA_SMALL_NUMBER;
	const bool bShouldPressPositive = Axis > KINDA_SMALL_NUMBER;
	if (bNegativePressed != bShouldPressNegative)
	{
		bNegativePressed = bShouldPressNegative;
		SetPlayerAbilityInputTag(NegativeTag, bNegativePressed);
	}
	if (bPositivePressed != bShouldPressPositive)
	{
		bPositivePressed = bShouldPressPositive;
		SetPlayerAbilityInputTag(PositiveTag, bPositivePressed);
	}
}

void AFRPlayerController::ReleasePlayerAbilityAxisInput()
{
	UpdatePlayerAbilityAxisInput(0.0f, FRGameplayTags::InputTag_MoveLeft, FRGameplayTags::InputTag_MoveRight, bMoveLeftAbilityInputPressed, bMoveRightAbilityInputPressed);
	UpdatePlayerAbilityAxisInput(0.0f, FRGameplayTags::InputTag_AimDown, FRGameplayTags::InputTag_AimUp, bAimDownAbilityInputPressed, bAimUpAbilityInputPressed);
}

void AFRPlayerController::HandleMove(const FInputActionValue& Value)
{
	EnhancedMoveAxis = Value.Get<float>();
}

void AFRPlayerController::HandleAim(const FInputActionValue& Value)
{
	EnhancedAimAxis = Value.Get<float>();
}

void AFRPlayerController::HandleFirePressed()
{
	BeginPlayerWeaponCharge();
}

void AFRPlayerController::HandleFireReleased()
{
	ReleasePlayerWeaponCharge();
}

void AFRPlayerController::HandleWeapon1()
{
	SelectPlayerWeapon(0);
}

void AFRPlayerController::HandleWeapon2()
{
	SelectPlayerWeapon(1);
}

void AFRPlayerController::HandleWeapon3()
{
	SelectPlayerWeapon(2);
}

void AFRPlayerController::HandleWeapon4()
{
	SelectPlayerWeapon(3);
}

void AFRPlayerController::HandleWeapon5()
{
	SelectPlayerWeapon(4);
}

void AFRPlayerController::HandleAttackItem()
{
	UsePlayerItem(EFRItemType::AttackMultiplier);
}

void AFRPlayerController::HandleHealItem()
{
	UsePlayerItem(EFRItemType::Heal);
}

void AFRPlayerController::HandleReward1()
{
	ChooseReward(0);
}

void AFRPlayerController::HandleReward2()
{
	ChooseReward(1);
}

void AFRPlayerController::HandleReward3()
{
	ChooseReward(2);
}

void AFRPlayerController::HandleReward4()
{
	ChooseReward(3);
}

void AFRPlayerController::HandleReward5()
{
	ChooseReward(4);
}

void AFRPlayerController::HandleCameraPan(const FInputActionValue& Value)
{
	EnhancedCameraAxis = Value.Get<FVector2D>();
	bCameraPanHeld = true;
}

void AFRPlayerController::HandleCameraPanReleased(const FInputActionValue& Value)
{
	EnhancedCameraAxis = FVector2D::ZeroVector;
	bCameraPanHeld = false;
}

void AFRPlayerController::ApplyMoveAxis(float Axis, float DeltaSeconds)
{
	AFRGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFRGameMode>() : nullptr;
	const bool bCanMove = GameMode && GameMode->GetBattleState() == EFRBattleState::PlayerTurn && GameMode->GetPlayerCharacter();
	UpdatePlayerAbilityAxisInput(bCanMove ? Axis : 0.0f, FRGameplayTags::InputTag_MoveLeft, FRGameplayTags::InputTag_MoveRight, bMoveLeftAbilityInputPressed, bMoveRightAbilityInputPressed);
	if (bCanMove)
	{
		GameMode->GetPlayerCharacter()->MoveHorizontal(Axis, DeltaSeconds);
	}
}

void AFRPlayerController::ApplyAimAxis(float Axis, float DeltaSeconds)
{
	AFRGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFRGameMode>() : nullptr;
	const bool bCanAim = GameMode && GameMode->GetBattleState() == EFRBattleState::PlayerTurn && GameMode->GetPlayerCharacter();
	UpdatePlayerAbilityAxisInput(bCanAim ? Axis : 0.0f, FRGameplayTags::InputTag_AimDown, FRGameplayTags::InputTag_AimUp, bAimDownAbilityInputPressed, bAimUpAbilityInputPressed);
	if (bCanAim)
	{
		GameMode->GetPlayerCharacter()->AdjustAim(Axis, DeltaSeconds);
	}
}

void AFRPlayerController::BeginPlayerWeaponCharge()
{
	AFRGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFRGameMode>() : nullptr;
	if (GameMode && GameMode->GetBattleState() == EFRBattleState::PlayerTurn && GameMode->GetPlayerCharacter())
	{
		SetPlayerFireAbilityInput(true);
		GameMode->GetPlayerCharacter()->BeginShotCharge();
	}
}

void AFRPlayerController::TickPlayerWeaponCharge(float DeltaSeconds)
{
	AFRGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFRGameMode>() : nullptr;
	if (GameMode && GameMode->GetBattleState() == EFRBattleState::PlayerTurn && GameMode->GetPlayerCharacter())
	{
		GameMode->GetPlayerCharacter()->UpdateShotCharge(DeltaSeconds);
	}
}

void AFRPlayerController::ReleasePlayerWeaponCharge()
{
	AFRGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFRGameMode>() : nullptr;
	if (!GameMode || GameMode->GetBattleState() != EFRBattleState::PlayerTurn || !GameMode->GetPlayerCharacter())
	{
		return;
	}

	const int32 SpawnedProjectiles = GameMode->GetPlayerCharacter()->ReleaseShotCharge();
	SetPlayerFireAbilityInput(false);
	GameMode->NotifyShotFired(GameMode->GetPlayerCharacter(), SpawnedProjectiles);
}

void AFRPlayerController::SelectPlayerWeapon(int32 WeaponIndex)
{
	AFRGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFRGameMode>() : nullptr;
	if (GameMode && GameMode->GetBattleState() == EFRBattleState::PlayerTurn && GameMode->GetPlayerCharacter() && GameMode->GetPlayerCharacter()->CanSelectWeapon(WeaponIndex))
	{
		GameMode->GetPlayerCharacter()->SelectWeapon(WeaponIndex);
	}
}

void AFRPlayerController::UsePlayerItem(EFRItemType ItemType)
{
	AFRGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFRGameMode>() : nullptr;
	if (GameMode && GameMode->GetBattleState() == EFRBattleState::PlayerTurn && GameMode->GetPlayerCharacter())
	{
		SetPlayerAbilityInputTag(FRGameplayTags::InputTag_UseItem, true);
		GameMode->GetPlayerCharacter()->UseItemByType(ItemType);
		SetPlayerAbilityInputTag(FRGameplayTags::InputTag_UseItem, false);
	}
}

void AFRPlayerController::ChooseReward(int32 ChoiceIndex)
{
	AFRGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFRGameMode>() : nullptr;
	if (GameMode && GameMode->CanApplyRewardChoice(ChoiceIndex))
	{
		GameMode->ApplyRewardChoice(ChoiceIndex);
	}
}
