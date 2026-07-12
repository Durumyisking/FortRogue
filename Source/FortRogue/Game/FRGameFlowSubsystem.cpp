// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/FRGameFlowSubsystem.h"

#include "FortRogue.h"
#include "FRGameMode.h"
#include "Game/FRGameFlowSettings.h"
#include "Game/FRGameModeDataAsset.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "CommonButtonBase.h"
#include "Components/Button.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"

bool UFRGameFlowSubsystem::StartStartupMode()
{
	const UFRGameFlowSettings* Settings = GetDefault<UFRGameFlowSettings>();
	return StartModeInternal(LoadModeData(Settings ? Settings->StartupModeData : TSoftObjectPtr<UFRGameModeDataAsset>()));
}

bool UFRGameFlowSubsystem::StartMainMenu()
{
	const UFRGameFlowSettings* Settings = GetDefault<UFRGameFlowSettings>();
	return StartModeInternal(LoadModeData(Settings ? Settings->MainMenuModeData : TSoftObjectPtr<UFRGameModeDataAsset>()));
}

bool UFRGameFlowSubsystem::StartCharacterSelect()
{
	const UFRGameFlowSettings* Settings = GetDefault<UFRGameFlowSettings>();
	UE_LOG(LogFortRogue, Log, TEXT("GameFlow: StartCharacterSelect requested. CharacterSelectModeData=%s"),
		Settings ? *Settings->CharacterSelectModeData.ToSoftObjectPath().ToString() : TEXT("<no settings>"));
	return StartModeInternal(LoadModeData(Settings ? Settings->CharacterSelectModeData : TSoftObjectPtr<UFRGameModeDataAsset>()));
}

bool UFRGameFlowSubsystem::StartMainGame()
{
	const UFRGameFlowSettings* Settings = GetDefault<UFRGameFlowSettings>();
	UE_LOG(LogFortRogue, Log, TEXT("GameFlow: StartMainGame requested. MainGameModeData=%s"),
		Settings ? *Settings->MainGameModeData.ToSoftObjectPath().ToString() : TEXT("<no settings>"));
	return StartModeInternal(LoadModeData(Settings ? Settings->MainGameModeData : TSoftObjectPtr<UFRGameModeDataAsset>()));
}

bool UFRGameFlowSubsystem::StartMode(UFRGameModeDataAsset* ModeData)
{
	return StartModeInternal(ModeData);
}

bool UFRGameFlowSubsystem::EnsureStartupMode()
{
	if (CurrentModeData)
	{
		return true;
	}
	return StartStartupMode();
}

UFRGameModeDataAsset* UFRGameFlowSubsystem::LoadModeData(const TSoftObjectPtr<UFRGameModeDataAsset>& ModeData) const
{
	return ModeData.LoadSynchronous();
}

bool UFRGameFlowSubsystem::StartModeInternal(UFRGameModeDataAsset* ModeData)
{
	if (!ModeData)
	{
		UE_LOG(LogFortRogue, Warning, TEXT("GameFlow: StartModeInternal failed because ModeData is null."));
		return false;
	}

	UE_LOG(LogFortRogue, Log, TEXT("GameFlow: Starting mode %s (%s)."),
		*ModeData->ModeName.ToString(),
		*GetNameSafe(ModeData));

	CurrentModeData = ModeData;

	FString LevelPackageName;
	if (ShouldTravelToModeLevel(ModeData, LevelPackageName))
	{
		UE_LOG(LogFortRogue, Log, TEXT("GameFlow: Traveling to %s for mode %s."),
			*LevelPackageName,
			*ModeData->ModeName.ToString());
		UGameplayStatics::OpenLevel(GetWorld(), FName(*LevelPackageName));
		return true;
	}

	ApplyCurrentModeToWorld();
	return true;
}

