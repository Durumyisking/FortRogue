// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FRMenuWidgets.h"

#include "CommonButtonBase.h"
#include "CommonNumericTextBlock.h"
#include "CommonTextBlock.h"
#include "Components/CheckBox.h"
#include "Components/Slider.h"
#include "MVVMBlueprintLibrary.h"

namespace
{
	template <typename UserClass>
	void BindMenuButton(UCommonButtonBase* Button, UserClass* UserObject, void (UserClass::* Handler)())
	{
		if (Button)
		{
			Button->OnClicked().AddUObject(UserObject, Handler);
		}
	}

	void SetMenuText(UCommonTextBlock* TextBlock, const FText& Text)
	{
		if (TextBlock)
		{
			TextBlock->SetText(Text);
		}
	}

	void SetMenuNumber(UCommonNumericTextBlock* TextBlock, float Value)
	{
		if (TextBlock)
		{
			TextBlock->SetCurrentValue(Value);
		}
	}

	float MasterVolumeToSliderValue(float Percent)
	{
		return FMath::Clamp(Percent / 100.0f, 0.0f, 1.0f);
	}

	float SliderValueToMasterVolume(float NormalizedValue)
	{
		return FMath::Clamp(NormalizedValue, 0.0f, 1.0f) * 100.0f;
	}

	float UIScaleToSliderValue(float Percent)
	{
		return FMath::Clamp((Percent - 50.0f) / 150.0f, 0.0f, 1.0f);
	}

	float SliderValueToUIScale(float NormalizedValue)
	{
		return 50.0f + FMath::Clamp(NormalizedValue, 0.0f, 1.0f) * 150.0f;
	}

	bool IsOptionEnabledText(const FText& Text)
	{
		return !Text.ToString().Equals(TEXT("Off"), ESearchCase::IgnoreCase);
	}

	void ApplyMenuTextStyle(UCommonTextBlock* TextBlock, TSubclassOf<UCommonTextStyle> TextStyle)
	{
		if (TextBlock && TextStyle)
		{
			TextBlock->SetStyle(TextStyle);
		}
	}

	void ApplyMenuButtonStyle(UCommonButtonBase* Button, TSubclassOf<UCommonButtonStyle> ButtonStyle)
	{
		if (Button && ButtonStyle)
		{
			Button->SetStyle(ButtonStyle);
		}
	}

	void ApplyMenuTextStyles(const FFRMenuStyleSet& StyleSet, UCommonTextBlock* TitleText, UCommonTextBlock* BodyText, UCommonTextBlock* StatusText)
	{
		ApplyMenuTextStyle(TitleText, StyleSet.TitleTextStyle);
		ApplyMenuTextStyle(BodyText, StyleSet.BodyTextStyle);
		ApplyMenuTextStyle(StatusText, StyleSet.StatusTextStyle);
	}

	void ApplyMenuViewModel(UUserWidget* Widget, UMVVMViewModelBase* ViewModel)
	{
		if (!Widget || !ViewModel)
		{
			return;
		}

		TScriptInterface<INotifyFieldValueChanged> ViewModelInterface;
		ViewModelInterface.SetObject(ViewModel);
		ViewModelInterface.SetInterface(Cast<INotifyFieldValueChanged>(ViewModel));
		UMVVMBlueprintLibrary::SetViewModelByClass(Widget, ViewModelInterface);
	}
}

void UFRMenuScreenViewModel::SetTitleText(const FText& InTitleText)
{
	UE_MVVM_SET_PROPERTY_VALUE(TitleText, InTitleText);
}

void UFRMenuScreenViewModel::SetBodyText(const FText& InBodyText)
{
	UE_MVVM_SET_PROPERTY_VALUE(BodyText, InBodyText);
}

void UFRMenuScreenViewModel::SetStatusText(const FText& InStatusText)
{
	UE_MVVM_SET_PROPERTY_VALUE(StatusText, InStatusText);
}

void UFROptionsMenuViewModel::SetTitleText(const FText& InTitleText)
{
	UE_MVVM_SET_PROPERTY_VALUE(TitleText, InTitleText);
}

void UFROptionsMenuViewModel::SetMasterVolumePercent(float InMasterVolumePercent)
{
	UE_MVVM_SET_PROPERTY_VALUE(MasterVolumePercent, InMasterVolumePercent);
}

void UFROptionsMenuViewModel::SetUIScalePercent(float InUIScalePercent)
{
	UE_MVVM_SET_PROPERTY_VALUE(UIScalePercent, InUIScalePercent);
}

void UFROptionsMenuViewModel::SetWindowModeText(const FText& InWindowModeText)
{
	UE_MVVM_SET_PROPERTY_VALUE(WindowModeText, InWindowModeText);
}

