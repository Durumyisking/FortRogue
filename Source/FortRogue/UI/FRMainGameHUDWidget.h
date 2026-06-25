// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "UI/FRPlayerHUDViewModel.h"
#include "FRMainGameHUDWidget.generated.h"

class AFRGameMode;
class UButton;
class UImage;
class UProgressBar;
class UTextBlock;
class UTexture2D;

UCLASS()
class FORTROGUE_API UFRMainGameHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|HUD")
	UFRPlayerHUDViewModel* GetPlayerViewModel() const;

protected:
	virtual void NativeOnInitialized() override;
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
	TObjectPtr<UButton> FireButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> WeaponSlot1Button;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> WeaponSlot2Button;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> WeaponSlot3Button;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> WeaponSlot4Button;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> WeaponSlot5Button;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WeaponSlot1Text;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WeaponSlot2Text;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WeaponSlot3Text;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WeaponSlot4Text;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WeaponSlot5Text;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WeaponSlot1IndexText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WeaponSlot2IndexText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WeaponSlot3IndexText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WeaponSlot4IndexText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WeaponSlot5IndexText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ItemSlot1Button;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ItemSlot2Button;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ItemSlot3Button;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ItemSlot4Button;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ItemSlot5Button;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ItemSlot1Text;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ItemSlot2Text;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ItemSlot3Text;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ItemSlot4Text;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ItemSlot5Text;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ItemSlot1ChargesText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ItemSlot2ChargesText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ItemSlot3ChargesText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ItemSlot4ChargesText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ItemSlot5ChargesText;

private:
	void BindButtonEvents();
	void RefreshHUD();
	void RefreshGameInfo(AFRGameMode* GameMode);
	void RefreshPlayerInfo(AFRGameMode* GameMode);
	void ApplyWeaponSlot(const FFRHUDLoadoutSlotViewData& SlotData, UButton* Button, UTextBlock* NameText, UTextBlock* IndexText);
	void ApplyItemSlot(const FFRHUDLoadoutSlotViewData& SlotData, UButton* Button, UTextBlock* NameText, UTextBlock* ChargesText);

	UFUNCTION()
	void HandleFirePressed();

	UFUNCTION()
	void HandleFireReleased();

	UFUNCTION()
	void HandleWeaponSlot1Clicked();

	UFUNCTION()
	void HandleWeaponSlot2Clicked();

	UFUNCTION()
	void HandleWeaponSlot3Clicked();

	UFUNCTION()
	void HandleWeaponSlot4Clicked();

	UFUNCTION()
	void HandleWeaponSlot5Clicked();

	UFUNCTION()
	void HandleItemSlot1Clicked();

	UFUNCTION()
	void HandleItemSlot2Clicked();

	UFUNCTION()
	void HandleItemSlot3Clicked();

	UFUNCTION()
	void HandleItemSlot4Clicked();

	UFUNCTION()
	void HandleItemSlot5Clicked();
};
