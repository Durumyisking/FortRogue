// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonUserWidget.h"
#include "CoreMinimal.h"
#include "FRFloatingCombatTextWidget.generated.h"

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

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> DamageText;

	float CachedDamageAmount = 0.0f;
};