void UFROptionsMenuViewModel::SetResolutionText(const FText& InResolutionText)
{
	UE_MVVM_SET_PROPERTY_VALUE(ResolutionText, InResolutionText);
}

void UFROptionsMenuViewModel::SetInputHintsText(const FText& InInputHintsText)
{
	UE_MVVM_SET_PROPERTY_VALUE(InputHintsText, InInputHintsText);
}

void UFROptionsMenuViewModel::SetAccessibilityText(const FText& InAccessibilityText)
{
	UE_MVVM_SET_PROPERTY_VALUE(AccessibilityText, InAccessibilityText);
}

void UFRMainMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	CreateMenuScreenViewModel();
	ApplyMenuViewModel(this, MenuScreenViewModel);
	RefreshMainMenu();
	ApplyMenuStyleSet();

	BindMenuButton(StartRunButton, this, &UFRMainMenuWidget::HandleStartRunClicked);
	BindMenuButton(OptionsButton, this, &UFRMainMenuWidget::HandleOptionsClicked);
	BindMenuButton(QuitButton, this, &UFRMainMenuWidget::HandleQuitClicked);
}

void UFRMainMenuWidget::RefreshMainMenu()
{
	RefreshMenuViewModel();
	RefreshFromMenuViewModel();
}

void UFRMainMenuWidget::CreateMenuScreenViewModel()
{
	if (!MenuScreenViewModel)
	{
		MenuScreenViewModel = NewObject<UFRMenuScreenViewModel>(this);
	}
}

void UFRMainMenuWidget::RefreshMenuViewModel()
{
	CreateMenuScreenViewModel();
	if (!MenuScreenViewModel)
	{
		return;
	}

	MenuScreenViewModel->SetTitleText(DefaultTitleText);
	MenuScreenViewModel->SetBodyText(DefaultBodyText);
	MenuScreenViewModel->SetStatusText(DefaultStatusText);
}

void UFRMainMenuWidget::RefreshFromMenuViewModel()
{
	if (!MenuScreenViewModel)
	{
		return;
	}

	SetMenuText(TitleText, MenuScreenViewModel->GetTitleText());
	SetMenuText(BodyText, MenuScreenViewModel->GetBodyText());
	SetMenuText(StatusText, MenuScreenViewModel->GetStatusText());
}

void UFRMainMenuWidget::ApplyMenuStyleSet()
{
	ApplyMenuTextStyles(MenuStyleSet, TitleText, BodyText, StatusText);
	ApplyMenuButtonStyle(StartRunButton, MenuStyleSet.PrimaryButtonStyle);
	ApplyMenuButtonStyle(OptionsButton, MenuStyleSet.SecondaryButtonStyle);
	ApplyMenuButtonStyle(QuitButton, MenuStyleSet.SecondaryButtonStyle);
}

void UFRMainMenuWidget::HandleStartRunClicked()
{
	OnStartRunRequested.Broadcast();
}

void UFRMainMenuWidget::HandleOptionsClicked()
{
	OnOptionsRequested.Broadcast();
}

void UFRMainMenuWidget::HandleQuitClicked()
{
	OnQuitRequested.Broadcast();
}

void UFROptionsMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	CreateOptionsMenuViewModel();
	if (OptionsMenuViewModel)
	{
		ApplyMenuViewModel(this, OptionsMenuViewModel);
	}
	RefreshOptionsMenu();
	ApplyMenuStyleSet();

	BindMenuButton(ApplyButton, this, &UFROptionsMenuWidget::HandleApplyClicked);
	BindMenuButton(ResetButton, this, &UFROptionsMenuWidget::HandleResetClicked);
	BindMenuButton(BackButton, this, &UFROptionsMenuWidget::HandleBackClicked);
	if (MasterVolumeSlider)
	{
		MasterVolumeSlider->OnValueChanged.AddDynamic(this, &UFROptionsMenuWidget::HandleMasterVolumeChanged);
	}
	if (UIScaleSlider)
	{
		UIScaleSlider->OnValueChanged.AddDynamic(this, &UFROptionsMenuWidget::HandleUIScaleChanged);
	}
	if (InputHintsCheckBox)
	{
		InputHintsCheckBox->OnCheckStateChanged.AddDynamic(this, &UFROptionsMenuWidget::HandleInputHintsChanged);
	}
}

void UFROptionsMenuWidget::RefreshOptionsMenu()
{
	RefreshOptionsViewModel();
	RefreshFromViewModel();
}

void UFROptionsMenuWidget::CreateOptionsMenuViewModel()
{
	if (!OptionsMenuViewModel)
	{
		OptionsMenuViewModel = NewObject<UFROptionsMenuViewModel>(this);
	}
}