bool UFRGameFlowSubsystem::ShouldTravelToModeLevel(const UFRGameModeDataAsset* ModeData, FString& OutLevelPackageName) const
{
	OutLevelPackageName.Reset();
	if (!ModeData)
	{
		return false;
	}

	OutLevelPackageName = ModeData->GetLevelPackageName();
	if (OutLevelPackageName.IsEmpty() || !GetWorld())
	{
		return false;
	}

	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(GetWorld(), true);
	const FString TargetLevelName = FPackageName::GetShortName(OutLevelPackageName);
	return !CurrentLevelName.Equals(TargetLevelName, ESearchCase::IgnoreCase);
}

void UFRGameFlowSubsystem::ApplyCurrentModeToWorld()
{
	UWorld* World = GetWorld();
	if (!World || !CurrentModeData)
	{
		UE_LOG(LogFortRogue, Warning, TEXT("GameFlow: ApplyCurrentModeToWorld skipped. World=%s Mode=%s"),
			*GetNameSafe(World),
			*GetNameSafe(CurrentModeData));
		return;
	}

	if (AFRGameMode* GameMode = World->GetAuthGameMode<AFRGameMode>())
	{
		UE_LOG(LogFortRogue, Log, TEXT("GameFlow: Applying mode %s to GameMode %s."),
			*CurrentModeData->ModeName.ToString(),
			*GetNameSafe(GameMode));
		GameMode->EnterGameFlowMode(CurrentModeData);
	}
	else
	{
		UE_LOG(LogFortRogue, Warning, TEXT("GameFlow: No AFRGameMode found while applying mode %s."),
			*CurrentModeData->ModeName.ToString());
	}

	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0))
	{
		ApplyCurrentModeToPlayerController(PlayerController);
	}
}

void UFRGameFlowSubsystem::ApplyCurrentModeToPlayerController(APlayerController* PlayerController)
{
	if (!PlayerController || !CurrentModeData)
	{
		return;
	}

	ClearActiveHUD();

	if (TSubclassOf<UUserWidget> HUDWidgetClass = CurrentModeData->LoadHUDWidgetClass())
	{
		UE_LOG(LogFortRogue, Log, TEXT("GameFlow: Creating HUD %s for mode %s."),
			*GetNameSafe(HUDWidgetClass.Get()),
			*CurrentModeData->ModeName.ToString());
		ActiveHUDWidget = CreateWidget<UUserWidget>(PlayerController, HUDWidgetClass);
		if (ActiveHUDWidget)
		{
			ActiveHUDWidget->AddToViewport(0);
			BindModeWidgetActions();
		}
	}

	PlayerController->bShowMouseCursor = CurrentModeData->bShowMouseCursor;
	switch (CurrentModeData->InputMode)
	{
	case EFRGameFlowInputMode::UIOnly:
		{
			FInputModeUIOnly InputMode;
			if (ActiveHUDWidget)
			{
				InputMode.SetWidgetToFocus(ActiveHUDWidget->TakeWidget());
			}
			PlayerController->SetInputMode(InputMode);
			break;
		}
	case EFRGameFlowInputMode::GameAndUI:
		{
			FInputModeGameAndUI InputMode;
			if (ActiveHUDWidget)
			{
				InputMode.SetWidgetToFocus(ActiveHUDWidget->TakeWidget());
			}
			InputMode.SetHideCursorDuringCapture(false);
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputMode);
			break;
		}
	case EFRGameFlowInputMode::GameOnly:
	default:
		PlayerController->SetInputMode(FInputModeGameOnly());
		break;
	}
}

void UFRGameFlowSubsystem::BindModeWidgetActions()
{
	if (!ActiveHUDWidget || !CurrentModeData)
	{
		UE_LOG(LogFortRogue, Log, TEXT("GameFlow: Mode widget binding skipped. HUD=%s Mode=%s"),
			*GetNameSafe(ActiveHUDWidget),
			*GetNameSafe(CurrentModeData));
		return;
	}

	BindModeButton(CurrentModeData->StartMainGameButtonName, GET_FUNCTION_NAME_CHECKED(UFRGameFlowSubsystem, HandleStartMainGameClicked));
	BindModeButton(CurrentModeData->OpenCharacterSelectButtonName, GET_FUNCTION_NAME_CHECKED(UFRGameFlowSubsystem, HandleOpenCharacterSelectClicked));
}

