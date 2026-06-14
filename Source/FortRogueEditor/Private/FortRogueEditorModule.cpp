// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/FortRogueTerrainMapDefinition.h"
#include "Editor.h"
#include "Engine/Texture2D.h"
#include "FileHelpers.h"
#include "Framework/Docking/TabManager.h"
#include "Modules/ModuleManager.h"
#include "PropertyCustomizationHelpers.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FortRogueEditor"

namespace FortRogueEditor
{
static const FName TerrainMapEditorTabName(TEXT("FortRogueTerrainMapEditor"));
}

enum class EFortRogueTerrainEditMode : int32
{
	PaintCircle,
	EraseCircle,
	FillRect,
	EraseRect,
	TextureCircle,
	TextureRect
};

class SFortRogueTerrainMapCanvas : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SFortRogueTerrainMapCanvas) {}
		SLATE_ATTRIBUTE(UFortRogueTerrainMapDefinition*, Map)
		SLATE_ATTRIBUTE(int32, EditMode)
		SLATE_ATTRIBUTE(int32, BrushRadius)
		SLATE_ATTRIBUTE(int32, TextureLayer)
		SLATE_EVENT(FSimpleDelegate, OnEdited)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		MapAttribute = InArgs._Map;
		EditModeAttribute = InArgs._EditMode;
		BrushRadiusAttribute = InArgs._BrushRadius;
		TextureLayerAttribute = InArgs._TextureLayer;
		OnEdited = InArgs._OnEdited;
	}

	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override
	{
		return FVector2D(760.0f, 420.0f);
	}

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
	{
		const FVector2D Size = AllottedGeometry.GetLocalSize();
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), FCoreStyle::Get().GetBrush("WhiteBrush"), ESlateDrawEffect::None, FLinearColor(0.025f, 0.025f, 0.028f, 1.0f));

		const UFortRogueTerrainMapDefinition* Map = MapAttribute.Get();
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
				const FVector2D RunSize(FMath::Max(1.0f, RunLength * CellPixels - 1.0f), FMath::Max(1.0f, CellPixels - 1.0f));
				const FPaintGeometry RunGeometry = AllottedGeometry.ToPaintGeometry(RunSize, FSlateLayoutTransform(RunPosition));
				FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 1, RunGeometry, FCoreStyle::Get().GetBrush("WhiteBrush"), ESlateDrawEffect::None, GetLayerColor(Layer));
			}
		}

		if (bDragging && DragStart.X >= 0 && DragCurrent.X >= 0 && IsRectMode(GetEditMode()))
		{
			const int32 MinX = FMath::Min(DragStart.X, DragCurrent.X);
			const int32 MaxX = FMath::Max(DragStart.X, DragCurrent.X);
			const int32 MinZ = FMath::Min(DragStart.Y, DragCurrent.Y);
			const int32 MaxZ = FMath::Max(DragStart.Y, DragCurrent.Y);
			const FVector2D RectPosition = Origin + FVector2D(MinX * CellPixels, (Map->CellsZ - 1 - MaxZ) * CellPixels);
			const FVector2D RectSize((MaxX - MinX + 1) * CellPixels, (MaxZ - MinZ + 1) * CellPixels);
			FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 2, AllottedGeometry.ToPaintGeometry(RectSize, FSlateLayoutTransform(RectPosition)), FCoreStyle::Get().GetBrush("WhiteBrush"), ESlateDrawEffect::None, FLinearColor(1.0f, 1.0f, 1.0f, 0.18f));
		}

		return LayerId + 3;
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
		if (!IsRectMode(GetEditMode()))
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
			DragCurrent = Cell;
			if (!IsRectMode(GetEditMode()))
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
		if (IsRectMode(GetEditMode()))
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
		return EditMode == static_cast<int32>(EFortRogueTerrainEditMode::FillRect)
			|| EditMode == static_cast<int32>(EFortRogueTerrainEditMode::EraseRect)
			|| EditMode == static_cast<int32>(EFortRogueTerrainEditMode::TextureRect);
	}

	float GetCellPixels(const FGeometry& Geometry) const
	{
		const UFortRogueTerrainMapDefinition* Map = MapAttribute.Get();
		if (!Map || Map->CellsX <= 0 || Map->CellsZ <= 0)
		{
			return 1.0f;
		}

		const FVector2D Size = Geometry.GetLocalSize();
		return FMath::Max(1.0f, FMath::Min(Size.X / Map->CellsX, Size.Y / Map->CellsZ));
	}

	FVector2D GetCanvasOrigin(const FGeometry& Geometry, float CellPixels) const
	{
		const UFortRogueTerrainMapDefinition* Map = MapAttribute.Get();
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
		const UFortRogueTerrainMapDefinition* Map = MapAttribute.Get();
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
		UFortRogueTerrainMapDefinition* Map = MapAttribute.Get();
		if (!Map)
		{
			return;
		}

		Map->Modify();
		const int32 EditMode = GetEditMode();
		const int32 Radius = FMath::Max(0, BrushRadiusAttribute.Get());
		if (EditMode == static_cast<int32>(EFortRogueTerrainEditMode::PaintCircle))
		{
			Map->ApplyCircle(Cell.X, Cell.Y, Radius, true);
		}
		else if (EditMode == static_cast<int32>(EFortRogueTerrainEditMode::EraseCircle))
		{
			Map->ApplyCircle(Cell.X, Cell.Y, Radius, false);
		}
		else if (EditMode == static_cast<int32>(EFortRogueTerrainEditMode::TextureCircle))
		{
			Map->ApplyTextureCircle(Cell.X, Cell.Y, Radius, TextureLayerAttribute.Get());
		}

		Map->MarkPackageDirty();
		OnEdited.ExecuteIfBound();
	}

	void ApplyRect(const FIntPoint& StartCell, const FIntPoint& EndCell)
	{
		UFortRogueTerrainMapDefinition* Map = MapAttribute.Get();
		if (!Map)
		{
			return;
		}

		Map->Modify();
		const int32 EditMode = GetEditMode();
		if (EditMode == static_cast<int32>(EFortRogueTerrainEditMode::FillRect))
		{
			Map->FillRect(StartCell.X, StartCell.Y, EndCell.X, EndCell.Y, true);
		}
		else if (EditMode == static_cast<int32>(EFortRogueTerrainEditMode::EraseRect))
		{
			Map->FillRect(StartCell.X, StartCell.Y, EndCell.X, EndCell.Y, false);
		}
		else if (EditMode == static_cast<int32>(EFortRogueTerrainEditMode::TextureRect))
		{
			Map->ApplyTextureRect(StartCell.X, StartCell.Y, EndCell.X, EndCell.Y, TextureLayerAttribute.Get());
		}

		Map->MarkPackageDirty();
		OnEdited.ExecuteIfBound();
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

	TAttribute<UFortRogueTerrainMapDefinition*> MapAttribute;
	TAttribute<int32> EditModeAttribute;
	TAttribute<int32> BrushRadiusAttribute;
	TAttribute<int32> TextureLayerAttribute;
	FSimpleDelegate OnEdited;
	mutable bool bDragging = false;
	mutable FIntPoint DragStart = FIntPoint(-1, -1);
	mutable FIntPoint DragCurrent = FIntPoint(-1, -1);
};

class SFortRogueTerrainMapEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFortRogueTerrainMapEditor) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
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
					.AllowedClass(UFortRogueTerrainMapDefinition::StaticClass())
					.ObjectPath(this, &SFortRogueTerrainMapEditor::GetAssetPath)
					.OnObjectChanged(this, &SFortRogueTerrainMapEditor::OnAssetChanged)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(8.0f)
				[
					MakeCanvasControls()
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.Padding(8.0f)
				[
					SNew(SFortRogueTerrainMapCanvas)
					.Map_Lambda([this]() { return EditingAsset.Get(); })
					.EditMode_Lambda([this]() { return EditMode; })
					.BrushRadius_Lambda([this]() { return CircleRadius; })
					.TextureLayer_Lambda([this]() { return TextureLayerIndex; })
					.OnEdited(FSimpleDelegate::CreateSP(this, &SFortRogueTerrainMapEditor::OnCanvasEdited))
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
					.Text(this, &SFortRogueTerrainMapEditor::GetStatusText)
				]
			]
		];
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
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 6.0f, 0.0f)[MakeModeButton(LOCTEXT("ModePaint", "Paint"), static_cast<int32>(EFortRogueTerrainEditMode::PaintCircle))]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 6.0f, 0.0f)[MakeModeButton(LOCTEXT("ModeErase", "Erase"), static_cast<int32>(EFortRogueTerrainEditMode::EraseCircle))]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 6.0f, 0.0f)[MakeModeButton(LOCTEXT("ModeFillRect", "Fill Rect"), static_cast<int32>(EFortRogueTerrainEditMode::FillRect))]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 6.0f, 0.0f)[MakeModeButton(LOCTEXT("ModeEraseRect", "Erase Rect"), static_cast<int32>(EFortRogueTerrainEditMode::EraseRect))]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 6.0f, 0.0f)[MakeModeButton(LOCTEXT("ModeTexturePaint", "Texture Paint"), static_cast<int32>(EFortRogueTerrainEditMode::TextureCircle))]
				+ SHorizontalBox::Slot().AutoWidth()[MakeModeButton(LOCTEXT("ModeTextureRect", "Texture Rect"), static_cast<int32>(EFortRogueTerrainEditMode::TextureRect))]
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
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("ResizeButton", "Resize"))
					.OnClicked(this, &SFortRogueTerrainMapEditor::ResizeMap)
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
					.OnClicked(this, &SFortRogueTerrainMapEditor::FillRect)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("EraseRectButton", "Erase Rect"))
					.OnClicked(this, &SFortRogueTerrainMapEditor::EraseRect)
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
					.OnClicked(this, &SFortRogueTerrainMapEditor::PaintCircle)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("EraseCircleButton", "Erase Circle"))
					.OnClicked(this, &SFortRogueTerrainMapEditor::EraseCircle)
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
					MakeIntField(LOCTEXT("TextureLayerIndex", "Layer"), TextureLayerIndex)
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SObjectPropertyEntryBox)
					.AllowedClass(UTexture2D::StaticClass())
					.ObjectPath(this, &SFortRogueTerrainMapEditor::GetTexturePath)
					.OnObjectChanged(this, &SFortRogueTerrainMapEditor::OnTextureChanged)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(8.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("SetTextureButton", "Set Layer Texture"))
					.OnClicked(this, &SFortRogueTerrainMapEditor::SetLayerTexture)
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
					.OnClicked(this, &SFortRogueTerrainMapEditor::ImportMaskFromTexture)
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
					.OnClicked(this, &SFortRogueTerrainMapEditor::ImportColorMaskFromTexture)
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
					.OnClicked(this, &SFortRogueTerrainMapEditor::ApplyTextureRect)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("TextureCircleButton", "Apply To Circle"))
					.OnClicked(this, &SFortRogueTerrainMapEditor::ApplyTextureCircle)
				]
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
				.OnClicked(this, &SFortRogueTerrainMapEditor::ClearMap)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("SaveButton", "Save Asset"))
				.OnClicked(this, &SFortRogueTerrainMapEditor::SaveAsset)
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

	FString GetAssetPath() const
	{
		return EditingAsset.IsValid() ? EditingAsset->GetPathName() : FString();
	}

	FString GetTexturePath() const
	{
		return EditingTexture.IsValid() ? EditingTexture->GetPathName() : FString();
	}

	void OnAssetChanged(const FAssetData& AssetData)
	{
		EditingAsset = Cast<UFortRogueTerrainMapDefinition>(AssetData.GetAsset());
		if (EditingAsset.IsValid())
		{
			CellsX = EditingAsset->CellsX;
			CellsZ = EditingAsset->CellsZ;
			StatusText = LOCTEXT("AssetLoaded", "Loaded terrain map asset.");
		}
		else
		{
			StatusText = LOCTEXT("NoAsset", "No terrain map asset selected.");
		}
	}

	void OnTextureChanged(const FAssetData& AssetData)
	{
		EditingTexture = Cast<UTexture2D>(AssetData.GetAsset());
	}

	FReply ResizeMap()
	{
		if (UFortRogueTerrainMapDefinition* Map = GetEditableMap())
		{
			Map->Resize(CellsX, CellsZ);
			StatusText = LOCTEXT("Resized", "Resized terrain map.");
		}
		return FReply::Handled();
	}

	FReply ClearMap()
	{
		if (UFortRogueTerrainMapDefinition* Map = GetEditableMap())
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
		if (UFortRogueTerrainMapDefinition* Map = GetEditableMap())
		{
			Map->SetTextureLayer(TextureLayerIndex, EditingTexture.Get());
			StatusText = LOCTEXT("TextureSet", "Set texture layer.");
		}
		return FReply::Handled();
	}

	FReply ApplyTextureRect()
	{
		if (UFortRogueTerrainMapDefinition* Map = GetEditableMap())
		{
			Map->ApplyTextureRect(RectMinX, RectMinZ, RectMaxX, RectMaxZ, TextureLayerIndex);
			StatusText = LOCTEXT("TextureRectApplied", "Applied texture layer to rectangle.");
		}
		return FReply::Handled();
	}

	FReply ApplyTextureCircle()
	{
		if (UFortRogueTerrainMapDefinition* Map = GetEditableMap())
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

		if (UFortRogueTerrainMapDefinition* Map = GetEditableMap())
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

		if (UFortRogueTerrainMapDefinition* Map = GetEditableMap())
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

	FText GetStatusText() const
	{
		return StatusText;
	}

	UFortRogueTerrainMapDefinition* GetEditableMap()
	{
		UFortRogueTerrainMapDefinition* Map = EditingAsset.Get();
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
		if (UFortRogueTerrainMapDefinition* Map = GetEditableMap())
		{
			Map->FillRect(RectMinX, RectMinZ, RectMaxX, RectMaxZ, bSolid);
			StatusText = bSolid ? LOCTEXT("RectFilled", "Filled rectangle terrain.") : LOCTEXT("RectErased", "Erased rectangle terrain.");
		}
	}

	void ApplyCircle(bool bSolid)
	{
		if (UFortRogueTerrainMapDefinition* Map = GetEditableMap())
		{
			Map->ApplyCircle(CircleX, CircleZ, CircleRadius, bSolid);
			StatusText = bSolid ? LOCTEXT("CirclePainted", "Painted circle terrain.") : LOCTEXT("CircleErased", "Erased circle terrain.");
		}
	}

	void OnCanvasEdited()
	{
		StatusText = LOCTEXT("CanvasEdited", "Edited terrain map from canvas.");
	}

	TWeakObjectPtr<UFortRogueTerrainMapDefinition> EditingAsset;
	TWeakObjectPtr<UTexture2D> EditingTexture;
	FText StatusText = LOCTEXT("InitialStatus", "Select a terrain map asset.");
	int32 EditMode = static_cast<int32>(EFortRogueTerrainEditMode::PaintCircle);
	int32 CellsX = 1280;
	int32 CellsZ = 960;
	int32 RectMinX = 0;
	int32 RectMinZ = 0;
	int32 RectMaxX = 32;
	int32 RectMaxZ = 16;
	int32 CircleX = 160;
	int32 CircleZ = 120;
	int32 CircleRadius = 8;
	int32 TextureLayerIndex = 0;
	int32 ImportSourceX = 0;
	int32 ImportSourceY = 0;
	int32 ImportSourceWidth = 0;
	int32 ImportSourceHeight = 0;
	bool bImportMaskUsesAlpha = true;
	bool bImportKeepCurrentSize = true;
	float ImportMaskThreshold = 0.5f;
	float ImportColorR = 0.43f;
	float ImportColorG = 0.34f;
	float ImportColorB = 0.21f;
	float ImportColorTolerance = 0.08f;
};

class FFortRogueEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FortRogueEditor::TerrainMapEditorTabName, FOnSpawnTab::CreateRaw(this, &FFortRogueEditorModule::SpawnTerrainMapEditorTab))
			.SetDisplayName(LOCTEXT("TerrainMapEditorTabTitle", "FortRogue Terrain Map Editor"))
			.SetMenuType(ETabSpawnerMenuType::Hidden);

		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FFortRogueEditorModule::RegisterMenus));
	}

	virtual void ShutdownModule() override
	{
		if (UObjectInitialized())
		{
			UToolMenus::UnRegisterStartupCallback(this);
			UToolMenus::UnregisterOwner(this);
		}

		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FortRogueEditor::TerrainMapEditorTabName);
	}

private:
	TSharedRef<SDockTab> SpawnTerrainMapEditorTab(const FSpawnTabArgs& Args)
	{
		return SNew(SDockTab)
			.TabRole(ETabRole::NomadTab)
			[
				SNew(SFortRogueTerrainMapEditor)
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
			FUIAction(FExecuteAction::CreateRaw(this, &FFortRogueEditorModule::OpenTerrainMapEditor)));
	}

	void OpenTerrainMapEditor()
	{
		FGlobalTabmanager::Get()->TryInvokeTab(FortRogueEditor::TerrainMapEditorTabName);
	}
};

IMPLEMENT_MODULE(FFortRogueEditorModule, FortRogueEditor)

#undef LOCTEXT_NAMESPACE
