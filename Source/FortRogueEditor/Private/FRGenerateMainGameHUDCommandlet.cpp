// Copyright Epic Games, Inc. All Rights Reserved.

#include "FRGenerateMainGameHUDCommandlet.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/SlateWrapperTypes.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "CommonBorder.h"
#include "CommonButtonBase.h"
#include "CommonTextBlock.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
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
#include "UI/FRCharacterSelectWidget.h"
#include "UI/FRCommonButtonWidgets.h"
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
constexpr const TCHAR* CharacterSelectHUDPath = TEXT("/Game/FortRogue/Widget/CharacterSelect");
constexpr const TCHAR* CharacterSelectHUDName = TEXT("WBP_CharacterSelectHUD");
constexpr const TCHAR* CommonComponentPath = TEXT("/Game/FortRogue/Widget/Common/Components");
constexpr const TCHAR* CommonStylePath = TEXT("/Game/FortRogue/Widget/Common/Styles");
constexpr const TCHAR* RobotoFontObjectPath = TEXT("/Engine/EngineFonts/Roboto.Roboto");
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

// 폴더/이름과 픽셀 함수를 받아 절차 텍스처를 만들거나 갱신합니다. UI 장식(비네트/그라데이션)에 사용합니다.
UTexture2D* LoadOrCreateGeneratedTexture(const TCHAR* FolderPath, const TCHAR* TextureName, int32 Width, int32 Height, TFunctionRef<FColor(int32 X, int32 Y)> PixelFunction)
{
	const FString ObjectPath = FString::Printf(TEXT("%s/%s.%s"), FolderPath, TextureName, TextureName);
	UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, *ObjectPath);
	bool bCreatedTexture = false;
	if (!Texture)
	{
		const FString LongPackageName = FString::Printf(TEXT("%s/%s"), FolderPath, TextureName);
		UPackage* Package = CreatePackage(*LongPackageName);
		Texture = NewObject<UTexture2D>(Package, TextureName, RF_Public | RF_Standalone | RF_Transactional);
		bCreatedTexture = true;
	}
	if (!Texture)
	{
		return nullptr;
	}

	Texture->Modify();

	TArray<uint8> Pixels;
	Pixels.SetNumZeroed(Width * Height * 4);
	for (int32 Y = 0; Y < Height; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			const FColor Color = PixelFunction(X, Y);
			const int32 Index = (Y * Width + X) * 4;
			Pixels[Index] = Color.B;
			Pixels[Index + 1] = Color.G;
			Pixels[Index + 2] = Color.R;
			Pixels[Index + 3] = Color.A;
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

// 공용 UI 팔레트: 어두운 네이비 카드 + 따뜻한 주황 액센트.
const FLinearColor UIAccentColor(0.95f, 0.52f, 0.12f);
const FLinearColor UIAccentDarkColor(0.5f, 0.24f, 0.04f);
const FLinearColor UIPanelFillColor(0.035f, 0.045f, 0.07f, 0.82f);
const FLinearColor UIPanelOutlineColor(1.0f, 1.0f, 1.0f, 0.08f);
const FLinearColor UISlotFillColor(0.035f, 0.045f, 0.07f, 0.85f);
const FLinearColor UISlotFillHoveredColor(0.07f, 0.09f, 0.13f, 0.9f);
const FLinearColor UISlotFillPressedColor(0.02f, 0.028f, 0.045f, 0.9f);
const FLinearColor UICreamColor(0.96f, 0.93f, 0.85f);
const FLinearColor UIDimColor(0.72f, 0.75f, 0.82f);
const FLinearColor UIDarkTextColor(0.16f, 0.09f, 0.02f);

FSlateBrush MakeRoundedBrush(const FLinearColor& FillColor, float CornerRadius, const FLinearColor& OutlineColor, float OutlineWidth)
{
	FSlateBrush Brush;
	Brush.DrawAs = ESlateBrushDrawType::RoundedBox;
	Brush.TintColor = FillColor;
	Brush.OutlineSettings = FSlateBrushOutlineSettings(FVector4(CornerRadius, CornerRadius, CornerRadius, CornerRadius), OutlineColor, OutlineWidth);
	Brush.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
	return Brush;
}

// ---------- CommonUI 스타일 에셋 생성 ----------
// 스타일 클래스(UCommonButtonStyle 등)의 프로퍼티는 protected라 리플렉션으로 CDO에 값을 씁니다.

template <typename TValue>
bool SetStyleProperty(UObject* StyleObject, const TCHAR* PropertyName, const TValue& Value)
{
	FProperty* Property = StyleObject ? StyleObject->GetClass()->FindPropertyByName(PropertyName) : nullptr;
	if (!Property)
	{
		UE_LOG(LogTemp, Warning, TEXT("Style property '%s' not found on %s."), PropertyName, *GetNameSafe(StyleObject));
		return false;
	}
	*Property->ContainerPtrToValuePtr<TValue>(StyleObject) = Value;
	return true;
}

bool SetStyleBoolProperty(UObject* StyleObject, const TCHAR* PropertyName, bool bValue)
{
	FBoolProperty* Property = StyleObject ? CastField<FBoolProperty>(StyleObject->GetClass()->FindPropertyByName(PropertyName)) : nullptr;
	if (!Property)
	{
		UE_LOG(LogTemp, Warning, TEXT("Style bool property '%s' not found on %s."), PropertyName, *GetNameSafe(StyleObject));
		return false;
	}
	Property->SetPropertyValue_InContainer(StyleObject, bValue);
	return true;
}

UBlueprint* LoadOrCreateBlueprintClassAsset(const TCHAR* FolderPath, const TCHAR* AssetName, UClass* ParentClass)
{
	const FString ObjectPath = FString::Printf(TEXT("%s/%s.%s"), FolderPath, AssetName, AssetName);
	if (UBlueprint* ExistingBlueprint = LoadObject<UBlueprint>(nullptr, *ObjectPath))
	{
		return ExistingBlueprint;
	}

	UPackage* Package = CreatePackage(*FString::Printf(TEXT("%s/%s"), FolderPath, AssetName));
	UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(ParentClass, Package, FName(AssetName), BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
	if (Blueprint)
	{
		FAssetRegistryModule::AssetCreated(Blueprint);
		Package->MarkPackageDirty();
	}
	return Blueprint;
}

FSlateFontInfo MakeRobotoFont(int32 Size, bool bBold)
{
	UObject* FontObject = LoadObject<UObject>(nullptr, RobotoFontObjectPath);
	return FSlateFontInfo(FontObject, Size, bBold ? FName(TEXT("Bold")) : FName(TEXT("Regular")));
}

UClass* ConfigureTextStyleAsset(const TCHAR* AssetName, int32 FontSize, bool bBold, const FLinearColor& Color, bool bShadow, TArray<UObject*>& OutStyleAssets)
{
	UBlueprint* Blueprint = LoadOrCreateBlueprintClassAsset(CommonStylePath, AssetName, UCommonTextStyle::StaticClass());
	if (!Blueprint || !Blueprint->GeneratedClass)
	{
		return nullptr;
	}

	UObject* StyleDefaults = Blueprint->GeneratedClass->GetDefaultObject();
	StyleDefaults->Modify();
	SetStyleProperty(StyleDefaults, TEXT("Font"), MakeRobotoFont(FontSize, bBold));
	SetStyleProperty(StyleDefaults, TEXT("Color"), Color);
	SetStyleBoolProperty(StyleDefaults, TEXT("bUsesDropShadow"), bShadow);
	if (bShadow)
	{
		SetStyleProperty(StyleDefaults, TEXT("ShadowOffset"), FVector2D(1.5, 2.0));
		SetStyleProperty(StyleDefaults, TEXT("ShadowColor"), FLinearColor(0.0f, 0.0f, 0.0f, 0.45f));
	}
	Blueprint->MarkPackageDirty();
	OutStyleAssets.Add(Blueprint);
	return Blueprint->GeneratedClass;
}

UClass* ConfigureButtonStyleAsset(
	const TCHAR* AssetName,
	const FSlateBrush& NormalBase, const FSlateBrush& NormalHovered, const FSlateBrush& NormalPressed,
	const FSlateBrush& SelectedBase, const FSlateBrush& DisabledBrush,
	const FMargin& ButtonPadding, int32 MinWidth, int32 MinHeight,
	UClass* NormalTextStyle, UClass* SelectedTextStyle, UClass* DisabledTextStyle,
	TArray<UObject*>& OutStyleAssets)
{
	UBlueprint* Blueprint = LoadOrCreateBlueprintClassAsset(CommonStylePath, AssetName, UCommonButtonStyle::StaticClass());
	if (!Blueprint || !Blueprint->GeneratedClass)
	{
		return nullptr;
	}

	UObject* StyleDefaults = Blueprint->GeneratedClass->GetDefaultObject();
	StyleDefaults->Modify();
	SetStyleProperty(StyleDefaults, TEXT("NormalBase"), NormalBase);
	SetStyleProperty(StyleDefaults, TEXT("NormalHovered"), NormalHovered);
	SetStyleProperty(StyleDefaults, TEXT("NormalPressed"), NormalPressed);
	SetStyleProperty(StyleDefaults, TEXT("SelectedBase"), SelectedBase);
	SetStyleProperty(StyleDefaults, TEXT("SelectedHovered"), SelectedBase);
	SetStyleProperty(StyleDefaults, TEXT("SelectedPressed"), NormalPressed);
	SetStyleProperty(StyleDefaults, TEXT("Disabled"), DisabledBrush);
	SetStyleProperty(StyleDefaults, TEXT("ButtonPadding"), ButtonPadding);
	SetStyleProperty(StyleDefaults, TEXT("CustomPadding"), FMargin(0.0f));
	SetStyleProperty(StyleDefaults, TEXT("MinWidth"), MinWidth);
	SetStyleProperty(StyleDefaults, TEXT("MinHeight"), MinHeight);
	SetStyleProperty(StyleDefaults, TEXT("NormalTextStyle"), TSubclassOf<UCommonTextStyle>(NormalTextStyle));
	SetStyleProperty(StyleDefaults, TEXT("NormalHoveredTextStyle"), TSubclassOf<UCommonTextStyle>(NormalTextStyle));
	SetStyleProperty(StyleDefaults, TEXT("SelectedTextStyle"), TSubclassOf<UCommonTextStyle>(SelectedTextStyle ? SelectedTextStyle : NormalTextStyle));
	SetStyleProperty(StyleDefaults, TEXT("SelectedHoveredTextStyle"), TSubclassOf<UCommonTextStyle>(SelectedTextStyle ? SelectedTextStyle : NormalTextStyle));
	SetStyleProperty(StyleDefaults, TEXT("DisabledTextStyle"), TSubclassOf<UCommonTextStyle>(DisabledTextStyle ? DisabledTextStyle : NormalTextStyle));
	Blueprint->MarkPackageDirty();
	OutStyleAssets.Add(Blueprint);
	return Blueprint->GeneratedClass;
}

UClass* ConfigureBorderStyleAsset(const TCHAR* AssetName, const FSlateBrush& Background, TArray<UObject*>& OutStyleAssets)
{
	UBlueprint* Blueprint = LoadOrCreateBlueprintClassAsset(CommonStylePath, AssetName, UCommonBorderStyle::StaticClass());
	if (!Blueprint || !Blueprint->GeneratedClass)
	{
		return nullptr;
	}

	UObject* StyleDefaults = Blueprint->GeneratedClass->GetDefaultObject();
	StyleDefaults->Modify();
	SetStyleProperty(StyleDefaults, TEXT("Background"), Background);
	Blueprint->MarkPackageDirty();
	OutStyleAssets.Add(Blueprint);
	return Blueprint->GeneratedClass;
}

/** 커맨드릿이 생성하는 CommonUI 스타일/컴포넌트 클래스 모음입니다. HUD 트리 구성 함수에 전달됩니다. */
struct FFRGeneratedUIKit
{
	UClass* TitleTextStyle = nullptr;
	UClass* HeadingTextStyle = nullptr;
	UClass* BodyTextStyle = nullptr;
	UClass* LabelTextStyle = nullptr;
	UClass* SmallTextStyle = nullptr;
	UClass* SectionTitleTextStyle = nullptr;
	UClass* ButtonDarkTextStyle = nullptr;
	UClass* PanelBorderStyle = nullptr;
	UClass* BottomBarBorderStyle = nullptr;
	UClass* SlotButtonStyle = nullptr;
	UClass* AccentButtonStyle = nullptr;
	UClass* LoadoutSlotButtonStyle = nullptr;
	UClass* CharacterSlotButtonClass = nullptr;
	UClass* TextButtonClass = nullptr;
	UClass* LoadoutSlotButtonClass = nullptr;
	UClass* RewardChoiceButtonClass = nullptr;

	bool IsComplete() const
	{
		return TitleTextStyle && HeadingTextStyle && BodyTextStyle && LabelTextStyle && SmallTextStyle
			&& SectionTitleTextStyle && ButtonDarkTextStyle && PanelBorderStyle && BottomBarBorderStyle
			&& SlotButtonStyle && AccentButtonStyle && LoadoutSlotButtonStyle
			&& CharacterSlotButtonClass && TextButtonClass && LoadoutSlotButtonClass && RewardChoiceButtonClass;
	}
};

FFRGeneratedUIKit ConfigureUIStyleAssets(TArray<UObject*>& OutStyleAssets)
{
	FFRGeneratedUIKit Kit;
	Kit.TitleTextStyle = ConfigureTextStyleAsset(TEXT("TS_Title"), 36, true, UICreamColor, true, OutStyleAssets);
	Kit.HeadingTextStyle = ConfigureTextStyleAsset(TEXT("TS_Heading"), 26, true, UICreamColor, true, OutStyleAssets);
	Kit.BodyTextStyle = ConfigureTextStyleAsset(TEXT("TS_Body"), 18, false, UIDimColor, false, OutStyleAssets);
	Kit.LabelTextStyle = ConfigureTextStyleAsset(TEXT("TS_Label"), 16, true, UICreamColor, true, OutStyleAssets);
	Kit.SmallTextStyle = ConfigureTextStyleAsset(TEXT("TS_Small"), 13, false, UIDimColor, false, OutStyleAssets);
	Kit.SectionTitleTextStyle = ConfigureTextStyleAsset(TEXT("TS_SectionTitle"), 17, true, UIAccentColor, true, OutStyleAssets);
	Kit.ButtonDarkTextStyle = ConfigureTextStyleAsset(TEXT("TS_ButtonDark"), 24, true, UIDarkTextColor, false, OutStyleAssets);

	Kit.PanelBorderStyle = ConfigureBorderStyleAsset(TEXT("BS_Panel"), MakeRoundedBrush(UIPanelFillColor, 18.0f, UIPanelOutlineColor, 1.5f), OutStyleAssets);
	Kit.BottomBarBorderStyle = ConfigureBorderStyleAsset(TEXT("BS_BottomBar"), MakeRoundedBrush(FLinearColor(0.03f, 0.04f, 0.06f, 0.72f), 0.0f, FLinearColor::Transparent, 0.0f), OutStyleAssets);

	Kit.SlotButtonStyle = ConfigureButtonStyleAsset(TEXT("BTS_Slot"),
		MakeRoundedBrush(UISlotFillColor, 12.0f, UIPanelOutlineColor, 1.5f),
		MakeRoundedBrush(UISlotFillHoveredColor, 12.0f, FLinearColor(1.0f, 1.0f, 1.0f, 0.28f), 1.5f),
		MakeRoundedBrush(UISlotFillPressedColor, 12.0f, UIAccentColor, 2.0f),
		MakeRoundedBrush(UISlotFillColor, 12.0f, UIAccentColor, 3.0f),
		MakeRoundedBrush(FLinearColor(0.06f, 0.07f, 0.09f, 0.5f), 12.0f, FLinearColor(1.0f, 1.0f, 1.0f, 0.05f), 1.0f),
		FMargin(0.0f), 0, 0,
		Kit.LabelTextStyle, Kit.SectionTitleTextStyle, Kit.SmallTextStyle, OutStyleAssets);

	Kit.AccentButtonStyle = ConfigureButtonStyleAsset(TEXT("BTS_Accent"),
		MakeRoundedBrush(UIAccentColor, 16.0f, UIAccentDarkColor, 2.0f),
		MakeRoundedBrush(FLinearColor(1.0f, 0.62f, 0.2f), 16.0f, UIAccentDarkColor, 2.0f),
		MakeRoundedBrush(FLinearColor(0.78f, 0.4f, 0.08f), 16.0f, UIAccentDarkColor, 2.0f),
		MakeRoundedBrush(UIAccentColor, 16.0f, UIAccentDarkColor, 2.0f),
		MakeRoundedBrush(FLinearColor(0.16f, 0.17f, 0.2f, 0.65f), 16.0f, FLinearColor(1.0f, 1.0f, 1.0f, 0.06f), 1.5f),
		FMargin(0.0f), 240, 78,
		Kit.ButtonDarkTextStyle, Kit.ButtonDarkTextStyle, Kit.BodyTextStyle, OutStyleAssets);

	Kit.LoadoutSlotButtonStyle = ConfigureButtonStyleAsset(TEXT("BTS_LoadoutSlot"),
		MakeRoundedBrush(UISlotFillColor, 8.0f, UIPanelOutlineColor, 1.2f),
		MakeRoundedBrush(UISlotFillHoveredColor, 8.0f, FLinearColor(1.0f, 1.0f, 1.0f, 0.28f), 1.2f),
		MakeRoundedBrush(UISlotFillPressedColor, 8.0f, UIAccentColor, 1.6f),
		MakeRoundedBrush(UISlotFillColor, 8.0f, UIAccentColor, 2.0f),
		MakeRoundedBrush(FLinearColor(0.06f, 0.07f, 0.09f, 0.4f), 8.0f, FLinearColor(1.0f, 1.0f, 1.0f, 0.04f), 1.0f),
		FMargin(0.0f), 0, 0,
		Kit.SmallTextStyle, Kit.SectionTitleTextStyle, Kit.SmallTextStyle, OutStyleAssets);

	return Kit;
}

// ---------- 공용 위젯 트리 헬퍼 ----------

UCommonTextBlock* MakeCommonText(UWidgetTree* WidgetTree, const FName& Name, const FText& Text, UClass* StyleClass, ETextJustify::Type Justification = ETextJustify::Left, bool bWrap = false)
{
	UCommonTextBlock* TextBlock = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), Name);
	TextBlock->SetText(Text);
	if (StyleClass)
	{
		TextBlock->SetStyle(StyleClass);
	}
	TextBlock->SetJustification(Justification);
	if (bWrap)
	{
		TextBlock->SetAutoWrapText(true);
	}
	return TextBlock;
}

