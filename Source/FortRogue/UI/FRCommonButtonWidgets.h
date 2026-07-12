// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonButtonBase.h"
#include "CoreMinimal.h"
#include "FRCommonButtonWidgets.generated.h"

class UCommonTextBlock;
class UImage;
class UPaperFlipbook;

/**
 * 라벨 텍스트 하나를 내장한 공용 CommonUI 버튼입니다 (GAME START, FIRE 등).
 * 라벨은 버튼 콘텐츠로 들어가므로 별도 텍스트 위젯이 입력을 가로채지 않습니다.
 */
UCLASS()
class FORTROGUE_API UFRCommonTextButton : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FortRogue|UI", meta = (ToolTip = "버튼 라벨 텍스트를 바꿉니다."))
	void SetButtonLabel(const FText& InLabel);

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeOnCurrentTextStyleChanged() override;

	UPROPERTY(EditAnywhere, Category = "FortRogue|Button", meta = (ToolTip = "버튼에 표시할 라벨입니다. 인스턴스별로 지정합니다."))
	FText ButtonLabel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> ButtonLabelText;
};

/**
 * 캐릭터 선택창 리스트 슬롯 버튼입니다. 초상화(Idle 플립북 반복 재생)와 이름을 내장하고,
 * CommonUI 선택 상태(SetIsSelected)로 선택 강조를 표현합니다.
 */
UCLASS()
class FORTROGUE_API UFRCharacterSelectSlotButton : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	UFRCharacterSelectSlotButton();

	UFUNCTION(BlueprintCallable, Category = "FortRogue|UI", meta = (ToolTip = "슬롯에 표시할 캐릭터 초상화 플립북과 이름을 지정합니다. 플립북이 nullptr이면 초상화를 숨깁니다."))
	void SetCharacterEntry(UPaperFlipbook* IdleFlipbook, const FText& CharacterName);

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeOnCurrentTextStyleChanged() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> PortraitImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> NameText;

private:
	UPROPERTY(Transient)
	TObjectPtr<UPaperFlipbook> PortraitFlipbook;

	float PortraitAnimationTime = 0.0f;
};

/**
 * 전투 HUD의 무기/아이템 슬롯 버튼입니다. 슬롯 이름과 단축키(또는 사용 횟수) 텍스트를 내장합니다.
 */
UCLASS()
class FORTROGUE_API UFRLoadoutSlotButton : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FortRogue|UI", meta = (ToolTip = "슬롯 이름과 단축키/사용 횟수 텍스트를 갱신합니다."))
	void SetSlotDisplay(const FText& SlotName, const FText& HotkeyOrCharges);

protected:
	virtual void NativeOnCurrentTextStyleChanged() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> SlotNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> SlotHotkeyText;
};

/**
 * 보상 선택 버튼입니다. 보상 이름과 설명 텍스트를 내장합니다.
 */
UCLASS()
class FORTROGUE_API UFRRewardChoiceButton : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FortRogue|UI", meta = (ToolTip = "보상 이름과 설명 텍스트를 갱신합니다."))
	void SetRewardDisplay(const FText& RewardName, const FText& RewardDescription);

protected:
	virtual void NativeOnCurrentTextStyleChanged() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> RewardNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> RewardDescriptionText;
};
