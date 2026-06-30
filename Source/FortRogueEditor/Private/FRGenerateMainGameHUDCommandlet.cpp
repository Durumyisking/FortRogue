// Copyright Epic Games, Inc. All Rights Reserved.

#include "FRGenerateMainGameHUDCommandlet.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Engine/Font.h"
#include "Engine/Texture2D.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Materials/MaterialExpressionCustom.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialInterface.h"
#include "Game/FRGameModeDataAsset.h"
#include "HAL/FileManager.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "UI/FRBattleCharacterAimIndicatorWidget.h"
#include "UI/FRBattleCharacterStatusWidget.h"
#include "UI/FRFloatingDamageTextWidget.h"
#include "UI/FRMainGameHUDWidget.h"
#include "UObject/SavePackage.h"
#include "WidgetBlueprint.h"
#include "WidgetBlueprintFactory.h"

namespace
{
constexpr const TCHAR* MainGameHUDPath = TEXT("/Game/FortRogue/Widget/MainGame");
constexpr const TCHAR* MainGameHUDName = TEXT("WBP_MainGameHUD");
constexpr const TCHAR* WindArrowTextureName = TEXT("T_WindArrow");
constexpr const TCHAR* WindArrowTextureObjectPath = TEXT("/Game/FortRogue/Widget/MainGame/T_WindArrow.T_WindArrow");
constexpr const TCHAR* CharacterWidgetPath = TEXT("/Game/FortRogue/Widget/Character");
constexpr const TCHAR* BattleCharacterStatusName = TEXT("WBP_BattleCharacterStatus");
constexpr const TCHAR* BattleCharacterAimIndicatorName = TEXT("WBP_BattleCharacterAimIndicator");
constexpr const TCHAR* FloatingDamageTextName = TEXT("WBP_FloatingDamageText");
constexpr const TCHAR* MainGameModeObjectPath = TEXT("/Game/FortRogue/DataAsset/Global/DA_MainGameMode.DA_MainGameMode");
constexpr const TCHAR* MainGameHUDClassPath = TEXT("/Game/FortRogue/Widget/MainGame/WBP_MainGameHUD.WBP_MainGameHUD_C");
constexpr const TCHAR* WidgetMaterialPath = TEXT("/Game/FortRogue/Material/Widget");
constexpr const TCHAR* AimAngleIndicatorMaterialName = TEXT("M_AimAngleIndicator");
constexpr const TCHAR* AimAngleIndicatorMaterialObjectPath = TEXT("/Game/FortRogue/Material/Widget/M_AimAngleIndicator.M_AimAngleIndicator");

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

UWidgetBlueprint* LoadOrCreateWidgetBlueprint(const TCHAR* FolderPath, const TCHAR* AssetName, TSubclassOf<UUserWidget> ParentClass)
{
	const FString LongPackageName = FString::Printf(TEXT("%s/%s"), FolderPath, AssetName);
	const FString ObjectPath = FString::Printf(TEXT("%s.%s"), *LongPackageName, AssetName);
	if (UWidgetBlueprint* ExistingBlueprint = LoadObject<UWidgetBlueprint>(nullptr, *ObjectPath))
	{
		return ExistingBlueprint;
	}

	UWidgetBlueprintFactory* Factory = NewObject<UWidgetBlueprintFactory>();
	Factory->ParentClass = ParentClass;

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	return Cast<UWidgetBlueprint>(AssetToolsModule.Get().CreateAsset(AssetName, FolderPath, UWidgetBlueprint::StaticClass(), Factory));
}

UTexture2D* LoadOrCreateWindArrowTexture()
{
	constexpr int32 Width = 64;
	constexpr int32 Height = 32;
	constexpr int32 CenterY = Height / 2;

	UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, WindArrowTextureObjectPath);
	bool bCreatedTexture = false;
	if (!Texture)
	{
		const FString LongPackageName = FString::Printf(TEXT("%s/%s"), MainGameHUDPath, WindArrowTextureName);
		UPackage* Package = CreatePackage(*LongPackageName);
		Texture = NewObject<UTexture2D>(Package, WindArrowTextureName, RF_Public | RF_Standalone | RF_Transactional);
		bCreatedTexture = true;
	}
	if (!Texture)
	{
		return nullptr;
	}