UWidgetTree* EnsureWidgetTree(UWidgetBlueprint* WidgetBlueprint)
{
	UWidgetTree* WidgetTree = WidgetBlueprint->WidgetTree;
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(WidgetBlueprint, TEXT("WidgetTree"), RF_Transactional);
		WidgetBlueprint->WidgetTree = WidgetTree;
	}
	WidgetTree->Modify();

	// 재생성 시 기존 위젯(고아 포함)이 새 위젯과 이름/클래스가 충돌하지 않도록 트랜지언트 패키지로 치웁니다.
	TArray<UObject*> OldSubobjects;
	GetObjectsWithOuter(WidgetTree, OldSubobjects, false);
	for (UObject* OldSubobject : OldSubobjects)
	{
		if (UWidget* OldWidget = Cast<UWidget>(OldSubobject))
		{
			OldWidget->SetFlags(RF_Transient);
			OldWidget->Rename(
				*MakeUniqueObjectName(GetTransientPackage(), OldWidget->GetClass()).ToString(),
				GetTransientPackage(),
				REN_DontCreateRedirectors | REN_NonTransactional);
		}
	}
	WidgetTree->RootWidget = nullptr;
	return WidgetTree;
}

void FinalizeWidgetBlueprint(UWidgetBlueprint* WidgetBlueprint)
{
	ResetWidgetVariableGuids(WidgetBlueprint);
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(WidgetBlueprint);
	// C++ 프로퍼티 타입이 바뀐 경우 이전 컴파일이 만든 스켈레톤 변수와 충돌하지 않도록 스켈레톤을 강제 재생성합니다.
	FKismetEditorUtilities::GenerateBlueprintSkeleton(WidgetBlueprint, true);
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
	WidgetBlueprint->MarkPackageDirty();
}

