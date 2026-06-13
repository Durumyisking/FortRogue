// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FortRogueDestructibleTerrain.generated.h"

class UInstancedStaticMeshComponent;

UCLASS()
class FORTROGUE_API AFortRogueDestructibleTerrain : public AActor
{
	GENERATED_BODY()

public:
	AFortRogueDestructibleTerrain();

	virtual void BeginPlay() override;

	bool IsSolidAtWorldLocation(const FVector& WorldLocation) const;
	bool CarveCircle(const FVector& WorldLocation, float Radius);
	float GetSurfaceZ() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain")
	float Width = 2400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain")
	float Height = 520.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain")
	float CellSize = 32.0f;

private:
	void InitializeMask();
	void RebuildVisuals();
	int32 ToIndex(int32 X, int32 Z) const;
	bool WorldToCell(const FVector& WorldLocation, int32& OutX, int32& OutZ) const;

	UPROPERTY(VisibleAnywhere, Category = "Terrain")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "Terrain")
	TObjectPtr<UInstancedStaticMeshComponent> TerrainInstances;

	int32 CellsX = 0;
	int32 CellsZ = 0;
	TArray<uint8> SolidMask;
};
