// Copyright Epic Games, Inc. All Rights Reserved.

#include "Characters/FRCharacterDefinition.h"
#include "Combat/FRTerrainMapDefinition.h"
#include "FRProjectileEffectSpecCustomization.h"
#include "FRSpriteFlipbookGenerator.h"
#include "AssetImportTask.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "AssetTypeActions_Base.h"
#include "Editor.h"
#include "Engine/Texture2D.h"
#include "FileHelpers.h"
#include "Framework/Docking/TabManager.h"
#include "HAL/FileManager.h"
#include "IAssetTools.h"
#include "Materials/MaterialInterface.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "PaperFlipbook.h"
#include "PaperFlipbookFactory.h"
#include "PaperSprite.h"
#include "PaperSpriteFactory.h"
#include "PropertyEditorModule.h"
#include "PropertyCustomizationHelpers.h"
#include "SpriteEditorOnlyTypes.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FREditor"

class SFRTerrainMapEditor;

namespace FREditor
{
static const FName TerrainMapEditorTabName(TEXT("FRTerrainMapEditor"));
static const FName ProjectileEffectSpecTypeName(TEXT("FRProjectileEffectSpec"));
constexpr float DefaultSpawnClearance = 80.0f;
constexpr int32 GeneratedSpriteSheetColumns = 4;
constexpr int32 GeneratedSpriteSheetRows = 3;
constexpr int32 GeneratedSpriteFrameRun = 1;
constexpr float GeneratedSpriteFramesPerSecond = 8.0f;
constexpr uint8 GeneratedSpriteMagentaTolerance = 32;
static const TCHAR* GeneratedSpriteSourcePath = TEXT("/Game/GeneratedSprites");
static const TCHAR* GeneratedSpriteSourceRelativePath = TEXT("GeneratedSprites");
static const TCHAR* GeneratedSpriteOutputPath = TEXT("/Game/FortRogue/Character/GeneratedSprites");
static TWeakObjectPtr<UFRTerrainMapDefinition> PendingTerrainMapAsset;
static TWeakPtr<SFRTerrainMapEditor> ActiveTerrainMapEditor;
static void OpenTerrainMapEditorForAsset(UFRTerrainMapDefinition* Asset);

struct FGeneratedSpriteAnimationRow
{
	const TCHAR* Suffix;
	int32 RowIndex;
};

static const FGeneratedSpriteAnimationRow GeneratedSpriteAnimationRows[] =
{
	{ TEXT("_move_flipbook"), 0 },
	{ TEXT("_attack_flipbook"), 1 },
	{ TEXT("_aim_flipbook"), 2 }
};

static FString GetGeneratedSpriteBaseName(const UTexture2D& Texture)
{
	FString BaseName = Texture.GetName();
	BaseName.RemoveFromEnd(TEXT("_sprite_sheet"));
	return BaseName;
}

template <typename AssetType>
static AssetType* LoadGeneratedSpriteAsset(const FString& AssetName)
{
	const FString PackagePath = FString::Printf(TEXT("%s/%s"), GeneratedSpriteOutputPath, *AssetName);
	const FString PackageFilename = FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension());
	if (!FPaths::FileExists(PackageFilename))
	{
		return nullptr;
	}

	const FString ObjectPath = FString::Printf(TEXT("%s.%s"), *PackagePath, *AssetName);
	return LoadObject<AssetType>(nullptr, *ObjectPath);
}

static UTexture2D* LoadGeneratedSpriteTexture(const FString& AssetName)
{
	const FString PackagePath = FString::Printf(TEXT("%s/%s"), GeneratedSpriteSourcePath, *AssetName);
	const FString PackageFilename = FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension());
	if (!FPaths::FileExists(PackageFilename))
	{
		return nullptr;
	}

	const FString ObjectPath = FString::Printf(TEXT("%s.%s"), *PackagePath, *AssetName);
	return LoadObject<UTexture2D>(nullptr, *ObjectPath);
}

static bool IsGeneratedSpriteKeyColor(const uint8 Red, const uint8 Green, const uint8 Blue)
{
	return Red >= 255 - GeneratedSpriteMagentaTolerance
		&& Green <= GeneratedSpriteMagentaTolerance
		&& Blue >= 255 - GeneratedSpriteMagentaTolerance;
}

static UMaterialInterface* LoadGeneratedSpriteMaterial(const TCHAR* MaterialPath)
{
	return LoadObject<UMaterialInterface>(nullptr, MaterialPath);
}

static void ApplyGeneratedSpriteTextureAlphaKey(UTexture2D* Texture, TArray<UPackage*>& OutPackagesToSave)
{
	if (!Texture || !Texture->Source.IsValid() || Texture->Source.GetFormat() != TSF_BGRA8)
	{
		return;
	}

	uint8* MipData = Texture->Source.LockMip(0);
	if (!MipData)
	{
		return;
	}

	bool bChanged = false;
	const int32 PixelCount = Texture->Source.GetSizeX() * Texture->Source.GetSizeY();
	for (int32 PixelIndex = 0; PixelIndex < PixelCount; ++PixelIndex)
	{
		uint8* Pixel = MipData + PixelIndex * 4;
		uint8& Blue = Pixel[0];
		uint8& Green = Pixel[1];
		uint8& Red = Pixel[2];
		uint8& Alpha = Pixel[3];
		if (Alpha != 0 && IsGeneratedSpriteKeyColor(Red, Green, Blue))
		{
			Alpha = 0;
			bChanged = true;
		}
	}

	Texture->Source.UnlockMip(0);

	if (bChanged)
	{
		Texture->Modify();
		Texture->PostEditChange();
		Texture->MarkPackageDirty();
		OutPackagesToSave.AddUnique(Texture->GetPackage());
	}
}

static TArray<UTexture2D*> ImportGeneratedSpriteTextures(TArray<UPackage*>& OutPackagesToSave)
{
	TArray<FString> SourceFiles;
	const FString SourceDirectory = FPaths::Combine(FPaths::ProjectContentDir(), GeneratedSpriteSourceRelativePath);
	IFileManager::Get().FindFilesRecursive(SourceFiles, *SourceDirectory, TEXT("*.png"), true, false);
	SourceFiles.Sort();

	TArray<UTexture2D*> Textures;
	TArray<UAssetImportTask*> ImportTasks;
	for (const FString& SourceFile : SourceFiles)
	{
		const FString AssetName = FPaths::GetBaseFilename(SourceFile);
		if (UTexture2D* ExistingTexture = LoadGeneratedSpriteTexture(AssetName))
		{
			ApplyGeneratedSpriteTextureAlphaKey(ExistingTexture, OutPackagesToSave);
			Textures.Add(ExistingTexture);
			continue;
		}

		UAssetImportTask* ImportTask = NewObject<UAssetImportTask>();
		ImportTask->Filename = SourceFile;
		ImportTask->DestinationPath = GeneratedSpriteSourcePath;
		ImportTask->DestinationName = AssetName;
		ImportTask->bAutomated = true;
		ImportTask->bAsync = false;
		ImportTask->bReplaceExisting = false;
		ImportTask->bSave = true;
		ImportTasks.Add(ImportTask);
	}

	if (ImportTasks.Num() > 0)
	{
		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
		AssetToolsModule.Get().ImportAssetTasks(ImportTasks);
		for (UAssetImportTask* ImportTask : ImportTasks)
		{
			for (UObject* ImportedObject : ImportTask->GetObjects())
			{
				if (UTexture2D* Texture = Cast<UTexture2D>(ImportedObject))
				{
					ApplyGeneratedSpriteTextureAlphaKey(Texture, OutPackagesToSave);
					Textures.Add(Texture);
					OutPackagesToSave.AddUnique(Texture->GetPackage());
				}
			}
		}
	}

	return Textures;
}

static void UpdateGeneratedSpriteSource(UPaperSprite* Sprite, UTexture2D* Texture, const FIntPoint& SourceUV, const FIntPoint& SourceDimension, TArray<UPackage*>& OutPackagesToSave)
{
	if (!Sprite || !Texture)
	{
		return;
	}

	const FVector2D ExpectedSourceUV(static_cast<double>(SourceUV.X), static_cast<double>(SourceUV.Y));
	const FVector2D ExpectedSourceDimension(static_cast<double>(SourceDimension.X), static_cast<double>(SourceDimension.Y));
	FVector2D CurrentCustomPivot;
	const bool bSourceMatches = Sprite->GetSourceTexture() == Texture
		&& Sprite->GetSourceUV() == ExpectedSourceUV
		&& Sprite->GetSourceSize() == ExpectedSourceDimension;
	const bool bPivotMatches = Sprite->GetPivotMode(CurrentCustomPivot) == ESpritePivotMode::Bottom_Center;
	UMaterialInterface* MaskedMaterial = LoadGeneratedSpriteMaterial(TEXT("/Paper2D/MaskedUnlitSpriteMaterial.MaskedUnlitSpriteMaterial"));
	UMaterialInterface* OpaqueMaterial = LoadGeneratedSpriteMaterial(TEXT("/Paper2D/OpaqueUnlitSpriteMaterial.OpaqueUnlitSpriteMaterial"));
	const bool bDefaultMaterialMatches = !MaskedMaterial || Sprite->GetDefaultMaterial() == MaskedMaterial;
	const bool bAlternateMaterialMatches = !OpaqueMaterial || Sprite->GetAlternateMaterial() == OpaqueMaterial;
	if (bSourceMatches && bPivotMatches && bDefaultMaterialMatches && bAlternateMaterialMatches)
	{
		return;
	}

	FSpriteAssetInitParameters InitParams;
	InitParams.Texture = Texture;
	InitParams.Offset = SourceUV;
	InitParams.Dimension = SourceDimension;
	InitParams.DefaultMaterialOverride = MaskedMaterial;
	InitParams.AlternateMaterialOverride = OpaqueMaterial;

	Sprite->Modify();
	Sprite->InitializeSprite(InitParams, false);
	Sprite->SetPivotMode(ESpritePivotMode::Bottom_Center, FVector2D::ZeroVector);
	Sprite->PostEditChange();
	Sprite->MarkPackageDirty();
	OutPackagesToSave.AddUnique(Sprite->GetPackage());
}

