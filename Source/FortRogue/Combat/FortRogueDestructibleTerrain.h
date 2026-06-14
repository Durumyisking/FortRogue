// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FortRogueDestructibleTerrain.generated.h"

class UInstancedStaticMeshComponent;
class UFortRogueTerrainMapDefinition;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UStaticMeshComponent;
class UTexture2D;

UCLASS()
class FORTROGUE_API AFortRogueDestructibleTerrain : public AActor
{
	GENERATED_BODY()

public:
	AFortRogueDestructibleTerrain();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	bool IsSolidAtWorldLocation(const FVector& WorldLocation) const;
	bool FindSurfaceZAtWorldX(float WorldX, float StartWorldZ, float SearchDistance, float& OutSurfaceZ) const;
	bool FindFirstSolidAlongWorldSegment(const FVector& StartWorldLocation, const FVector& EndWorldLocation, FVector& OutImpactLocation) const;
	bool CarveCircle(const FVector& WorldLocation, float Radius);
	float GetSurfaceZ() const;
	FVector GetPlayerSpawnWorldLocation() const;
	FVector GetEnemySpawnWorldLocation() const;
	UTexture2D* GetRuntimeTerrainTexture() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain")
	TObjectPtr<UFortRogueTerrainMapDefinition> MapDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain")
	float Width = 2400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain")
	float Height = 520.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain")
	float CellSize = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain|Rendering")
	TObjectPtr<UMaterialInterface> TextureTerrainMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain|Rendering")
	FName TextureParameterName = TEXT("TerrainTexture");

private:
	void InitializeMask();
	void InitializeGeneratedMask();
	void InitializeMaskFromDefinition();
	void ApplyDefinitionDimensions();
	void NormalizeActorRotationForGameplayPlane();
	void ConfigureTexturePlane();
	void RebuildVisuals();
	void InitializeRuntimeTexture();
	void UpdateRuntimeTexture();
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
