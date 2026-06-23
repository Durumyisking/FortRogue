// Copyright Epic Games, Inc. All Rights Reserved.

#include "FRCommonUIWidgetGenerator.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/BlueprintExtension.h"
#include "CommonBorder.h"
#include "CommonButtonBase.h"
#include "CommonNumericTextBlock.h"
#include "CommonTextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ProgressBar.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "HAL/FileManager.h"
#include "IAssetTools.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "MVVMWidgetBlueprintExtension_View.h"
#include "UI/FRBattleHUDModuleWidgets.h"
#include "UI/FRTrajectoryPreviewPointWidget.h"
#include "UI/FRMenuWidgets.h"
#include "UI/FRUIRootWidget.h"
#include "UI/FRWorldStatusMarkerWidget.h"
#include "UObject/SavePackage.h"
#include "WidgetBlueprint.h"
#include "WidgetBlueprintFactory.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

namespace FREditor
{
static const TCHAR* CommonUIComponentsPath = TEXT("/Game/FortRogue/Widget/MainGame/Components");
static const TCHAR* CommonUIMainGamePath = TEXT("/Game/FortRogue/Widget/MainGame");
static const TCHAR* CommonUIGlobalPath = TEXT("/Game/FortRogue/Widget/Global");
static const TCHAR* CommonUIGlobalComponentsPath = TEXT("/Game/FortRogue/Widget/Global/Components");
static const TCHAR* CommonUIButtonsPath = TEXT("/Game/FortRogue/Widget/Global/Components/Buttons");
static const TCHAR* CommonUIMainMenuPath = TEXT("/Game/FortRogue/Widget/MainMenu");
static const TCHAR* BodyTextStylePath = TEXT("/Game/FortRogue/Widget/Styles/BP_UI_Text_Body.BP_UI_Text_Body_C");
static const TCHAR* TitleTextStylePath = TEXT("/Game/FortRogue/Widget/Styles/BP_UI_Text_Title.BP_UI_Text_Title_C");
static const TCHAR* NumberTextStylePath = TEXT("/Game/FortRogue/Widget/Styles/BP_UI_Text_Number.BP_UI_Text_Number_C");
static const TCHAR* PanelBorderStylePath = TEXT("/Game/FortRogue/Widget/Styles/BP_UI_Border_Panel.BP_UI_Border_Panel_C");
static const TCHAR* SlotBorderStylePath = TEXT("/Game/FortRogue/Widget/Styles/BP_UI_Border_Slot.BP_UI_Border_Slot_C");
static const TCHAR* StartRunButtonClassPath = TEXT("/Game/FortRogue/Widget/Global/Components/Buttons/WBP_Button_StartRun.WBP_Button_StartRun_C");
static const TCHAR* OptionsButtonClassPath = TEXT("/Game/FortRogue/Widget/Global/Components/Buttons/WBP_Button_Options.WBP_Button_Options_C");
static const TCHAR* QuitButtonClassPath = TEXT("/Game/FortRogue/Widget/Global/Components/Buttons/WBP_Button_Quit.WBP_Button_Quit_C");
static const TCHAR* BackButtonClassPath = TEXT("/Game/FortRogue/Widget/Global/Components/Buttons/WBP_Button_Back.WBP_Button_Back_C");
static const TCHAR* ConfirmButtonClassPath = TEXT("/Game/FortRogue/Widget/Global/Components/Buttons/WBP_Button_Confirm.WBP_Button_Confirm_C");
static const TCHAR* CancelButtonClassPath = TEXT("/Game/FortRogue/Widget/Global/Components/Buttons/WBP_Button_Cancel.WBP_Button_Cancel_C");
static const TCHAR* ResumeButtonClassPath = TEXT("/Game/FortRogue/Widget/Global/Components/Buttons/WBP_Button_Resume.WBP_Button_Resume_C");
static const TCHAR* MainMenuButtonClassPath = TEXT("/Game/FortRogue/Widget/Global/Components/Buttons/WBP_Button_MainMenu.WBP_Button_MainMenu_C");

template <typename StyleType>
static TSubclassOf<StyleType> LoadStyleClass(const TCHAR* ClassPath)
{
	return LoadClass<StyleType>(nullptr, ClassPath);
}

static UWidgetBlueprint* LoadWidgetBlueprint(const TCHAR* AssetPath, const TCHAR* AssetName)
{
	const FString ObjectPath = FString::Printf(TEXT("%s/%s.%s"), AssetPath, AssetName, AssetName);
	return LoadObject<UWidgetBlueprint>(nullptr, *ObjectPath);
}

static bool IsWidgetBlueprintParentedTo(const UWidgetBlueprint* WidgetBlueprint, UClass* ParentClass)
{
	if (!WidgetBlueprint || !ParentClass)
	{
		return false;
	}

	if (WidgetBlueprint->GeneratedClass && WidgetBlueprint->GeneratedClass->IsChildOf(ParentClass))
	{
		return true;
	}

	return WidgetBlueprint->ParentClass && WidgetBlueprint->ParentClass->IsChildOf(ParentClass);
}

static void RenameWidgetTemplate(UWidgetBlueprint* WidgetBlueprint, const FName& OldName, const FName& NewName)
{
	if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || WidgetBlueprint->WidgetTree->FindWidget(NewName))
	{
		return;
	}

	UWidget* Widget = WidgetBlueprint->WidgetTree->FindWidget(OldName);
	if (!Widget)
	{
		return;
	}

	WidgetBlueprint->Modify();
	WidgetBlueprint->WidgetTree->Modify();
	Widget->Modify();

