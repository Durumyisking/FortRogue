// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonUserWidget.h"
#include "MVVMViewModelBase.h"
#include "FRUIRootWidget.generated.h"

class UCommonActivatableWidget;
class UCommonActivatableWidgetStack;

UENUM(BlueprintType)
enum class EFRUILayer : uint8
{
	HUD,
	Menu,
	Modal
};

UCLASS(BlueprintType, Blueprintable)
class FORTROGUE_API UFRUIRootViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|Root")
	const FText& GetHUDLayerText() const { return HUDLayerText; }
	void SetHUDLayerText(const FText& InHUDLayerText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|Root")
	const FText& GetMenuLayerText() const { return MenuLayerText; }
	void SetMenuLayerText(const FText& InMenuLayerText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|Root")
	const FText& GetModalLayerText() const { return ModalLayerText; }
	void SetModalLayerText(const FText& InModalLayerText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|Root")
	bool HasHUDLayerWidget() const { return bHasHUDLayerWidget; }
	void SetHasHUDLayerWidget(bool bInHasHUDLayerWidget);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|Root")
	bool HasMenuLayerWidget() const { return bHasMenuLayerWidget; }
	void SetHasMenuLayerWidget(bool bInHasMenuLayerWidget);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|Root")
	bool HasModalLayerWidget() const { return bHasModalLayerWidget; }
	void SetHasModalLayerWidget(bool bInHasModalLayerWidget);

private:
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|Root", meta = (AllowPrivateAccess = "true"))
	FText HUDLayerText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|Root", meta = (AllowPrivateAccess = "true"))
	FText MenuLayerText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|Root", meta = (AllowPrivateAccess = "true"))
	FText ModalLayerText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|Root", meta = (AllowPrivateAccess = "true"))
	bool bHasHUDLayerWidget = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|Root", meta = (AllowPrivateAccess = "true"))
	bool bHasMenuLayerWidget = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|Root", meta = (AllowPrivateAccess = "true"))
	bool bHasModalLayerWidget = false;
};

UCLASS(Blueprintable)
class FORTROGUE_API UFRUIRootWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|Root")
	UFRUIRootViewModel* GetUIRootViewModel() const { return UIRootViewModel; }

	UFUNCTION(BlueprintCallable, Category = "FortRogue|UI")
	UCommonActivatableWidget* SetWidgetInLayer(EFRUILayer Layer, TSubclassOf<UCommonActivatableWidget> WidgetClass);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|UI")
	UCommonActivatableWidget* PushWidgetToLayer(EFRUILayer Layer, TSubclassOf<UCommonActivatableWidget> WidgetClass);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|UI")
	void ClearLayer(EFRUILayer Layer);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|UI")
	bool RemoveWidgetFromLayer(UCommonActivatableWidget* Widget);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	UCommonActivatableWidget* GetActiveWidgetInLayer(EFRUILayer Layer) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	UCommonActivatableWidgetStack* GetLayerStack(EFRUILayer Layer) const;

protected:
	virtual void NativeOnInitialized() override;

private:
	void CreateViewModel();
	void RefreshLayerViewModel();

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonActivatableWidgetStack> HUDLayer;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonActivatableWidgetStack> MenuLayer;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonActivatableWidgetStack> ModalLayer;

	UPROPERTY(Transient)
	TObjectPtr<UFRUIRootViewModel> UIRootViewModel;
};
