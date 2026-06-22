// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonActivatableWidget.h"
#include "MVVMViewModelBase.h"
#include "FRMenuWidgets.generated.h"

class UCommonButtonBase;
class UCommonNumericTextBlock;
class UCommonTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFRMenuActionRequested);

UCLASS(BlueprintType)
class FORTROGUE_API UFROptionsMenuViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|Options")
	const FText& GetTitleText() const { return TitleText; }
	void SetTitleText(const FText& InTitleText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|Options")
	float GetMasterVolumePercent() const { return MasterVolumePercent; }
	void SetMasterVolumePercent(float InMasterVolumePercent);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|Options")
	float GetUIScalePercent() const { return UIScalePercent; }
	void SetUIScalePercent(float InUIScalePercent);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|Options")
	const FText& GetWindowModeText() const { return WindowModeText; }
	void SetWindowModeText(const FText& InWindowModeText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|Options")
	const FText& GetResolutionText() const { return ResolutionText; }
	void SetResolutionText(const FText& InResolutionText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|Options")
	const FText& GetInputHintsText() const { return InputHintsText; }
	void SetInputHintsText(const FText& InInputHintsText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|Options")
	const FText& GetAccessibilityText() const { return AccessibilityText; }
	void SetAccessibilityText(const FText& InAccessibilityText);

private:
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|Options", meta = (AllowPrivateAccess = "true"))
	FText TitleText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|Options", meta = (AllowPrivateAccess = "true"))
	float MasterVolumePercent = 100.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|Options", meta = (AllowPrivateAccess = "true"))
	float UIScalePercent = 100.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|Options", meta = (AllowPrivateAccess = "true"))
	FText WindowModeText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|Options", meta = (AllowPrivateAccess = "true"))
	FText ResolutionText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|Options", meta = (AllowPrivateAccess = "true"))
	FText InputHintsText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|Options", meta = (AllowPrivateAccess = "true"))
	FText AccessibilityText;
};

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
	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|Options")
	UFROptionsMenuViewModel* GetOptionsMenuViewModel() const { return OptionsMenuViewModel; }

	UFUNCTION(BlueprintCallable, Category = "FortRogue|UI|Options")
	void RefreshOptionsMenu();

	UPROPERTY(BlueprintAssignable, Category = "FortRogue|UI|Menu")
	FFRMenuActionRequested OnApplyRequested;

	UPROPERTY(BlueprintAssignable, Category = "FortRogue|UI|Menu")
	FFRMenuActionRequested OnResetRequested;

	UPROPERTY(BlueprintAssignable, Category = "FortRogue|UI|Menu")
	FFRMenuActionRequested OnBackRequested;

protected:
	virtual void NativeOnInitialized() override;

private:
	void CreateOptionsMenuViewModel();
	void RefreshOptionsViewModel();
	void RefreshFromViewModel();
	void HandleApplyClicked();
	void HandleResetClicked();
	void HandleBackClicked();

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Options")
	FText DefaultTitleText = FText::FromString(TEXT("Options"));

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Options", meta = (ClampMin = "0", ClampMax = "100"))
	float DefaultMasterVolumePercent = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Options", meta = (ClampMin = "50", ClampMax = "200"))
	float DefaultUIScalePercent = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Options")
	FText DefaultWindowModeText = FText::FromString(TEXT("Windowed Fullscreen"));

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Options")
	FText DefaultResolutionText = FText::FromString(TEXT("Native"));

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Options")
	FText DefaultInputHintsText = FText::FromString(TEXT("On"));

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Options")
	FText DefaultAccessibilityText = FText::FromString(TEXT("Default"));

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> TitleText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonNumericTextBlock> MasterVolumeText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonNumericTextBlock> UIScaleText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> WindowModeText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> ResolutionText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> InputHintsText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> AccessibilityText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> ApplyButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> ResetButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> BackButton;

	UPROPERTY(Transient)
	TObjectPtr<UFROptionsMenuViewModel> OptionsMenuViewModel;
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
