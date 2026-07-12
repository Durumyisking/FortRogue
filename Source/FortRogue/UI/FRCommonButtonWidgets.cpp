// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FRCommonButtonWidgets.h"

#include "CommonTextBlock.h"
#include "Components/Image.h"
#include "PaperFlipbook.h"
#include "PaperSprite.h"

namespace
{
void ApplyTextStyleIfSet(UCommonTextBlock* TextBlock, TSubclassOf<UCommonTextStyle> StyleClass)
{
	if (TextBlock && StyleClass)
	{
		TextBlock->SetStyle(StyleClass);
	}
}
}

void UFRCommonTextButton::SetButtonLabel(const FText& InLabel)
{
	ButtonLabel = InLabel;
	if (ButtonLabelText)
	{
		ButtonLabelText->SetText(ButtonLabel);
	}
}

void UFRCommonTextButton::NativePreConstruct()
{
	Super::NativePreConstruct();
	if (ButtonLabelText)
	{
		ButtonLabelText->SetText(ButtonLabel);
	}
}

void UFRCommonTextButton::NativeOnCurrentTextStyleChanged()
{
	Super::NativeOnCurrentTextStyleChanged();
	ApplyTextStyleIfSet(ButtonLabelText, GetCurrentTextStyleClass());
}

UFRCharacterSelectSlotButton::UFRCharacterSelectSlotButton()
{
	// 리스트에서 하나만 선택 유지되는 라디오 버튼처럼 동작합니다. 선택 해제는 다른 슬롯 선택으로만 일어납니다.
	bSelectable = true;
	bToggleable = false;
	bShouldSelectUponReceivingFocus = false;
}

void UFRCharacterSelectSlotButton::SetCharacterEntry(UPaperFlipbook* IdleFlipbook, const FText& CharacterName)
{
	PortraitFlipbook = IdleFlipbook;
	PortraitAnimationTime = 0.0f;

	if (NameText)
	{
		NameText->SetText(CharacterName);
	}
	if (PortraitImage)
	{
		UPaperSprite* FirstFrame = IdleFlipbook ? IdleFlipbook->GetSpriteAtFrame(0) : nullptr;
		PortraitImage->SetVisibility(FirstFrame ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		if (FirstFrame)
		{
			PortraitImage->SetBrushFromAtlasInterface(FirstFrame, false);
		}
	}
}

void UFRCharacterSelectSlotButton::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!PortraitFlipbook || !PortraitImage)
	{
		return;
	}

	const float Duration = PortraitFlipbook->GetTotalDuration();
	if (Duration <= 0.0f)
	{
		return;
	}

	PortraitAnimationTime += InDeltaTime;
	if (UPaperSprite* FrameSprite = PortraitFlipbook->GetSpriteAtTime(FMath::Fmod(PortraitAnimationTime, Duration)))
	{
		PortraitImage->SetBrushFromAtlasInterface(FrameSprite, false);
	}
}

void UFRCharacterSelectSlotButton::NativeOnCurrentTextStyleChanged()
{
	Super::NativeOnCurrentTextStyleChanged();
	ApplyTextStyleIfSet(NameText, GetCurrentTextStyleClass());
}

void UFRLoadoutSlotButton::SetSlotDisplay(const FText& SlotName, const FText& HotkeyOrCharges)
{
	if (SlotNameText)
	{
		SlotNameText->SetText(SlotName);
	}
	if (SlotHotkeyText)
	{
		SlotHotkeyText->SetText(HotkeyOrCharges);
	}
}

void UFRLoadoutSlotButton::NativeOnCurrentTextStyleChanged()
{
	Super::NativeOnCurrentTextStyleChanged();
	ApplyTextStyleIfSet(SlotNameText, GetCurrentTextStyleClass());
}

void UFRRewardChoiceButton::SetRewardDisplay(const FText& RewardName, const FText& RewardDescription)
{
	if (RewardNameText)
	{
		RewardNameText->SetText(RewardName);
	}
	if (RewardDescriptionText)
	{
		RewardDescriptionText->SetText(RewardDescription);
	}
}

void UFRRewardChoiceButton::NativeOnCurrentTextStyleChanged()
{
	Super::NativeOnCurrentTextStyleChanged();
	ApplyTextStyleIfSet(RewardNameText, GetCurrentTextStyleClass());
}
