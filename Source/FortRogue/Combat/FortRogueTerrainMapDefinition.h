// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FortRogueTerrainMapDefinition.generated.h"

class UTexture2D;

USTRUCT(BlueprintType)
struct FFortRogueTerrainTextureLayer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain")
	FName LayerId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain")
	TObjectPtr<UTexture2D> Texture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain")
	FLinearColor FallbackColor = FLinearColor(0.43f, 0.34f, 0.21f, 1.0f);
};

UCLASS(BlueprintType)
class FORTROGUE_API UFortRogueTerrainMapDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFortRogueTerrainMapDefinition();

	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Terrain")
	void Resize(int32 NewCellsX, int32 NewCellsZ);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Terrain")
	void NormalizeMapData();

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Terrain")
	void ResizeResampled(int32 NewCellsX, int32 NewCellsZ);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Terrain")
	void SetCellSizePreservingSpawns(float NewCellSize);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Terrain")
	void Clear(bool bSolid);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Terrain")
	void FillRect(int32 MinX, int32 MinZ, int32 MaxX, int32 MaxZ, bool bSolid);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Terrain")
	void FillTexturedRect(int32 MinX, int32 MinZ, int32 MaxX, int32 MaxZ, int32 LayerIndex);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Terrain")
	void ApplyCircle(int32 CenterX, int32 CenterZ, int32 RadiusCells, bool bSolid);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Terrain")
	void ApplyTexturedCircle(int32 CenterX, int32 CenterZ, int32 RadiusCells, int32 LayerIndex);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Terrain")
	void ApplyCircleStroke(int32 StartX, int32 StartZ, int32 EndX, int32 EndZ, int32 RadiusCells, bool bSolid);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Terrain")
	void ApplyTexturedCircleStroke(int32 StartX, int32 StartZ, int32 EndX, int32 EndZ, int32 RadiusCells, int32 LayerIndex);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Terrain")
	void SetTextureLayer(int32 LayerIndex, UTexture2D* Texture);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Terrain")
	void ApplyTextureRect(int32 MinX, int32 MinZ, int32 MaxX, int32 MaxZ, int32 LayerIndex);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Terrain")
	void ApplyTextureCircle(int32 CenterX, int32 CenterZ, int32 RadiusCells, int32 LayerIndex);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Terrain")
	void ApplyTextureCircleStroke(int32 StartX, int32 StartZ, int32 EndX, int32 EndZ, int32 RadiusCells, int32 LayerIndex);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Terrain")
	bool ImportSolidMaskFromTexture(UTexture2D* SourceTexture, bool bUseAlpha, float Threshold, int32 LayerIndex, bool bResizeToTexture = true);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Terrain")
	bool ImportSolidMaskFromTextureByColor(UTexture2D* SourceTexture, FLinearColor TargetColor, float Tolerance, int32 LayerIndex, bool bResizeToTexture = true);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Terrain")
	bool ImportSolidMaskFromTextureRegion(UTexture2D* SourceTexture, int32 SourceMinX, int32 SourceMinY, int32 SourceWidth, int32 SourceHeight, bool bUseAlpha, float Threshold, int32 LayerIndex, bool bResizeToRegion = true);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Terrain")
	bool ImportSolidMaskFromTextureRegionByColor(UTexture2D* SourceTexture, int32 SourceMinX, int32 SourceMinY, int32 SourceWidth, int32 SourceHeight, FLinearColor TargetColor, float Tolerance, int32 LayerIndex, bool bResizeToRegion = true);

	bool IsValidCell(int32 X, int32 Z) const;
	int32 ToIndex(int32 X, int32 Z) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain", meta = (ClampMin = "1"))
	int32 CellsX = 1280;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain", meta = (ClampMin = "1"))
	int32 CellsZ = 960;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain", meta = (ClampMin = "1.0"))
	float CellSize = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain")
	TArray<uint8> SolidMask;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain")
	TArray<uint8> TextureLayerMask;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain")
	TArray<FFortRogueTerrainTextureLayer> TextureLayers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	FVector PlayerSpawnLocal = FVector(-448.0f, 0.0f, 1040.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	FVector EnemySpawnLocal = FVector(448.0f, 0.0f, 1040.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile Bounds", meta = (ClampMin = "0.0"))
	float ProjectileBoundsPadding = 800.0f;
};