/** 버튼 위젯 블루프린트 CDO에 CommonUI 버튼 스타일 클래스를 지정합니다 (Style 프로퍼티는 protected라 리플렉션 사용). */
void ApplyButtonStyleToBlueprintDefaults(UWidgetBlueprint* WidgetBlueprint, UClass* StyleClass)
{
	if (!WidgetBlueprint || !WidgetBlueprint->GeneratedClass || !StyleClass)
	{
		return;
	}
	UObject* ButtonDefaults = WidgetBlueprint->GeneratedClass->GetDefaultObject();
	ButtonDefaults->Modify();
	SetStyleProperty(ButtonDefaults, TEXT("Style"), TSubclassOf<UCommonButtonStyle>(StyleClass));
	WidgetBlueprint->MarkPackageDirty();
}

// ---------- 재사용 버튼 컴포넌트 WBP ----------

// 캐릭터 선택 슬롯: SizeBox 루트 + Overlay(초상화/이름). 배경과 선택 강조는 버튼 스타일이 담당합니다.
void ConfigureCharacterSlotButtonTree(UWidgetBlueprint* WidgetBlueprint, const FFRGeneratedUIKit& Kit)
{
	if (!WidgetBlueprint)
	{
		return;
	}

	WidgetBlueprint->Modify();
	WidgetBlueprint->ParentClass = UFRCharacterSelectSlotButton::StaticClass();
	UWidgetTree* WidgetTree = EnsureWidgetTree(WidgetBlueprint);

	USizeBox* RootSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("RootSizeBox"));
	RootSizeBox->SetWidthOverride(164.0f);
	RootSizeBox->SetHeightOverride(190.0f);
	WidgetTree->RootWidget = RootSizeBox;

	UOverlay* ContentOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("ContentOverlay"));
	RootSizeBox->AddChild(ContentOverlay);

	UImage* PortraitImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("PortraitImage"));
	FSlateBrush PortraitBrush;
	PortraitBrush.ImageSize = FVector2D(132.0f, 132.0f);
	PortraitImage->SetBrush(PortraitBrush);
	PortraitImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	if (UOverlaySlot* PortraitSlot = ContentOverlay->AddChildToOverlay(PortraitImage))
	{
		PortraitSlot->SetHorizontalAlignment(HAlign_Center);
		PortraitSlot->SetVerticalAlignment(VAlign_Top);
		PortraitSlot->SetPadding(FMargin(0.0f, 14.0f, 0.0f, 0.0f));
	}

	UCommonTextBlock* NameText = MakeCommonText(WidgetTree, TEXT("NameText"), FText::FromString(TEXT("-")), Kit.LabelTextStyle, ETextJustify::Center);
	NameText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	if (UOverlaySlot* NameSlot = ContentOverlay->AddChildToOverlay(NameText))
	{
		NameSlot->SetHorizontalAlignment(HAlign_Center);
		NameSlot->SetVerticalAlignment(VAlign_Bottom);
		NameSlot->SetPadding(FMargin(4.0f, 0.0f, 4.0f, 12.0f));
	}

	FinalizeWidgetBlueprint(WidgetBlueprint);
	ApplyButtonStyleToBlueprintDefaults(WidgetBlueprint, Kit.SlotButtonStyle);
}

