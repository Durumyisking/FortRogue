// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FRCharacterHealthBarWidget.h"

#include "Blueprint/WidgetTree.h"
#include "CommonNumericTextBlock.h"
#include "CommonTextBlock.h"
#include "Components/ProgressBar.h"
#include "MVVMBlueprintLibrary.h"

namespace
{
	void SetWorldHealthText(UCommonTextBlock* TextBlock, const FText& Text)
	{
		if (TextBlock)
		{
			TextBlock->SetText(Text);
		}
	}

	void SetWorldHealthNumber(UCommonNumericTextBlock* TextBlock, float Value)
	{
		if (TextBlock)
		{
			TextBlock->SetCurrentValue(Value);
		}
	}

	void ApplyWorldHealthTextStyle(UCommonTextBlock* TextBlock, TSubclassOf<UCommonTextStyle> TextStyle)
	{
		if (TextBlock && TextStyle)
		{
			TextBlock->SetStyle(TextStyle);
		}
	}
}

void UFRCharacterHealthBarViewModel::SetHealthLabelText(const FText& InHealthLabelText)
{
	UE_MVVM_SET_PROPERTY_VALUE(HealthLabelText, InHealthLabelText);
}

void UFRCharacterHealthBarViewModel::SetCurrentHealthValue(float InCurrentHealthValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(CurrentHealthValue, InCurrentHealthValue);
}

void UFRCharacterHealthBarViewModel::SetMaxHealthValue(float InMaxHealthValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(MaxHealthValue, InMaxHealthValue);
}

void UFRCharacterHealthBarViewModel::SetHealthPercent(float InHealthPercent)
{
	UE_MVVM_SET_PROPERTY_VALUE(HealthPercent, InHealthPercent);
}

void UFRCharacterHealthBarViewModel::SetEnemy(bool bInEnemy)
{
	UE_MVVM_SET_PROPERTY_VALUE(bEnemy, bInEnemy);
}

void UFRCharacterHealthBarWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	CreateViewModel();
	if (HealthBarViewModel)
	{
		TScriptInterface<INotifyFieldValueChanged> ViewModelInterface;
		ViewModelInterface.SetObject(HealthBarViewModel);
		ViewModelInterface.SetInterface(Cast<INotifyFieldValueChanged>(HealthBarViewModel));
		UMVVMBlueprintLibrary::SetViewModelByClass(this, ViewModelInterface);
	}

	if (!HealthBar && WidgetTree)
	{
		HealthBar = Cast<UProgressBar>(WidgetTree->FindWidget(TEXT("HealthBar")));
	}
	if (!HealthText && WidgetTree)
	{
		HealthText = Cast<UCommonTextBlock>(WidgetTree->FindWidget(TEXT("HealthText")));
	}
	if (!CurrentHealthValueText && WidgetTree)
	{
		CurrentHealthValueText = Cast<UCommonNumericTextBlock>(WidgetTree->FindWidget(TEXT("CurrentHealthValueText")));
	}
	if (!MaxHealthValueText && WidgetTree)
	{
		MaxHealthValueText = Cast<UCommonNumericTextBlock>(WidgetTree->FindWidget(TEXT("MaxHealthValueText")));
	}

	ApplyTextStyles();
	UpdateHealthBar();
}

void UFRCharacterHealthBarWidget::SetHealth(float CurrentHealth, float MaxHealth, bool bEnemy)
{
	CachedCurrentHealth = FMath::Max(0.0f, CurrentHealth);
	CachedMaxHealth = FMath::Max(1.0f, MaxHealth);
	bCachedEnemy = bEnemy;

	CreateViewModel();
	if (HealthBarViewModel)
	{
		HealthBarViewModel->SetHealthLabelText(HealthLabelText);
		HealthBarViewModel->SetCurrentHealthValue(CachedCurrentHealth);
		HealthBarViewModel->SetMaxHealthValue(CachedMaxHealth);
		HealthBarViewModel->SetHealthPercent(FMath::Clamp(CachedCurrentHealth / CachedMaxHealth, 0.0f, 1.0f));
		HealthBarViewModel->SetEnemy(bCachedEnemy);
	}

	UpdateHealthBar();
}

void UFRCharacterHealthBarWidget::CreateViewModel()
{
	if (!HealthBarViewModel)
	{
		HealthBarViewModel = NewObject<UFRCharacterHealthBarViewModel>(this);
	}
	if (HealthBarViewModel)
	{
		HealthBarViewModel->SetHealthLabelText(HealthLabelText);
		HealthBarViewModel->SetCurrentHealthValue(CachedCurrentHealth);
		HealthBarViewModel->SetMaxHealthValue(CachedMaxHealth);
		HealthBarViewModel->SetHealthPercent(FMath::Clamp(CachedCurrentHealth / CachedMaxHealth, 0.0f, 1.0f));
		HealthBarViewModel->SetEnemy(bCachedEnemy);
	}
}

void UFRCharacterHealthBarWidget::UpdateHealthBar()
{
	CreateViewModel();
	if (!HealthBarViewModel)
	{
		return;
	}

	if (HealthBar)
	{
		HealthBar->SetPercent(HealthBarViewModel->GetHealthPercent());
		HealthBar->SetFillColorAndOpacity(HealthBarViewModel->IsEnemy() ? EnemyFillColor : PlayerFillColor);
	}
	SetWorldHealthText(HealthText, HealthBarViewModel->GetHealthLabelText());
	SetWorldHealthNumber(CurrentHealthValueText, HealthBarViewModel->GetCurrentHealthValue());
	SetWorldHealthNumber(MaxHealthValueText, HealthBarViewModel->GetMaxHealthValue());
}

void UFRCharacterHealthBarWidget::ApplyTextStyles()
{
	ApplyWorldHealthTextStyle(HealthText, LabelTextStyle);
	ApplyWorldHealthTextStyle(CurrentHealthValueText, NumericTextStyle);
	ApplyWorldHealthTextStyle(MaxHealthValueText, NumericTextStyle);
}
