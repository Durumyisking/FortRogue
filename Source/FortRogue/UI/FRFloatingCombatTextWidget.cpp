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
		FSlateFontInfo Font = DamageText->GetFont();
		Font.Size = 24;
		DamageText->SetFont(Font);
		DamageText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.9f, 0.12f, 1.0f)));
		DamageText->SetJustification(ETextJustify::Center);
		DamageText->SetShadowOffset(FVector2D(1.0f, 1.0f));
		DamageText->SetShadowColorAndOpacity(FLinearColor::Black);
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