	Texture->Modify();

	TArray<uint8> Pixels;
	Pixels.SetNumZeroed(Width * Height * 4);
	auto SetPixel = [&Pixels, Width](int32 X, int32 Y, const FColor& Color)
	{
		const int32 Index = (Y * Width + X) * 4;
		Pixels[Index] = Color.B;
		Pixels[Index + 1] = Color.G;
		Pixels[Index + 2] = Color.R;
		Pixels[Index + 3] = Color.A;
	};

	const FColor ArrowColor(245, 250, 255, 210);
	for (int32 Y = 0; Y < Height; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			const int32 DistanceFromCenter = FMath::Abs(Y - CenterY);
			const bool bShaft = X >= 6 && X <= 38 && DistanceFromCenter <= 4;
			const bool bHead = X >= 34 && X <= 58 && DistanceFromCenter <= FMath::Max(1, FMath::RoundToInt((58 - X) * 0.52f));
			if (bShaft || bHead)
			{
				SetPixel(X, Y, ArrowColor);
			}
		}
	}

	Texture->Source.Init(Width, Height, 1, 1, TSF_BGRA8, Pixels.GetData());
	Texture->SRGB = true;
	Texture->CompressionSettings = TC_EditorIcon;
	Texture->MipGenSettings = TMGS_NoMipmaps;
	Texture->UpdateResource();
	Texture->MarkPackageDirty();
	if (bCreatedTexture)
	{
		FAssetRegistryModule::AssetCreated(Texture);
	}
	return Texture;
}
template <typename TExpression>
TExpression* AddMaterialExpression(UMaterial* Material, int32 EditorX, int32 EditorY)
{
	TExpression* Expression = NewObject<TExpression>(Material);
	Expression->Material = Material;
	Expression->MaterialExpressionEditorX = EditorX;
	Expression->MaterialExpressionEditorY = EditorY;
	Material->GetExpressionCollection().AddExpression(Expression);
	return Expression;
}

UMaterialExpressionScalarParameter* AddScalarParameter(UMaterial* Material, const TCHAR* Name, float DefaultValue, int32 EditorX, int32 EditorY)
{
	UMaterialExpressionScalarParameter* Parameter = AddMaterialExpression<UMaterialExpressionScalarParameter>(Material, EditorX, EditorY);
	Parameter->ParameterName = FName(Name);
	Parameter->DefaultValue = DefaultValue;
	Parameter->SliderMin = -180.0f;
	Parameter->SliderMax = 180.0f;
	return Parameter;
}

void AddCustomInput(UMaterialExpressionCustom* CustomExpression, const TCHAR* Name, UMaterialExpression* Expression)
{
	FCustomInput Input;
	Input.InputName = FName(Name);
	Input.Input.Connect(0, Expression);
	CustomExpression->Inputs.Add(Input);
}

