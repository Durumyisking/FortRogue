// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FRMenuWidgets.h"

#include "CommonButtonBase.h"
#include "CommonTextBlock.h"

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

	BindMenuButton(ApplyButton, this, &UFROptionsMenuWidget::HandleApplyClicked);
	BindMenuButton(ResetButton, this, &UFROptionsMenuWidget::HandleResetClicked);
	BindMenuButton(BackButton, this, &UFROptionsMenuWidget::HandleBackClicked);
}

void UFROptionsMenuWidget::HandleApplyClicked()
{
	OnApplyRequested.Broadcast();
}

void UFROptionsMenuWidget::HandleResetClicked()
{
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
