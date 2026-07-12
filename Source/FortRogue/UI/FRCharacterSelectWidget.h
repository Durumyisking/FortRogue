// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "FRCharacterSelectWidget.generated.h"

class AFRCharacterSelectPreviewActor;
class UCommonButtonBase;
class UFRCharacterDefinition;
class UFRCharacterSelectSlotButton;
class UTextBlock;

/**
 * 캐릭터 선택 화면 위젯입니다.
 * 하단 캐릭터 리스트(슬롯 버튼)에서 초상화를 클릭하면 오른쪽 정보 패널(이름/설명/특수무기/기본 스펙)이 갱신되고
 * 월드에 스폰한 프리뷰 액터가 해당 캐릭터를 크게 보여줍니다.
 * 게임 시작 버튼을 누르면 선택을 런 서브시스템에 기록하고 메인 게임을 시작합니다.
 * 선택 가능한 캐릭터 목록은 현재 게임 플로우 모드 데이터의 SelectableCharacterDefinitions에서 읽습니다.
 * 슬롯/시작 버튼은 CommonUI 기반(UFRCharacterSelectSlotButton / UFRCommonTextButton)이며 선택 강조는 CommonUI 선택 상태로 표현합니다.
 */
UCLASS()
class FORTROGUE_API UFRCharacterSelectWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Character Select", meta = (ToolTip = "월드에 스폰할 캐릭터 프리뷰 액터 클래스입니다."))
	TSubclassOf<AFRCharacterSelectPreviewActor> PreviewActorClass;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Character Select", meta = (ToolTip = "프리뷰 액터를 스폰할 월드 위치입니다. 레벨의 다른 액터와 겹치지 않는 곳을 사용하세요."))
	FVector PreviewSpawnLocation = FVector(0.0f, 0.0f, 6000.0f);

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DescriptionText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SpecialWeaponNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SpecialWeaponDetailText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StatsText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> GameStartButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UFRCharacterSelectSlotButton> CharacterSlot1Button;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UFRCharacterSelectSlotButton> CharacterSlot2Button;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UFRCharacterSelectSlotButton> CharacterSlot3Button;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UFRCharacterSelectSlotButton> CharacterSlot4Button;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UFRCharacterSelectSlotButton> CharacterSlot5Button;

private:
	void CollectSlotWidgets();
	void PopulateCharacterList();
	void SpawnPreviewActor();
	void SelectCharacter(int32 CharacterIndex);
	void RefreshInfoPanel(const UFRCharacterDefinition* CharacterDefinition);
	void HandleGameStartClicked();

	UPROPERTY(Transient)
	TArray<TObjectPtr<UFRCharacterDefinition>> SelectableCharacters;

	UPROPERTY(Transient)
	TObjectPtr<AFRCharacterSelectPreviewActor> PreviewActor;

	TArray<TObjectPtr<UFRCharacterSelectSlotButton>> SlotButtons;

	int32 SelectedCharacterIndex = INDEX_NONE;
};
