// Copyright Epic Games, Inc. All Rights Reserved.

#include "FRCommonUIWidgetGenerator.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "CommonBorder.h"
#include "CommonNumericTextBlock.h"
#include "CommonTextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "HAL/FileManager.h"
#include "IAssetTools.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "UI/FRTrajectoryPreviewPointWidget.h"
#include "UI/FRWorldStatusMarkerWidget.h"
#include "UObject/SavePackage.h"
#include "WidgetBlueprint.h"
#include "WidgetBlueprintFactory.h"

namespace FREditor
{
static const TCHAR* CommonUIComponentsPath = TEXT("/Game/FortRogue/Widget/MainGame/Components");
static const TCHAR* BodyTextStylePath = TEXT("/Game/FortRogue/Widget/Styles/BP_UI_Text_Body.BP_UI_Text_Body_C");
static const TCHAR* NumberTextStylePath = TEXT("/Game/FortRogue/Widget/Styles/BP_UI_Text_Number.BP_UI_Text_Number_C");
static const TCHAR* SlotBorderStylePath = TEXT("/Game/FortRogue/Widget/Styles/BP_UI_Border_Slot.BP_UI_Border_Slot_C");

template <typename StyleType>
static TSubclassOf<StyleType> LoadStyleClass(const TCHAR* ClassPath)
{
	return LoadClass<StyleType>(nullptr, ClassPath);
}

static UWidgetBlueprint* LoadWidgetBlueprint(const TCHAR* AssetName)
{
	const FString ObjectPath = FString::Printf(TEXT("%s/%s.%s"), CommonUIComponentsPath, AssetName, AssetName);
	return LoadObject<UWidgetBlueprint>(nullptr, *ObjectPath);
}

static UWidgetBlueprint* CreateWidgetBlueprint(const TCHAR* AssetName, TSubclassOf<UUserWidget> ParentClass)
{
	if (UWidgetBlueprint* ExistingBlueprint = LoadWidgetBlueprint(AssetName))
	{
		UE_LOG(LogTemp, Display, TEXT("CommonUI widget already exists: %s/%s"), CommonUIComponentsPath, AssetName);
		return ExistingBlueprint;
	}

	UWidgetBlueprintFactory* Factory = NewObject<UWidgetBlueprintFactory>();
	Factory->BlueprintType = BPTYPE_Normal;
	Factory->ParentClass = ParentClass;

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(AssetToolsModule.Get().CreateAsset(
		AssetName,
		CommonUIComponentsPath,
		UWidgetBlueprint::StaticClass(),
		Factory));

	if (WidgetBlueprint)
	{
		FAssetRegistryModule::AssetCreated(WidgetBlueprint);
		WidgetBlueprint->MarkPackageDirty();
	}
	return WidgetBlueprint;
}

static bool SaveAsset(UObject* Asset)
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

static void ConfigureWorldStatusMarker(UWidgetBlueprint* WidgetBlueprint)
{
	if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || WidgetBlueprint->WidgetTree->RootWidget)
	{
		return;
	}

	WidgetBlueprint->Modify();
	WidgetBlueprint->WidgetTree->Modify();

	UCommonBorder* MarkerBorder = WidgetBlueprint->WidgetTree->ConstructWidget<UCommonBorder>(UCommonBorder::StaticClass(), TEXT("MarkerBorder"));
	UCommonTextBlock* MarkerText = WidgetBlueprint->WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), TEXT("MarkerText"));
	if (!MarkerBorder || !MarkerText)
	{
		return;
	}

	MarkerBorder->SetStyle(LoadStyleClass<UCommonBorderStyle>(SlotBorderStylePath));
	MarkerText->SetText(FText::FromString(TEXT("TARGET")));
	MarkerText->SetStyle(LoadStyleClass<UCommonTextStyle>(BodyTextStylePath));
	MarkerBorder->SetContent(MarkerText);
	WidgetBlueprint->WidgetTree->RootWidget = MarkerBorder;
	WidgetBlueprint->MarkPackageDirty();
}

static void ConfigureTrajectoryPreviewPoint(UWidgetBlueprint* WidgetBlueprint)
{
	if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || WidgetBlueprint->WidgetTree->RootWidget)
	{
		return;
	}

	WidgetBlueprint->Modify();
	WidgetBlueprint->WidgetTree->Modify();

	UCommonBorder* PointBorder = WidgetBlueprint->WidgetTree->ConstructWidget<UCommonBorder>(UCommonBorder::StaticClass(), TEXT("PointBorder"));
	UHorizontalBox* PointRow = WidgetBlueprint->WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("PointRow"));
	UCommonNumericTextBlock* PointIndexText = WidgetBlueprint->WidgetTree->ConstructWidget<UCommonNumericTextBlock>(UCommonNumericTextBlock::StaticClass(), TEXT("PointIndexText"));
	UCommonTextBlock* PointText = WidgetBlueprint->WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), TEXT("PointText"));
	if (!PointBorder || !PointRow || !PointIndexText || !PointText)
	{
		return;
	}

	PointBorder->SetStyle(LoadStyleClass<UCommonBorderStyle>(SlotBorderStylePath));
	PointIndexText->SetCurrentValue(1.0f);
	PointIndexText->SetStyle(LoadStyleClass<UCommonTextStyle>(NumberTextStylePath));
	PointText->SetText(FText::FromString(TEXT("HIT")));
	PointText->SetStyle(LoadStyleClass<UCommonTextStyle>(BodyTextStylePath));

	if (UHorizontalBoxSlot* NumberSlot = PointRow->AddChildToHorizontalBox(PointIndexText))
	{
		NumberSlot->SetPadding(FMargin(0.0f, 0.0f, 4.0f, 0.0f));
	}
	PointRow->AddChildToHorizontalBox(PointText);
	PointBorder->SetContent(PointRow);
	WidgetBlueprint->WidgetTree->RootWidget = PointBorder;
	WidgetBlueprint->MarkPackageDirty();
}

int32 GenerateCommonUIWidgets()
{
	TArray<UWidgetBlueprint*> WidgetBlueprints;

	UWidgetBlueprint* WorldStatusMarker = CreateWidgetBlueprint(TEXT("WBP_WorldStatusMarker"), UFRWorldStatusMarkerWidget::StaticClass());
	ConfigureWorldStatusMarker(WorldStatusMarker);
	WidgetBlueprints.Add(WorldStatusMarker);

	UWidgetBlueprint* TrajectoryPreviewPoint = CreateWidgetBlueprint(TEXT("WBP_TrajectoryPreviewPoint"), UFRTrajectoryPreviewPointWidget::StaticClass());
	ConfigureTrajectoryPreviewPoint(TrajectoryPreviewPoint);
	WidgetBlueprints.Add(TrajectoryPreviewPoint);

	bool bSavedAll = true;
	for (UWidgetBlueprint* WidgetBlueprint : WidgetBlueprints)
	{
		if (!WidgetBlueprint)
		{
			bSavedAll = false;
			continue;
		}

		FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
		bSavedAll &= SaveAsset(WidgetBlueprint);
	}

	UE_LOG(LogTemp, Display, TEXT("Generated FortRogue CommonUI widget assets. SavedAll=%s"), bSavedAll ? TEXT("true") : TEXT("false"));
	return bSavedAll ? 0 : 1;
}
}
