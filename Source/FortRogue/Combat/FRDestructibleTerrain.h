// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FRDestructibleTerrain.generated.h"

class UInstancedStaticMeshComponent;
class UFRTerrainMapDefinition;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UStaticMeshComponent;
class UTexture2D;

UCLASS()
class FORTROGUE_API AFRDestructibleTerrain : public AActor
{
	GENERATED_BODY()

public:
	AFRDestructibleTerrain();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
#if WITH_EDITOR
	virtual void PostEditMove(bool bFinished) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	bool IsSolidAtWorldLocation(const FVector& WorldLocation) const;
	bool FindSurfaceZAtWorldX(float WorldX, float StartWorldZ, float SearchDistance, float& OutSurfaceZ) const;
	bool FindFirstSolidAlongWorldSegment(const FVector& StartWorldLocation, const FVector& EndWorldLocation, FVector& OutImpactLocation) const;
	bool CarveCircle(const FVector& WorldLocation, float Radius);
	bool FillCircle(const FVector& WorldLocation, float Radius, uint8 TextureLayer = 0);
	bool IsProjectileOutOfBounds(const FVector& WorldLocation) const;
	float GetSurfaceZ() const;
	FVector GetPlayerSpawnWorldLocation() const;
	FVector GetEnemySpawnWorldLocation() const;
	FVector ResolveMapLocalSpawnWorldLocation(const FVector& PreferredLocalLocation, const FVector& FallbackLocalLocation) const;
	UTexture2D* GetRuntimeTerrainTexture() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain", meta = (ToolTip = "이 액터가 사용할 지형 맵 데이터입니다. 지정하면 Width/Height/CellSize와 마스크를 맵 데이터에서 가져옵니다."))
	TObjectPtr<UFRTerrainMapDefinition> MapDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain", meta = (ToolTip = "MapDefinition이 없을 때 생성할 기본 지형의 월드 가로 크기입니다."))
	float Width = 2400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain", meta = (ToolTip = "MapDefinition이 없을 때 생성할 기본 지형의 월드 세로 크기입니다."))
	float Height = 520.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain", meta = (ToolTip = "MapDefinition이 없을 때 사용할 셀 크기입니다. 값이 작을수록 지형 해상도가 높아집니다."))
	float CellSize = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile Bounds", meta = (ClampMin = "0.0", ToolTip = "투사체가 지형 바깥으로 나갔다고 판단하기 전에 허용할 여유 거리입니다."))
	float ProjectileBoundsPadding = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain|Rendering", meta = (ToolTip = "런타임 지형 텍스처를 표시할 머티리얼입니다. 비워두면 인스턴스 메시 기반 fallback 렌더링을 사용합니다."))
	TObjectPtr<UMaterialInterface> TextureTerrainMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain|Rendering", meta = (ToolTip = "TextureTerrainMaterial 안에서 런타임 지형 텍스처를 받을 파라미터 이름입니다."))
	FName TextureParameterName = TEXT("TerrainTexture");

private:
	void InitializeMask();
	void InitializeGeneratedMask();
	void InitializeMaskFromDefinition();
	void ApplyDefinitionDimensions();
	void NormalizeActorTransformForGameplayPlane();
	void ConfigureTexturePlane();
	void RebuildVisuals();
	void InitializeRuntimeTexture();
	void UpdateRuntimeTexture();
	void UpdateRuntimeTextureRegion(int32 MinX, int32 MinZ, int32 MaxX, int32 MaxZ);
	void CacheLayerTextures();
	FVector ResolveSpawnWorldLocation(const FVector& PreferredLocalLocation, const FVector& FallbackLocalLocation) const;
	FColor GetTerrainPixelColor(int32 X, int32 Z) const;
	bool TryGetLayerTextureColor(uint8 LayerIndex, int32 X, int32 Z, FColor& OutColor) const;
	float GetGeneratedSurfaceZ(int32 X) const;
	int32 ToIndex(int32 X, int32 Z) const;
	bool WorldToCell(const FVector& WorldLocation, int32& OutX, int32& OutZ) const;
	bool WorldXToCell(float WorldX, int32& OutX) const;

	UPROPERTY(VisibleAnywhere, Category = "Terrain")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "Terrain")
	TObjectPtr<UInstancedStaticMeshComponent> TerrainInstances;

	UPROPERTY(VisibleAnywhere, Category = "Terrain")
	TObjectPtr<UStaticMeshComponent> TerrainTexturePlane;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> RuntimeTerrainTexture;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> RuntimeTerrainMaterial;

	int32 CellsX = 0;
	int32 CellsZ = 0;
	TArray<uint8> SolidMask;
	TArray<uint8> TextureLayerMask;
	TArray<FColor> RuntimeTexturePixels;
	TArray<TArray<FColor>> CachedLayerTexturePixels;
	TArray<FIntPoint> CachedLayerTextureSizes;
};
