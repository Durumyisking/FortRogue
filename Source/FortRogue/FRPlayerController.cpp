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
#include "InputCoreTypes.h"
#include "InputActionValue.h"
#include "Items/FRItemDefinition.h"


AFRPlayerController::AFRPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	bShowMouseCursor = false;
}

void AFRPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (BattleInputMappingContext)
	{
		if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				Subsystem->AddMappingContext(BattleInputMappingContext, BattleInputMappingPriority);
			}
		}
	}

	ApplyCurrentGameFlowMode();
}

void AFRPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInput)
	{
		return;
	}

	if (MoveAction)
	{
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFRPlayerController::HandleMove);
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this, &AFRPlayerController::HandleMove);
	}
	if (AimAction)
	{
		EnhancedInput->BindAction(AimAction, ETriggerEvent::Triggered, this, &AFRPlayerController::HandleAim);
		EnhancedInput->BindAction(AimAction, ETriggerEvent::Completed, this, &AFRPlayerController::HandleAim);
	}
	if (FireAction)
	{
		EnhancedInput->BindAction(FireAction, ETriggerEvent::Started, this, &AFRPlayerController::HandleFirePressed);
		EnhancedInput->BindAction(FireAction, ETriggerEvent::Completed, this, &AFRPlayerController::HandleFireReleased);
		EnhancedInput->BindAction(FireAction, ETriggerEvent::Canceled, this, &AFRPlayerController::HandleFireReleased);
	}
	if (Weapon1Action)
	{
		EnhancedInput->BindAction(Weapon1Action, ETriggerEvent::Started, this, &AFRPlayerController::HandleWeapon1);
	}
	if (Weapon2Action)
	{
		EnhancedInput->BindAction(Weapon2Action, ETriggerEvent::Started, this, &AFRPlayerController::HandleWeapon2);
	}
	if (Weapon3Action)
	{
		EnhancedInput->BindAction(Weapon3Action, ETriggerEvent::Started, this, &AFRPlayerController::HandleWeapon3);
	}
	if (Weapon4Action)
	{
		EnhancedInput->BindAction(Weapon4Action, ETriggerEvent::Started, this, &AFRPlayerController::HandleWeapon4);
	}
	if (Weapon5Action)
	{
		EnhancedInput->BindAction(Weapon5Action, ETriggerEvent::Started, this, &AFRPlayerController::HandleWeapon5);
	}
	if (AttackItemAction)
	{
		EnhancedInput->BindAction(AttackItemAction, ETriggerEvent::Started, this, &AFRPlayerController::HandleAttackItem);
	}
	if (HealItemAction)
	{
		EnhancedInput->BindAction(HealItemAction, ETriggerEvent::Started, this, &AFRPlayerController::HandleHealItem);
	}
	if (Reward1Action)
	{
		EnhancedInput->BindAction(Reward1Action, ETriggerEvent::Started, this, &AFRPlayerController::HandleReward1);
	}
	if (Reward2Action)
	{
		EnhancedInput->BindAction(Reward2Action, ETriggerEvent::Started, this, &AFRPlayerController::HandleReward2);
	}
	if (Reward3Action)
	{
		EnhancedInput->BindAction(Reward3Action, ETriggerEvent::Started, this, &AFRPlayerController::HandleReward3);
	}
	if (Reward4Action)
	{
		EnhancedInput->BindAction(Reward4Action, ETriggerEvent::Started, this, &AFRPlayerController::HandleReward4);
	}
	if (Reward5Action)
	{
		EnhancedInput->BindAction(Reward5Action, ETriggerEvent::Started, this, &AFRPlayerController::HandleReward5);
	}
}

void AFRPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	TickRewardInput();

	if (HasEnhancedInputBindings())
	{
		ApplyMoveAxis(EnhancedMoveAxis, DeltaSeconds);
		ApplyAimAxis(EnhancedAimAxis, DeltaSeconds);
		TickKeyboardFireInput();
		TickKeyboardWeaponInput();
		TickKeyboardItemInput();
		TickPlayerWeaponCharge(DeltaSeconds);
		ProcessPlayerAbilityInput(DeltaSeconds);
		return;
	}

	TickBattleInput(DeltaSeconds);
	ProcessPlayerAbilityInput(DeltaSeconds);
}