UMaterial* LoadOrCreateAimAngleIndicatorMaterial()
{
	UMaterial* Material = LoadObject<UMaterial>(nullptr, AimAngleIndicatorMaterialObjectPath);
	bool bCreatedMaterial = false;
	if (!Material)
	{
		const FString LongPackageName = FString::Printf(TEXT("%s/%s"), WidgetMaterialPath, AimAngleIndicatorMaterialName);
		UPackage* Package = CreatePackage(*LongPackageName);
		Material = NewObject<UMaterial>(Package, AimAngleIndicatorMaterialName, RF_Public | RF_Standalone | RF_Transactional);
		bCreatedMaterial = true;
	}
	if (!Material)
	{
		return nullptr;
	}

	Material->Modify();
	Material->MaterialDomain = MD_UI;
	Material->BlendMode = BLEND_Translucent;
	Material->SetShadingModel(MSM_Unlit);
	Material->GetExpressionCollection().Empty();

	if (FExpressionInput* EmissiveInput = Material->GetExpressionInputForProperty(MP_EmissiveColor))
	{
		*EmissiveInput = FExpressionInput();
	}
	if (FExpressionInput* OpacityInput = Material->GetExpressionInputForProperty(MP_Opacity))
	{
		*OpacityInput = FExpressionInput();
	}

	UMaterialExpressionTextureCoordinate* UV = AddMaterialExpression<UMaterialExpressionTextureCoordinate>(Material, -760, -120);
	UMaterialExpressionScalarParameter* MinAngle = AddScalarParameter(Material, TEXT("MinAngle"), 0.0f, -760, -20);
	UMaterialExpressionScalarParameter* MaxAngle = AddScalarParameter(Material, TEXT("MaxAngle"), 90.0f, -760, 80);
	UMaterialExpressionScalarParameter* CurrentAngle = AddScalarParameter(Material, TEXT("CurrentAngle"), 45.0f, -760, 180);
	UMaterialExpressionCustom* Custom = AddMaterialExpression<UMaterialExpressionCustom>(Material, -420, 20);
	Custom->Description = TEXT("Aim range sector and current angle line");
	Custom->OutputType = CMOT_Float4;
	Custom->ContainsClipInstruction = CMCI_No;
	Custom->Code = TEXT(R"(
float2 p = float2(UV.x, 1.0 - UV.y);
float2 origin = float2(0.5, 0.5);
float2 d = p - origin;
float radius = length(d);
float angle = atan2(d.y, d.x) * 57.2957795;
float minAngle = min(MinAngle, MaxAngle);
float maxAngle = max(MinAngle, MaxAngle);
float radiusMask = smoothstep(0.006, 0.018, radius) * smoothstep(0.49, 0.46, radius);
float angleMask = step(minAngle, angle) * step(angle, maxAngle);
float sectorAlpha = radiusMask * angleMask * 0.36;
float currentAngle = clamp(CurrentAngle, minAngle, maxAngle);
float currentRad = currentAngle * 0.01745329252;
float2 lineDir = float2(cos(currentRad), sin(currentRad));
float alongLine = dot(d, lineDir);
float lineDistance = abs(d.x * lineDir.y - d.y * lineDir.x);
float lineAlpha = smoothstep(0.014, 0.004, lineDistance) * step(0.0, alongLine) * step(alongLine, 0.49);
float alpha = saturate(max(sectorAlpha, lineAlpha));
float3 color = lerp(float3(0.0, 0.0, 0.0), float3(1.0, 0.0, 0.0), step(0.01, lineAlpha));
return float4(color, alpha);
)");
	AddCustomInput(Custom, TEXT("UV"), UV);
	AddCustomInput(Custom, TEXT("MinAngle"), MinAngle);
	AddCustomInput(Custom, TEXT("MaxAngle"), MaxAngle);
	AddCustomInput(Custom, TEXT("CurrentAngle"), CurrentAngle);

	UMaterialExpressionComponentMask* RGBMask = AddMaterialExpression<UMaterialExpressionComponentMask>(Material, -80, -20);
	RGBMask->Input.Connect(0, Custom);
	RGBMask->R = true;
	RGBMask->G = true;
	RGBMask->B = true;

	UMaterialExpressionComponentMask* AlphaMask = AddMaterialExpression<UMaterialExpressionComponentMask>(Material, -80, 120);
	AlphaMask->Input.Connect(0, Custom);
	AlphaMask->A = true;

	if (FExpressionInput* EmissiveInput = Material->GetExpressionInputForProperty(MP_EmissiveColor))
	{
		EmissiveInput->Connect(0, RGBMask);
	}
	if (FExpressionInput* OpacityInput = Material->GetExpressionInputForProperty(MP_Opacity))
	{
		OpacityInput->Connect(0, AlphaMask);
	}

	Material->PostEditChange();
	Material->MarkPackageDirty();
	if (bCreatedMaterial)
	{
		FAssetRegistryModule::AssetCreated(Material);
	}
	return Material;
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

UTextBlock* AddText(UWidgetTree* WidgetTree, UCanvasPanel* Root, const FName& Name, const FText& Text, const FAnchors& Anchors, const FVector2D& Position, const FVector2D& Size, const FVector2D& Alignment, int32 FontSize, ETextJustify::Type Justification = ETextJustify::Left, const FLinearColor& Color = FLinearColor::White)
{
	UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
	ConfigureText(TextBlock, Text, FontSize, Color, Justification);
	ConfigureCanvasSlot(Root->AddChildToCanvas(TextBlock), Anchors, Position, Size, Alignment);
	return TextBlock;
}

UImage* AddImage(UWidgetTree* WidgetTree, UCanvasPanel* Root, const FName& Name, UTexture2D* Texture, const FAnchors& Anchors, const FVector2D& Position, const FVector2D& Size, const FVector2D& Alignment)
{
	UImage* Image = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), Name);
	if (Texture)
	{
		Image->SetBrushFromTexture(Texture, true);
	}
	Image->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.9f));
	ConfigureCanvasSlot(Root->AddChildToCanvas(Image), Anchors, Position, Size, Alignment);
	return Image;
}
UImage* AddMaterialImage(UWidgetTree* WidgetTree, UCanvasPanel* Root, const FName& Name, UMaterialInterface* Material, const FAnchors& Anchors, const FVector2D& Position, const FVector2D& Size, const FVector2D& Alignment)
{
	UImage* Image = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), Name);
	if (Material)
	{
		Image->SetBrushFromMaterial(Material);
	}
	Image->SetColorAndOpacity(FLinearColor::White);
	ConfigureCanvasSlot(Root->AddChildToCanvas(Image), Anchors, Position, Size, Alignment);
	return Image;
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