	FGuid VariableGuid;
	if (WidgetBlueprint->WidgetVariableNameToGuidMap.RemoveAndCopyValue(OldName, VariableGuid))
	{
		WidgetBlueprint->WidgetVariableNameToGuidMap.Add(NewName, VariableGuid);
	}

	Widget->Rename(*NewName.ToString(), WidgetBlueprint->WidgetTree, REN_DontCreateRedirectors);
	WidgetBlueprint->MarkPackageDirty();
}

static void RepairUIRootLayerNames(UWidgetBlueprint* WidgetBlueprint)
{
	RenameWidgetTemplate(WidgetBlueprint, TEXT("HUDLayerStack"), TEXT("HUDLayer"));
	RenameWidgetTemplate(WidgetBlueprint, TEXT("MenuLayerStack"), TEXT("MenuLayer"));
	RenameWidgetTemplate(WidgetBlueprint, TEXT("ModalLayerStack"), TEXT("ModalLayer"));
}

static void RemovePrototypeMVVMBlueprintExtension(UWidgetBlueprint* WidgetBlueprint)
{
	if (!WidgetBlueprint)
	{
		return;
	}

	const int32 RemovedCount = WidgetBlueprint->RemoveAllExtension([](UBlueprintExtension* Extension)
	{
		return Extension && Extension->IsA<UMVVMWidgetBlueprintExtension_View>();
	});
	if (RemovedCount <= 0)
	{
		return;
	}

	WidgetBlueprint->Modify();
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(WidgetBlueprint);
	WidgetBlueprint->MarkPackageDirty();
	UE_LOG(LogTemp, Display, TEXT("Removed prototype MVVM Blueprint extension from %s"), *WidgetBlueprint->GetPathName());
}

static UWidgetBlueprint* CreateWidgetBlueprint(const TCHAR* AssetPath, const TCHAR* AssetName, TSubclassOf<UUserWidget> ParentClass, bool& bOutNeedsConfigure)
{
	bOutNeedsConfigure = false;
	if (UWidgetBlueprint* ExistingBlueprint = LoadWidgetBlueprint(AssetPath, AssetName))
	{
		const bool bParentMatches = IsWidgetBlueprintParentedTo(ExistingBlueprint, ParentClass);
		bOutNeedsConfigure = !ExistingBlueprint->WidgetTree || !ExistingBlueprint->WidgetTree->RootWidget;
		if (!bParentMatches || bOutNeedsConfigure)
		{
			UE_LOG(LogTemp, Display, TEXT("Repairing CommonUI widget: %s/%s"), AssetPath, AssetName);
		}
		if (!bParentMatches)
		{
			ExistingBlueprint->Modify();
			ExistingBlueprint->ParentClass = ParentClass;
			FBlueprintEditorUtils::RefreshAllNodes(ExistingBlueprint);
			FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(ExistingBlueprint);
		}
		if (!bOutNeedsConfigure)
		{
			UE_LOG(LogTemp, Display, TEXT("CommonUI widget already exists: %s/%s"), AssetPath, AssetName);
		}
		return ExistingBlueprint;
	}

	bOutNeedsConfigure = true;
	UWidgetBlueprintFactory* Factory = NewObject<UWidgetBlueprintFactory>();
	Factory->BlueprintType = BPTYPE_Normal;
	Factory->ParentClass = ParentClass;

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(AssetToolsModule.Get().CreateAsset(
		AssetName,
		AssetPath,
		UWidgetBlueprint::StaticClass(),
		Factory));

	if (WidgetBlueprint)
	{
		FAssetRegistryModule::AssetCreated(WidgetBlueprint);
		WidgetBlueprint->MarkPackageDirty();
	}
	return WidgetBlueprint;
}

static UCommonTextBlock* ConstructText(UWidgetBlueprint* WidgetBlueprint, const TCHAR* WidgetName, const FText& Text, TSubclassOf<UCommonTextStyle> TextStyle)
{
	UCommonTextBlock* TextBlock = WidgetBlueprint && WidgetBlueprint->WidgetTree
		? WidgetBlueprint->WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), WidgetName)
		: nullptr;
	if (TextBlock)
	{
		TextBlock->SetText(Text);
		TextBlock->SetStyle(TextStyle);
	}
	return TextBlock;
}

static UCommonNumericTextBlock* ConstructNumber(UWidgetBlueprint* WidgetBlueprint, const TCHAR* WidgetName, float Value)
{
	UCommonNumericTextBlock* TextBlock = WidgetBlueprint && WidgetBlueprint->WidgetTree
		? WidgetBlueprint->WidgetTree->ConstructWidget<UCommonNumericTextBlock>(UCommonNumericTextBlock::StaticClass(), WidgetName)
		: nullptr;
	if (TextBlock)
	{
		TextBlock->SetCurrentValue(Value);
		TextBlock->SetStyle(LoadStyleClass<UCommonTextStyle>(NumberTextStylePath));
	}
	return TextBlock;
}

static UProgressBar* ConstructProgressBar(UWidgetBlueprint* WidgetBlueprint, const TCHAR* WidgetName, float Percent)
{
	UProgressBar* ProgressBar = WidgetBlueprint && WidgetBlueprint->WidgetTree
		? WidgetBlueprint->WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), WidgetName)
		: nullptr;
	if (ProgressBar)
	{
		ProgressBar->SetPercent(FMath::Clamp(Percent, 0.0f, 1.0f));
	}
	return ProgressBar;
}

