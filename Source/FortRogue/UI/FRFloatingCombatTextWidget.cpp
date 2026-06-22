// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FRFloatingCombatTextWidget.h"

#include "Blueprint/WidgetTree.h"
#include "CommonTextBlock.h"
#include "Components/TextBlock.h"

void UFRFloatingCombatTextWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (!DamageText && WidgetTree)
	{
		DamageText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("DamageText")));
	}

	if (!DamageText && WidgetTree)
	{
		DamageText = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), TEXT("DamageText"));
		if (UCommonTextBlock* CommonDamageText = Cast<UCommonTextBlock>(DamageText))
		{
			if (FallbackTextStyle)
			{
				CommonDamageText->SetStyle(FallbackTextStyle);
			}
		}
		if (!FallbackTextStyle)
		{
			FSlateFontInfo Font = DamageText->GetFont();
			Font.Size = FMath::RoundToInt(FallbackFontSize);
			DamageText->SetFont(Font);
			DamageText->SetColorAndOpacity(FSlateColor(FallbackTextColor));
		}
		DamageText->SetJustification(ETextJustify::Center);
		DamageText->SetShadowOffset(FallbackShadowOffset);
		DamageText->SetShadowColorAndOpacity(FallbackShadowColor);
		WidgetTree->RootWidget = DamageText;
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