void ResetWidgetVariableGuids(UWidgetBlueprint* WidgetBlueprint)
{
	if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree)
	{
		return;
	}

	WidgetBlueprint->WidgetVariableNameToGuidMap.Empty();
	WidgetBlueprint->WidgetTree->ForEachWidget([WidgetBlueprint](UWidget* Widget)
	{
		if (Widget)
		{
			WidgetBlueprint->WidgetVariableNameToGuidMap.Add(Widget->GetFName(), FGuid::NewGuid());
		}
	});
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

void ConfigureMainGameHUDTree(UWidgetBlueprint* WidgetBlueprint, UTexture2D* WindArrowTexture)
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

	AddImage(WidgetTree, Root, TEXT("WindArrowVisual"), WindArrowTexture, FAnchors(0.0f, 1.0f), FVector2D(36.0f, -124.0f), FVector2D(48.0f, 28.0f), FVector2D(0.0f, 1.0f));
	AddText(WidgetTree, Root, TEXT("WindText"), FText::FromString(TEXT("Wind 0")), FAnchors(0.0f, 1.0f), FVector2D(96.0f, -116.0f), FVector2D(260.0f, 46.0f), FVector2D(0.0f, 1.0f), 28);
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

	ResetWidgetVariableGuids(WidgetBlueprint);
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(WidgetBlueprint);
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
	WidgetBlueprint->MarkPackageDirty();
}