// 라벨 버튼(GAME START / FIRE): Overlay 루트 + 라벨. 최소 크기는 버튼 스타일 MinWidth/MinHeight가 정합니다.
void ConfigureTextButtonTree(UWidgetBlueprint* WidgetBlueprint, const FFRGeneratedUIKit& Kit)
{
	if (!WidgetBlueprint)
	{
		return;
	}

	WidgetBlueprint->Modify();
	WidgetBlueprint->ParentClass = UFRCommonTextButton::StaticClass();
	UWidgetTree* WidgetTree = EnsureWidgetTree(WidgetBlueprint);

	UOverlay* ContentOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("ContentOverlay"));
	WidgetTree->RootWidget = ContentOverlay;

	UCommonTextBlock* ButtonLabelText = MakeCommonText(WidgetTree, TEXT("ButtonLabelText"), FText::FromString(TEXT("BUTTON")), Kit.ButtonDarkTextStyle, ETextJustify::Center);
	ButtonLabelText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	if (UOverlaySlot* LabelSlot = ContentOverlay->AddChildToOverlay(ButtonLabelText))
	{
		LabelSlot->SetHorizontalAlignment(HAlign_Center);
		LabelSlot->SetVerticalAlignment(VAlign_Center);
		LabelSlot->SetPadding(FMargin(28.0f, 14.0f));
	}

	FinalizeWidgetBlueprint(WidgetBlueprint);
	ApplyButtonStyleToBlueprintDefaults(WidgetBlueprint, Kit.AccentButtonStyle);
}

// 전투 로드아웃 슬롯: 좌상단 단축키/횟수 + 중앙 이름.
void ConfigureLoadoutSlotButtonTree(UWidgetBlueprint* WidgetBlueprint, const FFRGeneratedUIKit& Kit)
{
	if (!WidgetBlueprint)
	{
		return;
	}

	WidgetBlueprint->Modify();
	WidgetBlueprint->ParentClass = UFRLoadoutSlotButton::StaticClass();
	UWidgetTree* WidgetTree = EnsureWidgetTree(WidgetBlueprint);

	USizeBox* RootSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("RootSizeBox"));
	RootSizeBox->SetWidthOverride(112.0f);
	RootSizeBox->SetHeightOverride(84.0f);
	WidgetTree->RootWidget = RootSizeBox;

	UOverlay* ContentOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("ContentOverlay"));
	RootSizeBox->AddChild(ContentOverlay);

	UCommonTextBlock* SlotHotkeyText = MakeCommonText(WidgetTree, TEXT("SlotHotkeyText"), FText::GetEmpty(), Kit.SmallTextStyle);
	SlotHotkeyText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	if (UOverlaySlot* HotkeySlot = ContentOverlay->AddChildToOverlay(SlotHotkeyText))
	{
		HotkeySlot->SetHorizontalAlignment(HAlign_Left);
		HotkeySlot->SetVerticalAlignment(VAlign_Top);
		HotkeySlot->SetPadding(FMargin(8.0f, 5.0f, 0.0f, 0.0f));
	}

	UCommonTextBlock* SlotNameText = MakeCommonText(WidgetTree, TEXT("SlotNameText"), FText::GetEmpty(), Kit.SmallTextStyle, ETextJustify::Center, true);
	SlotNameText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	if (UOverlaySlot* NameSlot = ContentOverlay->AddChildToOverlay(SlotNameText))
	{
		NameSlot->SetHorizontalAlignment(HAlign_Fill);
		NameSlot->SetVerticalAlignment(VAlign_Center);
		NameSlot->SetPadding(FMargin(6.0f, 18.0f, 6.0f, 6.0f));
	}

	FinalizeWidgetBlueprint(WidgetBlueprint);
	ApplyButtonStyleToBlueprintDefaults(WidgetBlueprint, Kit.LoadoutSlotButtonStyle);
}

// 보상 선택 버튼: 이름 + 설명 세로 배치.
void ConfigureRewardChoiceButtonTree(UWidgetBlueprint* WidgetBlueprint, const FFRGeneratedUIKit& Kit)
{
	if (!WidgetBlueprint)
	{
		return;
	}

	WidgetBlueprint->Modify();
	WidgetBlueprint->ParentClass = UFRRewardChoiceButton::StaticClass();
	UWidgetTree* WidgetTree = EnsureWidgetTree(WidgetBlueprint);

	USizeBox* RootSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("RootSizeBox"));
	RootSizeBox->SetWidthOverride(300.0f);
	RootSizeBox->SetMinDesiredHeight(140.0f);
	WidgetTree->RootWidget = RootSizeBox;

	UVerticalBox* ContentBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ContentBox"));
	RootSizeBox->AddChild(ContentBox);

	UCommonTextBlock* RewardNameText = MakeCommonText(WidgetTree, TEXT("RewardNameText"), FText::GetEmpty(), Kit.LabelTextStyle, ETextJustify::Center, true);
	RewardNameText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	if (UVerticalBoxSlot* NameSlot = ContentBox->AddChildToVerticalBox(RewardNameText))
	{
		NameSlot->SetPadding(FMargin(14.0f, 14.0f, 14.0f, 4.0f));
	}

	UCommonTextBlock* RewardDescriptionText = MakeCommonText(WidgetTree, TEXT("RewardDescriptionText"), FText::GetEmpty(), Kit.SmallTextStyle, ETextJustify::Center, true);
	RewardDescriptionText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	if (UVerticalBoxSlot* DescriptionSlot = ContentBox->AddChildToVerticalBox(RewardDescriptionText))
	{
		DescriptionSlot->SetPadding(FMargin(14.0f, 0.0f, 14.0f, 14.0f));
	}

	FinalizeWidgetBlueprint(WidgetBlueprint);
	ApplyButtonStyleToBlueprintDefaults(WidgetBlueprint, Kit.SlotButtonStyle);
}