static UPaperSprite* GetOrCreateGeneratedSprite(UTexture2D* Texture, const FString& AssetName, const FIntPoint& SourceUV, const FIntPoint& SourceDimension, TArray<UPackage*>& OutPackagesToSave)
{
	if (UPaperSprite* ExistingSprite = LoadGeneratedSpriteAsset<UPaperSprite>(AssetName))
	{
		UpdateGeneratedSpriteSource(ExistingSprite, Texture, SourceUV, SourceDimension, OutPackagesToSave);
		return ExistingSprite;
	}

	UPaperSpriteFactory* SpriteFactory = NewObject<UPaperSpriteFactory>();
	SpriteFactory->InitialTexture = Texture;
	SpriteFactory->bUseSourceRegion = true;
	SpriteFactory->InitialSourceUV = SourceUV;
	SpriteFactory->InitialSourceDimension = SourceDimension;

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	UPaperSprite* CreatedSprite = Cast<UPaperSprite>(AssetToolsModule.Get().CreateAsset(AssetName, GeneratedSpriteOutputPath, UPaperSprite::StaticClass(), SpriteFactory));
	if (CreatedSprite)
	{
		UpdateGeneratedSpriteSource(CreatedSprite, Texture, SourceUV, SourceDimension, OutPackagesToSave);
		OutPackagesToSave.AddUnique(CreatedSprite->GetPackage());
	}
	return CreatedSprite;
}

static UPaperFlipbook* GetOrCreateGeneratedFlipbook(const FString& AssetName, const TArray<UPaperSprite*>& Sprites, TArray<UPackage*>& OutPackagesToSave)
{
	UPaperFlipbook* Flipbook = LoadGeneratedSpriteAsset<UPaperFlipbook>(AssetName);
	if (!Flipbook)
	{
		UPaperFlipbookFactory* FlipbookFactory = NewObject<UPaperFlipbookFactory>();
		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
		Flipbook = Cast<UPaperFlipbook>(AssetToolsModule.Get().CreateAsset(AssetName, GeneratedSpriteOutputPath, UPaperFlipbook::StaticClass(), FlipbookFactory));
	}

	if (Flipbook)
	{
		Flipbook->Modify();
		FScopedFlipbookMutator FlipbookMutator(Flipbook);
		FlipbookMutator.FramesPerSecond = GeneratedSpriteFramesPerSecond;
		FlipbookMutator.KeyFrames.Reset();
		for (UPaperSprite* Sprite : Sprites)
		{
			if (!Sprite)
			{
				continue;
			}
			FPaperFlipbookKeyFrame& KeyFrame = FlipbookMutator.KeyFrames.AddDefaulted_GetRef();
			KeyFrame.Sprite = Sprite;
			KeyFrame.FrameRun = GeneratedSpriteFrameRun;
		}
		Flipbook->MarkPackageDirty();
		OutPackagesToSave.AddUnique(Flipbook->GetPackage());
	}

	return Flipbook;
}

static void NormalizeExistingGeneratedSprites(FAssetRegistryModule& AssetRegistryModule, TArray<UPackage*>& OutPackagesToSave)
{
	TArray<FAssetData> GeneratedSpriteAssets;
	AssetRegistryModule.Get().GetAssetsByPath(FName(GeneratedSpriteOutputPath), GeneratedSpriteAssets, false);
	for (const FAssetData& GeneratedSpriteAsset : GeneratedSpriteAssets)
	{
		UPaperSprite* Sprite = Cast<UPaperSprite>(GeneratedSpriteAsset.GetAsset());
		if (!Sprite || !Sprite->GetSourceTexture())
		{
			continue;
		}

		const FVector2D SourceUV = Sprite->GetSourceUV();
		const FVector2D SourceSize = Sprite->GetSourceSize();
		UpdateGeneratedSpriteSource(
			Sprite,
			Sprite->GetSourceTexture(),
			FIntPoint(FMath::RoundToInt(SourceUV.X), FMath::RoundToInt(SourceUV.Y)),
			FIntPoint(FMath::RoundToInt(SourceSize.X), FMath::RoundToInt(SourceSize.Y)),
			OutPackagesToSave);
	}
}

int32 GenerateSpriteFlipbooks()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	AssetRegistryModule.Get().SearchAllAssets(true);

	TArray<UPackage*> PackagesToSave;
	TArray<UTexture2D*> Textures = FREditor::ImportGeneratedSpriteTextures(PackagesToSave);
	TArray<FAssetData> TextureAssets;
	AssetRegistryModule.Get().ScanPathsSynchronous({ FREditor::GeneratedSpriteSourcePath }, true);
	AssetRegistryModule.Get().GetAssetsByPath(FName(FREditor::GeneratedSpriteSourcePath), TextureAssets, false);
	for (const FAssetData& TextureAsset : TextureAssets)
	{
		if (UTexture2D* Texture = Cast<UTexture2D>(TextureAsset.GetAsset()))
		{
			Textures.AddUnique(Texture);
		}
	}

	int32 GeneratedFlipbooks = 0;
	for (UTexture2D* Texture : Textures)
	{
		if (!Texture)
		{
			continue;
		}

		const FIntPoint TextureSize = Texture->GetImportedSize();
		const int32 FrameWidth = TextureSize.X / FREditor::GeneratedSpriteSheetColumns;
		const int32 FrameHeight = TextureSize.Y / FREditor::GeneratedSpriteSheetRows;
		if (FrameWidth <= 0 || FrameHeight <= 0)
		{
			continue;
		}

		const FString BaseName = FREditor::GetGeneratedSpriteBaseName(*Texture);
		TArray<UPaperSprite*> FrameSprites;
		for (int32 Row = 0; Row < FREditor::GeneratedSpriteSheetRows; ++Row)
		{
			for (int32 Column = 0; Column < FREditor::GeneratedSpriteSheetColumns; ++Column)
			{
				const int32 FrameIndex = Row * FREditor::GeneratedSpriteSheetColumns + Column;
				const FString SpriteName = FString::Printf(TEXT("%s_frame_%02d"), *BaseName, FrameIndex);
				const FIntPoint SourceUV(Column * FrameWidth, Row * FrameHeight);
				const FIntPoint SourceDimension(FrameWidth, FrameHeight);
				FrameSprites.Add(FREditor::GetOrCreateGeneratedSprite(Texture, SpriteName, SourceUV, SourceDimension, PackagesToSave));
			}
		}

		TArray<UPaperSprite*> MoveSprites;
		for (int32 Column = 0; Column < FREditor::GeneratedSpriteSheetColumns; ++Column)
		{
			MoveSprites.Add(FrameSprites[Column]);
		}
		if (FREditor::GetOrCreateGeneratedFlipbook(FString::Printf(TEXT("%s_flipbook"), *BaseName), MoveSprites, PackagesToSave))
		{
			++GeneratedFlipbooks;
		}

		for (const FGeneratedSpriteAnimationRow& AnimationRow : FREditor::GeneratedSpriteAnimationRows)
		{
			TArray<UPaperSprite*> AnimationSprites;
			for (int32 Column = 0; Column < FREditor::GeneratedSpriteSheetColumns; ++Column)
			{
				AnimationSprites.Add(FrameSprites[AnimationRow.RowIndex * FREditor::GeneratedSpriteSheetColumns + Column]);
			}

			const FString FlipbookName = FString::Printf(TEXT("%s%s"), *BaseName, AnimationRow.Suffix);
			if (FREditor::GetOrCreateGeneratedFlipbook(FlipbookName, AnimationSprites, PackagesToSave))
			{
				++GeneratedFlipbooks;
			}
		}
	}

	FREditor::NormalizeExistingGeneratedSprites(AssetRegistryModule, PackagesToSave);

	if (PackagesToSave.Num() > 0)
	{
		if (FApp::IsUnattended())
		{
			UEditorLoadingAndSavingUtils::SavePackages(PackagesToSave, true);
		}
		else
		{
			FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, false, false);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("FortRogue generated %d sprite flipbooks from %d textures."), GeneratedFlipbooks, Textures.Num());
	return GeneratedFlipbooks;
}
}

enum class EFRTerrainEditMode : int32
{
	PaintCircle,
	EraseCircle,
	FillRect,
	EraseRect,
	TextureCircle,
	TextureRect,
	PlayerSpawn,
	EnemySpawn
};

enum class EFREnemyPlacementCanvasAction : uint8
{
	Pressed,
	Dragged,
	Released
};

DECLARE_DELEGATE_RetVal_TwoParams(bool, FFREnemyPlacementCanvasActionDelegate, EFREnemyPlacementCanvasAction, const FIntPoint&);

