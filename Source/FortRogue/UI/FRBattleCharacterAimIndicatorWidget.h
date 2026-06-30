// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "FRBattleCharacterAimIndicatorWidget.generated.h"

class AFRBattleCharacter;
class UImage;
class UMaterialInstanceDynamic;

UCLASS()
class FORTROGUE_API UFRBattleCharacterAimIndicatorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FortRogue|Aim Indicator")
	void SetBattleCharacter(AFRBattleCharacter* InBattleCharacter);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Aim Indicator")
	void RefreshIndicator();

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> AimIndicatorImage;

private:
	UPROPERTY()
	TObjectPtr<AFRBattleCharacter> BattleCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> AimIndicatorMaterial;
};

