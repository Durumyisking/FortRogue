// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonUserWidget.h"
#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "FRCharacterHealthBarWidget.generated.h"

class UCommonNumericTextBlock;
class UCommonTextBlock;
class UCommonTextStyle;
class UProgressBar;

UCLASS(BlueprintType)
class FORTROGUE_API UFRCharacterHealthBarViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|World")
	const FText& GetHealthLabelText() const { return HealthLabelText; }
	void SetHealthLabelText(const FText& InHealthLabelText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|World")
	float GetCurrentHealthValue() const { return CurrentHealthValue; }
	void SetCurrentHealthValue(float InCurrentHealthValue);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|World")
	float GetMaxHealthValue() const { return MaxHealthValue; }
	void SetMaxHealthValue(float InMaxHealthValue);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|World")
	float GetHealthPercent() const { return HealthPercent; }
	void SetHealthPercent(float InHealthPercent);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|World")
	bool IsEnemy() const { return bEnemy; }
	void SetEnemy(bool bInEnemy);

private:
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|World", meta = (AllowPrivateAccess = "true"))
	FText HealthLabelText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|World", meta = (AllowPrivateAccess = "true"))
	float CurrentHealthValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|World", meta = (AllowPrivateAccess = "true"))
	float MaxHealthValue = 1.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|World", meta = (AllowPrivateAccess = "true"))
	float HealthPercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|World", meta = (AllowPrivateAccess = "true"))
	bool bEnemy = false;
};

UCLASS()
class FORTROGUE_API UFRCharacterHealthBarWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	void SetHealth(float CurrentHealth, float MaxHealth, bool bEnemy);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|World")
	UFRCharacterHealthBarViewModel* GetHealthBarViewModel() const { return HealthBarViewModel; }

protected:
	virtual void NativeOnInitialized() override;

private:
	void CreateViewModel();
	void UpdateHealthBar();
	void ApplyTextStyles();

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Presentation")
	FText HealthLabelText = FText::FromString(TEXT("HP"));

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Presentation")
	FLinearColor PlayerFillColor = FLinearColor(0.18f, 0.82f, 0.32f, 1.0f);

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Presentation")
	FLinearColor EnemyFillColor = FLinearColor(0.95f, 0.2f, 0.16f, 1.0f);

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Style")
	TSubclassOf<UCommonTextStyle> LabelTextStyle;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Style")
	TSubclassOf<UCommonTextStyle> NumericTextStyle;

	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> HealthText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonNumericTextBlock> CurrentHealthValueText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonNumericTextBlock> MaxHealthValueText;

	UPROPERTY(Transient)
	TObjectPtr<UFRCharacterHealthBarViewModel> HealthBarViewModel;

	float CachedCurrentHealth = 0.0f;
	float CachedMaxHealth = 1.0f;
	bool bCachedEnemy = false;
};