static UCommonButtonBase* ConstructButton(UWidgetBlueprint* WidgetBlueprint, const TCHAR* WidgetName, const TCHAR* ButtonClassPath)
{
	TSubclassOf<UCommonButtonBase> ButtonClass = LoadClass<UCommonButtonBase>(nullptr, ButtonClassPath);
	if (!ButtonClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not load button class for %s: %s"), WidgetName, ButtonClassPath);
		return nullptr;
	}

	return WidgetBlueprint && WidgetBlueprint->WidgetTree
		? WidgetBlueprint->WidgetTree->ConstructWidget<UCommonButtonBase>(ButtonClass, WidgetName)
		: nullptr;
}

static void AddVerticalChild(UVerticalBox* Parent, UWidget* Child, const FMargin& Padding = FMargin(0.0f, 0.0f, 0.0f, 8.0f))
{
	if (!Parent || !Child)
	{
		return;
	}

	if (UVerticalBoxSlot* Slot = Parent->AddChildToVerticalBox(Child))
	{
		Slot->SetPadding(Padding);
		Slot->SetHorizontalAlignment(HAlign_Fill);
	}
}

static void AddHorizontalChild(UHorizontalBox* Parent, UWidget* Child, const FMargin& Padding = FMargin(0.0f, 0.0f, 8.0f, 0.0f))
{
	if (!Parent || !Child)
	{
		return;
	}

	if (UHorizontalBoxSlot* Slot = Parent->AddChildToHorizontalBox(Child))
	{
		Slot->SetPadding(Padding);
		Slot->SetVerticalAlignment(VAlign_Center);
	}
}

static void AddOverlayFillChild(UOverlay* Parent, UWidget* Child)
{
	if (!Parent || !Child)
	{
		return;
	}

	if (UOverlaySlot* Slot = Parent->AddChildToOverlay(Child))
	{
		Slot->SetHorizontalAlignment(HAlign_Fill);
		Slot->SetVerticalAlignment(VAlign_Fill);
	}
}

static UVerticalBox* CreateMenuPanel(UWidgetBlueprint* WidgetBlueprint, const TCHAR* RootName)
{
	if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree)
	{
		return nullptr;
	}

	UCommonBorder* RootBorder = WidgetBlueprint->WidgetTree->ConstructWidget<UCommonBorder>(UCommonBorder::StaticClass(), RootName);
	UVerticalBox* RootBox = WidgetBlueprint->WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ContentBox"));
	if (!RootBorder || !RootBox)
	{
		return nullptr;
	}

	RootBorder->SetStyle(LoadStyleClass<UCommonBorderStyle>(PanelBorderStylePath));
	RootBorder->SetContent(RootBox);
	WidgetBlueprint->WidgetTree->RootWidget = RootBorder;
	return RootBox;
}

static void AddOptionRow(UWidgetBlueprint* WidgetBlueprint, UVerticalBox* Parent, const TCHAR* RowName, const TCHAR* LabelName, const FText& LabelText, UWidget* ValueWidget)
{
	UHorizontalBox* Row = WidgetBlueprint && WidgetBlueprint->WidgetTree
		? WidgetBlueprint->WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), RowName)
		: nullptr;
	if (!Row)
	{
		return;
	}

	UCommonTextBlock* Label = ConstructText(WidgetBlueprint, LabelName, LabelText, LoadStyleClass<UCommonTextStyle>(BodyTextStylePath));
	if (Label)
	{
		if (UHorizontalBoxSlot* LabelSlot = Row->AddChildToHorizontalBox(Label))
		{
			LabelSlot->SetPadding(FMargin(0.0f, 0.0f, 12.0f, 0.0f));
		}
	}
	if (ValueWidget)
	{
		Row->AddChildToHorizontalBox(ValueWidget);
	}
	AddVerticalChild(Parent, Row, FMargin(0.0f, 0.0f, 0.0f, 6.0f));
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

static void ConfigureUIRoot(UWidgetBlueprint* WidgetBlueprint)
{
	if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || WidgetBlueprint->WidgetTree->RootWidget)
	{
		return;
	}

	WidgetBlueprint->Modify();
	WidgetBlueprint->WidgetTree->Modify();

	UOverlay* RootOverlay = WidgetBlueprint->WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("RootOverlay"));
	UCommonActivatableWidgetStack* HUDLayer = WidgetBlueprint->WidgetTree->ConstructWidget<UCommonActivatableWidgetStack>(UCommonActivatableWidgetStack::StaticClass(), TEXT("HUDLayer"));
	UCommonActivatableWidgetStack* MenuLayer = WidgetBlueprint->WidgetTree->ConstructWidget<UCommonActivatableWidgetStack>(UCommonActivatableWidgetStack::StaticClass(), TEXT("MenuLayer"));
	UCommonActivatableWidgetStack* ModalLayer = WidgetBlueprint->WidgetTree->ConstructWidget<UCommonActivatableWidgetStack>(UCommonActivatableWidgetStack::StaticClass(), TEXT("ModalLayer"));
	if (!RootOverlay || !HUDLayer || !MenuLayer || !ModalLayer)
	{
		return;
	}

	AddOverlayFillChild(RootOverlay, HUDLayer);
	AddOverlayFillChild(RootOverlay, MenuLayer);
	AddOverlayFillChild(RootOverlay, ModalLayer);
	WidgetBlueprint->WidgetTree->RootWidget = RootOverlay;
	WidgetBlueprint->MarkPackageDirty();
}

