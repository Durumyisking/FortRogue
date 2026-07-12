// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FRCharacterSelectWidget.h"

#include "Characters/FRCharacterDefinition.h"
#include "CommonButtonBase.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "FortRogue.h"
#include "Game/FRGameFlowSubsystem.h"
#include "Game/FRGameModeDataAsset.h"
#include "Game/FRRunSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "PaperFlipbook.h"
#include "UI/FRCharacterSelectPreviewActor.h"
#include "UI/FRCommonButtonWidgets.h"
#include "Weapons/FRWeaponDefinition.h"

namespace
{
UPaperFlipbook* GetCharacterPreviewFlipbook(const UFRCharacterDefinition* CharacterDefinition)
{
	if (!CharacterDefinition)
	{
		return nullptr;
	}
	return CharacterDefinition->AnimationSet.Idle ? CharacterDefinition->AnimationSet.Idle.Get() : CharacterDefinition->BodyFlipbook.Get();
}
}

void UFRCharacterSelectWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (!PreviewActorClass)
	{
		PreviewActorClass = AFRCharacterSelectPreviewActor::StaticClass();
	}

	CollectSlotWidgets();

	for (int32 SlotIndex = 0; SlotIndex < SlotButtons.Num(); ++SlotIndex)
	{
		if (UFRCharacterSelectSlotButton* SlotButton = SlotButtons[SlotIndex])
		{
			SlotButton->OnClicked().AddWeakLambda(this, [this, SlotIndex]()
			{
				SelectCharacter(SlotIndex);
			});
		}
	}

	if (GameStartButton)
	{
		GameStartButton->OnClicked().AddUObject(this, &UFRCharacterSelectWidget::HandleGameStartClicked);
	}
}

void UFRCharacterSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	PopulateCharacterList();
	SpawnPreviewActor();

	SelectedCharacterIndex = INDEX_NONE;
	if (GameStartButton)
	{
		GameStartButton->SetIsInteractionEnabled(false);
	}

	// 빈 화면 대신 첫 캐릭터를 기본 선택해 정보 패널과 프리뷰를 바로 채웁니다.
	if (!SelectableCharacters.IsEmpty())
	{
		SelectCharacter(0);
	}
	else
	{
		RefreshInfoPanel(nullptr);
	}
}

void UFRCharacterSelectWidget::NativeDestruct()
{
	if (PreviewActor)
	{
		PreviewActor->Destroy();
		PreviewActor = nullptr;
	}

	Super::NativeDestruct();
}

void UFRCharacterSelectWidget::CollectSlotWidgets()
{
	SlotButtons = { CharacterSlot1Button, CharacterSlot2Button, CharacterSlot3Button, CharacterSlot4Button, CharacterSlot5Button };
}

void UFRCharacterSelectWidget::PopulateCharacterList()
{
	SelectableCharacters.Reset();

	const UGameInstance* GameInstance = GetGameInstance();
	const UFRGameFlowSubsystem* GameFlow = GameInstance ? GameInstance->GetSubsystem<UFRGameFlowSubsystem>() : nullptr;
	const UFRGameModeDataAsset* ModeData = GameFlow ? GameFlow->GetCurrentModeData() : nullptr;
	if (ModeData)
	{
		for (const TObjectPtr<UFRCharacterDefinition>& CharacterDefinition : ModeData->SelectableCharacterDefinitions)
		{
			if (CharacterDefinition)
			{
				SelectableCharacters.Add(CharacterDefinition);
			}
		}
	}

	if (SelectableCharacters.IsEmpty())
	{
		UE_LOG(LogFortRogue, Warning, TEXT("CharacterSelect: No selectable characters found on mode data %s."), *GetNameSafe(ModeData));
	}

	for (int32 SlotIndex = 0; SlotIndex < SlotButtons.Num(); ++SlotIndex)
	{
		UFRCharacterSelectSlotButton* SlotButton = SlotButtons[SlotIndex];
		if (!SlotButton)
		{
			continue;
		}

		UFRCharacterDefinition* CharacterDefinition = SelectableCharacters.IsValidIndex(SlotIndex) ? SelectableCharacters[SlotIndex].Get() : nullptr;
		SlotButton->SetVisibility(CharacterDefinition ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		if (CharacterDefinition)
		{
			SlotButton->SetCharacterEntry(GetCharacterPreviewFlipbook(CharacterDefinition), CharacterDefinition->DisplayName);
		}
	}
}

void UFRCharacterSelectWidget::SpawnPreviewActor()
{
	UWorld* World = GetWorld();
	if (!World || PreviewActor || !PreviewActorClass)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	PreviewActor = World->SpawnActor<AFRCharacterSelectPreviewActor>(PreviewActorClass, PreviewSpawnLocation, FRotator::ZeroRotator, SpawnParams);
	if (!PreviewActor)
	{
		UE_LOG(LogFortRogue, Warning, TEXT("CharacterSelect: Failed to spawn preview actor %s."), *GetNameSafe(PreviewActorClass.Get()));
		return;
	}

	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		PlayerController->SetViewTargetWithBlend(PreviewActor, 0.0f);
	}
}