UTexture2D* LoadOrCreateSelectVignetteTexture()
{
	constexpr int32 Size = 256;
	return LoadOrCreateGeneratedTexture(CharacterSelectHUDPath, TEXT("T_SelectVignette"), Size, Size, [](int32 X, int32 Y)
	{
		const float CenterOffset = (Size - 1) * 0.5f;
		const float MaxDistance = CenterOffset * 1.4142f;
		const float Distance = FMath::Sqrt(FMath::Square(X - CenterOffset) + FMath::Square(Y - CenterOffset)) / MaxDistance;
		const float Falloff = FMath::Clamp((Distance - 0.5f) / 0.5f, 0.0f, 1.0f);
		const uint8 Alpha = static_cast<uint8>(FMath::Pow(Falloff, 1.6f) * 0.6f * 255.0f);
		return FColor(0, 0, 0, Alpha);
	});
}

UTexture2D* LoadOrCreateSelectBottomGradientTexture()
{
	constexpr int32 Width = 8;
	constexpr int32 Height = 256;
	return LoadOrCreateGeneratedTexture(CharacterSelectHUDPath, TEXT("T_SelectBottomGradient"), Width, Height, [](int32 X, int32 Y)
	{
		const float T = static_cast<float>(Y) / (Height - 1);
		const uint8 Alpha = static_cast<uint8>(FMath::Pow(T, 1.5f) * 0.62f * 255.0f);
		return FColor(2, 4, 8, Alpha);
	});
}

UProgressBar* MakeProgressBar(UWidgetTree* WidgetTree, const FName& Name, const FLinearColor& FillColor)
{
	UProgressBar* ProgressBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), Name);
	ProgressBar->SetPercent(1.0f);
	ProgressBar->SetFillColorAndOpacity(FillColor);
	return ProgressBar;
}

// 진행 바 + 수치 텍스트 한 줄을 세로 박스에 추가합니다.
void AddStatBarRow(UWidgetTree* WidgetTree, UVerticalBox* ParentBox, const TCHAR* RowName, const TCHAR* BarName, const TCHAR* TextName, const FLinearColor& FillColor, UClass* TextStyle)
{
	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), FName(RowName));
	if (UVerticalBoxSlot* RowSlot = ParentBox->AddChildToVerticalBox(Row))
	{
		RowSlot->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 0.0f));
	}

	USizeBox* BarSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), FName(*FString::Printf(TEXT("%sSizeBox"), BarName)));
	BarSizeBox->SetHeightOverride(20.0f);
	BarSizeBox->AddChild(MakeProgressBar(WidgetTree, FName(BarName), FillColor));
	if (UHorizontalBoxSlot* BarSlot = Row->AddChildToHorizontalBox(BarSizeBox))
	{
		BarSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		BarSlot->SetVerticalAlignment(VAlign_Center);
	}

	UCommonTextBlock* ValueText = MakeCommonText(WidgetTree, FName(TextName), FText::FromString(TEXT("-")), TextStyle);
	if (UHorizontalBoxSlot* TextSlot = Row->AddChildToHorizontalBox(ValueText))
	{
		TextSlot->SetPadding(FMargin(10.0f, 0.0f, 0.0f, 0.0f));
		TextSlot->SetVerticalAlignment(VAlign_Center);
	}
}

// 로드아웃 슬롯 버튼 컴포넌트 인스턴스 5개를 가로 박스로 추가합니다.
UHorizontalBox* AddLoadoutSlotRow(UWidgetTree* WidgetTree, UVerticalBox* ParentBox, const TCHAR* RowName, const TCHAR* SlotNamePrefix, UClass* SlotButtonClass)
{
	UHorizontalBox* SlotsBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), FName(RowName));
	if (UVerticalBoxSlot* SlotsSlot = ParentBox->AddChildToVerticalBox(SlotsBox))
	{
		SlotsSlot->SetHorizontalAlignment(HAlign_Center);
		SlotsSlot->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 0.0f));
	}

	for (int32 SlotIndex = 0; SlotIndex < 5; ++SlotIndex)
	{
		const FName ButtonName(*FString::Printf(TEXT("%s%dButton"), SlotNamePrefix, SlotIndex + 1));
		UUserWidget* SlotButton = WidgetTree->ConstructWidget<UUserWidget>(SlotButtonClass, ButtonName);
		if (UHorizontalBoxSlot* ButtonSlot = SlotsBox->AddChildToHorizontalBox(SlotButton))
		{
			ButtonSlot->SetPadding(FMargin(4.0f, 0.0f));
		}
	}
	return SlotsBox;
}

