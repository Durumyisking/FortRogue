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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain", meta = (ToolTip = "텍스처 레이어를 구분하는 이름입니다. 에디터에서 레이어 의미를 알아볼 수 있게 지정합니다."))
	FName LayerId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain", meta = (ToolTip = "이 레이어에 칠해진 지형을 표시할 텍스처입니다. 비워두면 FallbackColor를 사용합니다."))
	TObjectPtr<UTexture2D> Texture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain", meta = (ToolTip = "Texture가 없거나 샘플링할 수 없을 때 사용할 기본 색입니다."))
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain", meta = (ClampMin = "1", ToolTip = "지형 마스크의 가로 셀 수입니다. 값이 커질수록 더 넓거나 더 정밀한 지형을 만들 수 있습니다."))
	int32 CellsX = 1280;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain", meta = (ClampMin = "1", ToolTip = "지형 마스크의 세로 셀 수입니다. 값이 커질수록 더 높은 지형을 만들 수 있습니다."))
	int32 CellsZ = 960;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain", meta = (ClampMin = "1.0", ToolTip = "셀 하나가 월드에서 차지하는 크기입니다. Spawn 위치를 유지하려면 전용 편집 도구의 크기 변경 기능을 사용하세요."))
	float CellSize = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain", meta = (ToolTip = "셀별 고체 여부 마스크입니다. 1이면 지형이 있고 0이면 비어 있습니다. 보통 직접 수정하지 않고 지형 에디터로 편집합니다."))
	TArray<uint8> SolidMask;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain", meta = (ToolTip = "셀별 텍스처 레이어 인덱스입니다. SolidMask와 같은 크기로 유지되어야 하며 보통 지형 에디터로 편집합니다."))
	TArray<uint8> TextureLayerMask;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain", meta = (TitleProperty = LayerId, ToolTip = "지형 셀에 적용할 수 있는 텍스처 레이어 목록입니다. TextureLayerMask 값이 이 배열의 인덱스를 가리킵니다."))
	TArray<FFortRogueTerrainTextureLayer> TextureLayers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (ToolTip = "맵 로컬 좌표 기준 플레이어 시작 위치입니다. Z는 지형 위에서 시작하도록 충분히 높게 둡니다."))
	FVector PlayerSpawnLocal = FVector(-448.0f, 0.0f, 1040.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (ToolTip = "맵 로컬 좌표 기준 적 시작 위치입니다. Z는 지형 위에서 시작하도록 충분히 높게 둡니다."))
	FVector EnemySpawnLocal = FVector(448.0f, 0.0f, 1040.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile Bounds", meta = (ClampMin = "0.0", ToolTip = "투사체가 지형 영역 밖으로 나갔다고 판단하기 전에 허용할 여유 거리입니다."))
	float ProjectileBoundsPadding = 800.0f;
};
