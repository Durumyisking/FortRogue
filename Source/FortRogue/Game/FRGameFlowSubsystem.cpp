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
	if (!ActiveHUDWidget || !CurrentModeData || CurrentModeData->StartMainGameButtonName.IsNone())
	{
		UE_LOG(LogFortRogue, Log, TEXT("GameFlow: StartMainGame button binding skipped. HUD=%s Mode=%s Button=%s"),
			*GetNameSafe(ActiveHUDWidget),
			*GetNameSafe(CurrentModeData),
			CurrentModeData ? *CurrentModeData->StartMainGameButtonName.ToString() : TEXT("<no mode>"));
		return;
	}

	UWidgetTree* WidgetTree = ActiveHUDWidget->WidgetTree;
	if (!WidgetTree)
	{
		UE_LOG(LogFortRogue, Warning, TEXT("GameFlow: HUD %s has no widget tree."),
			*GetNameSafe(ActiveHUDWidget));
		return;
	}

	UWidget* StartWidget = WidgetTree->FindWidget(CurrentModeData->StartMainGameButtonName);
	if (UButton* StartButton = Cast<UButton>(StartWidget))
	{
		StartButton->OnClicked.RemoveDynamic(this, &UFRGameFlowSubsystem::HandleStartMainGameClicked);
		StartButton->OnClicked.AddDynamic(this, &UFRGameFlowSubsystem::HandleStartMainGameClicked);
		UE_LOG(LogFortRogue, Log, TEXT("GameFlow: Bound UButton StartMainGame widget '%s' on HUD %s."),
			*CurrentModeData->StartMainGameButtonName.ToString(),
			*GetNameSafe(ActiveHUDWidget));
		return;
	}

	if (UCommonButtonBase* CommonStartButton = Cast<UCommonButtonBase>(StartWidget))
	{
		CommonStartButton->OnClicked().RemoveAll(this);
		CommonStartButton->OnClicked().AddUObject(this, &UFRGameFlowSubsystem::HandleStartMainGameClicked);
		UE_LOG(LogFortRogue, Log, TEXT("GameFlow: Bound CommonButton StartMainGame widget '%s' on HUD %s."),
			*CurrentModeData->StartMainGameButtonName.ToString(),
			*GetNameSafe(ActiveHUDWidget));
		return;
	}

	TArray<FString> WidgetNames;
	WidgetTree->ForEachWidget([&WidgetNames](UWidget* Widget)
	{
		WidgetNames.Add(FString::Printf(TEXT("%s:%s"), *GetNameSafe(Widget), *GetNameSafe(Widget ? Widget->GetClass() : nullptr)));
	});
	UE_LOG(LogFortRogue, Warning, TEXT("GameFlow: Could not bind StartMainGame widget '%s' in HUD %s. Found=%s Widgets=[%s]"),
		*CurrentModeData->StartMainGameButtonName.ToString(),
		*GetNameSafe(ActiveHUDWidget),
		*GetNameSafe(StartWidget),
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
