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

	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> HealthBar;

	float CachedCurrentHealth = 0.0f;
	float CachedMaxHealth = 1.0f;
	bool bCachedEnemy = false;
};