void UFRGameFlowSubsystem::BindModeButton(const FName& ButtonName, const FName& HandlerFunctionName)
{
	if (ButtonName.IsNone() || !ActiveHUDWidget)
	{
		return;
	}

	UWidgetTree* WidgetTree = ActiveHUDWidget->WidgetTree;
	if (!WidgetTree)
	{
		UE_LOG(LogFortRogue, Warning, TEXT("GameFlow: HUD %s has no widget tree."),
			*GetNameSafe(ActiveHUDWidget));
		return;
	}

	UWidget* FoundWidget = WidgetTree->FindWidget(ButtonName);
	if (UButton* Button = Cast<UButton>(FoundWidget))
	{
		FScriptDelegate ClickDelegate;
		ClickDelegate.BindUFunction(this, HandlerFunctionName);
		Button->OnClicked.Remove(ClickDelegate);
		Button->OnClicked.Add(ClickDelegate);
		UE_LOG(LogFortRogue, Log, TEXT("GameFlow: Bound UButton '%s' to %s on HUD %s."),
			*ButtonName.ToString(),
			*HandlerFunctionName.ToString(),
			*GetNameSafe(ActiveHUDWidget));
		return;
	}

	if (UCommonButtonBase* CommonButton = Cast<UCommonButtonBase>(FoundWidget))
	{
		CommonButton->OnClicked().RemoveAll(this);
		CommonButton->OnClicked().AddWeakLambda(this, [this, HandlerFunctionName]()
		{
			if (UFunction* HandlerFunction = FindFunction(HandlerFunctionName))
			{
				ProcessEvent(HandlerFunction, nullptr);
			}
		});
		UE_LOG(LogFortRogue, Log, TEXT("GameFlow: Bound CommonButton '%s' to %s on HUD %s."),
			*ButtonName.ToString(),
			*HandlerFunctionName.ToString(),
			*GetNameSafe(ActiveHUDWidget));
		return;
	}

	TArray<FString> WidgetNames;
	WidgetTree->ForEachWidget([&WidgetNames](UWidget* Widget)
	{
		WidgetNames.Add(FString::Printf(TEXT("%s:%s"), *GetNameSafe(Widget), *GetNameSafe(Widget ? Widget->GetClass() : nullptr)));
	});
	UE_LOG(LogFortRogue, Warning, TEXT("GameFlow: Could not bind widget '%s' in HUD %s. Found=%s Widgets=[%s]"),
		*ButtonName.ToString(),
		*GetNameSafe(ActiveHUDWidget),
		*GetNameSafe(FoundWidget),
		*FString::Join(WidgetNames, TEXT(", ")));
}

void UFRGameFlowSubsystem::ClearActiveHUD()
{
	if (ActiveHUDWidget)
	{
		ActiveHUDWidget->RemoveFromParent();
		ActiveHUDWidget = nullptr;
	}
}

void UFRGameFlowSubsystem::HandleStartMainGameClicked()
{
	UE_LOG(LogFortRogue, Log, TEXT("GameFlow: StartMainGame button clicked."));
	const bool bStarted = StartMainGame();
	UE_LOG(LogFortRogue, Log, TEXT("GameFlow: StartMainGame button result=%s."), bStarted ? TEXT("true") : TEXT("false"));
}

void UFRGameFlowSubsystem::HandleOpenCharacterSelectClicked()
{
	UE_LOG(LogFortRogue, Log, TEXT("GameFlow: OpenCharacterSelect button clicked."));
	const bool bStarted = StartCharacterSelect();
	UE_LOG(LogFortRogue, Log, TEXT("GameFlow: OpenCharacterSelect button result=%s."), bStarted ? TEXT("true") : TEXT("false"));
}
