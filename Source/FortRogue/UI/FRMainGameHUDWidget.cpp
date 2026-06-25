// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FRMainGameHUDWidget.h"

#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "FRGameMode.h"
#include "FRPlayerController.h"
#include "Game/FRTurnBasedGameState.h"

namespace
{
void SetTextBlockText(UTextBlock* TextBlock, const FText& Text)
{
	if (TextBlock)
	{
		TextBlock->SetText(Text);
	}
}

void SetProgressBarPercent(UProgressBar* ProgressBar, float Percent)
{
	if (ProgressBar)
	{
		ProgressBar->SetPercent(FMath::Clamp(Percent, 0.0f, 1.0f));
	}
}

FText GetBattleStateText(EFRBattleState BattleState)
{
	switch (BattleState)
	{
	case EFRBattleState::PlayerTurn:
		return FText::FromString(TEXT("Player turn"));
	case EFRBattleState::EnemyTurn:
		return FText::FromString(TEXT("Enemy turn"));
	case EFRBattleState::ResolvingShot:
		return FText::FromString(TEXT("Shot resolving"));
	case EFRBattleState::Reward:
		return FText::FromString(TEXT("Reward"));
	case EFRBattleState::Won:
		return FText::FromString(TEXT("Run complete"));
	case EFRBattleState::Lost:
		return FText::FromString(TEXT("Defeat"));
	default:
		return FText::GetEmpty();
	}
}

AFRPlayerController* GetFortRoguePlayerController(const UUserWidget& Widget)
{
	return Cast<AFRPlayerController>(Widget.GetOwningPlayer());
}
}

UFRPlayerHUDViewModel* UFRMainGameHUDWidget::GetPlayerViewModel() const
{
	return PlayerViewModel;
}

void UFRMainGameHUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	PlayerViewModel = NewObject<UFRPlayerHUDViewModel>(this);
	BindButtonEvents();
}

void UFRMainGameHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	RefreshHUD();
}

void UFRMainGameHUDWidget::BindButtonEvents()
{
	if (FireButton)
	{
		FireButton->OnPressed.AddDynamic(this, &UFRMainGameHUDWidget::HandleFirePressed);
		FireButton->OnReleased.AddDynamic(this, &UFRMainGameHUDWidget::HandleFireReleased);
	}

	if (WeaponSlot1Button)
	{
		WeaponSlot1Button->OnClicked.AddDynamic(this, &UFRMainGameHUDWidget::HandleWeaponSlot1Clicked);
	}
	if (WeaponSlot2Button)
	{
		WeaponSlot2Button->OnClicked.AddDynamic(this, &UFRMainGameHUDWidget::HandleWeaponSlot2Clicked);
	}
	if (WeaponSlot3Button)
	{
		WeaponSlot3Button->OnClicked.AddDynamic(this, &UFRMainGameHUDWidget::HandleWeaponSlot3Clicked);
	}
	if (WeaponSlot4Button)
	{
		WeaponSlot4Button->OnClicked.AddDynamic(this, &UFRMainGameHUDWidget::HandleWeaponSlot4Clicked);
	}
	if (WeaponSlot5Button)
	{
		WeaponSlot5Button->OnClicked.AddDynamic(this, &UFRMainGameHUDWidget::HandleWeaponSlot5Clicked);
	}

	if (ItemSlot1Button)
	{
		ItemSlot1Button->OnClicked.AddDynamic(this, &UFRMainGameHUDWidget::HandleItemSlot1Clicked);
	}
	if (ItemSlot2Button)
	{
		ItemSlot2Button->OnClicked.AddDynamic(this, &UFRMainGameHUDWidget::HandleItemSlot2Clicked);
	}
	if (ItemSlot3Button)
	{
		ItemSlot3Button->OnClicked.AddDynamic(this, &UFRMainGameHUDWidget::HandleItemSlot3Clicked);
	}
	if (ItemSlot4Button)
	{
		ItemSlot4Button->OnClicked.AddDynamic(this, &UFRMainGameHUDWidget::HandleItemSlot4Clicked);
	}
	if (ItemSlot5Button)
	{
		ItemSlot5Button->OnClicked.AddDynamic(this, &UFRMainGameHUDWidget::HandleItemSlot5Clicked);
	}
}