static void ConfigureMainMenu(UWidgetBlueprint* WidgetBlueprint)
{
	if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || WidgetBlueprint->WidgetTree->RootWidget)
	{
		return;
	}

	WidgetBlueprint->Modify();
	WidgetBlueprint->WidgetTree->Modify();

	UVerticalBox* RootBox = CreateMenuPanel(WidgetBlueprint, TEXT("MainMenuPanel"));
	if (!RootBox)
	{
		return;
	}

	AddVerticalChild(RootBox, ConstructText(WidgetBlueprint, TEXT("TitleText"), FText::FromString(TEXT("FortRogue")), LoadStyleClass<UCommonTextStyle>(TitleTextStylePath)));
	AddVerticalChild(RootBox, ConstructText(WidgetBlueprint, TEXT("BodyText"), FText::FromString(TEXT("Tank tactics. One clean shot at a time.")), LoadStyleClass<UCommonTextStyle>(BodyTextStylePath)));
	AddVerticalChild(RootBox, ConstructText(WidgetBlueprint, TEXT("StatusText"), FText::GetEmpty(), LoadStyleClass<UCommonTextStyle>(BodyTextStylePath)));
	AddVerticalChild(RootBox, ConstructButton(WidgetBlueprint, TEXT("StartRunButton"), StartRunButtonClassPath));
	AddVerticalChild(RootBox, ConstructButton(WidgetBlueprint, TEXT("OptionsButton"), OptionsButtonClassPath));
	AddVerticalChild(RootBox, ConstructButton(WidgetBlueprint, TEXT("QuitButton"), QuitButtonClassPath));
	WidgetBlueprint->MarkPackageDirty();
}

static void ConfigureOptionsMenu(UWidgetBlueprint* WidgetBlueprint)
{
	if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || WidgetBlueprint->WidgetTree->RootWidget)
	{
		return;
	}

	WidgetBlueprint->Modify();
	WidgetBlueprint->WidgetTree->Modify();

	UVerticalBox* RootBox = CreateMenuPanel(WidgetBlueprint, TEXT("OptionsPanel"));
	if (!RootBox)
	{
		return;
	}

	AddVerticalChild(RootBox, ConstructText(WidgetBlueprint, TEXT("TitleText"), FText::FromString(TEXT("Options")), LoadStyleClass<UCommonTextStyle>(TitleTextStylePath)));
	AddOptionRow(WidgetBlueprint, RootBox, TEXT("MasterVolumeRow"), TEXT("MasterVolumeLabel"), FText::FromString(TEXT("Master Volume")), ConstructNumber(WidgetBlueprint, TEXT("MasterVolumeText"), 100.0f));
	AddOptionRow(WidgetBlueprint, RootBox, TEXT("UIScaleRow"), TEXT("UIScaleLabel"), FText::FromString(TEXT("UI Scale")), ConstructNumber(WidgetBlueprint, TEXT("UIScaleText"), 100.0f));
	AddOptionRow(WidgetBlueprint, RootBox, TEXT("WindowModeRow"), TEXT("WindowModeLabel"), FText::FromString(TEXT("Window Mode")), ConstructText(WidgetBlueprint, TEXT("WindowModeText"), FText::FromString(TEXT("Windowed Fullscreen")), LoadStyleClass<UCommonTextStyle>(BodyTextStylePath)));
	AddOptionRow(WidgetBlueprint, RootBox, TEXT("ResolutionRow"), TEXT("ResolutionLabel"), FText::FromString(TEXT("Resolution")), ConstructText(WidgetBlueprint, TEXT("ResolutionText"), FText::FromString(TEXT("Native")), LoadStyleClass<UCommonTextStyle>(BodyTextStylePath)));
	AddOptionRow(WidgetBlueprint, RootBox, TEXT("InputHintsRow"), TEXT("InputHintsLabel"), FText::FromString(TEXT("Input Hints")), ConstructText(WidgetBlueprint, TEXT("InputHintsText"), FText::FromString(TEXT("On")), LoadStyleClass<UCommonTextStyle>(BodyTextStylePath)));
	AddOptionRow(WidgetBlueprint, RootBox, TEXT("AccessibilityRow"), TEXT("AccessibilityLabel"), FText::FromString(TEXT("Accessibility")), ConstructText(WidgetBlueprint, TEXT("AccessibilityText"), FText::FromString(TEXT("Default")), LoadStyleClass<UCommonTextStyle>(BodyTextStylePath)));
	AddVerticalChild(RootBox, ConstructButton(WidgetBlueprint, TEXT("ApplyButton"), ConfirmButtonClassPath));
	AddVerticalChild(RootBox, ConstructButton(WidgetBlueprint, TEXT("ResetButton"), CancelButtonClassPath));
	AddVerticalChild(RootBox, ConstructButton(WidgetBlueprint, TEXT("BackButton"), BackButtonClassPath));
	WidgetBlueprint->MarkPackageDirty();
}

static void ConfigurePauseMenu(UWidgetBlueprint* WidgetBlueprint)
{
	if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || WidgetBlueprint->WidgetTree->RootWidget)
	{
		return;
	}

	WidgetBlueprint->Modify();
	WidgetBlueprint->WidgetTree->Modify();

	UVerticalBox* RootBox = CreateMenuPanel(WidgetBlueprint, TEXT("PausePanel"));
	if (!RootBox)
	{
		return;
	}

	AddVerticalChild(RootBox, ConstructText(WidgetBlueprint, TEXT("TitleText"), FText::FromString(TEXT("Paused")), LoadStyleClass<UCommonTextStyle>(TitleTextStylePath)));
	AddVerticalChild(RootBox, ConstructText(WidgetBlueprint, TEXT("BodyText"), FText::FromString(TEXT("Run suspended.")), LoadStyleClass<UCommonTextStyle>(BodyTextStylePath)));
	AddVerticalChild(RootBox, ConstructText(WidgetBlueprint, TEXT("StatusText"), FText::GetEmpty(), LoadStyleClass<UCommonTextStyle>(BodyTextStylePath)));
	AddVerticalChild(RootBox, ConstructButton(WidgetBlueprint, TEXT("ResumeButton"), ResumeButtonClassPath));
	AddVerticalChild(RootBox, ConstructButton(WidgetBlueprint, TEXT("OptionsButton"), OptionsButtonClassPath));
	AddVerticalChild(RootBox, ConstructButton(WidgetBlueprint, TEXT("RestartButton"), ConfirmButtonClassPath));
	AddVerticalChild(RootBox, ConstructButton(WidgetBlueprint, TEXT("MainMenuButton"), MainMenuButtonClassPath));
	AddVerticalChild(RootBox, ConstructButton(WidgetBlueprint, TEXT("QuitButton"), QuitButtonClassPath));
	WidgetBlueprint->MarkPackageDirty();
}

