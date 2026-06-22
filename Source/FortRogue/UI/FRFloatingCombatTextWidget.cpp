// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FRFloatingCombatTextWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"

void UFRFloatingCombatTextWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (!DamageText && WidgetTree)
	{
		DamageText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("DamageText")));
	}

	UpdateDamageText();
}

void UFRFloatingCombatTextWidget::SetDamageAmount(float DamageAmount)
{
	CachedDamageAmount = FMath::Max(0.0f, DamageAmount);
	UpdateDamageText();
}

void UFRFloatingCombatTextWidget::UpdateDamageText()
{
	if (!DamageText)
	{
		return;
	}

	DamageText->SetText(FText::FromString(FString::Printf(TEXT("-%.0f"), CachedDamageAmount)));
}
