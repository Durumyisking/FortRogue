// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FRBattleCharacterStatusWidget.h"

#include "Combat/FRBattleCharacter.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"

void UFRBattleCharacterStatusWidget::SetBattleCharacter(AFRBattleCharacter* InBattleCharacter)
{
	BattleCharacter = InBattleCharacter;
	RefreshStatus();
}

void UFRBattleCharacterStatusWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	RefreshStatus();
}

void UFRBattleCharacterStatusWidget::RefreshStatus()
{
	if (!BattleCharacter)
	{
		return;
	}

	const float Health = BattleCharacter->GetHealth();
	const float MaxHealth = FMath::Max(BattleCharacter->GetMaxHealth(), KINDA_SMALL_NUMBER);
	if (CharacterNameText)
	{
		CharacterNameText->SetText(BattleCharacter->GetCharacterDisplayName());
	}
	if (HealthBar)
	{
		HealthBar->SetPercent(FMath::Clamp(Health / MaxHealth, 0.0f, 1.0f));
	}
	if (HealthText)
	{
		HealthText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), Health, MaxHealth)));
	}
	if (AimText)
	{
		AimText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