void AFRPlayerController::TickBattleInput(float DeltaSeconds)
{
	AFRGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFRGameMode>() : nullptr;
	if (!GameMode || GameMode->GetBattleState() != EFRBattleState::PlayerTurn)
	{
		return;
	}

	AFRBattleCharacter* PlayerCharacter = GameMode->GetPlayerCharacter();
	if (!PlayerCharacter)
	{
		return;
	}

	ApplyMoveAxis((IsInputKeyDown(EKeys::D) ? 1.0f : 0.0f) + (IsInputKeyDown(EKeys::A) ? -1.0f : 0.0f), DeltaSeconds);
	ApplyAimAxis((IsInputKeyDown(EKeys::W) ? 1.0f : 0.0f) + (IsInputKeyDown(EKeys::S) ? -1.0f : 0.0f), DeltaSeconds);
	TickKeyboardWeaponInput();
	TickKeyboardItemInput();

	if (WasInputKeyJustPressed(EKeys::SpaceBar))
	{
		BeginPlayerWeaponCharge();
	}
	if (IsInputKeyDown(EKeys::SpaceBar))
	{
		TickPlayerWeaponCharge(DeltaSeconds);
	}
	if (WasInputKeyJustReleased(EKeys::SpaceBar))
	{
		ReleasePlayerWeaponCharge();
	}
}

void AFRPlayerController::TickKeyboardFireInput()
{
	AFRGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFRGameMode>() : nullptr;
	if (!GameMode || GameMode->GetBattleState() != EFRBattleState::PlayerTurn || !GameMode->GetPlayerCharacter())
	{
		return;
	}

	if (WasInputKeyJustPressed(EKeys::SpaceBar))
	{
		BeginPlayerWeaponCharge();
	}
	if (WasInputKeyJustReleased(EKeys::SpaceBar))
	{
		ReleasePlayerWeaponCharge();
	}
}

void AFRPlayerController::TickKeyboardWeaponInput()
{
	if (!Weapon1Action && WasInputKeyJustPressed(EKeys::One))
	{
		SelectPlayerWeapon(0);
	}
	if (!Weapon2Action && WasInputKeyJustPressed(EKeys::Two))
	{
		SelectPlayerWeapon(1);
	}
	if (!Weapon3Action && WasInputKeyJustPressed(EKeys::Three))
	{
		SelectPlayerWeapon(2);
	}
	if (!Weapon4Action && WasInputKeyJustPressed(EKeys::Four))
	{
		SelectPlayerWeapon(3);
	}
	if (!Weapon5Action && WasInputKeyJustPressed(EKeys::Five))
	{
		SelectPlayerWeapon(4);
	}
}

void AFRPlayerController::TickKeyboardItemInput()
{
	if (!AttackItemAction && WasInputKeyJustPressed(EKeys::J))
	{
		UsePlayerItem(EFRItemType::AttackMultiplier);
	}
	if (!HealItemAction && WasInputKeyJustPressed(EKeys::H))
	{
		UsePlayerItem(EFRItemType::Heal);
	}
}

void AFRPlayerController::TickRewardInput()
{
	AFRGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFRGameMode>() : nullptr;
	if (!GameMode || GameMode->GetBattleState() != EFRBattleState::Reward)
	{
		return;
	}

	if (WasInputKeyJustPressed(EKeys::One))
	{
		ChooseReward(0);
	}
	else if (WasInputKeyJustPressed(EKeys::Two))
	{
		ChooseReward(1);
	}
	else if (WasInputKeyJustPressed(EKeys::Three))
	{
		ChooseReward(2);
	}
	else if (WasInputKeyJustPressed(EKeys::Four))
	{
		ChooseReward(3);
	}
	else if (WasInputKeyJustPressed(EKeys::Five))
	{
		ChooseReward(4);
	}
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

bool AFRPlayerController::HasEnhancedInputBindings() const
{
	return MoveAction || AimAction || FireAction
		|| Weapon1Action || Weapon2Action || Weapon3Action || Weapon4Action || Weapon5Action
		|| AttackItemAction || HealItemAction
		|| Reward1Action || Reward2Action || Reward3Action || Reward4Action || Reward5Action;
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
