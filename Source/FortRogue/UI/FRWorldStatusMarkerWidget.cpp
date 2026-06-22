// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FRWorldStatusMarkerWidget.h"

#include "Blueprint/WidgetTree.h"
#include "CommonBorder.h"
#include "CommonTextBlock.h"
#include "MVVMBlueprintLibrary.h"

namespace
{
	void SetWorldMarkerText(UCommonTextBlock* TextBlock, const FText& Text)
	{
		if (TextBlock)
		{
			TextBlock->SetText(Text);
		}
	}

	void ApplyWorldMarkerTextStyle(UCommonTextBlock* TextBlock, TSubclassOf<UCommonTextStyle> TextStyle)
	{
		if (TextBlock && TextStyle)
		{
			TextBlock->SetStyle(TextStyle);
		}
	}

	void ApplyWorldMarkerBorderStyle(UCommonBorder* Border, TSubclassOf<UCommonBorderStyle> BorderStyle)
	{
		if (Border && BorderStyle)
		{
			Border->SetStyle(BorderStyle);
		}
	}
}

void UFRWorldStatusMarkerViewModel::SetMarkerText(const FText& InMarkerText)
{
	UE_MVVM_SET_PROPERTY_VALUE(MarkerText, InMarkerText);
}

void UFRWorldStatusMarkerViewModel::SetVisible(bool bInVisible)
{
	UE_MVVM_SET_PROPERTY_VALUE(bVisible, bInVisible);
}

void UFRWorldStatusMarkerViewModel::SetActiveTurn(bool bInActiveTurn)
{
	UE_MVVM_SET_PROPERTY_VALUE(bActiveTurn, bInActiveTurn);
}

void UFRWorldStatusMarkerViewModel::SetTarget(bool bInTarget)
{
	UE_MVVM_SET_PROPERTY_VALUE(bTarget, bInTarget);
}

void UFRWorldStatusMarkerViewModel::SetEnemy(bool bInEnemy)
{
	UE_MVVM_SET_PROPERTY_VALUE(bEnemy, bInEnemy);
}

void UFRWorldStatusMarkerWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	CreateViewModel();
	if (MarkerViewModel)
	{
		TScriptInterface<INotifyFieldValueChanged> ViewModelInterface;
		ViewModelInterface.SetObject(MarkerViewModel);
		ViewModelInterface.SetInterface(Cast<INotifyFieldValueChanged>(MarkerViewModel));
		UMVVMBlueprintLibrary::SetViewModelByClass(this, ViewModelInterface);
	}

	if (!MarkerTextBlock && WidgetTree)
	{
		MarkerTextBlock = Cast<UCommonTextBlock>(WidgetTree->FindWidget(TEXT("MarkerText")));
	}
	if (!MarkerBorder && WidgetTree)
	{
		MarkerBorder = Cast<UCommonBorder>(WidgetTree->FindWidget(TEXT("MarkerBorder")));
	}

	ApplyStyle();
	UpdateMarker();
}

void UFRWorldStatusMarkerWidget::SetMarkerState(const FText& MarkerText, bool bVisible, bool bActiveTurn, bool bTarget, bool bEnemy)
{
	CachedMarkerText = MarkerText.IsEmpty() ? DefaultMarkerText : MarkerText;
	bCachedVisible = bVisible;
	bCachedActiveTurn = bActiveTurn;
	bCachedTarget = bTarget;
	bCachedEnemy = bEnemy;

	CreateViewModel();
	UpdateMarker();
}

void UFRWorldStatusMarkerWidget::CreateViewModel()
{
	if (!MarkerViewModel)
	{
		MarkerViewModel = NewObject<UFRWorldStatusMarkerViewModel>(this);
	}
	if (MarkerViewModel)
	{
		MarkerViewModel->SetMarkerText(CachedMarkerText.IsEmpty() ? DefaultMarkerText : CachedMarkerText);
		MarkerViewModel->SetVisible(bCachedVisible);
		MarkerViewModel->SetActiveTurn(bCachedActiveTurn);
		MarkerViewModel->SetTarget(bCachedTarget);
		MarkerViewModel->SetEnemy(bCachedEnemy);
	}
}

void UFRWorldStatusMarkerWidget::UpdateMarker()
{
	CreateViewModel();
	if (!MarkerViewModel)
	{
		return;
	}

	const ESlateVisibility MarkerVisibility = MarkerViewModel->IsVisible()
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed;
	SetVisibility(MarkerVisibility);
	if (MarkerBorder)
	{
		MarkerBorder->SetVisibility(MarkerVisibility);
	}
	SetWorldMarkerText(MarkerTextBlock, MarkerViewModel->GetMarkerText());
}

void UFRWorldStatusMarkerWidget::ApplyStyle()
{
	ApplyWorldMarkerTextStyle(MarkerTextBlock, MarkerTextStyle);
	ApplyWorldMarkerBorderStyle(MarkerBorder, MarkerBorderStyle);
}