static void ConfigureConfirmDialog(UWidgetBlueprint* WidgetBlueprint)
{
	if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || WidgetBlueprint->WidgetTree->RootWidget)
	{
		return;
	}

	WidgetBlueprint->Modify();
	WidgetBlueprint->WidgetTree->Modify();

	UVerticalBox* RootBox = CreateMenuPanel(WidgetBlueprint, TEXT("ConfirmPanel"));
	if (!RootBox)
	{
		return;
	}

	AddVerticalChild(RootBox, ConstructText(WidgetBlueprint, TEXT("TitleText"), FText::FromString(TEXT("Confirm")), LoadStyleClass<UCommonTextStyle>(TitleTextStylePath)));
	AddVerticalChild(RootBox, ConstructText(WidgetBlueprint, TEXT("MessageText"), FText::FromString(TEXT("Continue?")), LoadStyleClass<UCommonTextStyle>(BodyTextStylePath)));
	AddVerticalChild(RootBox, ConstructButton(WidgetBlueprint, TEXT("ConfirmButton"), ConfirmButtonClassPath));
	AddVerticalChild(RootBox, ConstructButton(WidgetBlueprint, TEXT("CancelButton"), CancelButtonClassPath));
	WidgetBlueprint->MarkPackageDirty();
}

static UVerticalBox* CreateHUDModuleRoot(UWidgetBlueprint* WidgetBlueprint, const TCHAR* RootName)
{
	if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree)
	{
		return nullptr;
	}

	UCommonBorder* RootBorder = WidgetBlueprint->WidgetTree->ConstructWidget<UCommonBorder>(UCommonBorder::StaticClass(), RootName);
	UVerticalBox* RootBox = WidgetBlueprint->WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RootBox"));
	if (!RootBorder || !RootBox)
	{
		return nullptr;
	}

	RootBorder->SetStyle(LoadStyleClass<UCommonBorderStyle>(PanelBorderStylePath));
	RootBorder->SetContent(RootBox);
	WidgetBlueprint->WidgetTree->RootWidget = RootBorder;
	return RootBox;
}

static void ConfigureTurnBanner(UWidgetBlueprint* WidgetBlueprint)
{
	if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || WidgetBlueprint->WidgetTree->RootWidget)
	{
		return;
	}

	WidgetBlueprint->Modify();
	WidgetBlueprint->WidgetTree->Modify();

	UHorizontalBox* RootBox = WidgetBlueprint->WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("RootBox"));
	if (!RootBox)
	{
		return;
	}

	AddHorizontalChild(RootBox, ConstructText(WidgetBlueprint, TEXT("TurnText"), FText::FromString(TEXT("PLAYER TURN")), LoadStyleClass<UCommonTextStyle>(TitleTextStylePath)));
	AddHorizontalChild(RootBox, ConstructText(WidgetBlueprint, TEXT("RunProgressText"), FText::FromString(TEXT("Stage 1/7")), LoadStyleClass<UCommonTextStyle>(BodyTextStylePath)));
	AddHorizontalChild(RootBox, ConstructText(WidgetBlueprint, TEXT("StatusText"), FText::FromString(TEXT("Move, aim, select shell, then hold fire")), LoadStyleClass<UCommonTextStyle>(BodyTextStylePath)));
	WidgetBlueprint->WidgetTree->RootWidget = RootBox;
	WidgetBlueprint->MarkPackageDirty();
}

static void ConfigureCombatantStatusPanel(UWidgetBlueprint* WidgetBlueprint)
{
	if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || WidgetBlueprint->WidgetTree->RootWidget)
	{
		return;
	}

	WidgetBlueprint->Modify();
	WidgetBlueprint->WidgetTree->Modify();

	UVerticalBox* RootBox = CreateHUDModuleRoot(WidgetBlueprint, TEXT("StatusPanelRoot"));
	if (!RootBox)
	{
		return;
	}

	AddVerticalChild(RootBox, ConstructText(WidgetBlueprint, TEXT("TitleText"), FText::FromString(TEXT("PLAYER ARMOR")), LoadStyleClass<UCommonTextStyle>(BodyTextStylePath)));
	AddVerticalChild(RootBox, ConstructText(WidgetBlueprint, TEXT("HealthText"), FText::FromString(TEXT("115 / 115")), LoadStyleClass<UCommonTextStyle>(BodyTextStylePath)));
	AddVerticalChild(RootBox, ConstructProgressBar(WidgetBlueprint, TEXT("HealthBar"), 1.0f));
	AddVerticalChild(RootBox, ConstructText(WidgetBlueprint, TEXT("MoveBudgetText"), FText::FromString(TEXT("390 / 390")), LoadStyleClass<UCommonTextStyle>(BodyTextStylePath)));
	AddVerticalChild(RootBox, ConstructProgressBar(WidgetBlueprint, TEXT("MoveBudgetBar"), 1.0f));
	WidgetBlueprint->MarkPackageDirty();
}