// 전투 HUD: 상단 상태 텍스트, 하단 CommonBorder 바(바람/로드아웃/스탯/발사), 중앙 보상 패널로 구성합니다.
// 캔버스에는 영역 앵커만 두고 내부는 전부 컨테이너입니다.
void ConfigureMainGameHUDTree(UWidgetBlueprint* WidgetBlueprint, UTexture2D* WindArrowTexture, const FFRGeneratedUIKit& Kit)
{
	if (!WidgetBlueprint)
	{
		return;
	}

	WidgetBlueprint->Modify();
	WidgetBlueprint->ParentClass = UFRMainGameHUDWidget::StaticClass();
	UWidgetTree* WidgetTree = EnsureWidgetTree(WidgetBlueprint);

	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = Root;

	// 상단: 좌측 스테이지 진행, 중앙 턴 상태.
	UCommonTextBlock* TurnText = MakeCommonText(WidgetTree, TEXT("TurnText"), FText::FromString(TEXT("Stage 1/1")), Kit.HeadingTextStyle);
	if (UCanvasPanelSlot* TurnSlot = Root->AddChildToCanvas(TurnText))
	{
		ConfigureCanvasSlot(TurnSlot, FAnchors(0.0f, 0.0f), FVector2D(28.0f, 20.0f), FVector2D::ZeroVector, FVector2D::ZeroVector);
		TurnSlot->SetAutoSize(true);
	}

	UVerticalBox* TopStatusBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("TopStatusBox"));
	if (UCanvasPanelSlot* TopStatusSlot = Root->AddChildToCanvas(TopStatusBox))
	{
		ConfigureCanvasSlot(TopStatusSlot, FAnchors(0.5f, 0.0f), FVector2D(0.0f, 18.0f), FVector2D::ZeroVector, FVector2D(0.5f, 0.0f));
		TopStatusSlot->SetAutoSize(true);
	}
	TopStatusBox->AddChildToVerticalBox(MakeCommonText(WidgetTree, TEXT("ShotTimerText"), FText::FromString(TEXT("Player turn")), Kit.HeadingTextStyle, ETextJustify::Center));
	if (UVerticalBoxSlot* StatusSlot = TopStatusBox->AddChildToVerticalBox(MakeCommonText(WidgetTree, TEXT("StatusText"), FText::FromString(TEXT("Ready")), Kit.BodyTextStyle, ETextJustify::Center)))
	{
		StatusSlot->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 0.0f));
	}

	// 하단 바: 바람/조준 | 로드아웃(무기+아이템) | 플레이어 스탯 | 발사.
	UCommonBorder* BottomPanelBorder = WidgetTree->ConstructWidget<UCommonBorder>(UCommonBorder::StaticClass(), TEXT("BottomPanelBorder"));
	if (Kit.BottomBarBorderStyle)
	{
		BottomPanelBorder->SetStyle(Kit.BottomBarBorderStyle);
	}
	BottomPanelBorder->SetPadding(FMargin(24.0f, 14.0f));
	ConfigureCanvasSlot(Root->AddChildToCanvas(BottomPanelBorder), FAnchors(0.0f, 1.0f, 1.0f, 1.0f), FVector2D(0.0f, 0.0f), FVector2D(0.0f, 236.0f), FVector2D(0.0f, 1.0f));

	UHorizontalBox* BottomRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("BottomRow"));
	BottomPanelBorder->AddChild(BottomRow);

	// 바람/조준 컬럼.
	UVerticalBox* WindBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("WindBox"));
	if (UHorizontalBoxSlot* WindBoxSlot = BottomRow->AddChildToHorizontalBox(WindBox))
	{
		WindBoxSlot->SetVerticalAlignment(VAlign_Center);
	}
	UImage* WindArrowVisual = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("WindArrowVisual"));
	if (WindArrowTexture)
	{
		WindArrowVisual->SetBrushFromTexture(WindArrowTexture, true);
	}
	WindArrowVisual->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	WindBox->AddChildToVerticalBox(WindArrowVisual);
	if (UVerticalBoxSlot* WindTextSlot = WindBox->AddChildToVerticalBox(MakeCommonText(WidgetTree, TEXT("WindText"), FText::FromString(TEXT("Wind 0")), Kit.HeadingTextStyle)))
	{
		WindTextSlot->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 0.0f));
	}
	if (UVerticalBoxSlot* AimTextSlot = WindBox->AddChildToVerticalBox(MakeCommonText(WidgetTree, TEXT("AimText"), FText::FromString(TEXT("45 deg")), Kit.LabelTextStyle)))
	{
		AimTextSlot->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 0.0f));
	}

	// 로드아웃 컬럼: 현재 무기 + 무기 슬롯 5개 + 샷 요약 + 아이템 슬롯 5개.
	UVerticalBox* LoadoutColumn = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LoadoutColumn"));
	if (UHorizontalBoxSlot* LoadoutColumnSlot = BottomRow->AddChildToHorizontalBox(LoadoutColumn))
	{
		LoadoutColumnSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		LoadoutColumnSlot->SetPadding(FMargin(28.0f, 0.0f, 0.0f, 0.0f));
		LoadoutColumnSlot->SetVerticalAlignment(VAlign_Center);
	}
	LoadoutColumn->AddChildToVerticalBox(MakeCommonText(WidgetTree, TEXT("CurrentWeaponText"), FText::FromString(TEXT("Weapon")), Kit.LabelTextStyle, ETextJustify::Center));
	AddLoadoutSlotRow(WidgetTree, LoadoutColumn, TEXT("WeaponSlotsBox"), TEXT("WeaponSlot"), Kit.LoadoutSlotButtonClass);
	if (UVerticalBoxSlot* ShotTextSlot = LoadoutColumn->AddChildToVerticalBox(MakeCommonText(WidgetTree, TEXT("CurrentShotText"), FText::GetEmpty(), Kit.SmallTextStyle, ETextJustify::Center)))
	{
		ShotTextSlot->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 0.0f));
	}
	AddLoadoutSlotRow(WidgetTree, LoadoutColumn, TEXT("ItemSlotsBox"), TEXT("ItemSlot"), Kit.LoadoutSlotButtonClass);

	// 플레이어 스탯 컬럼: 이름 + HP/파워/이동 바.
	UVerticalBox* StatusColumn = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("StatusColumn"));
	if (UHorizontalBoxSlot* StatusColumnSlot = BottomRow->AddChildToHorizontalBox(StatusColumn))
	{
		StatusColumnSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		StatusColumnSlot->SetPadding(FMargin(28.0f, 0.0f, 0.0f, 0.0f));
		StatusColumnSlot->SetVerticalAlignment(VAlign_Center);
	}
	StatusColumn->AddChildToVerticalBox(MakeCommonText(WidgetTree, TEXT("PlayerNameText"), FText::FromString(TEXT("Player")), Kit.LabelTextStyle));
	AddStatBarRow(WidgetTree, StatusColumn, TEXT("PlayerHPRow"), TEXT("PlayerHPBar"), TEXT("PlayerHPText"), FLinearColor(0.9f, 0.12f, 0.1f), Kit.LabelTextStyle);
	AddStatBarRow(WidgetTree, StatusColumn, TEXT("ShotPowerRow"), TEXT("ShotPowerBar"), TEXT("ShotPowerText"), FLinearColor(0.95f, 0.68f, 0.13f), Kit.LabelTextStyle);
	AddStatBarRow(WidgetTree, StatusColumn, TEXT("MoveBudgetRow"), TEXT("MoveBudgetBar"), TEXT("MoveBudgetText"), FLinearColor(0.1f, 0.72f, 0.95f), Kit.LabelTextStyle);

	// 발사 버튼.
	UUserWidget* FireButton = WidgetTree->ConstructWidget<UUserWidget>(Kit.TextButtonClass, TEXT("FireButton"));
	if (UFRCommonTextButton* FireTextButton = Cast<UFRCommonTextButton>(FireButton))
	{
		FireTextButton->SetButtonLabel(FText::FromString(TEXT("FIRE")));
	}
	if (UHorizontalBoxSlot* FireSlot = BottomRow->AddChildToHorizontalBox(FireButton))
	{
		FireSlot->SetPadding(FMargin(28.0f, 0.0f, 0.0f, 0.0f));
		FireSlot->SetVerticalAlignment(VAlign_Center);
	}

	// 보상 패널: 중앙 카드, 보상 상태에서만 표시됩니다.
	UCommonBorder* RewardPanel = WidgetTree->ConstructWidget<UCommonBorder>(UCommonBorder::StaticClass(), TEXT("RewardPanel"));
	if (Kit.PanelBorderStyle)
	{
		RewardPanel->SetStyle(Kit.PanelBorderStyle);
	}
	RewardPanel->SetPadding(FMargin(28.0f, 22.0f));
	RewardPanel->SetVisibility(ESlateVisibility::Collapsed);
	if (UCanvasPanelSlot* RewardPanelSlot = Root->AddChildToCanvas(RewardPanel))
	{
		ConfigureCanvasSlot(RewardPanelSlot, FAnchors(0.5f, 0.5f), FVector2D(0.0f, -40.0f), FVector2D::ZeroVector, FVector2D(0.5f, 0.5f));
		RewardPanelSlot->SetAutoSize(true);
	}

	UVerticalBox* RewardBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RewardBox"));
	RewardPanel->AddChild(RewardBox);
	RewardBox->AddChildToVerticalBox(MakeCommonText(WidgetTree, TEXT("RewardTitleText"), FText::FromString(TEXT("Choose a reward")), Kit.HeadingTextStyle, ETextJustify::Center));

	UHorizontalBox* RewardChoicesBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("RewardChoicesBox"));
	if (UVerticalBoxSlot* ChoicesSlot = RewardBox->AddChildToVerticalBox(RewardChoicesBox))
	{
		ChoicesSlot->SetPadding(FMargin(0.0f, 14.0f, 0.0f, 0.0f));
		ChoicesSlot->SetHorizontalAlignment(HAlign_Center);
	}
	for (int32 ChoiceIndex = 0; ChoiceIndex < 3; ++ChoiceIndex)
	{
		const FName ButtonName(*FString::Printf(TEXT("RewardChoice%dButton"), ChoiceIndex + 1));
		UUserWidget* RewardButton = WidgetTree->ConstructWidget<UUserWidget>(Kit.RewardChoiceButtonClass, ButtonName);
		if (UHorizontalBoxSlot* RewardButtonSlot = RewardChoicesBox->AddChildToHorizontalBox(RewardButton))
		{
			RewardButtonSlot->SetPadding(FMargin(8.0f, 0.0f));
		}
	}

	FinalizeWidgetBlueprint(WidgetBlueprint);
}

