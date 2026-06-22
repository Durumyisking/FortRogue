// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonUserWidget.h"
#include "CoreMinimal.h"
#include "FRFloatingCombatTextWidget.generated.h"

class UCommonTextStyle;
class UTextBlock;

UCLASS()
class FORTROGUE_API UFRFloatingCombatTextWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	void SetDamageAmount(float DamageAmount);

protected:
	virtual void NativeOnInitialized() override;

private:
	void UpdateDamageText();

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Fallback Style")
	TSubclassOf<UCommonTextStyle> FallbackTextStyle;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Fallback Style")
	float FallbackFontSize = 24.0f;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Fallback Style")
	FLinearColor FallbackTextColor = FLinearColor(1.0f, 0.9f, 0.12f, 1.0f);

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Fallback Style")
	FVector2D FallbackShadowOffset = FVector2D(1.0f, 1.0f);

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Fallback Style")
	FLinearColor FallbackShadowColor = FLinearColor::Black;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> DamageText;

	float CachedDamageAmount = 0.0f;
};