static void ConfigureAimWindIndicator(UWidgetBlueprint* WidgetBlueprint)
{
	if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || WidgetBlueprint->WidgetTree->RootWidget)
	{
		return;
	}

	WidgetBlueprint->Modify();
	WidgetBlueprint->WidgetTree->Modify();

	UVerticalBox* RootBox = CreateHUDModuleRoot(WidgetBlueprint, TEXT("AimWindRoot"));
	if (!RootBox)
	{
		return;
	}

	AddVerticalChild(RootBox, ConstructText(WidgetBlueprint, TEXT("WindText"), FText::FromString(TEXT("Wind <- 78")), LoadStyleClass<UCommonTextStyle>(BodyTextStylePath)));
	AddVerticalChild(RootBox, ConstructNumber(WidgetBlueprint, TEXT("WindValueText"), -78.0f));
	AddVerticalChild(RootBox, ConstructText(WidgetBlueprint, TEXT("AimText"), FText::FromString(TEXT("Aim 45 deg")), LoadStyleClass<UCommonTextStyle>(BodyTextStylePath)));
	AddVerticalChild(RootBox, ConstructNumber(WidgetBlueprint, TEXT("AimAngleValueText"), 45.0f));
	WidgetBlueprint->MarkPackageDirty();
}

static void ConfigureShotPowerMeter(UWidgetBlueprint* WidgetBlueprint)
{
	if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || WidgetBlueprint->WidgetTree->RootWidget)
	{
		return;
	}

	WidgetBlueprint->Modify();
	WidgetBlueprint->WidgetTree->Modify();

	UVerticalBox* RootBox = CreateHUDModuleRoot(WidgetBlueprint, TEXT("ShotPowerRoot"));
	if (!RootBox)
	{
		return;
	}

	AddVerticalChild(RootBox, ConstructText(WidgetBlueprint, TEXT("ShotPowerText"), FText::FromString(TEXT("0%")), LoadStyleClass<UCommonTextStyle>(BodyTextStylePath)));
	AddVerticalChild(RootBox, ConstructNumber(WidgetBlueprint, TEXT("ShotPowerValueText"), 0.0f));
	AddVerticalChild(RootBox, ConstructProgressBar(WidgetBlueprint, TEXT("ShotPowerBar"), 0.0f));
	WidgetBlueprint->MarkPackageDirty();
}

static void ConfigureLoadoutSlot(UWidgetBlueprint* WidgetBlueprint, const FText& SlotLabel, const FText& DisplayText)
{
	if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || WidgetBlueprint->WidgetTree->RootWidget)
	{
		return;
	}

	WidgetBlueprint->Modify();
	WidgetBlueprint->WidgetTree->Modify();

	UVerticalBox* RootBox = CreateHUDModuleRoot(WidgetBlueprint, TEXT("SlotRoot"));
	if (!RootBox)
	{
		return;
	}

	AddVerticalChild(RootBox, ConstructText(WidgetBlueprint, TEXT("SlotLabelText"), SlotLabel, LoadStyleClass<UCommonTextStyle>(BodyTextStylePath)));
	AddVerticalChild(RootBox, ConstructText(WidgetBlueprint, TEXT("DisplayText"), DisplayText, LoadStyleClass<UCommonTextStyle>(BodyTextStylePath)));
	AddVerticalChild(RootBox, ConstructNumber(WidgetBlueprint, TEXT("CountValueText"), 0.0f));
	AddVerticalChild(RootBox, ConstructText(WidgetBlueprint, TEXT("StatusText"), FText::GetEmpty(), LoadStyleClass<UCommonTextStyle>(BodyTextStylePath)));
	WidgetBlueprint->MarkPackageDirty();
}

static void ConfigureLoadoutBar(UWidgetBlueprint* WidgetBlueprint)
{
	if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || WidgetBlueprint->WidgetTree->RootWidget)
	{
		return;
	}

	WidgetBlueprint->Modify();
	WidgetBlueprint->WidgetTree->Modify();

	UVerticalBox* RootBox = CreateHUDModuleRoot(WidgetBlueprint, TEXT("LoadoutRoot"));
	if (!RootBox)
	{
		return;
	}

	AddVerticalChild(RootBox, ConstructText(WidgetBlueprint, TEXT("CurrentWeaponText"), FText::FromString(TEXT("Basic Shell")), LoadStyleClass<UCommonTextStyle>(BodyTextStylePath)));

	UHorizontalBox* WeaponSlotPanel = WidgetBlueprint->WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("WeaponSlotPanel"));
	UHorizontalBox* ItemSlotPanel = WidgetBlueprint->WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ItemSlotPanel"));
	AddVerticalChild(RootBox, WeaponSlotPanel);
	AddVerticalChild(RootBox, ItemSlotPanel);
	WidgetBlueprint->MarkPackageDirty();
}

static void ConfigureShotInfoPanel(UWidgetBlueprint* WidgetBlueprint)
{
	if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || WidgetBlueprint->WidgetTree->RootWidget)
	{
		return;
	}

	WidgetBlueprint->Modify();
	WidgetBlueprint->WidgetTree->Modify();

	UVerticalBox* RootBox = CreateHUDModuleRoot(WidgetBlueprint, TEXT("ShotInfoRoot"));
	if (!RootBox)
	{
		return;
	}

	AddVerticalChild(RootBox, ConstructText(WidgetBlueprint, TEXT("PrimaryText"), FText::FromString(TEXT("Damage 39  Blast 135")), LoadStyleClass<UCommonTextStyle>(BodyTextStylePath)));
	AddVerticalChild(RootBox, ConstructText(WidgetBlueprint, TEXT("SecondaryText"), FText::FromString(TEXT("Speed 300  Gravity 980")), LoadStyleClass<UCommonTextStyle>(BodyTextStylePath)));
	AddVerticalChild(RootBox, ConstructNumber(WidgetBlueprint, TEXT("DamageValueText"), 39.0f));
	AddVerticalChild(RootBox, ConstructNumber(WidgetBlueprint, TEXT("BlastRadiusValueText"), 135.0f));
	AddVerticalChild(RootBox, ConstructNumber(WidgetBlueprint, TEXT("ProjectileCountValueText"), 1.0f));
	WidgetBlueprint->MarkPackageDirty();
}

