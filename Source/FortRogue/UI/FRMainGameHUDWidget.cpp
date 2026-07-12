// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FRMainGameHUDWidget.h"

#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "FRGameMode.h"
#include "FRPlayerController.h"
#include "Game/FRTurnBasedGameState.h"
#include "UI/FRCommonButtonWidgets.h"

namespace
{
constexpr const TCHAR* WindArrowTexturePath = TEXT("/Game/FortRogue/Widget/MainGame/T_WindArrow.T_WindArrow");

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
	WindArrowVisualWidget = Cast<UImage>(GetWidgetFromName(TEXT("WindArrowVisual")));
	BindButtonEvents();
}

void UFRMainGameHUDWidget::NativeDestruct()
{
	SetRewardInputMode(false);
	Super::NativeDestruct();
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
		FireButton->OnPressed().AddUObject(this, &UFRMainGameHUDWidget::HandleFirePressed);
		FireButton->OnReleased().AddUObject(this, &UFRMainGameHUDWidget::HandleFireReleased);
	}

	UFRLoadoutSlotButton* WeaponSlotButtons[] = { WeaponSlot1Button, WeaponSlot2Button, WeaponSlot3Button, WeaponSlot4Button, WeaponSlot5Button };
	for (int32 SlotIndex = 0; SlotIndex < UE_ARRAY_COUNT(WeaponSlotButtons); ++SlotIndex)
	{
		if (WeaponSlotButtons[SlotIndex])
		{
			WeaponSlotButtons[SlotIndex]->OnClicked().AddWeakLambda(this, [this, SlotIndex]()
			{
				HandleWeaponSlotClicked(SlotIndex);
			});
		}
	}

	UFRLoadoutSlotButton* ItemSlotButtons[] = { ItemSlot1Button, ItemSlot2Button, ItemSlot3Button, ItemSlot4Button, ItemSlot5Button };
	for (int32 SlotIndex = 0; SlotIndex < UE_ARRAY_COUNT(ItemSlotButtons); ++SlotIndex)
	{
		if (ItemSlotButtons[SlotIndex])
		{
			ItemSlotButtons[SlotIndex]->OnClicked().AddWeakLambda(this, [this, SlotIndex]()
			{
				HandleItemSlotClicked(SlotIndex);
			});
		}
	}

	UFRRewardChoiceButton* RewardButtons[] = { RewardChoice1Button, RewardChoice2Button, RewardChoice3Button };
	for (int32 ChoiceIndex = 0; ChoiceIndex < UE_ARRAY_COUNT(RewardButtons); ++ChoiceIndex)
	{
		if (RewardButtons[ChoiceIndex])
		{
			RewardButtons[ChoiceIndex]->OnClicked().AddWeakLambda(this, [this, ChoiceIndex]()
			{
				ApplyRewardChoice(ChoiceIndex);
			});
		}
	}
}

void UFRMainGameHUDWidget::RefreshHUD()
{
	AFRGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFRGameMode>() : nullptr;
	RefreshGameInfo(GameMode);
	RefreshRewardInfo(GameMode);
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
		if (WindArrowVisualWidget)
		{
			WindArrowVisualWidget->SetVisibility(ESlateVisibility::Hidden);
		}
		return;
	}

	SetTextBlockText(TurnText, GameMode->GetRunProgressSummary());
	SetTextBlockText(ShotTimerText, GetBattleStateText(GameMode->GetBattleState()));
	SetTextBlockText(StatusText, GameMode->GetStatusText());
	SetTextBlockText(WindText, GameMode->GetWindSummary());
	if (!WindArrowVisualWidget)
	{
		WindArrowVisualWidget = Cast<UImage>(GetWidgetFromName(TEXT("WindArrowVisual")));
	}
	if (WindArrowVisualWidget)
	{
		if (!WindArrowTexture)
		{
			WindArrowTexture = LoadObject<UTexture2D>(nullptr, WindArrowTexturePath);
		}
		if (WindArrowTexture)
		{
			WindArrowVisualWidget->SetBrushFromTexture(WindArrowTexture, true);
		}

		const float Wind = GameMode->GetWind();
		WindArrowVisualWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		WindArrowVisualWidget->SetRenderScale(FVector2D(Wind < 0.0f ? -1.0f : 1.0f, 1.0f));
		WindArrowVisualWidget->SetRenderOpacity(FMath::IsNearlyZero(Wind) ? 0.25f : 0.9f);
	}
}