void UFRCharacterSelectWidget::SelectCharacter(int32 CharacterIndex)
{
	if (!SelectableCharacters.IsValidIndex(CharacterIndex))
	{
		return;
	}

	SelectedCharacterIndex = CharacterIndex;
	const UFRCharacterDefinition* CharacterDefinition = SelectableCharacters[CharacterIndex];
	RefreshInfoPanel(CharacterDefinition);

	for (int32 SlotIndex = 0; SlotIndex < SlotButtons.Num(); ++SlotIndex)
	{
		if (UFRCharacterSelectSlotButton* SlotButton = SlotButtons[SlotIndex])
		{
			SlotButton->SetIsSelected(SlotIndex == SelectedCharacterIndex, false);
		}
	}

	if (PreviewActor)
	{
		PreviewActor->ShowCharacter(CharacterDefinition);
	}
	if (GameStartButton)
	{
		GameStartButton->SetIsInteractionEnabled(true);
	}
}

void UFRCharacterSelectWidget::RefreshInfoPanel(const UFRCharacterDefinition* CharacterDefinition)
{
	if (!CharacterDefinition)
	{
		if (NameText)
		{
			NameText->SetText(FText::FromString(TEXT("Select a character")));
		}
		if (DescriptionText)
		{
			DescriptionText->SetText(FText::FromString(TEXT("Pick a character from the list below.")));
		}
		if (SpecialWeaponNameText)
		{
			SpecialWeaponNameText->SetText(FText::FromString(TEXT("-")));
		}
		if (SpecialWeaponDetailText)
		{
			SpecialWeaponDetailText->SetText(FText::GetEmpty());
		}
		if (StatsText)
		{
			StatsText->SetText(FText::GetEmpty());
		}
		return;
	}

	if (NameText)
	{
		NameText->SetText(CharacterDefinition->DisplayName);
	}
	if (DescriptionText)
	{
		DescriptionText->SetText(CharacterDefinition->Description);
	}

	const UFRWeaponDefinition* SpecialAttack = CharacterDefinition->SpecialAttackDefinition;
	if (SpecialWeaponNameText)
	{
		SpecialWeaponNameText->SetText(SpecialAttack ? SpecialAttack->Weapon.DisplayName : FText::FromString(TEXT("-")));
	}
	if (SpecialWeaponDetailText)
	{
		SpecialWeaponDetailText->SetText(SpecialAttack ? SpecialAttack->Weapon.Description : FText::GetEmpty());
	}

	if (StatsText)
	{
		const FString Stats = FString::Printf(
			TEXT("HP  %.0f\nMove  %.0f\nBonus Damage  +%.0f\nShot Power  x%.2f"),
			CharacterDefinition->MaxHealth,
			CharacterDefinition->MaxMoveBudget,
			CharacterDefinition->BonusDamage,
			CharacterDefinition->ShotPowerMultiplier);
		StatsText->SetText(FText::FromString(Stats));
	}
}

void UFRCharacterSelectWidget::HandleGameStartClicked()
{
	if (!SelectableCharacters.IsValidIndex(SelectedCharacterIndex))
	{
		UE_LOG(LogFortRogue, Warning, TEXT("CharacterSelect: Game start pressed without a selected character."));
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	if (UFRRunSubsystem* RunSubsystem = GameInstance->GetSubsystem<UFRRunSubsystem>())
	{
		RunSubsystem->SetPendingPlayerDefinition(SelectableCharacters[SelectedCharacterIndex]);
	}

	if (UFRGameFlowSubsystem* GameFlow = GameInstance->GetSubsystem<UFRGameFlowSubsystem>())
	{
		UE_LOG(LogFortRogue, Log, TEXT("CharacterSelect: Starting main game with %s."), *GetNameSafe(SelectableCharacters[SelectedCharacterIndex].Get()));
		GameFlow->StartMainGame();
	}
}
