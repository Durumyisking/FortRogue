// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FRBattleCharacterAimIndicatorWidget.h"

#include "Combat/FRBattleCharacter.h"
#include "Components/Image.h"
#include "Components/Widget.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

void UFRBattleCharacterAimIndicatorWidget::SetBattleCharacter(AFRBattleCharacter* InBattleCharacter)
{
	BattleCharacter = InBattleCharacter;
	RefreshIndicator();
}

void UFRBattleCharacterAimIndicatorWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	RefreshIndicator();
}

void UFRBattleCharacterAimIndicatorWidget::RefreshIndicator()
{
	if (!BattleCharacter || !AimIndicatorImage)
	{
		return;
	}

	AimIndicatorImage->SetVisibility(ESlateVisibility::HitTestInvisible);
	AimIndicatorImage->SetRenderScale(FVector2D(BattleCharacter->IsFacingRight() ? 1.0f : -1.0f, 1.0f));
	if (!AimIndicatorMaterial)
	{
		AimIndicatorMaterial = AimIndicatorImage->GetDynamicMaterial();
	}
	if (!AimIndicatorMaterial)
	{
		if (UMaterialInterface* AimIndicatorBaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/FortRogue/Material/Widget/M_AimAngleIndicator.M_AimAngleIndicator")))
		{
			AimIndicatorImage->SetBrushFromMaterial(AimIndicatorBaseMaterial);
			AimIndicatorMaterial = AimIndicatorImage->GetDynamicMaterial();
		}
	}
	if (!AimIndicatorMaterial)
	{
		return;
	}

	AimIndicatorMaterial->SetScalarParameterValue(TEXT("MinAngle"), BattleCharacter->GetMinAimAngle());
	AimIndicatorMaterial->SetScalarParameterValue(TEXT("MaxAngle"), BattleCharacter->GetMaxAimAngle());
	AimIndicatorMaterial->SetScalarParameterValue(TEXT("CurrentAngle"), BattleCharacter->GetAimAngle());
}