// 캐릭터 선택 HUD: 캔버스에는 영역 앵커만 두고, 각 영역 내부는 Border/VerticalBox/HorizontalBox 컨테이너로 구성해
// 레이아웃 편집이 쉽습니다. 버튼은 전부 CommonUI 컴포넌트 WBP 인스턴스이고 텍스트는 버튼 내부 콘텐츠입니다.
void ConfigureCharacterSelectHUDTree(UWidgetBlueprint* WidgetBlueprint, UTexture2D* VignetteTexture, UTexture2D* BottomGradientTexture, const FFRGeneratedUIKit& Kit)
{
	if (!WidgetBlueprint)
	{
		return;
	}

	WidgetBlueprint->Modify();
	WidgetBlueprint->ParentClass = UFRCharacterSelectWidget::StaticClass();
	UWidgetTree* WidgetTree = EnsureWidgetTree(WidgetBlueprint);

	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = Root;

	// 배경 장식: 가장자리 비네트 + 캐릭터 리스트 뒤 하단 그라데이션.
	UImage* Vignette = AddImage(WidgetTree, Root, TEXT("VignetteImage"), VignetteTexture, FAnchors(0.0f, 0.0f, 1.0f, 1.0f), FVector2D::ZeroVector, FVector2D::ZeroVector, FVector2D::ZeroVector);
	Vignette->SetColorAndOpacity(FLinearColor::White);
	Vignette->SetVisibility(ESlateVisibility::HitTestInvisible);
	UImage* BottomGradient = AddImage(WidgetTree, Root, TEXT("BottomGradientImage"), BottomGradientTexture, FAnchors(0.0f, 1.0f, 1.0f, 1.0f), FVector2D(0.0f, 0.0f), FVector2D(0.0f, 360.0f), FVector2D(0.0f, 1.0f));
	BottomGradient->SetColorAndOpacity(FLinearColor::White);
	BottomGradient->SetVisibility(ESlateVisibility::HitTestInvisible);

	// 오른쪽 정보 카드: CommonBorder 안에 세로 박스로 이름/설명/상세 2열을 쌓습니다.
	UCommonBorder* InfoPanelBorder = WidgetTree->ConstructWidget<UCommonBorder>(UCommonBorder::StaticClass(), TEXT("InfoPanelBorder"));
	if (Kit.PanelBorderStyle)
	{
		InfoPanelBorder->SetStyle(Kit.PanelBorderStyle);
	}
	InfoPanelBorder->SetPadding(FMargin(30.0f, 26.0f));
	ConfigureCanvasSlot(Root->AddChildToCanvas(InfoPanelBorder), FAnchors(1.0f, 0.0f), FVector2D(-40.0f, 44.0f), FVector2D(640.0f, 560.0f), FVector2D(1.0f, 0.0f));

	UVerticalBox* InfoPanelBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("InfoPanelBox"));
	InfoPanelBorder->AddChild(InfoPanelBox);

	UCommonTextBlock* NameText = MakeCommonText(WidgetTree, TEXT("NameText"), FText::FromString(TEXT("Select a character")), Kit.TitleTextStyle, ETextJustify::Center);
	InfoPanelBox->AddChildToVerticalBox(NameText);

	USizeBox* NameUnderlineSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("NameUnderlineSizeBox"));
	NameUnderlineSizeBox->SetWidthOverride(140.0f);
	NameUnderlineSizeBox->SetHeightOverride(5.0f);
	UImage* NameUnderline = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("NameUnderline"));
	NameUnderline->SetBrush(MakeRoundedBrush(UIAccentColor, 2.5f, FLinearColor::Transparent, 0.0f));
	NameUnderline->SetVisibility(ESlateVisibility::HitTestInvisible);
	NameUnderlineSizeBox->AddChild(NameUnderline);
	if (UVerticalBoxSlot* UnderlineSlot = InfoPanelBox->AddChildToVerticalBox(NameUnderlineSizeBox))
	{
		UnderlineSlot->SetHorizontalAlignment(HAlign_Center);
		UnderlineSlot->SetPadding(FMargin(0.0f, 10.0f, 0.0f, 0.0f));
	}

	UCommonTextBlock* DescriptionText = MakeCommonText(WidgetTree, TEXT("DescriptionText"), FText::FromString(TEXT("Pick a character from the list below.")), Kit.BodyTextStyle, ETextJustify::Center, true);
	if (UVerticalBoxSlot* DescriptionSlot = InfoPanelBox->AddChildToVerticalBox(DescriptionText))
	{
		DescriptionSlot->SetPadding(FMargin(0.0f, 18.0f, 0.0f, 0.0f));
	}

	USpacer* DetailSpacer = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass(), TEXT("DetailSpacer"));
	if (UVerticalBoxSlot* SpacerSlot = InfoPanelBox->AddChildToVerticalBox(DetailSpacer))
	{
		SpacerSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	UHorizontalBox* DetailRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("DetailRow"));
	InfoPanelBox->AddChildToVerticalBox(DetailRow);

	UVerticalBox* SpecialColumn = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("SpecialWeaponColumn"));
	if (UHorizontalBoxSlot* SpecialColumnSlot = DetailRow->AddChildToHorizontalBox(SpecialColumn))
	{
		SpecialColumnSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}
	SpecialColumn->AddChildToVerticalBox(MakeCommonText(WidgetTree, TEXT("SpecialWeaponTitleText"), FText::FromString(TEXT("SPECIAL WEAPON")), Kit.SectionTitleTextStyle));
	if (UVerticalBoxSlot* SpecialNameSlot = SpecialColumn->AddChildToVerticalBox(MakeCommonText(WidgetTree, TEXT("SpecialWeaponNameText"), FText::FromString(TEXT("-")), Kit.LabelTextStyle)))
	{
		SpecialNameSlot->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 0.0f));
	}
	if (UVerticalBoxSlot* SpecialDetailSlot = SpecialColumn->AddChildToVerticalBox(MakeCommonText(WidgetTree, TEXT("SpecialWeaponDetailText"), FText::GetEmpty(), Kit.SmallTextStyle, ETextJustify::Left, true)))
	{
		SpecialDetailSlot->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 0.0f));
	}

	UVerticalBox* StatsColumn = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("StatsColumn"));
	if (UHorizontalBoxSlot* StatsColumnSlot = DetailRow->AddChildToHorizontalBox(StatsColumn))
	{
		StatsColumnSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		StatsColumnSlot->SetPadding(FMargin(24.0f, 0.0f, 0.0f, 0.0f));
	}
	StatsColumn->AddChildToVerticalBox(MakeCommonText(WidgetTree, TEXT("StatsTitleText"), FText::FromString(TEXT("BASE STATS")), Kit.SectionTitleTextStyle));
	if (UVerticalBoxSlot* StatsTextSlot = StatsColumn->AddChildToVerticalBox(MakeCommonText(WidgetTree, TEXT("StatsText"), FText::GetEmpty(), Kit.LabelTextStyle)))
	{
		StatsTextSlot->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 0.0f));
	}

	// 하단 캐릭터 리스트: 슬롯 버튼 컴포넌트 인스턴스를 가로 박스에 나란히 배치합니다.
	UHorizontalBox* CharacterListBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("CharacterListBox"));
	if (UCanvasPanelSlot* ListSlot = Root->AddChildToCanvas(CharacterListBox))
	{
		ConfigureCanvasSlot(ListSlot, FAnchors(0.5f, 1.0f), FVector2D(0.0f, -96.0f), FVector2D::ZeroVector, FVector2D(0.5f, 1.0f));
		ListSlot->SetAutoSize(true);
	}
	for (int32 SlotIndex = 0; SlotIndex < 5; ++SlotIndex)
	{
		const FName ButtonName(*FString::Printf(TEXT("CharacterSlot%dButton"), SlotIndex + 1));
		UUserWidget* SlotButton = WidgetTree->ConstructWidget<UUserWidget>(Kit.CharacterSlotButtonClass, ButtonName);
		if (UHorizontalBoxSlot* SlotButtonSlot = CharacterListBox->AddChildToHorizontalBox(SlotButton))
		{
			SlotButtonSlot->SetPadding(FMargin(8.0f, 0.0f));
		}
	}

	// 게임 시작 버튼: 라벨 내장 액센트 버튼 컴포넌트.
	UUserWidget* GameStartButton = WidgetTree->ConstructWidget<UUserWidget>(Kit.TextButtonClass, TEXT("GameStartButton"));
	if (UFRCommonTextButton* GameStartTextButton = Cast<UFRCommonTextButton>(GameStartButton))
	{
		GameStartTextButton->SetButtonLabel(FText::FromString(TEXT("GAME START")));
	}
	if (UCanvasPanelSlot* StartSlot = Root->AddChildToCanvas(GameStartButton))
	{
		ConfigureCanvasSlot(StartSlot, FAnchors(1.0f, 1.0f), FVector2D(-60.0f, -56.0f), FVector2D::ZeroVector, FVector2D(1.0f, 1.0f));
		StartSlot->SetAutoSize(true);
	}

	FinalizeWidgetBlueprint(WidgetBlueprint);
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

	// CommonUI 스타일 에셋(텍스트/버튼/보더)을 먼저 만들고, 그 스타일을 쓰는 컴포넌트 WBP를 만든 뒤 HUD를 구성합니다.
	TArray<UObject*> StyleAssets;
	FFRGeneratedUIKit Kit = ConfigureUIStyleAssets(StyleAssets);

	UWidgetBlueprint* CharacterSlotButton = LoadOrCreateWidgetBlueprint(CommonComponentPath, TEXT("WBP_CharacterSlotButton"), UFRCharacterSelectSlotButton::StaticClass());
	UWidgetBlueprint* TextButton = LoadOrCreateWidgetBlueprint(CommonComponentPath, TEXT("WBP_TextButton"), UFRCommonTextButton::StaticClass());
	UWidgetBlueprint* LoadoutSlotButton = LoadOrCreateWidgetBlueprint(CommonComponentPath, TEXT("WBP_LoadoutSlotButton"), UFRLoadoutSlotButton::StaticClass());
	UWidgetBlueprint* RewardChoiceButton = LoadOrCreateWidgetBlueprint(CommonComponentPath, TEXT("WBP_RewardChoiceButton"), UFRRewardChoiceButton::StaticClass());
	if (!CharacterSlotButton || !TextButton || !LoadoutSlotButton || !RewardChoiceButton)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not create CommonUI button component widget blueprints."));
		return 1;
	}

	ConfigureCharacterSlotButtonTree(CharacterSlotButton, Kit);
	ConfigureTextButtonTree(TextButton, Kit);
	ConfigureLoadoutSlotButtonTree(LoadoutSlotButton, Kit);
	ConfigureRewardChoiceButtonTree(RewardChoiceButton, Kit);
	Kit.CharacterSlotButtonClass = CharacterSlotButton->GeneratedClass;
	Kit.TextButtonClass = TextButton->GeneratedClass;
	Kit.LoadoutSlotButtonClass = LoadoutSlotButton->GeneratedClass;
	Kit.RewardChoiceButtonClass = RewardChoiceButton->GeneratedClass;

	if (!Kit.IsComplete())
	{
		UE_LOG(LogTemp, Error, TEXT("Could not create the full CommonUI style/component kit."));
		return 1;
	}

	UWidgetBlueprint* MainGameHUD = LoadOrCreateWidgetBlueprint(MainGameHUDPath, MainGameHUDName, UFRMainGameHUDWidget::StaticClass());
	if (!MainGameHUD)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not create %s/%s."), MainGameHUDPath, MainGameHUDName);
		return 1;
	}

	UWidgetBlueprint* CharacterSelectHUD = LoadOrCreateWidgetBlueprint(CharacterSelectHUDPath, CharacterSelectHUDName, UFRCharacterSelectWidget::StaticClass());
	if (!CharacterSelectHUD)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not create %s/%s."), CharacterSelectHUDPath, CharacterSelectHUDName);
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

	UTexture2D* SelectVignetteTexture = LoadOrCreateSelectVignetteTexture();
	UTexture2D* SelectBottomGradientTexture = LoadOrCreateSelectBottomGradientTexture();

	ConfigureMainGameHUDTree(MainGameHUD, WindArrowTexture, Kit);
	ConfigureCharacterSelectHUDTree(CharacterSelectHUD, SelectVignetteTexture, SelectBottomGradientTexture, Kit);
	ConfigureBattleCharacterStatusTree(BattleCharacterStatus);
	ConfigureBattleCharacterAimIndicatorTree(BattleCharacterAimIndicator, AimAngleMaterial);
	ConfigureFloatingDamageTextTree(FloatingDamageText);
	UFRGameModeDataAsset* MainGameMode = ConfigureMainGameModeHUD();

	bool bSavedStyles = true;
	for (UObject* StyleAsset : StyleAssets)
	{
		bSavedStyles &= SaveAsset(StyleAsset);
	}
	const bool bSavedComponents = SaveAsset(CharacterSlotButton) && SaveAsset(TextButton) && SaveAsset(LoadoutSlotButton) && SaveAsset(RewardChoiceButton);
	const bool bSavedWindArrow = SaveAsset(WindArrowTexture);
	const bool bSavedAimAngleMaterial = SaveAsset(AimAngleMaterial);
	const bool bSavedHUD = SaveAsset(MainGameHUD);
	const bool bSavedCharacterSelectHUD = SaveAsset(CharacterSelectHUD) && SaveAsset(SelectVignetteTexture) && SaveAsset(SelectBottomGradientTexture);
	const bool bSavedStatus = SaveAsset(BattleCharacterStatus);
	const bool bSavedAimIndicator = SaveAsset(BattleCharacterAimIndicator);
	const bool bSavedDamageText = SaveAsset(FloatingDamageText);
	const bool bSavedMode = SaveAsset(MainGameMode);
	UE_LOG(LogTemp, Display, TEXT("Generated MainGame combat UI. SavedStyles=%s SavedComponents=%s SavedWindArrow=%s SavedAimAngleMaterial=%s SavedHUD=%s SavedCharacterSelectHUD=%s SavedStatus=%s SavedAimIndicator=%s SavedDamageText=%s SavedMode=%s"),
		bSavedStyles ? TEXT("true") : TEXT("false"),
		bSavedComponents ? TEXT("true") : TEXT("false"),
		bSavedWindArrow ? TEXT("true") : TEXT("false"),
		bSavedAimAngleMaterial ? TEXT("true") : TEXT("false"),
		bSavedHUD ? TEXT("true") : TEXT("false"),
		bSavedCharacterSelectHUD ? TEXT("true") : TEXT("false"),
		bSavedStatus ? TEXT("true") : TEXT("false"),
		bSavedAimIndicator ? TEXT("true") : TEXT("false"),
		bSavedDamageText ? TEXT("true") : TEXT("false"),
		bSavedMode ? TEXT("true") : TEXT("false"));
	return bSavedStyles && bSavedComponents && bSavedWindArrow && bSavedAimAngleMaterial && bSavedHUD && bSavedCharacterSelectHUD && bSavedStatus && bSavedAimIndicator && bSavedDamageText && bSavedMode ? 0 : 1;
}