class SFRTerrainMapCanvas : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SFRTerrainMapCanvas) {}
		SLATE_ATTRIBUTE(UFRTerrainMapDefinition*, Map)
		SLATE_ATTRIBUTE(int32, EditMode)
		SLATE_ATTRIBUTE(int32, BrushRadius)
		SLATE_ATTRIBUTE(int32, TextureLayer)
		SLATE_ATTRIBUTE(int32, CanvasCellPixels)
		SLATE_ATTRIBUTE(int32, SelectedEnemyPlacement)
		SLATE_EVENT(FSimpleDelegate, OnEdited)
		SLATE_EVENT(FFREnemyPlacementCanvasActionDelegate, OnEnemyPlacementCanvasAction)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		MapAttribute = InArgs._Map;
		EditModeAttribute = InArgs._EditMode;
		BrushRadiusAttribute = InArgs._BrushRadius;
		TextureLayerAttribute = InArgs._TextureLayer;
		CanvasCellPixelsAttribute = InArgs._CanvasCellPixels;
		SelectedEnemyPlacementAttribute = InArgs._SelectedEnemyPlacement;
		OnEdited = InArgs._OnEdited;
		OnEnemyPlacementCanvasAction = InArgs._OnEnemyPlacementCanvasAction;
	}

	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override
	{
		const UFRTerrainMapDefinition* Map = MapAttribute.Get();
		const int32 FixedCellPixels = GetFixedCellPixels();
		if (Map && FixedCellPixels > 0)
		{
			return FVector2D(Map->CellsX * FixedCellPixels, Map->CellsZ * FixedCellPixels);
		}

		return FVector2D(760.0f, 420.0f);
	}

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
	{
		const FVector2D Size = AllottedGeometry.GetLocalSize();
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), FCoreStyle::Get().GetBrush("WhiteBrush"), ESlateDrawEffect::None, FLinearColor(0.025f, 0.025f, 0.028f, 1.0f));

		const UFRTerrainMapDefinition* Map = MapAttribute.Get();
		if (!Map || Map->CellsX <= 0 || Map->CellsZ <= 0)
		{
			return LayerId + 1;
		}

		const float CellPixels = GetCellPixels(AllottedGeometry);
		const FVector2D Origin = GetCanvasOrigin(AllottedGeometry, CellPixels);
		for (int32 Z = 0; Z < Map->CellsZ; ++Z)
		{
			int32 X = 0;
			while (X < Map->CellsX)
			{
				const int32 Index = Map->ToIndex(X, Z);
				if (!Map->SolidMask.IsValidIndex(Index) || Map->SolidMask[Index] == 0)
				{
					++X;
					continue;
				}

				const uint8 Layer = Map->TextureLayerMask.IsValidIndex(Index) ? Map->TextureLayerMask[Index] : 0;
				const int32 RunStartX = X;
				++X;
				while (X < Map->CellsX)
				{
					const int32 RunIndex = Map->ToIndex(X, Z);
					const uint8 RunLayer = Map->TextureLayerMask.IsValidIndex(RunIndex) ? Map->TextureLayerMask[RunIndex] : 0;
					if (!Map->SolidMask.IsValidIndex(RunIndex) || Map->SolidMask[RunIndex] == 0 || RunLayer != Layer)
					{
						break;
					}
					++X;
				}

				const int32 RunLength = X - RunStartX;
				const FVector2D RunPosition = Origin + FVector2D(RunStartX * CellPixels, (Map->CellsZ - 1 - Z) * CellPixels);
				const FVector2D RunSize = GetCellDrawSize(CellPixels, RunLength, 1);
				const FPaintGeometry RunGeometry = AllottedGeometry.ToPaintGeometry(RunSize, FSlateLayoutTransform(RunPosition));
				FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 1, RunGeometry, FCoreStyle::Get().GetBrush("WhiteBrush"), ESlateDrawEffect::None, GetLayerColor(Layer));
			}
		}

		DrawSpawnMarker(AllottedGeometry, OutDrawElements, LayerId + 2, Origin, CellPixels, *Map, Map->PlayerSpawnLocal, FLinearColor(0.12f, 0.42f, 1.0f, 0.85f));
		if (Map->EnemyPlacements.Num() > 0)
		{
			const int32 SelectedEnemyPlacement = SelectedEnemyPlacementAttribute.Get();
			for (int32 PlacementIndex = 0; PlacementIndex < Map->EnemyPlacements.Num(); ++PlacementIndex)
			{
				const bool bSelected = PlacementIndex == SelectedEnemyPlacement;
				DrawSpawnMarker(AllottedGeometry, OutDrawElements, LayerId + 3 + PlacementIndex, Origin, CellPixels, *Map, Map->EnemyPlacements[PlacementIndex].SpawnLocal, bSelected ? FLinearColor(1.0f, 0.82f, 0.08f, 0.95f) : FLinearColor(1.0f, 0.18f, 0.12f, 0.85f), bSelected ? 4.0f : 2.0f);
			}
		}
		else
		{
			DrawSpawnMarker(AllottedGeometry, OutDrawElements, LayerId + 3, Origin, CellPixels, *Map, Map->EnemySpawnLocal, FLinearColor(1.0f, 0.18f, 0.12f, 0.85f));
		}

		if (bDragging && DragStart.X >= 0 && DragCurrent.X >= 0 && IsRectMode(GetEditMode()))
		{
			const int32 MinX = FMath::Min(DragStart.X, DragCurrent.X);
			const int32 MaxX = FMath::Max(DragStart.X, DragCurrent.X);
			const int32 MinZ = FMath::Min(DragStart.Y, DragCurrent.Y);
			const int32 MaxZ = FMath::Max(DragStart.Y, DragCurrent.Y);
			const FVector2D RectPosition = Origin + FVector2D(MinX * CellPixels, (Map->CellsZ - 1 - MaxZ) * CellPixels);
			const FVector2D RectSize = GetCellDrawSize(CellPixels, MaxX - MinX + 1, MaxZ - MinZ + 1);
			FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 4, AllottedGeometry.ToPaintGeometry(RectSize, FSlateLayoutTransform(RectPosition)), FCoreStyle::Get().GetBrush("WhiteBrush"), ESlateDrawEffect::None, FLinearColor(1.0f, 1.0f, 1.0f, 0.18f));
		}

		return LayerId + 5;
	}

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
		{
			return FReply::Unhandled();
		}

		FIntPoint Cell;
		if (!LocalPositionToCell(MyGeometry, MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()), Cell))
		{
			return FReply::Handled();
		}

		bDragging = true;
		DragStart = Cell;
		DragCurrent = Cell;
		if (IsEnemySpawnMode(GetEditMode()))
		{
			ApplyEnemyPlacementCanvasAction(EFREnemyPlacementCanvasAction::Pressed, Cell);
		}
		else if (!IsRectMode(GetEditMode()))
		{
			ApplyCircleAt(Cell);
		}

		return FReply::Handled().CaptureMouse(AsShared());
	}

	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (!bDragging)
		{
			return FReply::Unhandled();
		}

		FIntPoint Cell;
		if (LocalPositionToCell(MyGeometry, MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()), Cell))
		{
			const FIntPoint PreviousCell = DragCurrent;
			DragCurrent = Cell;
			if (IsEnemySpawnMode(GetEditMode()))
			{
				ApplyEnemyPlacementCanvasAction(EFREnemyPlacementCanvasAction::Dragged, Cell);
			}
			else if (IsStrokeMode(GetEditMode()) && PreviousCell.X >= 0)
			{
				ApplyCircleStroke(PreviousCell, Cell);
			}
			else if (!IsRectMode(GetEditMode()))
			{
				ApplyCircleAt(Cell);
			}
		}

		return FReply::Handled();
	}

	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton || !bDragging)
		{
			return FReply::Unhandled();
		}

		bDragging = false;
		if (IsEnemySpawnMode(GetEditMode()))
		{
			ApplyEnemyPlacementCanvasAction(EFREnemyPlacementCanvasAction::Released, DragCurrent);
		}
		else if (IsRectMode(GetEditMode()))
		{
			ApplyRect(DragStart, DragCurrent);
		}

		return FReply::Handled().ReleaseMouseCapture();
	}

