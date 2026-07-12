// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "UI/FRPlayerHUDViewModel.h"
#include "FRMainGameHUDWidget.generated.h"

class AFRGameMode;
class UFRCommonTextButton;
class UFRLoadoutSlotButton;
class UFRRewardChoiceButton;
class UImage;
class UProgressBar;
class UTextBlock;
class UTexture2D;
class UWidget;

/**
 * 전투 HUD 위젯입니다. 무기/아이템 슬롯과 발사·보상 버튼은 CommonUI 기반
 * (UFRLoadoutSlotButton / UFRCommonTextButton / UFRRewardChoiceButton)이며,
 * 슬롯 내부 텍스트는 각 버튼 위젯이 소유합니다.
 */
UCLASS()
class FORTROGUE_API UFRMainGameHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|HUD")
	UFRPlayerHUDViewModel* GetPlayerViewModel() const;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(BlueprintReadOnly, Category = "FortRogue|HUD")
	TObjectPtr<UFRPlayerHUDViewModel> PlayerViewModel;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FortRogue|HUD")
	TObjectPtr<UTexture2D> WindArrowTexture;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TurnText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ShotTimerText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WindText;

	UPROPERTY(Transient)
	TObjectPtr<UImage> WindArrowVisualWidget;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PlayerNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> PlayerHPBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PlayerHPText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> ShotPowerBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ShotPowerText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> MoveBudgetBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MoveBudgetText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> AimText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CurrentWeaponText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CurrentShotText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UFRCommonTextButton> FireButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UFRLoadoutSlotButton> WeaponSlot1Button;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UFRLoadoutSlotButton> WeaponSlot2Button;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UFRLoadoutSlotButton> WeaponSlot3Button;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UFRLoadoutSlotButton> WeaponSlot4Button;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UFRLoadoutSlotButton> WeaponSlot5Button;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UFRLoadoutSlotButton> ItemSlot1Button;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UFRLoadoutSlotButton> ItemSlot2Button;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UFRLoadoutSlotButton> ItemSlot3Button;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UFRLoadoutSlotButton> ItemSlot4Button;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UFRLoadoutSlotButton> ItemSlot5Button;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> RewardPanel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UFRRewardChoiceButton> RewardChoice1Button;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UFRRewardChoiceButton> RewardChoice2Button;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UFRRewardChoiceButton> RewardChoice3Button;

private:
	void BindButtonEvents();
	void RefreshHUD();
	void RefreshGameInfo(AFRGameMode* GameMode);
	void RefreshRewardInfo(AFRGameMode* GameMode);
	void RefreshPlayerInfo(AFRGameMode* GameMode);
	void ApplyRewardChoice(int32 ChoiceIndex);
	void SetRewardInputMode(bool bActive);
	void ApplyLoadoutSlot(const FFRHUDLoadoutSlotViewData& SlotData, UFRLoadoutSlotButton* SlotButton, bool bShowChargesAsHotkey);
	void HandleFirePressed();
	void HandleFireReleased();
	void HandleWeaponSlotClicked(int32 SlotIndex);
	void HandleItemSlotClicked(int32 SlotIndex);

	bool bRewardInputModeActive = false;
};
