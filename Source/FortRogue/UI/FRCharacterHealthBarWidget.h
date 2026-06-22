// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonUserWidget.h"
#include "CoreMinimal.h"
#include "FRCharacterHealthBarWidget.generated.h"

class UProgressBar;

UCLASS()
class FORTROGUE_API UFRCharacterHealthBarWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	void SetHealth(float CurrentHealth, float MaxHealth, bool bEnemy);

protected:
	virtual void NativeOnInitialized() override;

private:
	void UpdateHealthBar();

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Fallback Style")
	FVector2D FallbackBarSize = FVector2D(86.0f, 9.0f);

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Fallback Style")
	FLinearColor PlayerFillColor = FLinearColor(0.18f, 0.82f, 0.32f, 1.0f);

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Fallback Style")
	FLinearColor EnemyFillColor = FLinearColor(0.95f, 0.2f, 0.16f, 1.0f);

	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> HealthBar;

	float CachedCurrentHealth = 0.0f;
	float CachedMaxHealth = 1.0f;
	bool bCachedEnemy = false;
};