private:
	int32 GetEditMode() const
	{
		return EditModeAttribute.Get();
	}

	static bool IsRectMode(int32 EditMode)
	{
		return EditMode == static_cast<int32>(EFRTerrainEditMode::FillRect)
			|| EditMode == static_cast<int32>(EFRTerrainEditMode::EraseRect)
			|| EditMode == static_cast<int32>(EFRTerrainEditMode::TextureRect);
	}

	static bool IsStrokeMode(int32 EditMode)
	{
		return EditMode == static_cast<int32>(EFRTerrainEditMode::PaintCircle)
			|| EditMode == static_cast<int32>(EFRTerrainEditMode::EraseCircle)
			|| EditMode == static_cast<int32>(EFRTerrainEditMode::TextureCircle);
	}

	static bool IsEnemySpawnMode(int32 EditMode)
	{
		return EditMode == static_cast<int32>(EFRTerrainEditMode::EnemySpawn);
	}

	float GetCellPixels(const FGeometry& Geometry) const
	{
		const UFRTerrainMapDefinition* Map = MapAttribute.Get();
		if (!Map || Map->CellsX <= 0 || Map->CellsZ <= 0)
		{
			return 1.0f;
		}

		const int32 FixedCellPixels = GetFixedCellPixels();
		if (FixedCellPixels > 0)
		{
			return static_cast<float>(FixedCellPixels);
		}

		const FVector2D Size = Geometry.GetLocalSize();
		return FMath::Max(0.05f, FMath::Min(Size.X / Map->CellsX, Size.Y / Map->CellsZ));
	}

	int32 GetFixedCellPixels() const
	{
		return FMath::Max(0, CanvasCellPixelsAttribute.Get());
	}

	static FVector2D GetCellDrawSize(float CellPixels, int32 CellCountX, int32 CellCountZ)
	{
		const float GapPixels = CellPixels >= 2.0f ? 1.0f : 0.0f;
		return FVector2D(
			FMath::Max(CellPixels, CellCountX * CellPixels - GapPixels),
			FMath::Max(CellPixels, CellCountZ * CellPixels - GapPixels));
	}

	FVector2D GetCanvasOrigin(const FGeometry& Geometry, float CellPixels) const
	{
		const UFRTerrainMapDefinition* Map = MapAttribute.Get();
		if (!Map)
		{
			return FVector2D::ZeroVector;
		}

		const FVector2D Size = Geometry.GetLocalSize();
		const FVector2D CanvasSize(Map->CellsX * CellPixels, Map->CellsZ * CellPixels);
		return (Size - CanvasSize) * 0.5f;
	}

	bool LocalPositionToCell(const FGeometry& Geometry, const FVector2D& LocalPosition, FIntPoint& OutCell) const
	{
		const UFRTerrainMapDefinition* Map = MapAttribute.Get();
		if (!Map)
		{
			return false;
		}

		const float CellPixels = GetCellPixels(Geometry);
		const FVector2D Origin = GetCanvasOrigin(Geometry, CellPixels);
		const FVector2D CanvasPosition = LocalPosition - Origin;
		const int32 X = FMath::FloorToInt(CanvasPosition.X / CellPixels);
		const int32 DisplayZ = FMath::FloorToInt(CanvasPosition.Y / CellPixels);
		const int32 Z = Map->CellsZ - 1 - DisplayZ;
		if (!Map->IsValidCell(X, Z))
		{
			return false;
		}

		OutCell = FIntPoint(X, Z);
		return true;
	}

	void ApplyCircleAt(const FIntPoint& Cell)
	{
		UFRTerrainMapDefinition* Map = MapAttribute.Get();
		if (!Map)
		{
			return;
		}

		Map->Modify();
		const int32 EditMode = GetEditMode();
		const int32 Radius = FMath::Max(0, BrushRadiusAttribute.Get());
		if (EditMode == static_cast<int32>(EFRTerrainEditMode::PaintCircle))
		{
			Map->ApplyTexturedCircle(Cell.X, Cell.Y, Radius, TextureLayerAttribute.Get());
		}
		else if (EditMode == static_cast<int32>(EFRTerrainEditMode::EraseCircle))
		{
			Map->ApplyCircle(Cell.X, Cell.Y, Radius, false);
		}
		else if (EditMode == static_cast<int32>(EFRTerrainEditMode::TextureCircle))
		{
			Map->ApplyTextureCircle(Cell.X, Cell.Y, Radius, TextureLayerAttribute.Get());
		}
		else if (EditMode == static_cast<int32>(EFRTerrainEditMode::PlayerSpawn))
		{
			SetSpawnAtCell(*Map, Cell, false);
		}
		else if (EditMode == static_cast<int32>(EFRTerrainEditMode::EnemySpawn))
		{
			SetSpawnAtCell(*Map, Cell, true);
		}

		Map->MarkPackageDirty();
		OnEdited.ExecuteIfBound();
	}

	void ApplyCircleStroke(const FIntPoint& StartCell, const FIntPoint& EndCell)
	{
		UFRTerrainMapDefinition* Map = MapAttribute.Get();
		if (!Map)
		{
			return;
		}

		Map->Modify();
		const int32 EditMode = GetEditMode();
		const int32 Radius = FMath::Max(0, BrushRadiusAttribute.Get());
		if (EditMode == static_cast<int32>(EFRTerrainEditMode::PaintCircle))
		{
			Map->ApplyTexturedCircleStroke(StartCell.X, StartCell.Y, EndCell.X, EndCell.Y, Radius, TextureLayerAttribute.Get());
		}
		else if (EditMode == static_cast<int32>(EFRTerrainEditMode::EraseCircle))
		{
			Map->ApplyCircleStroke(StartCell.X, StartCell.Y, EndCell.X, EndCell.Y, Radius, false);
		}
		else if (EditMode == static_cast<int32>(EFRTerrainEditMode::TextureCircle))
		{
			Map->ApplyTextureCircleStroke(StartCell.X, StartCell.Y, EndCell.X, EndCell.Y, Radius, TextureLayerAttribute.Get());
		}

		Map->MarkPackageDirty();
		OnEdited.ExecuteIfBound();
	}

	void ApplyRect(const FIntPoint& StartCell, const FIntPoint& EndCell)
	{
		UFRTerrainMapDefinition* Map = MapAttribute.Get();
		if (!Map)
		{
			return;
		}

		Map->Modify();
		const int32 EditMode = GetEditMode();
		if (EditMode == static_cast<int32>(EFRTerrainEditMode::FillRect))
		{
			Map->FillTexturedRect(StartCell.X, StartCell.Y, EndCell.X, EndCell.Y, TextureLayerAttribute.Get());
		}
		else if (EditMode == static_cast<int32>(EFRTerrainEditMode::EraseRect))
		{
			Map->FillRect(StartCell.X, StartCell.Y, EndCell.X, EndCell.Y, false);
		}
		else if (EditMode == static_cast<int32>(EFRTerrainEditMode::TextureRect))
		{
			Map->ApplyTextureRect(StartCell.X, StartCell.Y, EndCell.X, EndCell.Y, TextureLayerAttribute.Get());
		}

		Map->MarkPackageDirty();
		OnEdited.ExecuteIfBound();
	}

	void ApplyEnemyPlacementCanvasAction(EFREnemyPlacementCanvasAction Action, const FIntPoint& Cell)
	{
		if (OnEnemyPlacementCanvasAction.IsBound() && OnEnemyPlacementCanvasAction.Execute(Action, Cell))
		{
			OnEdited.ExecuteIfBound();
		}
	}

	static FLinearColor GetLayerColor(uint8 Layer)
	{
		static const FLinearColor Colors[] = {
			FLinearColor(0.43f, 0.34f, 0.21f, 1.0f),
			FLinearColor(0.25f, 0.42f, 0.28f, 1.0f),
			FLinearColor(0.45f, 0.42f, 0.38f, 1.0f),
			FLinearColor(0.52f, 0.31f, 0.24f, 1.0f),
			FLinearColor(0.24f, 0.36f, 0.50f, 1.0f),
			FLinearColor(0.58f, 0.50f, 0.22f, 1.0f)
		};

		return Colors[Layer % UE_ARRAY_COUNT(Colors)];
	}

	static void SetSpawnAtCell(UFRTerrainMapDefinition& Map, const FIntPoint& Cell, bool bEnemySpawn)
	{
		const FVector SpawnLocal = GetSpawnLocalForCell(Map, Cell);
		if (bEnemySpawn)
		{
			Map.EnemySpawnLocal = SpawnLocal;
			if (Map.EnemyPlacements.Num() == 0)
			{
				Map.EnemyPlacements.AddDefaulted();
			}
			Map.EnemyPlacements[0].SpawnLocal = Map.EnemySpawnLocal;
		}
		else
		{
			Map.PlayerSpawnLocal = SpawnLocal;
		}
	}

	static FVector GetSpawnLocalForCell(const UFRTerrainMapDefinition& Map, const FIntPoint& Cell)
	{
		const float LocalX = (static_cast<float>(Cell.X) + 0.5f) * Map.CellSize - Map.CellsX * Map.CellSize * 0.5f;
		const float LocalZ = Map.CellsZ * Map.CellSize + FREditor::DefaultSpawnClearance;
		return FVector(LocalX, 0.0f, LocalZ);
	}

	static void DrawSpawnMarker(const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FVector2D& Origin, float CellPixels, const UFRTerrainMapDefinition& Map, const FVector& SpawnLocal, const FLinearColor& Color, float MarkerWidth = 2.0f)
	{
		const float MapWidth = Map.CellsX * Map.CellSize;
		const float ClampedLocalX = FMath::Clamp(static_cast<float>(SpawnLocal.X), MapWidth * -0.5f, MapWidth * 0.5f);
		const float MarkerX = Origin.X + (ClampedLocalX + MapWidth * 0.5f) / FMath::Max(Map.CellSize, UE_SMALL_NUMBER) * CellPixels;
		const float CanvasHeight = Map.CellsZ * CellPixels;
		const FVector2D MarkerPosition(MarkerX - MarkerWidth * 0.5f, Origin.Y);
		const FVector2D MarkerSize(MarkerWidth, CanvasHeight);
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(MarkerSize, FSlateLayoutTransform(MarkerPosition)), FCoreStyle::Get().GetBrush("WhiteBrush"), ESlateDrawEffect::None, Color);
	}

	TAttribute<UFRTerrainMapDefinition*> MapAttribute;
	TAttribute<int32> EditModeAttribute;
	TAttribute<int32> BrushRadiusAttribute;
	TAttribute<int32> TextureLayerAttribute;
	TAttribute<int32> CanvasCellPixelsAttribute;
	TAttribute<int32> SelectedEnemyPlacementAttribute;
	FSimpleDelegate OnEdited;
	FFREnemyPlacementCanvasActionDelegate OnEnemyPlacementCanvasAction;
	mutable bool bDragging = false;
	mutable FIntPoint DragStart = FIntPoint(-1, -1);
	mutable FIntPoint DragCurrent = FIntPoint(-1, -1);
};

class SFRTerrainMapEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFRTerrainMapEditor) {}
		SLATE_ARGUMENT(UFRTerrainMapDefinition*, InitialAsset)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		SetEditingAsset(InArgs._InitialAsset);

		ChildSlot
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(8.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("AssetLabel", "Terrain Map Asset"))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(8.0f)
				[
					SNew(SObjectPropertyEntryBox)
					.AllowedClass(UFRTerrainMapDefinition::StaticClass())
					.ObjectPath(this, &SFRTerrainMapEditor::GetAssetPath)
					.OnObjectChanged(this, &SFRTerrainMapEditor::OnAssetChanged)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(8.0f)
				[
					MakeCanvasControls()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(8.0f)
				[
					MakeEnemyPlacementControls()
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.Padding(8.0f)
				[
					SNew(SScrollBox)
					.Orientation(Orient_Horizontal)
					+ SScrollBox::Slot()
					[
						SNew(SFRTerrainMapCanvas)
						.Map_Lambda([this]() { return EditingAsset.Get(); })
						.EditMode_Lambda([this]() { return EditMode; })
						.BrushRadius_Lambda([this]() { return CircleRadius; })
						.TextureLayer_Lambda([this]() { return TextureLayerIndex; })
						.CanvasCellPixels_Lambda([this]() { return CanvasCellPixels; })
						.SelectedEnemyPlacement_Lambda([this]() { return SelectedEnemyPlacementIndex; })
						.OnEdited(FSimpleDelegate::CreateSP(this, &SFRTerrainMapEditor::OnCanvasEdited))
						.OnEnemyPlacementCanvasAction(FFREnemyPlacementCanvasActionDelegate::CreateSP(this, &SFRTerrainMapEditor::HandleEnemyPlacementCanvasAction))
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(8.0f)
				[
					MakeResizeControls()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(8.0f)
				[
					MakeRectControls()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(8.0f)
				[
					MakeCircleControls()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(8.0f)
				[
					MakeTextureControls()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(8.0f)
				[
					MakeSaveControls()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(8.0f)
				[
					SNew(STextBlock)
					.Text(this, &SFRTerrainMapEditor::GetStatusText)
				]
			]
		];
	}

	void SetEditingAsset(UFRTerrainMapDefinition* Asset)
	{
		EditingAsset = Asset;
		if (EditingAsset.IsValid())
		{
			CellsX = EditingAsset->CellsX;
			CellsZ = EditingAsset->CellsZ;
			CellSize = EditingAsset->CellSize;
			StatusText = LOCTEXT("AssetLoaded", "Loaded terrain map asset.");
			SelectEnemyPlacement(EditingAsset->EnemyPlacements.Num() > 0 ? 0 : INDEX_NONE);
		}
		else
		{
			SelectedEnemyPlacementIndex = INDEX_NONE;
			StatusText = LOCTEXT("NoAsset", "No terrain map asset selected.");
		}
	}

private:
	TSharedRef<SWidget> MakeCanvasControls()
	{
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("CanvasLabel", "Canvas Drag Editing"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 6.0f, 0.0f)[MakeModeButton(LOCTEXT("ModePaint", "Paint"), static_cast<int32>(EFRTerrainEditMode::PaintCircle))]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 6.0f, 0.0f)[MakeModeButton(LOCTEXT("ModeErase", "Erase"), static_cast<int32>(EFRTerrainEditMode::EraseCircle))]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 6.0f, 0.0f)[MakeModeButton(LOCTEXT("ModeFillRect", "Fill Rect"), static_cast<int32>(EFRTerrainEditMode::FillRect))]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 6.0f, 0.0f)[MakeModeButton(LOCTEXT("ModeEraseRect", "Erase Rect"), static_cast<int32>(EFRTerrainEditMode::EraseRect))]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 6.0f, 0.0f)[MakeModeButton(LOCTEXT("ModeTexturePaint", "Texture Paint"), static_cast<int32>(EFRTerrainEditMode::TextureCircle))]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 6.0f, 0.0f)[MakeModeButton(LOCTEXT("ModeTextureRect", "Texture Rect"), static_cast<int32>(EFRTerrainEditMode::TextureRect))]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 6.0f, 0.0f)[MakeModeButton(LOCTEXT("ModePlayerSpawn", "Player Spawn"), static_cast<int32>(EFRTerrainEditMode::PlayerSpawn))]
				+ SHorizontalBox::Slot().AutoWidth()[MakeModeButton(LOCTEXT("ModeEnemySpawn", "Enemy Spawn"), static_cast<int32>(EFRTerrainEditMode::EnemySpawn))]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 6.0f, 0.0f)[MakeCanvasZoomButton(LOCTEXT("ZoomFit", "Fit"), 0)]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 6.0f, 0.0f)[MakeCanvasZoomButton(LOCTEXT("Zoom1x", "1x"), 1)]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 6.0f, 0.0f)[MakeCanvasZoomButton(LOCTEXT("Zoom2x", "2x"), 2)]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 6.0f, 0.0f)[MakeCanvasZoomButton(LOCTEXT("Zoom4x", "4x"), 4)]
				+ SHorizontalBox::Slot().AutoWidth()[MakeCanvasZoomButton(LOCTEXT("Zoom8x", "8x"), 8)]
			];
	}

	TSharedRef<SWidget> MakeModeButton(const FText& Label, int32 NewMode)
	{
		return SNew(SButton)
			.Text(Label)
			.ButtonColorAndOpacity_Lambda([this, NewMode]()
			{
				return EditMode == NewMode ? FLinearColor(0.30f, 0.48f, 0.75f, 1.0f) : FLinearColor::White;
			})
			.OnClicked_Lambda([this, NewMode]()
			{
				EditMode = NewMode;
				StatusText = LOCTEXT("ModeChanged", "Changed canvas edit mode.");
				return FReply::Handled();
			});
	}

	TSharedRef<SWidget> MakeCanvasZoomButton(const FText& Label, int32 NewCellPixels)
	{
		return SNew(SButton)
			.Text(Label)
			.ButtonColorAndOpacity_Lambda([this, NewCellPixels]()
			{
				return CanvasCellPixels == NewCellPixels ? FLinearColor(0.30f, 0.48f, 0.75f, 1.0f) : FLinearColor::White;
			})
			.OnClicked_Lambda([this, NewCellPixels]()
			{
				CanvasCellPixels = NewCellPixels;
				StatusText = LOCTEXT("ZoomChanged", "Changed canvas zoom.");
				return FReply::Handled();
			});
	}

	TSharedRef<SWidget> MakeResizeControls()
	{
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ResizeLabel", "Map Size"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					MakeIntField(LOCTEXT("CellsX", "Cells X"), CellsX)
				]
				+ SHorizontalBox::Slot()
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					MakeIntField(LOCTEXT("CellsZ", "Cells Z"), CellsZ)
				]
				+ SHorizontalBox::Slot()
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					MakePositiveFloatField(LOCTEXT("CellSize", "Cell Size"), CellSize)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("ResizeButton", "Resize"))
					.OnClicked(this, &SFRTerrainMapEditor::ResizeMap)
				]
			];
	}

	TSharedRef<SWidget> MakeRectControls()
	{
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("RectLabel", "Rectangle Terrain"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().Padding(0.0f, 0.0f, 8.0f, 0.0f)[MakeIntField(LOCTEXT("RectMinX", "Min X"), RectMinX)]
				+ SHorizontalBox::Slot().Padding(0.0f, 0.0f, 8.0f, 0.0f)[MakeIntField(LOCTEXT("RectMinZ", "Min Z"), RectMinZ)]
				+ SHorizontalBox::Slot().Padding(0.0f, 0.0f, 8.0f, 0.0f)[MakeIntField(LOCTEXT("RectMaxX", "Max X"), RectMaxX)]
				+ SHorizontalBox::Slot().Padding(0.0f, 0.0f, 8.0f, 0.0f)[MakeIntField(LOCTEXT("RectMaxZ", "Max Z"), RectMaxZ)]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.Padding(0.0f, 0.0f, 6.0f, 0.0f)
				[
					MakeIntField(LOCTEXT("ImportSourceX", "Source X"), ImportSourceX)
				]
				+ SHorizontalBox::Slot()
				.Padding(0.0f, 0.0f, 6.0f, 0.0f)
				[
					MakeIntField(LOCTEXT("ImportSourceY", "Source Y"), ImportSourceY)
				]
				+ SHorizontalBox::Slot()
				.Padding(0.0f, 0.0f, 6.0f, 0.0f)
				[
					MakeIntField(LOCTEXT("ImportSourceWidth", "Source W"), ImportSourceWidth)
				]
				+ SHorizontalBox::Slot()
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					MakeIntField(LOCTEXT("ImportSourceHeight", "Source H"), ImportSourceHeight)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("FillRectButton", "Fill Rect"))
					.OnClicked(this, &SFRTerrainMapEditor::FillRect)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("EraseRectButton", "Erase Rect"))
					.OnClicked(this, &SFRTerrainMapEditor::EraseRect)
				]
			];
	}

	TSharedRef<SWidget> MakeCircleControls()
	{
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("CircleLabel", "Circle Brush"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().Padding(0.0f, 0.0f, 8.0f, 0.0f)[MakeIntField(LOCTEXT("CircleX", "Center X"), CircleX)]
				+ SHorizontalBox::Slot().Padding(0.0f, 0.0f, 8.0f, 0.0f)[MakeIntField(LOCTEXT("CircleZ", "Center Z"), CircleZ)]
				+ SHorizontalBox::Slot().Padding(0.0f, 0.0f, 8.0f, 0.0f)[MakeIntField(LOCTEXT("CircleRadius", "Radius"), CircleRadius)]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("PaintCircleButton", "Paint Circle"))
					.OnClicked(this, &SFRTerrainMapEditor::PaintCircle)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("EraseCircleButton", "Erase Circle"))
					.OnClicked(this, &SFRTerrainMapEditor::EraseCircle)
				]
			];
	}

	TSharedRef<SWidget> MakeTextureControls()
	{
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("TextureLabel", "Texture Layer"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					MakeByteLayerField(LOCTEXT("TextureLayerIndex", "Layer"), TextureLayerIndex)
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SObjectPropertyEntryBox)
					.AllowedClass(UTexture2D::StaticClass())
					.ObjectPath(this, &SFRTerrainMapEditor::GetTexturePath)
					.OnObjectChanged(this, &SFRTerrainMapEditor::OnTextureChanged)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(8.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("SetTextureButton", "Set Layer Texture"))
					.OnClicked(this, &SFRTerrainMapEditor::SetLayerTexture)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(SCheckBox)
					.IsChecked_Lambda([this]() { return bImportMaskUsesAlpha ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
					.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) { bImportMaskUsesAlpha = NewState == ECheckBoxState::Checked; })
					[
						SNew(STextBlock)
						.Text(LOCTEXT("ImportUsesAlpha", "Use Alpha"))
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(SCheckBox)
					.IsChecked_Lambda([this]() { return bImportKeepCurrentSize ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
					.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) { bImportKeepCurrentSize = NewState == ECheckBoxState::Checked; })
					[
						SNew(STextBlock)
						.Text(LOCTEXT("ImportKeepSize", "Keep Size"))
					]
				]
				+ SHorizontalBox::Slot()
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					MakeFloatField(LOCTEXT("ImportThreshold", "Threshold"), ImportMaskThreshold)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("ImportMaskButton", "Import Mask"))
					.OnClicked(this, &SFRTerrainMapEditor::ImportMaskFromTexture)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.Padding(0.0f, 0.0f, 6.0f, 0.0f)
				[
					MakeFloatField(LOCTEXT("ImportColorR", "R"), ImportColorR)
				]
				+ SHorizontalBox::Slot()
				.Padding(0.0f, 0.0f, 6.0f, 0.0f)
				[
					MakeFloatField(LOCTEXT("ImportColorG", "G"), ImportColorG)
				]
				+ SHorizontalBox::Slot()
				.Padding(0.0f, 0.0f, 6.0f, 0.0f)
				[
					MakeFloatField(LOCTEXT("ImportColorB", "B"), ImportColorB)
				]
				+ SHorizontalBox::Slot()
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					MakeFloatField(LOCTEXT("ImportColorTolerance", "Tolerance"), ImportColorTolerance)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("ImportColorMaskButton", "Import Color Mask"))
					.OnClicked(this, &SFRTerrainMapEditor::ImportColorMaskFromTexture)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("TextureRectButton", "Apply To Rect"))
					.OnClicked(this, &SFRTerrainMapEditor::ApplyTextureRect)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("TextureCircleButton", "Apply To Circle"))
					.OnClicked(this, &SFRTerrainMapEditor::ApplyTextureCircle)
				]
			];
	}

	TSharedRef<SWidget> MakeEnemyPlacementControls()
	{
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("EnemyPlacementLabel", "Enemy Placement"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(SObjectPropertyEntryBox)
					.AllowedClass(UFRCharacterDefinition::StaticClass())
					.ObjectPath(this, &SFRTerrainMapEditor::GetEnemyCharacterPath)
					.OnObjectChanged(this, &SFRTerrainMapEditor::OnEnemyCharacterChanged)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(SCheckBox)
					.IsChecked_Lambda([this]() { return bEnemyPlacementUsesSpecialAttack ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
					.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) { bEnemyPlacementUsesSpecialAttack = NewState == ECheckBoxState::Checked; })
					[
						SNew(STextBlock)
						.Text(LOCTEXT("EnemyPlacementUseSpecial", "Use Special"))
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("ApplyEnemyPlacementButton", "Apply To Selected"))
					.IsEnabled_Lambda([this]() { return HasSelectedEnemyPlacement(); })
					.OnClicked(this, &SFRTerrainMapEditor::ApplySelectedEnemyPlacement)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("DeleteEnemyPlacementButton", "Delete Selected"))
					.IsEnabled_Lambda([this]() { return HasSelectedEnemyPlacement(); })
					.OnClicked(this, &SFRTerrainMapEditor::DeleteSelectedEnemyPlacement)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("ClearEnemySelectionButton", "Clear Selection"))
					.OnClicked(this, &SFRTerrainMapEditor::ClearEnemyPlacementSelection)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f)
			[
				SNew(STextBlock)
				.Text(this, &SFRTerrainMapEditor::GetSelectedEnemyPlacementText)
			];
	}

	TSharedRef<SWidget> MakeSaveControls()
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("ClearButton", "Clear"))
				.OnClicked(this, &SFRTerrainMapEditor::ClearMap)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("SaveButton", "Save Asset"))
				.OnClicked(this, &SFRTerrainMapEditor::SaveAsset)
			];
	}

	TSharedRef<SWidget> MakeIntField(const FText& Label, int32& Value)
	{
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(Label)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SNumericEntryBox<int32>)
				.MinValue(0)
				.Value_Lambda([&Value]() { return Value; })
				.OnValueChanged_Lambda([&Value](int32 NewValue) { Value = FMath::Max(0, NewValue); })
			];
	}

	TSharedRef<SWidget> MakeFloatField(const FText& Label, float& Value)
	{
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(Label)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SNumericEntryBox<float>)
				.MinValue(0.0f)
				.MaxValue(1.0f)
				.Value_Lambda([&Value]() { return Value; })
				.OnValueChanged_Lambda([&Value](float NewValue) { Value = FMath::Clamp(NewValue, 0.0f, 1.0f); })
			];
	}

	TSharedRef<SWidget> MakePositiveFloatField(const FText& Label, float& Value)
	{
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(Label)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SNumericEntryBox<float>)
				.MinValue(1.0f)
				.Value_Lambda([&Value]() { return Value; })
				.OnValueChanged_Lambda([&Value](float NewValue) { Value = FMath::Max(1.0f, NewValue); })
			];
	}

	TSharedRef<SWidget> MakeByteLayerField(const FText& Label, int32& Value)
	{
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(Label)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SNumericEntryBox<int32>)
				.MinValue(0)
				.MaxValue(255)
				.Value_Lambda([&Value]() { return Value; })
				.OnValueChanged_Lambda([&Value](int32 NewValue) { Value = FMath::Clamp(NewValue, 0, 255); })
			];
	}

	FString GetAssetPath() const
	{
		return EditingAsset.IsValid() ? EditingAsset->GetPathName() : FString();
	}

	FString GetTexturePath() const
	{
		return EditingTexture.IsValid() ? EditingTexture->GetPathName() : FString();
	}

	FString GetEnemyCharacterPath() const
	{
		return EditingEnemyCharacter.IsValid() ? EditingEnemyCharacter->GetPathName() : FString();
	}

	void OnAssetChanged(const FAssetData& AssetData)
	{
		SetEditingAsset(Cast<UFRTerrainMapDefinition>(AssetData.GetAsset()));
	}

	void OnTextureChanged(const FAssetData& AssetData)
	{
		EditingTexture = Cast<UTexture2D>(AssetData.GetAsset());
	}

	void OnEnemyCharacterChanged(const FAssetData& AssetData)
	{
		EditingEnemyCharacter = Cast<UFRCharacterDefinition>(AssetData.GetAsset());
	}

	FReply ResizeMap()
	{
		if (UFRTerrainMapDefinition* Map = GetEditableMap())
		{
			Map->ResizeResampled(CellsX, CellsZ);
			Map->SetCellSizePreservingSpawns(CellSize);
			CellSize = Map->CellSize;
			StatusText = LOCTEXT("Resized", "Updated terrain map size and preserved existing cells.");
		}
		return FReply::Handled();
	}

	FReply ClearMap()
	{
		if (UFRTerrainMapDefinition* Map = GetEditableMap())
		{
			Map->Clear(false);
			StatusText = LOCTEXT("Cleared", "Cleared terrain map.");
		}
		return FReply::Handled();
	}

	FReply FillRect()
	{
		ApplyRect(true);
		return FReply::Handled();
	}

	FReply EraseRect()
	{
		ApplyRect(false);
		return FReply::Handled();
	}

	FReply PaintCircle()
	{
		ApplyCircle(true);
		return FReply::Handled();
	}

	FReply EraseCircle()
	{
		ApplyCircle(false);
		return FReply::Handled();
	}

	FReply SaveAsset()
	{
		if (!EditingAsset.IsValid())
		{
			StatusText = LOCTEXT("SaveNoAsset", "Select a terrain map asset before saving.");
			return FReply::Handled();
		}

		TArray<UPackage*> PackagesToSave;
		PackagesToSave.Add(EditingAsset->GetPackage());
		const bool bSaved = FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, false, false) == FEditorFileUtils::EPromptReturnCode::PR_Success;
		StatusText = bSaved ? LOCTEXT("Saved", "Saved terrain map asset.") : LOCTEXT("SaveFailed", "Terrain map asset was not saved.");
		return FReply::Handled();
	}

	FReply SetLayerTexture()
	{
		if (UFRTerrainMapDefinition* Map = GetEditableMap())
		{
			Map->SetTextureLayer(TextureLayerIndex, EditingTexture.Get());
			StatusText = LOCTEXT("TextureSet", "Set texture layer.");
		}
		return FReply::Handled();
	}

	FReply ApplyTextureRect()
	{
		if (UFRTerrainMapDefinition* Map = GetEditableMap())
		{
			Map->ApplyTextureRect(RectMinX, RectMinZ, RectMaxX, RectMaxZ, TextureLayerIndex);
			StatusText = LOCTEXT("TextureRectApplied", "Applied texture layer to rectangle.");
		}
		return FReply::Handled();
	}

	FReply ApplyTextureCircle()
	{
		if (UFRTerrainMapDefinition* Map = GetEditableMap())
		{
			Map->ApplyTextureCircle(CircleX, CircleZ, CircleRadius, TextureLayerIndex);
			StatusText = LOCTEXT("TextureCircleApplied", "Applied texture layer to circle.");
		}
		return FReply::Handled();
	}

	FReply ImportMaskFromTexture()
	{
		if (!EditingTexture.IsValid())
		{
			StatusText = LOCTEXT("ImportMaskNoTexture", "Select a texture before importing a mask.");
			return FReply::Handled();
		}

		if (UFRTerrainMapDefinition* Map = GetEditableMap())
		{
			const bool bImported = Map->ImportSolidMaskFromTextureRegion(EditingTexture.Get(), ImportSourceX, ImportSourceY, ImportSourceWidth, ImportSourceHeight, bImportMaskUsesAlpha, ImportMaskThreshold, TextureLayerIndex, !bImportKeepCurrentSize);
			if (bImported)
			{
				Map->SetTextureLayer(TextureLayerIndex, EditingTexture.Get());
				CellsX = Map->CellsX;
				CellsZ = Map->CellsZ;
			}
			StatusText = bImported ? LOCTEXT("ImportMaskSucceeded", "Imported solid mask and assigned the texture layer.") : LOCTEXT("ImportMaskFailed", "Could not read texture pixels for mask import.");
		}

		return FReply::Handled();
	}

	FReply ImportColorMaskFromTexture()
	{
		if (!EditingTexture.IsValid())
		{
			StatusText = LOCTEXT("ImportColorMaskNoTexture", "Select a texture before importing a color mask.");
			return FReply::Handled();
		}

		if (UFRTerrainMapDefinition* Map = GetEditableMap())
		{
			const FLinearColor TargetColor(ImportColorR, ImportColorG, ImportColorB, 1.0f);
			const bool bImported = Map->ImportSolidMaskFromTextureRegionByColor(EditingTexture.Get(), ImportSourceX, ImportSourceY, ImportSourceWidth, ImportSourceHeight, TargetColor, ImportColorTolerance, TextureLayerIndex, !bImportKeepCurrentSize);
			if (bImported)
			{
				Map->SetTextureLayer(TextureLayerIndex, EditingTexture.Get());
				CellsX = Map->CellsX;
				CellsZ = Map->CellsZ;
			}
			StatusText = bImported ? LOCTEXT("ImportColorMaskSucceeded", "Imported color-matched solid mask and assigned the texture layer.") : LOCTEXT("ImportColorMaskFailed", "Could not read texture pixels for color mask import.");
		}

		return FReply::Handled();
	}

	FReply ApplySelectedEnemyPlacement()
	{
		if (UFRTerrainMapDefinition* Map = EditingAsset.Get())
		{
			if (Map->EnemyPlacements.IsValidIndex(SelectedEnemyPlacementIndex))
			{
				Map->Modify();
				FFREnemyPlacement& Placement = Map->EnemyPlacements[SelectedEnemyPlacementIndex];
				Placement.CharacterDefinition = EditingEnemyCharacter.Get();
				Placement.bUseSpecialAttack = bEnemyPlacementUsesSpecialAttack;
				Map->MarkPackageDirty();
				StatusText = LOCTEXT("EnemyPlacementApplied", "Updated selected enemy placement.");
			}
		}
		return FReply::Handled();
	}

	FReply DeleteSelectedEnemyPlacement()
	{
		if (UFRTerrainMapDefinition* Map = EditingAsset.Get())
		{
			if (Map->EnemyPlacements.IsValidIndex(SelectedEnemyPlacementIndex))
			{
				Map->Modify();
				Map->EnemyPlacements.RemoveAt(SelectedEnemyPlacementIndex);
				if (Map->EnemyPlacements.Num() > 0)
				{
					SelectEnemyPlacement(FMath::Min(SelectedEnemyPlacementIndex, Map->EnemyPlacements.Num() - 1));
					Map->EnemySpawnLocal = Map->EnemyPlacements[0].SpawnLocal;
				}
				else
				{
					SelectedEnemyPlacementIndex = INDEX_NONE;
				}
				Map->MarkPackageDirty();
				StatusText = LOCTEXT("EnemyPlacementDeleted", "Deleted selected enemy placement.");
			}
		}
		return FReply::Handled();
	}

	FReply ClearEnemyPlacementSelection()
	{
		SelectedEnemyPlacementIndex = INDEX_NONE;
		StatusText = LOCTEXT("EnemyPlacementSelectionCleared", "Cleared enemy placement selection.");
		return FReply::Handled();
	}

	FText GetStatusText() const
	{
		return StatusText;
	}

	FText GetSelectedEnemyPlacementText() const
	{
		const UFRTerrainMapDefinition* Map = EditingAsset.Get();
		if (!Map || !Map->EnemyPlacements.IsValidIndex(SelectedEnemyPlacementIndex))
		{
			const int32 PlacementCount = Map ? Map->EnemyPlacements.Num() : 0;
			return FText::Format(LOCTEXT("NoSelectedEnemyPlacement", "Selected Enemy: None ({0} placed)"), FText::AsNumber(PlacementCount));
		}

		const FFREnemyPlacement& Placement = Map->EnemyPlacements[SelectedEnemyPlacementIndex];
		const FString CharacterName = Placement.CharacterDefinition ? Placement.CharacterDefinition->GetName() : FString(TEXT("None"));
		return FText::Format(LOCTEXT("SelectedEnemyPlacement", "Selected Enemy: {0}/{1} {2}"), FText::AsNumber(SelectedEnemyPlacementIndex + 1), FText::AsNumber(Map->EnemyPlacements.Num()), FText::FromString(CharacterName));
	}

	UFRTerrainMapDefinition* GetEditableMap()
	{
		UFRTerrainMapDefinition* Map = EditingAsset.Get();
		if (!Map)
		{
			StatusText = LOCTEXT("EditNoAsset", "Select a terrain map asset first.");
			return nullptr;
		}

		Map->Modify();
		Map->MarkPackageDirty();
		return Map;
	}

	void ApplyRect(bool bSolid)
	{
		if (UFRTerrainMapDefinition* Map = GetEditableMap())
		{
			if (bSolid)
			{
				Map->FillTexturedRect(RectMinX, RectMinZ, RectMaxX, RectMaxZ, TextureLayerIndex);
			}
			else
			{
				Map->FillRect(RectMinX, RectMinZ, RectMaxX, RectMaxZ, false);
			}
			StatusText = bSolid ? LOCTEXT("RectFilled", "Filled rectangle terrain.") : LOCTEXT("RectErased", "Erased rectangle terrain.");
		}
	}

	void ApplyCircle(bool bSolid)
	{
		if (UFRTerrainMapDefinition* Map = GetEditableMap())
		{
			if (bSolid)
			{
				Map->ApplyTexturedCircle(CircleX, CircleZ, CircleRadius, TextureLayerIndex);
			}
			else
			{
				Map->ApplyCircle(CircleX, CircleZ, CircleRadius, false);
			}
			StatusText = bSolid ? LOCTEXT("CirclePainted", "Painted circle terrain.") : LOCTEXT("CircleErased", "Erased circle terrain.");
		}
	}

	void OnCanvasEdited()
	{
		if (EditMode != static_cast<int32>(EFRTerrainEditMode::EnemySpawn))
		{
			StatusText = LOCTEXT("CanvasEdited", "Edited terrain map from canvas.");
		}
	}

	bool HandleEnemyPlacementCanvasAction(EFREnemyPlacementCanvasAction Action, const FIntPoint& Cell)
	{
		UFRTerrainMapDefinition* Map = EditingAsset.Get();
		if (!Map)
		{
			StatusText = LOCTEXT("EnemyPlacementNoMap", "Select a terrain map asset first.");
			return false;
		}

		if (Action == EFREnemyPlacementCanvasAction::Pressed)
		{
			const int32 HitIndex = FindEnemyPlacementAtCell(*Map, Cell);
			if (HitIndex != INDEX_NONE)
			{
				SelectEnemyPlacement(HitIndex);
				StatusText = LOCTEXT("EnemyPlacementSelected", "Selected enemy placement.");
				return true;
			}

			if (!EditingEnemyCharacter.IsValid())
			{
				StatusText = LOCTEXT("EnemyPlacementNeedsCharacter", "Select a CharacterDefinition before placing an enemy.");
				return false;
			}

			Map->Modify();
			FFREnemyPlacement& Placement = Map->EnemyPlacements.AddDefaulted_GetRef();
			Placement.CharacterDefinition = EditingEnemyCharacter.Get();
			Placement.SpawnLocal = GetSpawnLocalForCell(*Map, Cell);
			Placement.bUseSpecialAttack = bEnemyPlacementUsesSpecialAttack;
			SelectedEnemyPlacementIndex = Map->EnemyPlacements.Num() - 1;
			Map->EnemySpawnLocal = Placement.SpawnLocal;
			Map->MarkPackageDirty();
			StatusText = LOCTEXT("EnemyPlacementAdded", "Added enemy placement.");
			return true;
		}

		if (Action == EFREnemyPlacementCanvasAction::Dragged)
		{
			return MoveSelectedEnemyPlacementToCell(*Map, Cell);
		}

		return false;
	}

	bool HasSelectedEnemyPlacement() const
	{
		const UFRTerrainMapDefinition* Map = EditingAsset.Get();
		return Map && Map->EnemyPlacements.IsValidIndex(SelectedEnemyPlacementIndex);
	}

	void SelectEnemyPlacement(int32 PlacementIndex)
	{
		UFRTerrainMapDefinition* Map = EditingAsset.Get();
		if (!Map || !Map->EnemyPlacements.IsValidIndex(PlacementIndex))
		{
			SelectedEnemyPlacementIndex = INDEX_NONE;
			return;
		}

		SelectedEnemyPlacementIndex = PlacementIndex;
		const FFREnemyPlacement& Placement = Map->EnemyPlacements[SelectedEnemyPlacementIndex];
		EditingEnemyCharacter = Placement.CharacterDefinition;
		bEnemyPlacementUsesSpecialAttack = Placement.bUseSpecialAttack;
	}

	bool MoveSelectedEnemyPlacementToCell(UFRTerrainMapDefinition& Map, const FIntPoint& Cell)
	{
		if (!Map.EnemyPlacements.IsValidIndex(SelectedEnemyPlacementIndex))
		{
			return false;
		}

		Map.Modify();
		const FVector SpawnLocal = GetSpawnLocalForCell(Map, Cell);
		Map.EnemyPlacements[SelectedEnemyPlacementIndex].SpawnLocal = SpawnLocal;
		if (SelectedEnemyPlacementIndex == 0)
		{
			Map.EnemySpawnLocal = SpawnLocal;
		}
		Map.MarkPackageDirty();
		StatusText = LOCTEXT("EnemyPlacementMoved", "Moved selected enemy placement.");
		return true;
	}

	int32 FindEnemyPlacementAtCell(const UFRTerrainMapDefinition& Map, const FIntPoint& Cell) const
	{
		if (Map.EnemyPlacements.Num() == 0)
		{
			return INDEX_NONE;
		}

		const FVector CellSpawnLocal = GetSpawnLocalForCell(Map, Cell);
		const float MaxHitDistance = FMath::Max(Map.CellSize * 10.0f, 10.0f);
		float BestDistance = MaxHitDistance;
		int32 BestIndex = INDEX_NONE;
		for (int32 PlacementIndex = 0; PlacementIndex < Map.EnemyPlacements.Num(); ++PlacementIndex)
		{
			const float Distance = FMath::Abs(static_cast<float>(Map.EnemyPlacements[PlacementIndex].SpawnLocal.X - CellSpawnLocal.X));
			if (Distance <= BestDistance)
			{
				BestDistance = Distance;
				BestIndex = PlacementIndex;
			}
		}

		return BestIndex;
	}

	static FVector GetSpawnLocalForCell(const UFRTerrainMapDefinition& Map, const FIntPoint& Cell)
	{
		const float LocalX = (static_cast<float>(Cell.X) + 0.5f) * Map.CellSize - Map.CellsX * Map.CellSize * 0.5f;
		const float LocalZ = Map.CellsZ * Map.CellSize + FREditor::DefaultSpawnClearance;
		return FVector(LocalX, 0.0f, LocalZ);
	}

	TWeakObjectPtr<UFRTerrainMapDefinition> EditingAsset;
	TWeakObjectPtr<UTexture2D> EditingTexture;
	TWeakObjectPtr<UFRCharacterDefinition> EditingEnemyCharacter;
	FText StatusText = LOCTEXT("InitialStatus", "Select a terrain map asset.");
	int32 EditMode = static_cast<int32>(EFRTerrainEditMode::PaintCircle);
	int32 SelectedEnemyPlacementIndex = INDEX_NONE;
	int32 CellsX = 1280;
	int32 CellsZ = 960;
	float CellSize = 1.0f;
	int32 RectMinX = 0;
	int32 RectMinZ = 0;
	int32 RectMaxX = 32;
	int32 RectMaxZ = 16;
	int32 CircleX = 160;
	int32 CircleZ = 120;
	int32 CircleRadius = 8;
	int32 TextureLayerIndex = 0;
	int32 CanvasCellPixels = 0;
	int32 ImportSourceX = 0;
	int32 ImportSourceY = 0;
	int32 ImportSourceWidth = 0;
	int32 ImportSourceHeight = 0;
	bool bImportMaskUsesAlpha = true;
	bool bImportKeepCurrentSize = true;
	bool bEnemyPlacementUsesSpecialAttack = false;
	float ImportMaskThreshold = 0.5f;
	float ImportColorR = 0.43f;
	float ImportColorG = 0.34f;
	float ImportColorB = 0.21f;
	float ImportColorTolerance = 0.08f;
};

