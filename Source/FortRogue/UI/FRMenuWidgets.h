// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonActivatableWidget.h"
#include "MVVMViewModelBase.h"
#include "FRMenuWidgets.generated.h"

class UCommonButtonBase;
class UCommonButtonStyle;
class UCommonNumericTextBlock;
class UCommonTextBlock;
class UCommonTextStyle;
class UCheckBox;
class USlider;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFRMenuActionRequested);

USTRUCT(BlueprintType)
struct FFRMenuStyleSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FortRogue|UI|Style")
	TSubclassOf<UCommonTextStyle> TitleTextStyle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FortRogue|UI|Style")
	TSubclassOf<UCommonTextStyle> BodyTextStyle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FortRogue|UI|Style")
	TSubclassOf<UCommonTextStyle> StatusTextStyle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FortRogue|UI|Style")
	TSubclassOf<UCommonButtonStyle> PrimaryButtonStyle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FortRogue|UI|Style")
	TSubclassOf<UCommonButtonStyle> SecondaryButtonStyle;
};

UCLASS(BlueprintType, Blueprintable)
class FORTROGUE_API UFRMenuScreenViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|Menu")
	const FText& GetTitleText() const { return TitleText; }
	void SetTitleText(const FText& InTitleText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|Menu")
	const FText& GetBodyText() const { return BodyText; }
	void SetBodyText(const FText& InBodyText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|Menu")
	const FText& GetStatusText() const { return StatusText; }
	void SetStatusText(const FText& InStatusText);

private:
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|Menu", meta = (AllowPrivateAccess = "true"))
	FText TitleText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|Menu", meta = (AllowPrivateAccess = "true"))
	FText BodyText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|Menu", meta = (AllowPrivateAccess = "true"))
	FText StatusText;
};

UCLASS(BlueprintType, Blueprintable)
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
	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|Menu")
	UFRMenuScreenViewModel* GetMenuScreenViewModel() const { return MenuScreenViewModel; }

	UFUNCTION(BlueprintCallable, Category = "FortRogue|UI|Menu")
	void RefreshMainMenu();

	UPROPERTY(BlueprintAssignable, Category = "FortRogue|UI|Menu")
	FFRMenuActionRequested OnStartRunRequested;

	UPROPERTY(BlueprintAssignable, Category = "FortRogue|UI|Menu")
	FFRMenuActionRequested OnOptionsRequested;

	UPROPERTY(BlueprintAssignable, Category = "FortRogue|UI|Menu")
	FFRMenuActionRequested OnQuitRequested;

protected:
	virtual void NativeOnInitialized() override;

private:
	void CreateMenuScreenViewModel();
	void RefreshMenuViewModel();
	void RefreshFromMenuViewModel();
	void ApplyMenuStyleSet();
	void HandleStartRunClicked();
	void HandleOptionsClicked();
	void HandleQuitClicked();

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Menu")
	FText DefaultTitleText = FText::FromString(TEXT("FortRogue"));

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Menu")
	FText DefaultBodyText = FText::FromString(TEXT("Tank tactics. One clean shot at a time."));

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Menu")
	FText DefaultStatusText = FText::GetEmpty();

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Style")
	FFRMenuStyleSet MenuStyleSet;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> TitleText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> BodyText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> StatusText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> StartRunButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> OptionsButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> QuitButton;

	UPROPERTY(Transient)
	TObjectPtr<UFRMenuScreenViewModel> MenuScreenViewModel;
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
	void ApplyMenuStyleSet();
	void HandleApplyClicked();
	void HandleResetClicked();
	void HandleBackClicked();

	UFUNCTION()
	void HandleMasterVolumeChanged(float NormalizedValue);

	UFUNCTION()
	void HandleUIScaleChanged(float NormalizedValue);

	UFUNCTION()
	void HandleInputHintsChanged(bool bIsChecked);

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

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Style")
	FFRMenuStyleSet MenuStyleSet;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> TitleText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonNumericTextBlock> MasterVolumeText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USlider> MasterVolumeSlider;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonNumericTextBlock> UIScaleText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USlider> UIScaleSlider;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> WindowModeText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> ResolutionText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> InputHintsText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCheckBox> InputHintsCheckBox;

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

	bool bUpdatingOptionControls = false;
};

UCLASS(Abstract, Blueprintable)
class FORTROGUE_API UFRPauseMenuWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|Menu")
	UFRMenuScreenViewModel* GetMenuScreenViewModel() const { return MenuScreenViewModel; }

	UFUNCTION(BlueprintCallable, Category = "FortRogue|UI|Menu")
	void RefreshPauseMenu();

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
	void CreateMenuScreenViewModel();
	void RefreshMenuViewModel();
	void RefreshFromMenuViewModel();
	void ApplyMenuStyleSet();
	void HandleResumeClicked();
	void HandleOptionsClicked();
	void HandleRestartClicked();
	void HandleMainMenuClicked();
	void HandleQuitClicked();

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Menu")
	FText DefaultTitleText = FText::FromString(TEXT("Paused"));

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Menu")
	FText DefaultBodyText = FText::FromString(TEXT("Run suspended."));

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Menu")
	FText DefaultStatusText = FText::GetEmpty();

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Style")
	FFRMenuStyleSet MenuStyleSet;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> TitleText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> BodyText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> StatusText;

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

	UPROPERTY(Transient)
	TObjectPtr<UFRMenuScreenViewModel> MenuScreenViewModel;
};

UCLASS(Abstract, Blueprintable)
class FORTROGUE_API UFRConfirmDialogWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|Dialog")
	UFRMenuScreenViewModel* GetDialogViewModel() const { return DialogViewModel; }

	UFUNCTION(BlueprintCallable, Category = "FortRogue|UI|Dialog")
	void SetDialogText(const FText& InTitleText, const FText& InMessageText);

	UPROPERTY(BlueprintAssignable, Category = "FortRogue|UI|Dialog")
	FFRMenuActionRequested OnConfirmRequested;

	UPROPERTY(BlueprintAssignable, Category = "FortRogue|UI|Dialog")
	FFRMenuActionRequested OnCancelRequested;

protected:
	virtual void NativeOnInitialized() override;

private:
	void CreateDialogViewModel();
	void RefreshFromDialogViewModel();
	void ApplyMenuStyleSet();
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

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Style")
	FFRMenuStyleSet MenuStyleSet;

	UPROPERTY(Transient)
	TObjectPtr<UFRMenuScreenViewModel> DialogViewModel;
};
