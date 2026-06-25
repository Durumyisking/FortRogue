// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "FRBattleCharacterStatusWidget.generated.h"

class AFRBattleCharacter;
class UProgressBar;
class UTextBlock;

UCLASS()
class FORTROGUE_API UFRBattleCharacterStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FortRogue|Character Status")
	void SetBattleCharacter(AFRBattleCharacter* InBattleCharacter);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Character Status")
	void RefreshStatus();

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CharacterNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HealthText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> AimText;

private:
	UPROPERTY()
	TObjectPtr<AFRBattleCharacter> BattleCharacter;
};
