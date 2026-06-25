// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FRFloatingDamageTextWidget.h"

#include "Components/TextBlock.h"

void UFRFloatingDamageTextWidget::SetDamageAmount(float DamageAmount)
{
	if (DamageText)
	{
		DamageText->SetText(FText::FromString(FString::Printf(TEXT("-%.0f"), FMath::Max(0.0f, DamageAmount))));
	}
}
