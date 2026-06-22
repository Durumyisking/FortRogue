// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FRFloatingCombatTextWidget.h"

#include "Blueprint/WidgetTree.h"
#include "CommonTextBlock.h"

void UFRFloatingCombatTextWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (!DamageText && WidgetTree)
	{
		DamageText = Cast<UCommonTextBlock>(WidgetTree->FindWidget(TEXT("DamageText")));
	}

	ApplyTextStyle();
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

void UFRFloatingCombatTextWidget::ApplyTextStyle()
{
	if (DamageText && DamageTextStyle)
	{
		DamageText->SetStyle(DamageTextStyle);
	}
}
