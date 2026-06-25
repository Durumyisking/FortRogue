// Copyright Epic Games, Inc. All Rights Reserved.

#include "FRGenerateMainGameHUDCommandlet.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Engine/Font.h"
#include "Game/FRGameModeDataAsset.h"
#include "HAL/FileManager.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "UI/FRMainGameHUDWidget.h"
#include "UObject/SavePackage.h"
#include "WidgetBlueprint.h"
#include "WidgetBlueprintFactory.h"

namespace
{
constexpr const TCHAR* MainGameHUDPath = TEXT("/Game/FortRogue/Widget/MainGame");
constexpr const TCHAR* MainGameHUDName = TEXT("WBP_MainGameHUD");
constexpr const TCHAR* MainGameModeObjectPath = TEXT("/Game/FortRogue/DataAsset/Global/DA_MainGameMode.DA_MainGameMode");
constexpr const TCHAR* MainGameHUDClassPath = TEXT("/Game/FortRogue/Widget/MainGame/WBP_MainGameHUD.WBP_MainGameHUD_C");

bool SaveAsset(UObject* Asset)
{
	if (!Asset)
	{
		return false;
	}

	UPackage* Package = Asset->GetOutermost();
	const FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(PackageFileName), true);

	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	SaveArgs.SaveFlags = SAVE_NoError;
	return UPackage::SavePackage(Package, Asset, *PackageFileName, SaveArgs);
}

UWidgetBlueprint* LoadOrCreateMainGameHUD()
{
	const FString LongPackageName = FString::Printf(TEXT("%s/%s"), MainGameHUDPath, MainGameHUDName);
	const FString ObjectPath = FString::Printf(TEXT("%s.%s"), *LongPackageName, MainGameHUDName);
	if (UWidgetBlueprint* ExistingBlueprint = LoadObject<UWidgetBlueprint>(nullptr, *ObjectPath))
	{
		return ExistingBlueprint;
	}

	UWidgetBlueprintFactory* Factory = NewObject<UWidgetBlueprintFactory>();
	Factory->ParentClass = UFRMainGameHUDWidget::StaticClass();

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	return Cast<UWidgetBlueprint>(AssetToolsModule.Get().CreateAsset(MainGameHUDName, MainGameHUDPath, UWidgetBlueprint::StaticClass(), Factory));
}

void ConfigureCanvasSlot(UCanvasPanelSlot* Slot, const FAnchors& Anchors, const FVector2D& Position, const FVector2D& Size, const FVector2D& Alignment)
{
	if (!Slot)
	{
		return;
	}

	Slot->SetAnchors(Anchors);
	Slot->SetPosition(Position);
	Slot->SetSize(Size);
	Slot->SetAlignment(Alignment);
}

void ConfigureText(UTextBlock* TextBlock, const FText& Text, int32 FontSize, const FLinearColor& Color, ETextJustify::Type Justification = ETextJustify::Left)
{
	if (!TextBlock)
	{
		return;
	}

	TextBlock->SetText(Text);
	TextBlock->SetColorAndOpacity(FSlateColor(Color));
	TextBlock->SetJustification(Justification);

	FSlateFontInfo Font = TextBlock->GetFont();
	Font.Size = FontSize;
	TextBlock->SetFont(Font);
}

UTextBlock* AddText(UWidgetTree* WidgetTree, UCanvasPanel* Root, const FName& Name, const FText& Text, const FAnchors& Anchors, const FVector2D& Position, const FVector2D& Size, const FVector2D& Alignment, int32 FontSize, ETextJustify::Type Justification = ETextJustify::Left)
{
	UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
	ConfigureText(TextBlock, Text, FontSize, FLinearColor::White, Justification);
	ConfigureCanvasSlot(Root->AddChildToCanvas(TextBlock), Anchors, Position, Size, Alignment);
	return TextBlock;
}

UProgressBar* AddProgressBar(UWidgetTree* WidgetTree, UCanvasPanel* Root, const FName& Name, const FAnchors& Anchors, const FVector2D& Position, const FVector2D& Size, const FLinearColor& FillColor)
{
	UProgressBar* ProgressBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), Name);
	ProgressBar->SetPercent(1.0f);
	ProgressBar->SetFillColorAndOpacity(FillColor);
	ConfigureCanvasSlot(Root->AddChildToCanvas(ProgressBar), Anchors, Position, Size, FVector2D::ZeroVector);
	return ProgressBar;
}

UButton* AddButton(UWidgetTree* WidgetTree, UCanvasPanel* Root, const FName& Name, const FAnchors& Anchors, const FVector2D& Position, const FVector2D& Size, const FVector2D& Alignment)
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
	ConfigureCanvasSlot(Root->AddChildToCanvas(Button), Anchors, Position, Size, Alignment);
	return Button;
}

