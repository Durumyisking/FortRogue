// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FRUIRootWidget.h"

#include "CommonActivatableWidget.h"
#include "MVVMBlueprintLibrary.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

namespace
{
	bool ContainsLayerWidget(const UCommonActivatableWidgetStack* Stack, const UCommonActivatableWidget* Widget)
	{
		if (!Stack || !Widget)
		{
			return false;
		}

		for (const UCommonActivatableWidget* LayerWidget : Stack->GetWidgetList())
		{
			if (LayerWidget == Widget)
			{
				return true;
			}
		}
		return false;
	}

	FText GetLayerWidgetText(const UCommonActivatableWidget* Widget)
	{
		return Widget ? FText::FromString(Widget->GetClass()->GetName()) : FText::GetEmpty();
	}
}

void UFRUIRootViewModel::SetHUDLayerText(const FText& InHUDLayerText)
{
	UE_MVVM_SET_PROPERTY_VALUE(HUDLayerText, InHUDLayerText);
}

void UFRUIRootViewModel::SetMenuLayerText(const FText& InMenuLayerText)
{
	UE_MVVM_SET_PROPERTY_VALUE(MenuLayerText, InMenuLayerText);
}

void UFRUIRootViewModel::SetModalLayerText(const FText& InModalLayerText)
{
	UE_MVVM_SET_PROPERTY_VALUE(ModalLayerText, InModalLayerText);
}

void UFRUIRootViewModel::SetHasHUDLayerWidget(bool bInHasHUDLayerWidget)
{
	UE_MVVM_SET_PROPERTY_VALUE(bHasHUDLayerWidget, bInHasHUDLayerWidget);
}

void UFRUIRootViewModel::SetHasMenuLayerWidget(bool bInHasMenuLayerWidget)
{
	UE_MVVM_SET_PROPERTY_VALUE(bHasMenuLayerWidget, bInHasMenuLayerWidget);
}

void UFRUIRootViewModel::SetHasModalLayerWidget(bool bInHasModalLayerWidget)
{
	UE_MVVM_SET_PROPERTY_VALUE(bHasModalLayerWidget, bInHasModalLayerWidget);
}

void UFRUIRootWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	CreateViewModel();
	if (UIRootViewModel)
	{
		TScriptInterface<INotifyFieldValueChanged> ViewModelInterface;
		ViewModelInterface.SetObject(UIRootViewModel);
		ViewModelInterface.SetInterface(Cast<INotifyFieldValueChanged>(UIRootViewModel));
		UMVVMBlueprintLibrary::SetViewModelByClass(this, ViewModelInterface);
	}
	RefreshLayerViewModel();
}

UCommonActivatableWidget* UFRUIRootWidget::SetWidgetInLayer(EFRUILayer Layer, TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
	ClearLayer(Layer);
	return PushWidgetToLayer(Layer, WidgetClass);
}

UCommonActivatableWidget* UFRUIRootWidget::PushWidgetToLayer(EFRUILayer Layer, TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
	UCommonActivatableWidgetStack* Stack = GetLayerStack(Layer);
	if (!Stack || !WidgetClass)
	{
		return nullptr;
	}

	UCommonActivatableWidget* Widget = Stack->AddWidget(WidgetClass);
	RefreshLayerViewModel();
	return Widget;
}

void UFRUIRootWidget::ClearLayer(EFRUILayer Layer)
{
	if (UCommonActivatableWidgetStack* Stack = GetLayerStack(Layer))
	{
		Stack->ClearWidgets();
	}
	RefreshLayerViewModel();
}

bool UFRUIRootWidget::RemoveWidgetFromLayer(UCommonActivatableWidget* Widget)
{
	if (!Widget)
	{
		return false;
	}

	const EFRUILayer Layers[] = { EFRUILayer::HUD, EFRUILayer::Menu, EFRUILayer::Modal };
	for (const EFRUILayer Layer : Layers)
	{
		UCommonActivatableWidgetStack* Stack = GetLayerStack(Layer);
		if (ContainsLayerWidget(Stack, Widget))
		{
			Stack->RemoveWidget(*Widget);
			RefreshLayerViewModel();
			return true;
		}
	}
	return false;
}

UCommonActivatableWidget* UFRUIRootWidget::GetActiveWidgetInLayer(EFRUILayer Layer) const
{
	const UCommonActivatableWidgetStack* Stack = GetLayerStack(Layer);
	return Stack ? Stack->GetActiveWidget() : nullptr;
}

UCommonActivatableWidgetStack* UFRUIRootWidget::GetLayerStack(EFRUILayer Layer) const
{
	switch (Layer)
	{
	case EFRUILayer::HUD:
		return HUDLayer;
	case EFRUILayer::Menu:
		return MenuLayer;
	case EFRUILayer::Modal:
		return ModalLayer;
	default:
		return nullptr;
	}
}

void UFRUIRootWidget::CreateViewModel()
{
	if (!UIRootViewModel)
	{
		UIRootViewModel = NewObject<UFRUIRootViewModel>(this);
	}
}

void UFRUIRootWidget::RefreshLayerViewModel()
{
	CreateViewModel();
	if (!UIRootViewModel)
	{
		return;
	}

	const UCommonActivatableWidget* ActiveHUDWidget = GetActiveWidgetInLayer(EFRUILayer::HUD);
	const UCommonActivatableWidget* ActiveMenuWidget = GetActiveWidgetInLayer(EFRUILayer::Menu);
	const UCommonActivatableWidget* ActiveModalWidget = GetActiveWidgetInLayer(EFRUILayer::Modal);
	UIRootViewModel->SetHasHUDLayerWidget(ActiveHUDWidget != nullptr);
	UIRootViewModel->SetHasMenuLayerWidget(ActiveMenuWidget != nullptr);
	UIRootViewModel->SetHasModalLayerWidget(ActiveModalWidget != nullptr);
	UIRootViewModel->SetHUDLayerText(GetLayerWidgetText(ActiveHUDWidget));
	UIRootViewModel->SetMenuLayerText(GetLayerWidgetText(ActiveMenuWidget));
	UIRootViewModel->SetModalLayerText(GetLayerWidgetText(ActiveModalWidget));
}