namespace FREditor
{
static void OpenTerrainMapEditorForAsset(UFRTerrainMapDefinition* Asset)
{
	PendingTerrainMapAsset = Asset;
	if (TSharedPtr<SFRTerrainMapEditor> Editor = ActiveTerrainMapEditor.Pin())
	{
		Editor->SetEditingAsset(Asset);
	}

	FGlobalTabmanager::Get()->TryInvokeTab(TerrainMapEditorTabName);
}
}

class FFRTerrainMapAssetTypeActions : public FAssetTypeActions_Base
{
public:
	virtual FText GetName() const override
	{
		return LOCTEXT("TerrainMapAssetTypeName", "FortRogue Terrain Map Definition");
	}

	virtual FColor GetTypeColor() const override
	{
		return FColor(110, 87, 54);
	}

	virtual UClass* GetSupportedClass() const override
	{
		return UFRTerrainMapDefinition::StaticClass();
	}

	virtual uint32 GetCategories() override
	{
		return EAssetTypeCategories::Misc;
	}

	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override
	{
		for (UObject* Object : InObjects)
		{
			if (UFRTerrainMapDefinition* Map = Cast<UFRTerrainMapDefinition>(Object))
			{
				FREditor::OpenTerrainMapEditorForAsset(Map);
				return;
			}
		}
	}
};

class FFREditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FREditor::TerrainMapEditorTabName, FOnSpawnTab::CreateRaw(this, &FFREditorModule::SpawnTerrainMapEditorTab))
			.SetDisplayName(LOCTEXT("TerrainMapEditorTabTitle", "FortRogue Terrain Map Editor"))
			.SetMenuType(ETabSpawnerMenuType::Hidden);

		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FFREditorModule::RegisterMenus));

		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
		TerrainMapAssetTypeActions = MakeShared<FFRTerrainMapAssetTypeActions>();
		AssetToolsModule.Get().RegisterAssetTypeActions(TerrainMapAssetTypeActions.ToSharedRef());

		FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyEditorModule.RegisterCustomPropertyTypeLayout(FREditor::ProjectileEffectSpecTypeName, FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FFRProjectileEffectSpecCustomization::MakeInstance));
		PropertyEditorModule.NotifyCustomizationModuleChanged();
	}

	virtual void ShutdownModule() override
	{
		if (TerrainMapAssetTypeActions.IsValid() && FModuleManager::Get().IsModuleLoaded("AssetTools"))
		{
			FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
			AssetToolsModule.Get().UnregisterAssetTypeActions(TerrainMapAssetTypeActions.ToSharedRef());
		}
		TerrainMapAssetTypeActions.Reset();
		FREditor::ActiveTerrainMapEditor.Reset();
		FREditor::PendingTerrainMapAsset.Reset();

		if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
		{
			FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
			PropertyEditorModule.UnregisterCustomPropertyTypeLayout(FREditor::ProjectileEffectSpecTypeName);
			PropertyEditorModule.NotifyCustomizationModuleChanged();
		}

		if (UObjectInitialized())
		{
			UToolMenus::UnRegisterStartupCallback(this);
			UToolMenus::UnregisterOwner(this);
		}

		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FREditor::TerrainMapEditorTabName);
	}