void AddWeaponSlot(UWidgetTree* WidgetTree, UCanvasPanel* Root, int32 SlotIndex, float X)
{
	const FAnchors BottomCenter(0.5f, 1.0f);
	const FName ButtonName(*FString::Printf(TEXT("WeaponSlot%dButton"), SlotIndex + 1));
	const FName NameText(*FString::Printf(TEXT("WeaponSlot%dText"), SlotIndex + 1));
	const FName IndexText(*FString::Printf(TEXT("WeaponSlot%dIndexText"), SlotIndex + 1));

	AddButton(WidgetTree, Root, ButtonName, BottomCenter, FVector2D(X, -300.0f), FVector2D(118.0f, 96.0f), FVector2D(0.5f, 1.0f));
	AddText(WidgetTree, Root, NameText, FText::FromString(TEXT("-")), BottomCenter, FVector2D(X, -248.0f), FVector2D(104.0f, 42.0f), FVector2D(0.5f, 0.5f), 15, ETextJustify::Center);
	AddText(WidgetTree, Root, IndexText, FText::AsNumber(SlotIndex + 1), BottomCenter, FVector2D(X - 48.0f, -286.0f), FVector2D(28.0f, 24.0f), FVector2D(0.5f, 0.5f), 15, ETextJustify::Center);
}

void AddItemSlot(UWidgetTree* WidgetTree, UCanvasPanel* Root, int32 SlotIndex, float X)
{
	const FAnchors BottomCenter(0.5f, 1.0f);
	const FName ButtonName(*FString::Printf(TEXT("ItemSlot%dButton"), SlotIndex + 1));
	const FName NameText(*FString::Printf(TEXT("ItemSlot%dText"), SlotIndex + 1));
	const FName ChargesText(*FString::Printf(TEXT("ItemSlot%dChargesText"), SlotIndex + 1));

	AddButton(WidgetTree, Root, ButtonName, BottomCenter, FVector2D(X, -300.0f), FVector2D(76.0f, 72.0f), FVector2D(0.5f, 1.0f));
	AddText(WidgetTree, Root, NameText, FText::FromString(TEXT("-")), BottomCenter, FVector2D(X, -260.0f), FVector2D(66.0f, 34.0f), FVector2D(0.5f, 0.5f), 12, ETextJustify::Center);
	AddText(WidgetTree, Root, ChargesText, FText::GetEmpty(), BottomCenter, FVector2D(X + 26.0f, -286.0f), FVector2D(24.0f, 22.0f), FVector2D(0.5f, 0.5f), 12, ETextJustify::Center);
}