void UFROptionsMenuWidget::RefreshOptionsViewModel()
{
	CreateOptionsMenuViewModel();
	if (!OptionsMenuViewModel)
	{
		return;
	}

	OptionsMenuViewModel->SetTitleText(DefaultTitleText);
	OptionsMenuViewModel->SetMasterVolumePercent(DefaultMasterVolumePercent);
	OptionsMenuViewModel->SetUIScalePercent(DefaultUIScalePercent);
	OptionsMenuViewModel->SetWindowModeText(DefaultWindowModeText);
	OptionsMenuViewModel->SetResolutionText(DefaultResolutionText);
	OptionsMenuViewModel->SetInputHintsText(DefaultInputHintsText);
	OptionsMenuViewModel->SetAccessibilityText(DefaultAccessibilityText);
}

void UFROptionsMenuWidget::RefreshFromViewModel()
{
	if (!OptionsMenuViewModel)
	{
		return;
	}

	SetMenuText(TitleText, OptionsMenuViewModel->GetTitleText());
	SetMenuNumber(MasterVolumeText, OptionsMenuViewModel->GetMasterVolumePercent());
	SetMenuNumber(UIScaleText, OptionsMenuViewModel->GetUIScalePercent());
	SetMenuText(WindowModeText, OptionsMenuViewModel->GetWindowModeText());
	SetMenuText(ResolutionText, OptionsMenuViewModel->GetResolutionText());
	SetMenuText(InputHintsText, OptionsMenuViewModel->GetInputHintsText());
	SetMenuText(AccessibilityText, OptionsMenuViewModel->GetAccessibilityText());

	bUpdatingOptionControls = true;
	if (MasterVolumeSlider)
	{
		MasterVolumeSlider->SetValue(MasterVolumeToSliderValue(OptionsMenuViewModel->GetMasterVolumePercent()));
	}
	if (UIScaleSlider)
	{
		UIScaleSlider->SetValue(UIScaleToSliderValue(OptionsMenuViewModel->GetUIScalePercent()));
	}
	if (InputHintsCheckBox)
	{
		InputHintsCheckBox->SetIsChecked(IsOptionEnabledText(OptionsMenuViewModel->GetInputHintsText()));
	}
	bUpdatingOptionControls = false;
}

void UFROptionsMenuWidget::ApplyMenuStyleSet()
{
	ApplyMenuTextStyles(MenuStyleSet, TitleText, nullptr, nullptr);
	ApplyMenuButtonStyle(ApplyButton, MenuStyleSet.PrimaryButtonStyle);
	ApplyMenuButtonStyle(ResetButton, MenuStyleSet.SecondaryButtonStyle);
	ApplyMenuButtonStyle(BackButton, MenuStyleSet.SecondaryButtonStyle);
}

void UFROptionsMenuWidget::HandleApplyClicked()
{
	OnApplyRequested.Broadcast();
}

void UFROptionsMenuWidget::HandleResetClicked()
{
	RefreshOptionsMenu();
	OnResetRequested.Broadcast();
}

void UFROptionsMenuWidget::HandleBackClicked()
{
	OnBackRequested.Broadcast();
}

void UFROptionsMenuWidget::HandleMasterVolumeChanged(float NormalizedValue)
{
	if (bUpdatingOptionControls)
	{
		return;
	}

	CreateOptionsMenuViewModel();
	if (OptionsMenuViewModel)
	{
		OptionsMenuViewModel->SetMasterVolumePercent(SliderValueToMasterVolume(NormalizedValue));
		SetMenuNumber(MasterVolumeText, OptionsMenuViewModel->GetMasterVolumePercent());
	}
}

void UFROptionsMenuWidget::HandleUIScaleChanged(float NormalizedValue)
{
	if (bUpdatingOptionControls)
	{
		return;
	}

	CreateOptionsMenuViewModel();
	if (OptionsMenuViewModel)
	{
		OptionsMenuViewModel->SetUIScalePercent(SliderValueToUIScale(NormalizedValue));
		SetMenuNumber(UIScaleText, OptionsMenuViewModel->GetUIScalePercent());
	}
}

void UFROptionsMenuWidget::HandleInputHintsChanged(bool bIsChecked)
{
	if (bUpdatingOptionControls)
	{
		return;
	}

	CreateOptionsMenuViewModel();
	if (OptionsMenuViewModel)
	{
		OptionsMenuViewModel->SetInputHintsText(FText::FromString(bIsChecked ? TEXT("On") : TEXT("Off")));
		SetMenuText(InputHintsText, OptionsMenuViewModel->GetInputHintsText());
	}
}

void UFRPauseMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	CreateMenuScreenViewModel();
	ApplyMenuViewModel(this, MenuScreenViewModel);
	RefreshPauseMenu();
	ApplyMenuStyleSet();

	BindMenuButton(ResumeButton, this, &UFRPauseMenuWidget::HandleResumeClicked);
	BindMenuButton(OptionsButton, this, &UFRPauseMenuWidget::HandleOptionsClicked);
	BindMenuButton(RestartButton, this, &UFRPauseMenuWidget::HandleRestartClicked);
	BindMenuButton(MainMenuButton, this, &UFRPauseMenuWidget::HandleMainMenuClicked);
	BindMenuButton(QuitButton, this, &UFRPauseMenuWidget::HandleQuitClicked);
}

