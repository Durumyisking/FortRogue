// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FRUIRootWidget.h"

#include "CommonActivatableWidget.h"
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

	return Stack->AddWidget(WidgetClass);
}

void UFRUIRootWidget::ClearLayer(EFRUILayer Layer)
{
	if (UCommonActivatableWidgetStack* Stack = GetLayerStack(Layer))
	{
		Stack->ClearWidgets();
	}
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