void UFRMainGameHUDWidget::RefreshHUD()
{
	AFRGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFRGameMode>() : nullptr;
	RefreshGameInfo(GameMode);
	RefreshPlayerInfo(GameMode);
}

void UFRMainGameHUDWidget::RefreshGameInfo(AFRGameMode* GameMode)
{
	if (!GameMode)
	{
		SetTextBlockText(TurnText, FText::GetEmpty());
		SetTextBlockText(ShotTimerText, FText::GetEmpty());
		SetTextBlockText(StatusText, FText::GetEmpty());
		SetTextBlockText(WindText, FText::GetEmpty());
		return;
	}

	SetTextBlockText(TurnText, GameMode->GetRunProgressSummary());
	SetTextBlockText(ShotTimerText, GetBattleStateText(GameMode->GetBattleState()));
	SetTextBlockText(StatusText, GameMode->GetStatusText());
	SetTextBlockText(WindText, GameMode->GetWindSummary());
}

void UFRMainGameHUDWidget::RefreshPlayerInfo(AFRGameMode* GameMode)
{
	if (!PlayerViewModel)
	{
		return;
	}

	PlayerViewModel->RefreshFromCharacter(GameMode ? GameMode->GetPlayerCharacter() : nullptr);

	SetTextBlockText(PlayerNameText, PlayerViewModel->PlayerName);
	SetProgressBarPercent(PlayerHPBar, PlayerViewModel->HealthPercent);
	SetTextBlockText(PlayerHPText, PlayerViewModel->HealthText);
	SetProgressBarPercent(ShotPowerBar, PlayerViewModel->ShotPowerPercent);
	SetTextBlockText(ShotPowerText, PlayerViewModel->ShotPowerText);
	SetProgressBarPercent(MoveBudgetBar, PlayerViewModel->MoveBudgetPercent);
	SetTextBlockText(MoveBudgetText, PlayerViewModel->MoveBudgetText);
	SetTextBlockText(AimText, PlayerViewModel->AimText);
	SetTextBlockText(CurrentWeaponText, PlayerViewModel->CurrentWeaponName);
	SetTextBlockText(CurrentShotText, PlayerViewModel->CurrentShotSummary);

	if (FireButton)
	{
		FireButton->SetIsEnabled(PlayerViewModel->bCanFire);
	}

	ApplyWeaponSlot(PlayerViewModel->GetWeaponSlot(0), WeaponSlot1Button, WeaponSlot1Text, WeaponSlot1IndexText);
	ApplyWeaponSlot(PlayerViewModel->GetWeaponSlot(1), WeaponSlot2Button, WeaponSlot2Text, WeaponSlot2IndexText);
	ApplyWeaponSlot(PlayerViewModel->GetWeaponSlot(2), WeaponSlot3Button, WeaponSlot3Text, WeaponSlot3IndexText);
	ApplyWeaponSlot(PlayerViewModel->GetWeaponSlot(3), WeaponSlot4Button, WeaponSlot4Text, WeaponSlot4IndexText);
	ApplyWeaponSlot(PlayerViewModel->GetWeaponSlot(4), WeaponSlot5Button, WeaponSlot5Text, WeaponSlot5IndexText);

	ApplyItemSlot(PlayerViewModel->GetItemSlot(0), ItemSlot1Button, ItemSlot1Text, ItemSlot1ChargesText);
	ApplyItemSlot(PlayerViewModel->GetItemSlot(1), ItemSlot2Button, ItemSlot2Text, ItemSlot2ChargesText);
	ApplyItemSlot(PlayerViewModel->GetItemSlot(2), ItemSlot3Button, ItemSlot3Text, ItemSlot3ChargesText);
	ApplyItemSlot(PlayerViewModel->GetItemSlot(3), ItemSlot4Button, ItemSlot4Text, ItemSlot4ChargesText);
	ApplyItemSlot(PlayerViewModel->GetItemSlot(4), ItemSlot5Button, ItemSlot5Text, ItemSlot5ChargesText);
}