void ConfigureMainGameHUDTree(UWidgetBlueprint* WidgetBlueprint)
{
	if (!WidgetBlueprint)
	{
		return;
	}

	WidgetBlueprint->Modify();
	WidgetBlueprint->ParentClass = UFRMainGameHUDWidget::StaticClass();

	UWidgetTree* WidgetTree = WidgetBlueprint->WidgetTree;
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(WidgetBlueprint, TEXT("WidgetTree"), RF_Transactional);
		WidgetBlueprint->WidgetTree = WidgetTree;
	}
	WidgetTree->Modify();

	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = Root;

	AddText(WidgetTree, Root, TEXT("TurnText"), FText::FromString(TEXT("Stage 1/1")), FAnchors(0.0f, 0.0f), FVector2D(28.0f, 20.0f), FVector2D(300.0f, 48.0f), FVector2D::ZeroVector, 30);
	AddText(WidgetTree, Root, TEXT("ShotTimerText"), FText::FromString(TEXT("Player turn")), FAnchors(0.5f, 0.0f), FVector2D(0.0f, 18.0f), FVector2D(360.0f, 48.0f), FVector2D(0.5f, 0.0f), 30, ETextJustify::Center);
	AddText(WidgetTree, Root, TEXT("StatusText"), FText::FromString(TEXT("Ready")), FAnchors(0.5f, 0.0f), FVector2D(0.0f, 66.0f), FVector2D(420.0f, 34.0f), FVector2D(0.5f, 0.0f), 18, ETextJustify::Center);

	AddText(WidgetTree, Root, TEXT("WindText"), FText::FromString(TEXT("Wind 0")), FAnchors(0.0f, 1.0f), FVector2D(48.0f, -116.0f), FVector2D(260.0f, 46.0f), FVector2D(0.0f, 1.0f), 28);
	AddText(WidgetTree, Root, TEXT("PlayerNameText"), FText::FromString(TEXT("Player")), FAnchors(0.5f, 1.0f), FVector2D(-560.0f, -176.0f), FVector2D(160.0f, 30.0f), FVector2D(0.0f, 1.0f), 20);

	AddText(WidgetTree, Root, TEXT("CurrentWeaponText"), FText::FromString(TEXT("Weapon")), FAnchors(0.5f, 1.0f), FVector2D(-120.0f, -332.0f), FVector2D(260.0f, 28.0f), FVector2D(0.5f, 1.0f), 18, ETextJustify::Center);
	AddText(WidgetTree, Root, TEXT("CurrentShotText"), FText::GetEmpty(), FAnchors(0.5f, 1.0f), FVector2D(0.0f, -205.0f), FVector2D(520.0f, 28.0f), FVector2D(0.5f, 1.0f), 14, ETextJustify::Center);

	AddText(WidgetTree, Root, TEXT("PlayerHPText"), FText::FromString(TEXT("0 / 0")), FAnchors(0.5f, 1.0f), FVector2D(360.0f, -151.0f), FVector2D(130.0f, 28.0f), FVector2D(0.0f, 1.0f), 18);
	AddProgressBar(WidgetTree, Root, TEXT("PlayerHPBar"), FAnchors(0.5f, 1.0f), FVector2D(-330.0f, -176.0f), FVector2D(560.0f, 28.0f), FLinearColor(0.9f, 0.12f, 0.1f));
	AddText(WidgetTree, Root, TEXT("ShotPowerText"), FText::FromString(TEXT("0%")), FAnchors(0.5f, 1.0f), FVector2D(360.0f, -108.0f), FVector2D(130.0f, 28.0f), FVector2D(0.0f, 1.0f), 18);
	AddProgressBar(WidgetTree, Root, TEXT("ShotPowerBar"), FAnchors(0.5f, 1.0f), FVector2D(-330.0f, -132.0f), FVector2D(560.0f, 28.0f), FLinearColor(0.95f, 0.68f, 0.13f));
	AddText(WidgetTree, Root, TEXT("MoveBudgetText"), FText::FromString(TEXT("0 / 0")), FAnchors(0.5f, 1.0f), FVector2D(360.0f, -64.0f), FVector2D(130.0f, 28.0f), FVector2D(0.0f, 1.0f), 18);
	AddProgressBar(WidgetTree, Root, TEXT("MoveBudgetBar"), FAnchors(0.5f, 1.0f), FVector2D(-330.0f, -88.0f), FVector2D(560.0f, 28.0f), FLinearColor(0.1f, 0.72f, 0.95f));
	AddText(WidgetTree, Root, TEXT("AimText"), FText::FromString(TEXT("45 deg")), FAnchors(0.5f, 1.0f), FVector2D(-520.0f, -62.0f), FVector2D(130.0f, 28.0f), FVector2D(0.0f, 1.0f), 18);

	for (int32 SlotIndex = 0; SlotIndex < 5; ++SlotIndex)
	{
		AddWeaponSlot(WidgetTree, Root, SlotIndex, -380.0f + SlotIndex * 126.0f);
	}
	for (int32 SlotIndex = 0; SlotIndex < 5; ++SlotIndex)
	{
		AddItemSlot(WidgetTree, Root, SlotIndex, 300.0f + SlotIndex * 86.0f);
	}

	AddButton(WidgetTree, Root, TEXT("FireButton"), FAnchors(1.0f, 1.0f), FVector2D(-86.0f, -112.0f), FVector2D(240.0f, 88.0f), FVector2D(1.0f, 1.0f));
	AddText(WidgetTree, Root, TEXT("FireButtonText"), FText::FromString(TEXT("FIRE")), FAnchors(1.0f, 1.0f), FVector2D(-206.0f, -156.0f), FVector2D(220.0f, 48.0f), FVector2D(0.5f, 0.5f), 30, ETextJustify::Center);

	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(WidgetBlueprint);
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
	WidgetBlueprint->MarkPackageDirty();
}

UFRGameModeDataAsset* ConfigureMainGameModeHUD()
{
	UFRGameModeDataAsset* MainGameMode = LoadObject<UFRGameModeDataAsset>(nullptr, MainGameModeObjectPath);
	if (!MainGameMode)
	{
		return nullptr;
	}

	MainGameMode->Modify();
	MainGameMode->HUDWidgetClass = TSoftClassPtr<UUserWidget>(FSoftObjectPath(MainGameHUDClassPath));
	MainGameMode->MarkPackageDirty();
	return MainGameMode;
}
}

UFRGenerateMainGameHUDCommandlet::UFRGenerateMainGameHUDCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

int32 UFRGenerateMainGameHUDCommandlet::Main(const FString& Params)
{
	UWidgetBlueprint* MainGameHUD = LoadOrCreateMainGameHUD();
	if (!MainGameHUD)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not create %s/%s."), MainGameHUDPath, MainGameHUDName);
		return 1;
	}

	ConfigureMainGameHUDTree(MainGameHUD);
	UFRGameModeDataAsset* MainGameMode = ConfigureMainGameModeHUD();

	const bool bSavedHUD = SaveAsset(MainGameHUD);
	const bool bSavedMode = SaveAsset(MainGameMode);
	UE_LOG(LogTemp, Display, TEXT("Generated WBP_MainGameHUD. SavedHUD=%s SavedMode=%s"), bSavedHUD ? TEXT("true") : TEXT("false"), bSavedMode ? TEXT("true") : TEXT("false"));
	return bSavedHUD && bSavedMode ? 0 : 1;
}