void UFRPauseMenuWidget::RefreshPauseMenu()
{
	RefreshMenuViewModel();
	RefreshFromMenuViewModel();
}

void UFRPauseMenuWidget::CreateMenuScreenViewModel()
{
	if (!MenuScreenViewModel)
	{
		MenuScreenViewModel = NewObject<UFRMenuScreenViewModel>(this);
	}
}

void UFRPauseMenuWidget::RefreshMenuViewModel()
{
	CreateMenuScreenViewModel();
	if (!MenuScreenViewModel)
	{
		return;
	}

	MenuScreenViewModel->SetTitleText(DefaultTitleText);
	MenuScreenViewModel->SetBodyText(DefaultBodyText);
	MenuScreenViewModel->SetStatusText(DefaultStatusText);
}

void UFRPauseMenuWidget::RefreshFromMenuViewModel()
{
	if (!MenuScreenViewModel)
	{
		return;
	}

	SetMenuText(TitleText, MenuScreenViewModel->GetTitleText());
	SetMenuText(BodyText, MenuScreenViewModel->GetBodyText());
	SetMenuText(StatusText, MenuScreenViewModel->GetStatusText());
}

void UFRPauseMenuWidget::ApplyMenuStyleSet()
{
	ApplyMenuTextStyles(MenuStyleSet, TitleText, BodyText, StatusText);
	ApplyMenuButtonStyle(ResumeButton, MenuStyleSet.PrimaryButtonStyle);
	ApplyMenuButtonStyle(OptionsButton, MenuStyleSet.SecondaryButtonStyle);
	ApplyMenuButtonStyle(RestartButton, MenuStyleSet.SecondaryButtonStyle);
	ApplyMenuButtonStyle(MainMenuButton, MenuStyleSet.SecondaryButtonStyle);
	ApplyMenuButtonStyle(QuitButton, MenuStyleSet.SecondaryButtonStyle);
}

void UFRPauseMenuWidget::HandleResumeClicked()
{
	OnResumeRequested.Broadcast();
}

void UFRPauseMenuWidget::HandleOptionsClicked()
{
	OnOptionsRequested.Broadcast();
}

void UFRPauseMenuWidget::HandleRestartClicked()
{
	OnRestartRequested.Broadcast();
}

void UFRPauseMenuWidget::HandleMainMenuClicked()
{
	OnMainMenuRequested.Broadcast();
}

void UFRPauseMenuWidget::HandleQuitClicked()
{
	OnQuitRequested.Broadcast();
}

void UFRConfirmDialogWidget::SetDialogText(const FText& InTitleText, const FText& InMessageText)
{
	CreateDialogViewModel();
	if (DialogViewModel)
	{
		DialogViewModel->SetTitleText(InTitleText);
		DialogViewModel->SetBodyText(InMessageText);
		DialogViewModel->SetStatusText(FText::GetEmpty());
	}
	RefreshFromDialogViewModel();
}

void UFRConfirmDialogWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	CreateDialogViewModel();
	ApplyMenuViewModel(this, DialogViewModel);
	RefreshFromDialogViewModel();
	ApplyMenuStyleSet();

	BindMenuButton(ConfirmButton, this, &UFRConfirmDialogWidget::HandleConfirmClicked);
	BindMenuButton(CancelButton, this, &UFRConfirmDialogWidget::HandleCancelClicked);
}

void UFRConfirmDialogWidget::CreateDialogViewModel()
{
	if (!DialogViewModel)
	{
		DialogViewModel = NewObject<UFRMenuScreenViewModel>(this);
	}
}

void UFRConfirmDialogWidget::RefreshFromDialogViewModel()
{
	if (!DialogViewModel)
	{
		return;
	}

	SetMenuText(TitleText, DialogViewModel->GetTitleText());
	SetMenuText(MessageText, DialogViewModel->GetBodyText());
}

void UFRConfirmDialogWidget::ApplyMenuStyleSet()
{
	ApplyMenuTextStyles(MenuStyleSet, TitleText, MessageText, nullptr);
	ApplyMenuButtonStyle(ConfirmButton, MenuStyleSet.PrimaryButtonStyle);
	ApplyMenuButtonStyle(CancelButton, MenuStyleSet.SecondaryButtonStyle);
}

void UFRConfirmDialogWidget::HandleConfirmClicked()
{
	OnConfirmRequested.Broadcast();
}

void UFRConfirmDialogWidget::HandleCancelClicked()
{
	OnCancelRequested.Broadcast();
}
