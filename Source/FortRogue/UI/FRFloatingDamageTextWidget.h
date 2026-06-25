// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "FRFloatingDamageTextWidget.generated.h"

class UTextBlock;

UCLASS()
class FORTROGUE_API UFRFloatingDamageTextWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FortRogue|Damage Text")
	void SetDamageAmount(float DamageAmount);

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DamageText;
};
