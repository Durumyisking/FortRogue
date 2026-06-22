// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FRTrajectoryPreviewPointWidget.h"

#include "Blueprint/WidgetTree.h"
#include "CommonBorder.h"
#include "CommonNumericTextBlock.h"
#include "CommonTextBlock.h"
#include "MVVMBlueprintLibrary.h"

namespace
{
	void SetTrajectoryPointText(UCommonTextBlock* TextBlock, const FText& Text)
	{
		if (TextBlock)
		{
			TextBlock->SetText(Text);
		}
	}

	void SetTrajectoryPointNumber(UCommonNumericTextBlock* TextBlock, float Value)
	{
		if (TextBlock)
		{
			TextBlock->SetCurrentValue(Value);
		}
	}

	void ApplyTrajectoryPointTextStyle(UCommonTextBlock* TextBlock, TSubclassOf<UCommonTextStyle> TextStyle)
	{
		if (TextBlock && TextStyle)
		{
			TextBlock->SetStyle(TextStyle);
		}
	}

	void ApplyTrajectoryPointBorderStyle(UCommonBorder* Border, TSubclassOf<UCommonBorderStyle> BorderStyle)
	{
		if (Border && BorderStyle)
		{
			Border->SetStyle(BorderStyle);
		}
	}
}

void UFRTrajectoryPreviewPointViewModel::SetPointText(const FText& InPointText)
{
	UE_MVVM_SET_PROPERTY_VALUE(PointText, InPointText);
}

void UFRTrajectoryPreviewPointViewModel::SetPointIndexValue(float InPointIndexValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(PointIndexValue, InPointIndexValue);
}

void UFRTrajectoryPreviewPointViewModel::SetVisible(bool bInVisible)
{
	UE_MVVM_SET_PROPERTY_VALUE(bVisible, bInVisible);
}

void UFRTrajectoryPreviewPointViewModel::SetImpact(bool bInImpact)
{
	UE_MVVM_SET_PROPERTY_VALUE(bImpact, bInImpact);
}

void UFRTrajectoryPreviewPointWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	CreateViewModel();
	if (PreviewPointViewModel)
	{
		TScriptInterface<INotifyFieldValueChanged> ViewModelInterface;
		ViewModelInterface.SetObject(PreviewPointViewModel);
		ViewModelInterface.SetInterface(Cast<INotifyFieldValueChanged>(PreviewPointViewModel));
		UMVVMBlueprintLibrary::SetViewModelByClass(this, ViewModelInterface);
	}

	if (!PointTextBlock && WidgetTree)
	{
		PointTextBlock = Cast<UCommonTextBlock>(WidgetTree->FindWidget(TEXT("PointText")));
	}
	if (!PointIndexText && WidgetTree)
	{
		PointIndexText = Cast<UCommonNumericTextBlock>(WidgetTree->FindWidget(TEXT("PointIndexText")));
	}
	if (!PointBorder && WidgetTree)
	{
		PointBorder = Cast<UCommonBorder>(WidgetTree->FindWidget(TEXT("PointBorder")));
	}

	ApplyStyle();
	UpdatePoint();
}

void UFRTrajectoryPreviewPointWidget::SetPreviewPointState(const FText& PointText, float PointIndexValue, bool bVisible, bool bImpact)
{
	CachedPointText = PointText.IsEmpty() ? (bImpact ? DefaultImpactText : DefaultPointText) : PointText;
	CachedPointIndexValue = PointIndexValue;
	bCachedVisible = bVisible;
	bCachedImpact = bImpact;

	CreateViewModel();
	UpdatePoint();
}

void UFRTrajectoryPreviewPointWidget::CreateViewModel()
{
	if (!PreviewPointViewModel)
	{
		PreviewPointViewModel = NewObject<UFRTrajectoryPreviewPointViewModel>(this);
	}
	if (PreviewPointViewModel)
	{
		PreviewPointViewModel->SetPointText(CachedPointText.IsEmpty() ? (bCachedImpact ? DefaultImpactText : DefaultPointText) : CachedPointText);
		PreviewPointViewModel->SetPointIndexValue(CachedPointIndexValue);
		PreviewPointViewModel->SetVisible(bCachedVisible);
		PreviewPointViewModel->SetImpact(bCachedImpact);
	}
}

void UFRTrajectoryPreviewPointWidget::UpdatePoint()
{
	CreateViewModel();
	if (!PreviewPointViewModel)
	{
		return;
	}

	const ESlateVisibility PointVisibility = PreviewPointViewModel->IsVisible()
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed;
	SetVisibility(PointVisibility);
	if (PointBorder)
	{
		PointBorder->SetVisibility(PointVisibility);
	}
	SetTrajectoryPointText(PointTextBlock, PreviewPointViewModel->GetPointText());
	SetTrajectoryPointNumber(PointIndexText, PreviewPointViewModel->GetPointIndexValue());
	ApplyStyle();
}

void UFRTrajectoryPreviewPointWidget::ApplyStyle()
{
	ApplyTrajectoryPointTextStyle(PointTextBlock, PointTextStyle);
	ApplyTrajectoryPointTextStyle(PointIndexText, PointNumberTextStyle ? PointNumberTextStyle : PointTextStyle);
	ApplyTrajectoryPointBorderStyle(PointBorder, bCachedImpact && ImpactBorderStyle ? ImpactBorderStyle : PointBorderStyle);
}