void UFRMainGameHUDWidget::ApplyWeaponSlot(const FFRHUDLoadoutSlotViewData& SlotData, UButton* Button, UTextBlock* NameText, UTextBlock* IndexText)
{
	if (Button)
	{
		Button->SetIsEnabled(SlotData.bEnabled);
	}
	SetTextBlockText(NameText, SlotData.bOccupied ? SlotData.DisplayName : FText::GetEmpty());
	SetTextBlockText(IndexText, SlotData.InputText);
}

void UFRMainGameHUDWidget::ApplyItemSlot(const FFRHUDLoadoutSlotViewData& SlotData, UButton* Button, UTextBlock* NameText, UTextBlock* ChargesText)
{
	if (Button)
	{
		Button->SetIsEnabled(SlotData.bEnabled);
	}
	SetTextBlockText(NameText, SlotData.bOccupied ? SlotData.DisplayName : FText::GetEmpty());
	SetTextBlockText(ChargesText, SlotData.bOccupied ? FText::AsNumber(SlotData.Charges) : FText::GetEmpty());
}

void UFRMainGameHUDWidget::HandleFirePressed()
{
	if (AFRPlayerController* PlayerController = GetFortRoguePlayerController(*this))
	{
		PlayerController->BeginHUDShotCharge();
	}
}

void UFRMainGameHUDWidget::HandleFireReleased()
{
	if (AFRPlayerController* PlayerController = GetFortRoguePlayerController(*this))
	{
		PlayerController->ReleaseHUDShotCharge();
	}
}

void UFRMainGameHUDWidget::HandleWeaponSlot1Clicked()
{
	if (AFRPlayerController* PlayerController = GetFortRoguePlayerController(*this))
	{
		PlayerController->SelectHUDWeapon(0);
	}
}

void UFRMainGameHUDWidget::HandleWeaponSlot2Clicked()
{
	if (AFRPlayerController* PlayerController = GetFortRoguePlayerController(*this))
	{
		PlayerController->SelectHUDWeapon(1);
	}
}

void UFRMainGameHUDWidget::HandleWeaponSlot3Clicked()
{
	if (AFRPlayerController* PlayerController = GetFortRoguePlayerController(*this))
	{
		PlayerController->SelectHUDWeapon(2);
	}
}

void UFRMainGameHUDWidget::HandleWeaponSlot4Clicked()
{
	if (AFRPlayerController* PlayerController = GetFortRoguePlayerController(*this))
	{
		PlayerController->SelectHUDWeapon(3);
	}
}

void UFRMainGameHUDWidget::HandleWeaponSlot5Clicked()
{
	if (AFRPlayerController* PlayerController = GetFortRoguePlayerController(*this))
	{
		PlayerController->SelectHUDWeapon(4);
	}
}

void UFRMainGameHUDWidget::HandleItemSlot1Clicked()
{
	if (AFRPlayerController* PlayerController = GetFortRoguePlayerController(*this))
	{
		PlayerController->UseHUDItem(0);
	}
}

void UFRMainGameHUDWidget::HandleItemSlot2Clicked()
{
	if (AFRPlayerController* PlayerController = GetFortRoguePlayerController(*this))
	{
		PlayerController->UseHUDItem(1);
	}
}

void UFRMainGameHUDWidget::HandleItemSlot3Clicked()
{
	if (AFRPlayerController* PlayerController = GetFortRoguePlayerController(*this))
	{
		PlayerController->UseHUDItem(2);
	}
}

void UFRMainGameHUDWidget::HandleItemSlot4Clicked()
{
	if (AFRPlayerController* PlayerController = GetFortRoguePlayerController(*this))
	{
		PlayerController->UseHUDItem(3);
	}
}

void UFRMainGameHUDWidget::HandleItemSlot5Clicked()
{
	if (AFRPlayerController* PlayerController = GetFortRoguePlayerController(*this))
	{
		PlayerController->UseHUDItem(4);
	}
}