void ConfigureBattleCharacterStatusTree(UWidgetBlueprint* WidgetBlueprint)
{
	if (!WidgetBlueprint)
	{
		return;
	}

	WidgetBlueprint->Modify();
	WidgetBlueprint->ParentClass = UFRBattleCharacterStatusWidget::StaticClass();

	UWidgetTree* WidgetTree = WidgetBlueprint->WidgetTree;
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(WidgetBlueprint, TEXT("WidgetTree"), RF_Transactional);
		WidgetBlueprint->WidgetTree = WidgetTree;
	}
	WidgetTree->Modify();

	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = Root;

	UImage* LegacyAimIndicatorImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("AimIndicatorImage"));
	LegacyAimIndicatorImage->SetVisibility(ESlateVisibility::Collapsed);
	ConfigureCanvasSlot(Root->AddChildToCanvas(LegacyAimIndicatorImage), FAnchors(0.0f, 0.0f), FVector2D::ZeroVector, FVector2D(1.0f, 1.0f), FVector2D::ZeroVector);

	AddProgressBar(WidgetTree, Root, TEXT("HealthBar"), FAnchors(0.0f, 0.0f), FVector2D(0.0f, 0.0f), FVector2D(160.0f, 18.0f), FLinearColor(0.9f, 0.1f, 0.08f));
	AddText(WidgetTree, Root, TEXT("CharacterNameText"), FText::GetEmpty(), FAnchors(0.0f, 0.0f), FVector2D::ZeroVector, FVector2D(1.0f, 1.0f), FVector2D::ZeroVector, 1)->SetVisibility(ESlateVisibility::Collapsed);
	AddText(WidgetTree, Root, TEXT("HealthText"), FText::GetEmpty(), FAnchors(0.0f, 0.0f), FVector2D::ZeroVector, FVector2D(1.0f, 1.0f), FVector2D::ZeroVector, 1)->SetVisibility(ESlateVisibility::Collapsed);
	AddText(WidgetTree, Root, TEXT("AimText"), FText::GetEmpty(), FAnchors(0.0f, 0.0f), FVector2D::ZeroVector, FVector2D(1.0f, 1.0f), FVector2D::ZeroVector, 1)->SetVisibility(ESlateVisibility::Collapsed);
	ResetWidgetVariableGuids(WidgetBlueprint);
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(WidgetBlueprint);
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
	WidgetBlueprint->MarkPackageDirty();
}