static void ConfigureModifierSummary(UWidgetBlueprint* WidgetBlueprint)
{
	if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || WidgetBlueprint->WidgetTree->RootWidget)
	{
		return;
	}

	WidgetBlueprint->Modify();
	WidgetBlueprint->WidgetTree->Modify();

	UVerticalBox* RootBox = CreateHUDModuleRoot(WidgetBlueprint, TEXT("ModifierRoot"));
	if (!RootBox)
	{
		return;
	}

	AddVerticalChild(RootBox, ConstructText(WidgetBlueprint, TEXT("GrantedModifierText"), FText::FromString(TEXT("None")), LoadStyleClass<UCommonTextStyle>(BodyTextStylePath)));
	AddVerticalChild(RootBox, ConstructText(WidgetBlueprint, TEXT("PendingModifierText"), FText::FromString(TEXT("None")), LoadStyleClass<UCommonTextStyle>(BodyTextStylePath)));
	AddVerticalChild(RootBox, ConstructText(WidgetBlueprint, TEXT("AbilitySetText"), FText::FromString(TEXT("None")), LoadStyleClass<UCommonTextStyle>(BodyTextStylePath)));
	AddVerticalChild(RootBox, ConstructText(WidgetBlueprint, TEXT("SummaryText"), FText::FromString(TEXT("Active: None\nNext: None\nTraits: None")), LoadStyleClass<UCommonTextStyle>(BodyTextStylePath)));
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
	FModuleManager::Get().LoadModule(TEXT("ModelViewViewModelBlueprint"));

	TArray<UWidgetBlueprint*> WidgetBlueprints;
	bool bNeedsConfigure = false;

	UWidgetBlueprint* UIRoot = CreateWidgetBlueprint(CommonUIGlobalPath, TEXT("WBP_UIRoot"), UFRUIRootWidget::StaticClass(), bNeedsConfigure);
	if (bNeedsConfigure)
	{
		ConfigureUIRoot(UIRoot);
	}
	RepairUIRootLayerNames(UIRoot);
	WidgetBlueprints.Add(UIRoot);

	UWidgetBlueprint* MainMenu = CreateWidgetBlueprint(CommonUIMainMenuPath, TEXT("WBP_MainMenu"), UFRMainMenuWidget::StaticClass(), bNeedsConfigure);
	if (bNeedsConfigure)
	{
		ConfigureMainMenu(MainMenu);
	}
	WidgetBlueprints.Add(MainMenu);

	UWidgetBlueprint* OptionsMenu = CreateWidgetBlueprint(CommonUIMainMenuPath, TEXT("WBP_OptionsMenu"), UFROptionsMenuWidget::StaticClass(), bNeedsConfigure);
	if (bNeedsConfigure)
	{
		ConfigureOptionsMenu(OptionsMenu);
	}
	WidgetBlueprints.Add(OptionsMenu);

	UWidgetBlueprint* PauseMenu = CreateWidgetBlueprint(CommonUIMainMenuPath, TEXT("WBP_PauseMenu"), UFRPauseMenuWidget::StaticClass(), bNeedsConfigure);
	if (bNeedsConfigure)
	{
		ConfigurePauseMenu(PauseMenu);
	}
	WidgetBlueprints.Add(PauseMenu);

	UWidgetBlueprint* ConfirmDialog = CreateWidgetBlueprint(CommonUIGlobalPath, TEXT("WBP_ConfirmDialog"), UFRConfirmDialogWidget::StaticClass(), bNeedsConfigure);
	if (bNeedsConfigure)
	{
		ConfigureConfirmDialog(ConfirmDialog);
	}
	WidgetBlueprints.Add(ConfirmDialog);

	UWidgetBlueprint* BattleHUD = LoadWidgetBlueprint(CommonUIMainGamePath, TEXT("WBP_BattleHUD"));
	WidgetBlueprints.Add(BattleHUD);

	WidgetBlueprints.Add(LoadWidgetBlueprint(CommonUIGlobalComponentsPath, TEXT("WBP_CommonTextButton")));
	WidgetBlueprints.Add(LoadWidgetBlueprint(CommonUIButtonsPath, TEXT("WBP_Button_StartRun")));
	WidgetBlueprints.Add(LoadWidgetBlueprint(CommonUIButtonsPath, TEXT("WBP_Button_Options")));
	WidgetBlueprints.Add(LoadWidgetBlueprint(CommonUIButtonsPath, TEXT("WBP_Button_Quit")));
	WidgetBlueprints.Add(LoadWidgetBlueprint(CommonUIButtonsPath, TEXT("WBP_Button_Back")));
	WidgetBlueprints.Add(LoadWidgetBlueprint(CommonUIButtonsPath, TEXT("WBP_Button_Confirm")));
	WidgetBlueprints.Add(LoadWidgetBlueprint(CommonUIButtonsPath, TEXT("WBP_Button_Cancel")));
	WidgetBlueprints.Add(LoadWidgetBlueprint(CommonUIButtonsPath, TEXT("WBP_Button_Resume")));
	WidgetBlueprints.Add(LoadWidgetBlueprint(CommonUIButtonsPath, TEXT("WBP_Button_MainMenu")));

	UWidgetBlueprint* TurnBanner = CreateWidgetBlueprint(CommonUIComponentsPath, TEXT("WBP_TurnBanner"), UFRBattleStatePanelWidget::StaticClass(), bNeedsConfigure);
	if (bNeedsConfigure)
	{
		ConfigureTurnBanner(TurnBanner);
	}
	WidgetBlueprints.Add(TurnBanner);

	UWidgetBlueprint* CombatantStatusPanel = CreateWidgetBlueprint(CommonUIComponentsPath, TEXT("WBP_CombatantStatusPanel"), UFRCombatantStatusPanelWidget::StaticClass(), bNeedsConfigure);
	if (bNeedsConfigure)
	{
		ConfigureCombatantStatusPanel(CombatantStatusPanel);
	}
	WidgetBlueprints.Add(CombatantStatusPanel);

	UWidgetBlueprint* AimWindIndicator = CreateWidgetBlueprint(CommonUIComponentsPath, TEXT("WBP_AimWindIndicator"), UFRAimWindIndicatorWidget::StaticClass(), bNeedsConfigure);
	if (bNeedsConfigure)
	{
		ConfigureAimWindIndicator(AimWindIndicator);
	}
	WidgetBlueprints.Add(AimWindIndicator);

	UWidgetBlueprint* ShotPowerMeter = CreateWidgetBlueprint(CommonUIComponentsPath, TEXT("WBP_ShotPowerMeter"), UFRShotPowerMeterWidget::StaticClass(), bNeedsConfigure);
	if (bNeedsConfigure)
	{
		ConfigureShotPowerMeter(ShotPowerMeter);
	}
	WidgetBlueprints.Add(ShotPowerMeter);

	UWidgetBlueprint* WeaponSlot = CreateWidgetBlueprint(CommonUIComponentsPath, TEXT("WBP_WeaponSlot"), UFRLoadoutSlotWidget::StaticClass(), bNeedsConfigure);
	if (bNeedsConfigure)
	{
		ConfigureLoadoutSlot(WeaponSlot, FText::FromString(TEXT("Basic")), FText::FromString(TEXT("Basic Shell")));
	}
	WidgetBlueprints.Add(WeaponSlot);

	UWidgetBlueprint* ItemSlot = CreateWidgetBlueprint(CommonUIComponentsPath, TEXT("WBP_ItemSlot"), UFRLoadoutSlotWidget::StaticClass(), bNeedsConfigure);
	if (bNeedsConfigure)
	{
		ConfigureLoadoutSlot(ItemSlot, FText::FromString(TEXT("Item")), FText::FromString(TEXT("-")));
	}
	WidgetBlueprints.Add(ItemSlot);

	UWidgetBlueprint* LoadoutBar = CreateWidgetBlueprint(CommonUIComponentsPath, TEXT("WBP_LoadoutBar"), UFRLoadoutBarWidget::StaticClass(), bNeedsConfigure);
	if (bNeedsConfigure)
	{
		ConfigureLoadoutBar(LoadoutBar);
	}
	WidgetBlueprints.Add(LoadoutBar);

	UWidgetBlueprint* ShotInfoPanel = CreateWidgetBlueprint(CommonUIComponentsPath, TEXT("WBP_ShotInfoPanel"), UFRShotInfoPanelWidget::StaticClass(), bNeedsConfigure);
	if (bNeedsConfigure)
	{
		ConfigureShotInfoPanel(ShotInfoPanel);
	}
	WidgetBlueprints.Add(ShotInfoPanel);

	UWidgetBlueprint* ModifierSummary = CreateWidgetBlueprint(CommonUIComponentsPath, TEXT("WBP_ModifierSummary"), UFRModifierSummaryWidget::StaticClass(), bNeedsConfigure);
	if (bNeedsConfigure)
	{
		ConfigureModifierSummary(ModifierSummary);
	}
	WidgetBlueprints.Add(ModifierSummary);

	UWidgetBlueprint* WorldStatusMarker = CreateWidgetBlueprint(CommonUIComponentsPath, TEXT("WBP_WorldStatusMarker"), UFRWorldStatusMarkerWidget::StaticClass(), bNeedsConfigure);
	if (bNeedsConfigure)
	{
		ConfigureWorldStatusMarker(WorldStatusMarker);
	}
	WidgetBlueprints.Add(WorldStatusMarker);

	UWidgetBlueprint* TrajectoryPreviewPoint = CreateWidgetBlueprint(CommonUIComponentsPath, TEXT("WBP_TrajectoryPreviewPoint"), UFRTrajectoryPreviewPointWidget::StaticClass(), bNeedsConfigure);
	if (bNeedsConfigure)
	{
		ConfigureTrajectoryPreviewPoint(TrajectoryPreviewPoint);
	}
	WidgetBlueprints.Add(TrajectoryPreviewPoint);

	bool bSavedAll = true;
	for (UWidgetBlueprint* WidgetBlueprint : WidgetBlueprints)
	{
		if (!WidgetBlueprint)
		{
			bSavedAll = false;
			continue;
		}

		RemovePrototypeMVVMBlueprintExtension(WidgetBlueprint);
		if (WidgetBlueprint->GetOutermost()->IsDirty())
		{
			FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
			bSavedAll &= SaveAsset(WidgetBlueprint);
		}
	}

	UE_LOG(LogTemp, Display, TEXT("Generated FortRogue CommonUI widget assets. SavedAll=%s"), bSavedAll ? TEXT("true") : TEXT("false"));
	return bSavedAll ? 0 : 1;
}
}
