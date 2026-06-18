// Copyright Epic Games, Inc. All Rights Reserved.

#include "FortRoguePlayerController.h"

#include "Combat/FortRogueBattleCharacter.h"
#include "FortRogueGameMode.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/World.h"
#include "InputCoreTypes.h"
#include "InputActionValue.h"
#include "Items/FortRogueItemDefinition.h"
#include "UI/FortRogueBattleHUDWidget.h"
#include "UI/FortRogueRewardScreenWidget.h"

AFortRoguePlayerController::AFortRoguePlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	bShowMouseCursor = false;
}

void AFortRoguePlayerController::BeginPlay()
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

	if (BattleHUDWidgetClass)
	{
		BattleHUDWidget = CreateWidget<UFortRogueBattleHUDWidget>(this, BattleHUDWidgetClass);
		if (BattleHUDWidget)
		{
			BattleHUDWidget->AddToViewport(0);
		}
	}

	if (RewardScreenWidgetClass)
	{
		RewardScreenWidget = CreateWidget<UFortRogueRewardScreenWidget>(this, RewardScreenWidgetClass);
		if (RewardScreenWidget)
		{
			RewardScreenWidget->AddToViewport(10);
			RewardScreenWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void AFortRoguePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInput)
	{
		return;
	}

	if (MoveAction)
	{
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFortRoguePlayerController::HandleMove);
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this, &AFortRoguePlayerController::HandleMove);
	}
	if (AimAction)
	{
		EnhancedInput->BindAction(AimAction, ETriggerEvent::Triggered, this, &AFortRoguePlayerController::HandleAim);
		EnhancedInput->BindAction(AimAction, ETriggerEvent::Completed, this, &AFortRoguePlayerController::HandleAim);
	}
	if (FireAction)
	{
		EnhancedInput->BindAction(FireAction, ETriggerEvent::Started, this, &AFortRoguePlayerController::HandleFirePressed);
		EnhancedInput->BindAction(FireAction, ETriggerEvent::Completed, this, &AFortRoguePlayerController::HandleFireReleased);
		EnhancedInput->BindAction(FireAction, ETriggerEvent::Canceled, this, &AFortRoguePlayerController::HandleFireReleased);
	}
	if (Weapon1Action)
	{
		EnhancedInput->BindAction(Weapon1Action, ETriggerEvent::Started, this, &AFortRoguePlayerController::HandleWeapon1);
	}
	if (Weapon2Action)
	{
		EnhancedInput->BindAction(Weapon2Action, ETriggerEvent::Started, this, &AFortRoguePlayerController::HandleWeapon2);
	}
	if (Weapon3Action)
	{
		EnhancedInput->BindAction(Weapon3Action, ETriggerEvent::Started, this, &AFortRoguePlayerController::HandleWeapon3);
	}
	if (Weapon4Action)
	{
		EnhancedInput->BindAction(Weapon4Action, ETriggerEvent::Started, this, &AFortRoguePlayerController::HandleWeapon4);
	}
	if (Weapon5Action)
	{
		EnhancedInput->BindAction(Weapon5Action, ETriggerEvent::Started, this, &AFortRoguePlayerController::HandleWeapon5);
	}
	if (AttackItemAction)
	{
		EnhancedInput->BindAction(AttackItemAction, ETriggerEvent::Started, this, &AFortRoguePlayerController::HandleAttackItem);
	}
	if (HealItemAction)
	{
		EnhancedInput->BindAction(HealItemAction, ETriggerEvent::Started, this, &AFortRoguePlayerController::HandleHealItem);
	}
	if (Reward1Action)
	{
		EnhancedInput->BindAction(Reward1Action, ETriggerEvent::Started, this, &AFortRoguePlayerController::HandleReward1);
	}
	if (Reward2Action)
	{
		EnhancedInput->BindAction(Reward2Action, ETriggerEvent::Started, this, &AFortRoguePlayerController::HandleReward2);
	}
	if (Reward3Action)
	{
		EnhancedInput->BindAction(Reward3Action, ETriggerEvent::Started, this, &AFortRoguePlayerController::HandleReward3);
	}
	if (Reward4Action)
	{
		EnhancedInput->BindAction(Reward4Action, ETriggerEvent::Started, this, &AFortRoguePlayerController::HandleReward4);
	}
	if (Reward5Action)
	{
		EnhancedInput->BindAction(Reward5Action, ETriggerEvent::Started, this, &AFortRoguePlayerController::HandleReward5);
	}
}

void AFortRoguePlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateOptionalWidgets();
	TickRewardInput();

	if (HasEnhancedInputBindings())
	{
		ApplyMoveAxis(EnhancedMoveAxis, DeltaSeconds);
		ApplyAimAxis(EnhancedAimAxis, DeltaSeconds);
		TickKeyboardFireInput();
		TickKeyboardWeaponInput();
		TickKeyboardItemInput();
		TickPlayerWeaponCharge(DeltaSeconds);
		return;
	}

	TickBattleInput(DeltaSeconds);
}

void AFortRoguePlayerController::TickBattleInput(float DeltaSeconds)
{
	AFortRogueGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFortRogueGameMode>() : nullptr;
	if (!GameMode || GameMode->GetBattleState() != EFortRogueBattleState::PlayerTurn)
	{
		return;
	}

	AFortRogueBattleCharacter* PlayerCharacter = GameMode->GetPlayerCharacter();
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

void AFortRoguePlayerController::TickKeyboardFireInput()
{
	AFortRogueGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFortRogueGameMode>() : nullptr;
	if (!GameMode || GameMode->GetBattleState() != EFortRogueBattleState::PlayerTurn || !GameMode->GetPlayerCharacter())
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

void AFortRoguePlayerController::TickKeyboardWeaponInput()
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

void AFortRoguePlayerController::TickKeyboardItemInput()
{
	if (!AttackItemAction && WasInputKeyJustPressed(EKeys::J))
	{
		UsePlayerItem(EFortRogueItemType::AttackMultiplier);
	}
	if (!HealItemAction && WasInputKeyJustPressed(EKeys::H))
	{
		UsePlayerItem(EFortRogueItemType::Heal);
	}
}

void AFortRoguePlayerController::TickRewardInput()
{
	AFortRogueGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFortRogueGameMode>() : nullptr;
	if (!GameMode || GameMode->GetBattleState() != EFortRogueBattleState::Reward)
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

void AFortRoguePlayerController::UpdateOptionalWidgets()
{
	AFortRogueGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFortRogueGameMode>() : nullptr;
	if (!GameMode)
	{
		return;
	}

	if (BattleHUDWidget)
	{
		BattleHUDWidget->SetVisibility(GameMode->GetBattleState() == EFortRogueBattleState::Reward ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
		BattleHUDWidget->RefreshBattleHUD();
	}
	if (RewardScreenWidget)
	{
		RewardScreenWidget->SetVisibility(GameMode->GetBattleState() == EFortRogueBattleState::Reward ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		RewardScreenWidget->RefreshRewardScreen();
	}
}

bool AFortRoguePlayerController::HasEnhancedInputBindings() const
{
	return MoveAction || AimAction || FireAction
		|| Weapon1Action || Weapon2Action || Weapon3Action || Weapon4Action || Weapon5Action
		|| AttackItemAction || HealItemAction
		|| Reward1Action || Reward2Action || Reward3Action || Reward4Action || Reward5Action;
}

void AFortRoguePlayerController::HandleMove(const FInputActionValue& Value)
{
	EnhancedMoveAxis = Value.Get<float>();
}

void AFortRoguePlayerController::HandleAim(const FInputActionValue& Value)
{
	EnhancedAimAxis = Value.Get<float>();
}

void AFortRoguePlayerController::HandleFirePressed()
{
	BeginPlayerWeaponCharge();
}

void AFortRoguePlayerController::HandleFireReleased()
{
	ReleasePlayerWeaponCharge();
}

void AFortRoguePlayerController::HandleWeapon1()
{
	SelectPlayerWeapon(0);
}

void AFortRoguePlayerController::HandleWeapon2()
{
	SelectPlayerWeapon(1);
}

void AFortRoguePlayerController::HandleWeapon3()
{
	SelectPlayerWeapon(2);
}

void AFortRoguePlayerController::HandleWeapon4()
{
	SelectPlayerWeapon(3);
}

void AFortRoguePlayerController::HandleWeapon5()
{
	SelectPlayerWeapon(4);
}

void AFortRoguePlayerController::HandleAttackItem()
{
	UsePlayerItem(EFortRogueItemType::AttackMultiplier);
}

void AFortRoguePlayerController::HandleHealItem()
{
	UsePlayerItem(EFortRogueItemType::Heal);
}

void AFortRoguePlayerController::HandleReward1()
{
	ChooseReward(0);
}

void AFortRoguePlayerController::HandleReward2()
{
	ChooseReward(1);
}

void AFortRoguePlayerController::HandleReward3()
{
	ChooseReward(2);
}

void AFortRoguePlayerController::HandleReward4()
{
	ChooseReward(3);
}

void AFortRoguePlayerController::HandleReward5()
{
	ChooseReward(4);
}

void AFortRoguePlayerController::ApplyMoveAxis(float Axis, float DeltaSeconds)
{
	AFortRogueGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFortRogueGameMode>() : nullptr;
	if (GameMode && GameMode->GetBattleState() == EFortRogueBattleState::PlayerTurn && GameMode->GetPlayerCharacter())
	{
		GameMode->GetPlayerCharacter()->MoveHorizontal(Axis, DeltaSeconds);
	}
}

void AFortRoguePlayerController::ApplyAimAxis(float Axis, float DeltaSeconds)
{
	AFortRogueGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFortRogueGameMode>() : nullptr;
	if (GameMode && GameMode->GetBattleState() == EFortRogueBattleState::PlayerTurn && GameMode->GetPlayerCharacter())
	{
		GameMode->GetPlayerCharacter()->AdjustAim(Axis, DeltaSeconds);
	}
}

void AFortRoguePlayerController::BeginPlayerWeaponCharge()
{
	AFortRogueGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFortRogueGameMode>() : nullptr;
	if (GameMode && GameMode->GetBattleState() == EFortRogueBattleState::PlayerTurn && GameMode->GetPlayerCharacter())
	{
		GameMode->GetPlayerCharacter()->BeginShotCharge();
	}
}

void AFortRoguePlayerController::TickPlayerWeaponCharge(float DeltaSeconds)
{
	AFortRogueGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFortRogueGameMode>() : nullptr;
	if (GameMode && GameMode->GetBattleState() == EFortRogueBattleState::PlayerTurn && GameMode->GetPlayerCharacter())
	{
		GameMode->GetPlayerCharacter()->UpdateShotCharge(DeltaSeconds);
	}
}

void AFortRoguePlayerController::ReleasePlayerWeaponCharge()
{
	AFortRogueGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFortRogueGameMode>() : nullptr;
	if (!GameMode || GameMode->GetBattleState() != EFortRogueBattleState::PlayerTurn || !GameMode->GetPlayerCharacter())
	{
		return;
	}

	const int32 SpawnedProjectiles = GameMode->GetPlayerCharacter()->ReleaseShotCharge();
	GameMode->NotifyShotFired(GameMode->GetPlayerCharacter(), SpawnedProjectiles);
}

void AFortRoguePlayerController::SelectPlayerWeapon(int32 WeaponIndex)
{
	AFortRogueGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFortRogueGameMode>() : nullptr;
	if (GameMode && GameMode->GetBattleState() == EFortRogueBattleState::PlayerTurn && GameMode->GetPlayerCharacter())
	{
		GameMode->GetPlayerCharacter()->SelectWeapon(WeaponIndex);
	}
}

void AFortRoguePlayerController::UsePlayerItem(EFortRogueItemType ItemType)
{
	AFortRogueGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFortRogueGameMode>() : nullptr;
	if (GameMode && GameMode->GetBattleState() == EFortRogueBattleState::PlayerTurn && GameMode->GetPlayerCharacter())
	{
		GameMode->GetPlayerCharacter()->UseItemByType(ItemType);
	}
}

void AFortRoguePlayerController::UsePlayerItemByTag(FGameplayTag ItemTag)
{
	AFortRogueGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFortRogueGameMode>() : nullptr;
	if (GameMode && GameMode->GetBattleState() == EFortRogueBattleState::PlayerTurn && GameMode->GetPlayerCharacter())
	{
		GameMode->GetPlayerCharacter()->UseItemByTag(ItemTag);
	}
}

void AFortRoguePlayerController::UsePlayerItemByIndex(int32 ItemIndex)
{
	AFortRogueGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFortRogueGameMode>() : nullptr;
	if (GameMode && GameMode->GetBattleState() == EFortRogueBattleState::PlayerTurn && GameMode->GetPlayerCharacter())
	{
		GameMode->GetPlayerCharacter()->UseItemByIndex(ItemIndex);
	}
}

bool AFortRoguePlayerController::CanUsePlayerItemByType(EFortRogueItemType ItemType) const
{
	AFortRogueGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFortRogueGameMode>() : nullptr;
	if (GameMode && GameMode->GetBattleState() == EFortRogueBattleState::PlayerTurn && GameMode->GetPlayerCharacter())
	{
		return GameMode->GetPlayerCharacter()->CanUseItemByType(ItemType);
	}
	return false;
}

bool AFortRoguePlayerController::CanUsePlayerItemByTag(FGameplayTag ItemTag) const
{
	AFortRogueGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFortRogueGameMode>() : nullptr;
	if (GameMode && GameMode->GetBattleState() == EFortRogueBattleState::PlayerTurn && GameMode->GetPlayerCharacter())
	{
		return GameMode->GetPlayerCharacter()->CanUseItemByTag(ItemTag);
	}
	return false;
}

bool AFortRoguePlayerController::CanUsePlayerItemByIndex(int32 ItemIndex) const
{
	AFortRogueGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFortRogueGameMode>() : nullptr;
	if (GameMode && GameMode->GetBattleState() == EFortRogueBattleState::PlayerTurn && GameMode->GetPlayerCharacter())
	{
		return GameMode->GetPlayerCharacter()->CanUseItemByIndex(ItemIndex);
	}
	return false;
}

int32 AFortRoguePlayerController::GetPlayerItemIndexByTag(FGameplayTag ItemTag) const
{
	AFortRogueGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFortRogueGameMode>() : nullptr;
	if (GameMode && GameMode->GetBattleState() == EFortRogueBattleState::PlayerTurn && GameMode->GetPlayerCharacter())
	{
		return GameMode->GetPlayerCharacter()->GetItemIndexByTag(ItemTag);
	}
	return INDEX_NONE;
}

bool AFortRoguePlayerController::SelectPlayerWeaponByTag(FGameplayTag WeaponTag)
{
	AFortRogueGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFortRogueGameMode>() : nullptr;
	if (GameMode && GameMode->GetBattleState() == EFortRogueBattleState::PlayerTurn && GameMode->GetPlayerCharacter())
	{
		return GameMode->GetPlayerCharacter()->SelectWeaponByTag(WeaponTag);
	}
	return false;
}

bool AFortRoguePlayerController::CanSelectPlayerWeapon(int32 WeaponIndex) const
{
	AFortRogueGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFortRogueGameMode>() : nullptr;
	if (GameMode && GameMode->GetBattleState() == EFortRogueBattleState::PlayerTurn && GameMode->GetPlayerCharacter())
	{
		return GameMode->GetPlayerCharacter()->CanSelectWeapon(WeaponIndex);
	}
	return false;
}

bool AFortRoguePlayerController::CanSelectPlayerWeaponByTag(FGameplayTag WeaponTag) const
{
	AFortRogueGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFortRogueGameMode>() : nullptr;
	if (GameMode && GameMode->GetBattleState() == EFortRogueBattleState::PlayerTurn && GameMode->GetPlayerCharacter())
	{
		return GameMode->GetPlayerCharacter()->CanSelectWeaponByTag(WeaponTag);
	}
	return false;
}

int32 AFortRoguePlayerController::GetPlayerWeaponIndexByTag(FGameplayTag WeaponTag) const
{
	AFortRogueGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFortRogueGameMode>() : nullptr;
	if (GameMode && GameMode->GetBattleState() == EFortRogueBattleState::PlayerTurn && GameMode->GetPlayerCharacter())
	{
		return GameMode->GetPlayerCharacter()->GetWeaponIndexByTag(WeaponTag);
	}
	return INDEX_NONE;
}

bool AFortRoguePlayerController::CanFirePlayerWeapon() const
{
	AFortRogueGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFortRogueGameMode>() : nullptr;
	if (GameMode && GameMode->GetBattleState() == EFortRogueBattleState::PlayerTurn && GameMode->GetPlayerCharacter())
	{
		return GameMode->GetPlayerCharacter()->CanFireSelectedWeapon();
	}
	return false;
}

void AFortRoguePlayerController::ChooseReward(int32 ChoiceIndex)
{
	AFortRogueGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFortRogueGameMode>() : nullptr;
	if (GameMode && GameMode->GetBattleState() == EFortRogueBattleState::Reward)
	{
		GameMode->ApplyRewardChoice(ChoiceIndex);
	}
}
