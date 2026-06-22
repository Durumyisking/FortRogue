// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonActivatableWidget.h"
#include "FRMenuWidgets.generated.h"

class UCommonButtonBase;
class UCommonTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFRMenuActionRequested);

UCLASS(Abstract, Blueprintable)
class FORTROGUE_API UFRMainMenuWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "FortRogue|UI|Menu")
	FFRMenuActionRequested OnStartRunRequested;

	UPROPERTY(BlueprintAssignable, Category = "FortRogue|UI|Menu")
	FFRMenuActionRequested OnOptionsRequested;

	UPROPERTY(BlueprintAssignable, Category = "FortRogue|UI|Menu")
	FFRMenuActionRequested OnQuitRequested;

protected:
	virtual void NativeOnInitialized() override;

private:
	void HandleStartRunClicked();
	void HandleOptionsClicked();
	void HandleQuitClicked();

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> StartRunButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> OptionsButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> QuitButton;
};

UCLASS(Abstract, Blueprintable)
class FORTROGUE_API UFROptionsMenuWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "FortRogue|UI|Menu")
	FFRMenuActionRequested OnApplyRequested;

	UPROPERTY(BlueprintAssignable, Category = "FortRogue|UI|Menu")
	FFRMenuActionRequested OnResetRequested;

	UPROPERTY(BlueprintAssignable, Category = "FortRogue|UI|Menu")
	FFRMenuActionRequested OnBackRequested;

protected:
	virtual void NativeOnInitialized() override;

private:
	void HandleApplyClicked();
	void HandleResetClicked();
	void HandleBackClicked();

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> ApplyButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> ResetButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> BackButton;
};

UCLASS(Abstract, Blueprintable)
class FORTROGUE_API UFRPauseMenuWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "FortRogue|UI|Menu")
	FFRMenuActionRequested OnResumeRequested;

	UPROPERTY(BlueprintAssignable, Category = "FortRogue|UI|Menu")
	FFRMenuActionRequested OnOptionsRequested;

	UPROPERTY(BlueprintAssignable, Category = "FortRogue|UI|Menu")
	FFRMenuActionRequested OnRestartRequested;

	UPROPERTY(BlueprintAssignable, Category = "FortRogue|UI|Menu")
	FFRMenuActionRequested OnMainMenuRequested;

	UPROPERTY(BlueprintAssignable, Category = "FortRogue|UI|Menu")
	FFRMenuActionRequested OnQuitRequested;

protected:
	virtual void NativeOnInitialized() override;

private:
	void HandleResumeClicked();
	void HandleOptionsClicked();
	void HandleRestartClicked();
	void HandleMainMenuClicked();
	void HandleQuitClicked();

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> ResumeButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> OptionsButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> RestartButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> MainMenuButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> QuitButton;
};

UCLASS(Abstract, Blueprintable)
class FORTROGUE_API UFRConfirmDialogWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FortRogue|UI|Dialog")
	void SetDialogText(const FText& InTitleText, const FText& InMessageText);

	UPROPERTY(BlueprintAssignable, Category = "FortRogue|UI|Dialog")
	FFRMenuActionRequested OnConfirmRequested;

	UPROPERTY(BlueprintAssignable, Category = "FortRogue|UI|Dialog")
	FFRMenuActionRequested OnCancelRequested;

protected:
	virtual void NativeOnInitialized() override;

private:
	void HandleConfirmClicked();
	void HandleCancelClicked();

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> TitleText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> MessageText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> ConfirmButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> CancelButton;
};
