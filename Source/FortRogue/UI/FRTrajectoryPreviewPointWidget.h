// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonUserWidget.h"
#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "FRTrajectoryPreviewPointWidget.generated.h"

class UCommonBorder;
class UCommonBorderStyle;
class UCommonNumericTextBlock;
class UCommonTextBlock;
class UCommonTextStyle;

UCLASS(BlueprintType)
class FORTROGUE_API UFRTrajectoryPreviewPointViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|World")
	const FText& GetPointText() const { return PointText; }
	void SetPointText(const FText& InPointText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|World")
	float GetPointIndexValue() const { return PointIndexValue; }
	void SetPointIndexValue(float InPointIndexValue);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|World")
	bool IsVisible() const { return bVisible; }
	void SetVisible(bool bInVisible);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|World")
	bool IsImpact() const { return bImpact; }
	void SetImpact(bool bInImpact);

private:
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|World", meta = (AllowPrivateAccess = "true"))
	FText PointText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|World", meta = (AllowPrivateAccess = "true"))
	float PointIndexValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|World", meta = (AllowPrivateAccess = "true"))
	bool bVisible = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|World", meta = (AllowPrivateAccess = "true"))
	bool bImpact = false;
};

UCLASS()
class FORTROGUE_API UFRTrajectoryPreviewPointWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	void SetPreviewPointState(const FText& PointText, float PointIndexValue, bool bVisible, bool bImpact);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|World")
	UFRTrajectoryPreviewPointViewModel* GetPreviewPointViewModel() const { return PreviewPointViewModel; }

protected:
	virtual void NativeOnInitialized() override;

private:
	void CreateViewModel();
	void UpdatePoint();
	void ApplyStyle();

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|World")
	FText DefaultPointText = FText::FromString(TEXT(""));

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|World")
	FText DefaultImpactText = FText::FromString(TEXT("HIT"));

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Style")
	TSubclassOf<UCommonTextStyle> PointTextStyle;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Style")
	TSubclassOf<UCommonTextStyle> PointNumberTextStyle;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Style")
	TSubclassOf<UCommonBorderStyle> PointBorderStyle;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Style")
	TSubclassOf<UCommonBorderStyle> ImpactBorderStyle;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> PointTextBlock;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonNumericTextBlock> PointIndexText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonBorder> PointBorder;

	UPROPERTY(Transient)
	TObjectPtr<UFRTrajectoryPreviewPointViewModel> PreviewPointViewModel;

	FText CachedPointText;
	float CachedPointIndexValue = 0.0f;
	bool bCachedVisible = false;
	bool bCachedImpact = false;
};
