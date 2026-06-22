// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonUserWidget.h"
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

UCLASS(Blueprintable)
class FORTROGUE_API UFRUIRootWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
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

private:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonActivatableWidgetStack> HUDLayer;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonActivatableWidgetStack> MenuLayer;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonActivatableWidgetStack> ModalLayer;
};
