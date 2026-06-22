// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FRMenuWidgets.h"

#include "CommonButtonBase.h"
#include "CommonNumericTextBlock.h"
#include "CommonTextBlock.h"
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

	BindMenuButton(StartRunButton, this, &UFRMainMenuWidget::HandleStartRunClicked);
	BindMenuButton(OptionsButton, this, &UFRMainMenuWidget::HandleOptionsClicked);
	BindMenuButton(QuitButton, this, &UFRMainMenuWidget::HandleQuitClicked);
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
		TScriptInterface<INotifyFieldValueChanged> ViewModelInterface;
		ViewModelInterface.SetObject(OptionsMenuViewModel);
		ViewModelInterface.SetInterface(Cast<INotifyFieldValueChanged>(OptionsMenuViewModel));
		UMVVMBlueprintLibrary::SetViewModelByClass(this, ViewModelInterface);
	}
	RefreshOptionsMenu();

	BindMenuButton(ApplyButton, this, &UFROptionsMenuWidget::HandleApplyClicked);
	BindMenuButton(ResetButton, this, &UFROptionsMenuWidget::HandleResetClicked);
	BindMenuButton(BackButton, this, &UFROptionsMenuWidget::HandleBackClicked);
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

void UFRPauseMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	BindMenuButton(ResumeButton, this, &UFRPauseMenuWidget::HandleResumeClicked);
	BindMenuButton(OptionsButton, this, &UFRPauseMenuWidget::HandleOptionsClicked);
	BindMenuButton(RestartButton, this, &UFRPauseMenuWidget::HandleRestartClicked);
	BindMenuButton(MainMenuButton, this, &UFRPauseMenuWidget::HandleMainMenuClicked);
	BindMenuButton(QuitButton, this, &UFRPauseMenuWidget::HandleQuitClicked);
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
	SetMenuText(TitleText, InTitleText);
	SetMenuText(MessageText, InMessageText);
}

void UFRConfirmDialogWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	BindMenuButton(ConfirmButton, this, &UFRConfirmDialogWidget::HandleConfirmClicked);
	BindMenuButton(CancelButton, this, &UFRConfirmDialogWidget::HandleCancelClicked);
}

void UFRConfirmDialogWidget::HandleConfirmClicked()
{
	OnConfirmRequested.Broadcast();
}

void UFRConfirmDialogWidget::HandleCancelClicked()
{
	OnCancelRequested.Broadcast();
}
