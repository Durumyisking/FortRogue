// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonUserWidget.h"
#include "CoreMinimal.h"
#include "FRFloatingCombatTextWidget.generated.h"

class UCommonTextBlock;
class UCommonTextStyle;

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
	void ApplyTextStyle();

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Style")
	TSubclassOf<UCommonTextStyle> DamageTextStyle;

	UPROPERTY(Transient)
	TObjectPtr<UCommonTextBlock> DamageText;

	float CachedDamageAmount = 0.0f;
};