private:
	TSharedRef<SDockTab> SpawnTerrainMapEditorTab(const FSpawnTabArgs& Args)
	{
		TSharedRef<SFRTerrainMapEditor> Editor = SNew(SFRTerrainMapEditor)
			.InitialAsset(FREditor::PendingTerrainMapAsset.Get());
		FREditor::ActiveTerrainMapEditor = Editor;

		return SNew(SDockTab)
			.TabRole(ETabRole::NomadTab)
			[
				Editor
			];
	}

	void RegisterMenus()
	{
		FToolMenuOwnerScoped OwnerScoped(this);
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
		FToolMenuSection& Section = Menu->FindOrAddSection("FortRogue");
		Section.AddMenuEntry(
			"OpenFortRogueTerrainMapEditor",
			LOCTEXT("OpenTerrainMapEditor", "FortRogue Terrain Map Editor"),
			LOCTEXT("OpenTerrainMapEditorTooltip", "Open the FortRogue terrain map editor."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &FFREditorModule::OpenTerrainMapEditor)));
		Section.AddMenuEntry(
			"GenerateFortRogueSpriteFlipbooks",
			LOCTEXT("GenerateSpriteFlipbooks", "Generate FortRogue Sprite Flipbooks"),
			LOCTEXT("GenerateSpriteFlipbooksTooltip", "Create Paper2D sprites and flipbooks from textures in /Game/GeneratedSprites."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &FFREditorModule::GenerateSpriteFlipbooks)));
	}

	void OpenTerrainMapEditor()
	{
		FGlobalTabmanager::Get()->TryInvokeTab(FREditor::TerrainMapEditorTabName);
	}

	void GenerateSpriteFlipbooks()
	{
		FREditor::GenerateSpriteFlipbooks();
	}
	TSharedPtr<IAssetTypeActions> TerrainMapAssetTypeActions;
};

IMPLEMENT_MODULE(FFREditorModule, FortRogueEditor)

#undef LOCTEXT_NAMESPACE