void UFRMainGameHUDWidget::RefreshRewardInfo(AFRGameMode* GameMode)
{
	const bool bShowRewards = GameMode
		&& GameMode->GetBattleState() == EFRBattleState::Reward
		&& GameMode->GetRewardChoiceCount() > 0;

	if (RewardPanel)
	{
		RewardPanel->SetVisibility(bShowRewards ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	UFRRewardChoiceButton* RewardButtons[] = { RewardChoice1Button, RewardChoice2Button, RewardChoice3Button };
	for (int32 ChoiceIndex = 0; ChoiceIndex < UE_ARRAY_COUNT(RewardButtons); ++ChoiceIndex)
	{
		UFRRewardChoiceButton* RewardButton = RewardButtons[ChoiceIndex];
		if (!RewardButton)
		{
			continue;
		}

		const bool bHasChoice = bShowRewards && ChoiceIndex < GameMode->GetRewardChoiceCount();
		RewardButton->SetVisibility(bHasChoice ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		if (bHasChoice)
		{
			RewardButton->SetIsInteractionEnabled(GameMode->CanApplyRewardChoice(ChoiceIndex));
			const FFRRewardChoice RewardChoice = GameMode->GetRewardChoice(ChoiceIndex);
			RewardButton->SetRewardDisplay(
				RewardChoice.GetResolvedDisplayName(),
				RewardChoice.Description.IsEmpty() ? GameMode->GetRewardChoiceSummary(ChoiceIndex) : RewardChoice.Description);
		}
		else
		{
			RewardButton->SetRewardDisplay(FText::GetEmpty(), FText::GetEmpty());
		}
	}

	SetRewardInputMode(bShowRewards);
}

void UFRMainGameHUDWidget::ApplyRewardChoice(int32 ChoiceIndex)
{
	AFRGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFRGameMode>() : nullptr;
	if (GameMode && GameMode->CanApplyRewardChoice(ChoiceIndex))
	{
		GameMode->ApplyRewardChoice(ChoiceIndex);
	}
}

void UFRMainGameHUDWidget::SetRewardInputMode(bool bActive)
{
	if (bRewardInputModeActive == bActive)
	{
		return;
	}

	bRewardInputModeActive = bActive;
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		PlayerController->bShowMouseCursor = bActive;
		if (bActive)
		{
			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(TakeWidget());
			InputMode.SetHideCursorDuringCapture(false);
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputMode);
		}
		else
		{
			PlayerController->SetInputMode(FInputModeGameOnly());
		}
	}
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
		FireButton->SetIsInteractionEnabled(PlayerViewModel->bCanFire);
	}

	ApplyLoadoutSlot(PlayerViewModel->GetWeaponSlot(0), WeaponSlot1Button, false);
	ApplyLoadoutSlot(PlayerViewModel->GetWeaponSlot(1), WeaponSlot2Button, false);
	ApplyLoadoutSlot(PlayerViewModel->GetWeaponSlot(2), WeaponSlot3Button, false);
	ApplyLoadoutSlot(PlayerViewModel->GetWeaponSlot(3), WeaponSlot4Button, false);
	ApplyLoadoutSlot(PlayerViewModel->GetWeaponSlot(4), WeaponSlot5Button, false);

	ApplyLoadoutSlot(PlayerViewModel->GetItemSlot(0), ItemSlot1Button, true);
	ApplyLoadoutSlot(PlayerViewModel->GetItemSlot(1), ItemSlot2Button, true);
	ApplyLoadoutSlot(PlayerViewModel->GetItemSlot(2), ItemSlot3Button, true);
	ApplyLoadoutSlot(PlayerViewModel->GetItemSlot(3), ItemSlot4Button, true);
	ApplyLoadoutSlot(PlayerViewModel->GetItemSlot(4), ItemSlot5Button, true);
}

void UFRMainGameHUDWidget::ApplyLoadoutSlot(const FFRHUDLoadoutSlotViewData& SlotData, UFRLoadoutSlotButton* SlotButton, bool bShowChargesAsHotkey)
{
	if (!SlotButton)
	{
		return;
	}

	SlotButton->SetIsInteractionEnabled(SlotData.bEnabled);
	const FText HotkeyText = bShowChargesAsHotkey
		? (SlotData.bOccupied ? FText::AsNumber(SlotData.Charges) : FText::GetEmpty())
		: SlotData.InputText;
	SlotButton->SetSlotDisplay(SlotData.bOccupied ? SlotData.DisplayName : FText::GetEmpty(), HotkeyText);
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

void UFRMainGameHUDWidget::HandleWeaponSlotClicked(int32 SlotIndex)
{
	if (AFRPlayerController* PlayerController = GetFortRoguePlayerController(*this))
	{
		PlayerController->SelectHUDWeapon(SlotIndex);
	}
}

void UFRMainGameHUDWidget::HandleItemSlotClicked(int32 SlotIndex)
{
	if (AFRPlayerController* PlayerController = GetFortRoguePlayerController(*this))
	{
		PlayerController->UseHUDItem(SlotIndex);
	}
}
