// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonUserWidget.h"
#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "FRWorldStatusMarkerWidget.generated.h"

class UCommonBorder;
class UCommonBorderStyle;
class UCommonTextBlock;
class UCommonTextStyle;

UCLASS(BlueprintType, Blueprintable)
class FORTROGUE_API UFRWorldStatusMarkerViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|World")
	const FText& GetMarkerText() const { return MarkerText; }
	void SetMarkerText(const FText& InMarkerText);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|World")
	bool IsVisible() const { return bVisible; }
	void SetVisible(bool bInVisible);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|World")
	bool IsActiveTurn() const { return bActiveTurn; }
	void SetActiveTurn(bool bInActiveTurn);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|World")
	bool IsTarget() const { return bTarget; }
	void SetTarget(bool bInTarget);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|World")
	bool IsEnemy() const { return bEnemy; }
	void SetEnemy(bool bInEnemy);

private:
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|World", meta = (AllowPrivateAccess = "true"))
	FText MarkerText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|World", meta = (AllowPrivateAccess = "true"))
	bool bVisible = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|World", meta = (AllowPrivateAccess = "true"))
	bool bActiveTurn = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|World", meta = (AllowPrivateAccess = "true"))
	bool bTarget = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "FortRogue|UI|World", meta = (AllowPrivateAccess = "true"))
	bool bEnemy = false;
};

UCLASS()
class FORTROGUE_API UFRWorldStatusMarkerWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	void SetMarkerState(const FText& MarkerText, bool bVisible, bool bActiveTurn, bool bTarget, bool bEnemy);

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI|World")
	UFRWorldStatusMarkerViewModel* GetMarkerViewModel() const { return MarkerViewModel; }

protected:
	virtual void NativeOnInitialized() override;

private:
	void CreateViewModel();
	void UpdateMarker();
	void ApplyStyle();

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|World")
	FText DefaultMarkerText = FText::FromString(TEXT("TARGET"));

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Style")
	TSubclassOf<UCommonTextStyle> MarkerTextStyle;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Style")
	TSubclassOf<UCommonBorderStyle> MarkerBorderStyle;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> MarkerTextBlock;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonBorder> MarkerBorder;

	UPROPERTY(Transient)
	TObjectPtr<UFRWorldStatusMarkerViewModel> MarkerViewModel;

	FText CachedMarkerText;
	bool bCachedVisible = false;
	bool bCachedActiveTurn = false;
	bool bCachedTarget = false;
	bool bCachedEnemy = false;
};