void ConfigureBattleCharacterAimIndicatorTree(UWidgetBlueprint* WidgetBlueprint, UMaterialInterface* AimAngleMaterial)
{
	if (!WidgetBlueprint)
	{
		return;
	}

	WidgetBlueprint->Modify();
	WidgetBlueprint->ParentClass = UFRBattleCharacterAimIndicatorWidget::StaticClass();

	UWidgetTree* WidgetTree = WidgetBlueprint->WidgetTree;
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(WidgetBlueprint, TEXT("WidgetTree"), RF_Transactional);
		WidgetBlueprint->WidgetTree = WidgetTree;
	}
	WidgetTree->Modify();

	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = Root;

	AddMaterialImage(WidgetTree, Root, TEXT("AimIndicatorImage"), AimAngleMaterial, FAnchors(0.0f, 0.0f), FVector2D::ZeroVector, FVector2D(128.0f, 128.0f), FVector2D::ZeroVector);
	ResetWidgetVariableGuids(WidgetBlueprint);
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(WidgetBlueprint);
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
	WidgetBlueprint->MarkPackageDirty();
}
void ConfigureFloatingDamageTextTree(UWidgetBlueprint* WidgetBlueprint)
{
	if (!WidgetBlueprint)
	{
		return;
	}

	WidgetBlueprint->Modify();
	WidgetBlueprint->ParentClass = UFRFloatingDamageTextWidget::StaticClass();

	UWidgetTree* WidgetTree = WidgetBlueprint->WidgetTree;
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(WidgetBlueprint, TEXT("WidgetTree"), RF_Transactional);
		WidgetBlueprint->WidgetTree = WidgetTree;
	}
	WidgetTree->Modify();

	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = Root;

	AddText(WidgetTree, Root, TEXT("DamageText"), FText::FromString(TEXT("-25")), FAnchors(0.0f, 0.0f), FVector2D(0.0f, 0.0f), FVector2D(160.0f, 52.0f), FVector2D::ZeroVector, 34, ETextJustify::Center, FLinearColor(1.0f, 0.18f, 0.05f));

	ResetWidgetVariableGuids(WidgetBlueprint);
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
	MainGameMode->InputMode = EFRGameFlowInputMode::GameAndUI;
	MainGameMode->bShowMouseCursor = true;
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
	UTexture2D* WindArrowTexture = LoadOrCreateWindArrowTexture();
	UMaterial* AimAngleMaterial = LoadOrCreateAimAngleIndicatorMaterial();
	UWidgetBlueprint* MainGameHUD = LoadOrCreateWidgetBlueprint(MainGameHUDPath, MainGameHUDName, UFRMainGameHUDWidget::StaticClass());
	if (!MainGameHUD)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not create %s/%s."), MainGameHUDPath, MainGameHUDName);
		return 1;
	}

	UWidgetBlueprint* BattleCharacterStatus = LoadOrCreateWidgetBlueprint(CharacterWidgetPath, BattleCharacterStatusName, UFRBattleCharacterStatusWidget::StaticClass());
	UWidgetBlueprint* BattleCharacterAimIndicator = LoadOrCreateWidgetBlueprint(CharacterWidgetPath, BattleCharacterAimIndicatorName, UFRBattleCharacterAimIndicatorWidget::StaticClass());
	UWidgetBlueprint* FloatingDamageText = LoadOrCreateWidgetBlueprint(CharacterWidgetPath, FloatingDamageTextName, UFRFloatingDamageTextWidget::StaticClass());
	if (!BattleCharacterStatus || !BattleCharacterAimIndicator || !FloatingDamageText || !WindArrowTexture || !AimAngleMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not create combat feedback UI assets."));
		return 1;
	}

	ConfigureMainGameHUDTree(MainGameHUD, WindArrowTexture);
	ConfigureBattleCharacterStatusTree(BattleCharacterStatus);
	ConfigureBattleCharacterAimIndicatorTree(BattleCharacterAimIndicator, AimAngleMaterial);
	ConfigureFloatingDamageTextTree(FloatingDamageText);
	UFRGameModeDataAsset* MainGameMode = ConfigureMainGameModeHUD();

	const bool bSavedWindArrow = SaveAsset(WindArrowTexture);
	const bool bSavedAimAngleMaterial = SaveAsset(AimAngleMaterial);
	const bool bSavedHUD = SaveAsset(MainGameHUD);
	const bool bSavedStatus = SaveAsset(BattleCharacterStatus);
	const bool bSavedAimIndicator = SaveAsset(BattleCharacterAimIndicator);
	const bool bSavedDamageText = SaveAsset(FloatingDamageText);
	const bool bSavedMode = SaveAsset(MainGameMode);
	UE_LOG(LogTemp, Display, TEXT("Generated MainGame combat UI. SavedWindArrow=%s SavedAimAngleMaterial=%s SavedHUD=%s SavedStatus=%s SavedAimIndicator=%s SavedDamageText=%s SavedMode=%s"),
		bSavedWindArrow ? TEXT("true") : TEXT("false"),
		bSavedAimAngleMaterial ? TEXT("true") : TEXT("false"),
		bSavedHUD ? TEXT("true") : TEXT("false"),
		bSavedStatus ? TEXT("true") : TEXT("false"),
		bSavedAimIndicator ? TEXT("true") : TEXT("false"),
		bSavedDamageText ? TEXT("true") : TEXT("false"),
		bSavedMode ? TEXT("true") : TEXT("false"));
	return bSavedWindArrow && bSavedAimAngleMaterial && bSavedHUD && bSavedStatus && bSavedAimIndicator && bSavedDamageText && bSavedMode ? 0 : 1;
}
