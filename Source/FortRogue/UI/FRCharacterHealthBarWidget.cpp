// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FRCharacterHealthBarWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/ProgressBar.h"

void UFRCharacterHealthBarWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (!HealthBar && WidgetTree)
	{
		HealthBar = Cast<UProgressBar>(WidgetTree->FindWidget(TEXT("HealthBar")));
	}

	UpdateHealthBar();
}

void UFRCharacterHealthBarWidget::SetHealth(float CurrentHealth, float MaxHealth, bool bEnemy)
{
	CachedCurrentHealth = FMath::Max(0.0f, CurrentHealth);
	CachedMaxHealth = FMath::Max(1.0f, MaxHealth);
	bCachedEnemy = bEnemy;
	UpdateHealthBar();
}

void UFRCharacterHealthBarWidget::UpdateHealthBar()
{
	if (!HealthBar)
	{
		return;
	}

	const float HealthPercent = FMath::Clamp(CachedCurrentHealth / CachedMaxHealth, 0.0f, 1.0f);
	HealthBar->SetPercent(HealthPercent);
	HealthBar->SetFillColorAndOpacity(bCachedEnemy ? EnemyFillColor : PlayerFillColor);
}
