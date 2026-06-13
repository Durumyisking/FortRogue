// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/FortRogueDestructibleTerrain.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AFortRogueDestructibleTerrain::AFortRogueDestructibleTerrain()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	TerrainInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("TerrainInstances"));
	TerrainInstances->SetupAttachment(Root);
	TerrainInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		TerrainInstances->SetStaticMesh(CubeMesh.Object);
	}
}

void AFortRogueDestructibleTerrain::BeginPlay()
{
	Super::BeginPlay();

	InitializeMask();
	RebuildVisuals();
}

bool AFortRogueDestructibleTerrain::IsSolidAtWorldLocation(const FVector& WorldLocation) const
{
	int32 X = 0;
	int32 Z = 0;
	return WorldToCell(WorldLocation, X, Z) && SolidMask.IsValidIndex(ToIndex(X, Z)) && SolidMask[ToIndex(X, Z)] != 0;
}

bool AFortRogueDestructibleTerrain::CarveCircle(const FVector& WorldLocation, float Radius)
{
	bool bChanged = false;
	const float RadiusSq = FMath::Square(Radius);

	for (int32 Z = 0; Z < CellsZ; ++Z)
	{
		for (int32 X = 0; X < CellsX; ++X)
		{
			const int32 Index = ToIndex(X, Z);
			if (!SolidMask.IsValidIndex(Index) || SolidMask[Index] == 0)
			{
				continue;
			}

			const FVector CellCenter = GetActorLocation() + FVector((X + 0.5f) * CellSize - Width * 0.5f, 0.0f, (Z + 0.5f) * CellSize);
			if (FVector::DistSquared2D(FVector(CellCenter.X, CellCenter.Z, 0.0f), FVector(WorldLocation.X, WorldLocation.Z, 0.0f)) <= RadiusSq)
			{
				SolidMask[Index] = 0;
				bChanged = true;
			}
		}
	}

	if (bChanged)
	{
		RebuildVisuals();
	}

	return bChanged;
}

float AFortRogueDestructibleTerrain::GetSurfaceZ() const
{
	return GetActorLocation().Z + Height;
}

void AFortRogueDestructibleTerrain::InitializeMask()
{
	CellsX = FMath::Max(1, FMath::RoundToInt(Width / CellSize));
	CellsZ = FMath::Max(1, FMath::RoundToInt(Height / CellSize));
	SolidMask.SetNumZeroed(CellsX * CellsZ);

	for (int32 Z = 0; Z < CellsZ; ++Z)
	{
		for (int32 X = 0; X < CellsX; ++X)
		{
			SolidMask[ToIndex(X, Z)] = 1;
		}
	}
}

void AFortRogueDestructibleTerrain::RebuildVisuals()
{
	TerrainInstances->ClearInstances();

	const FVector CellScale(CellSize / 100.0f, 0.08f, CellSize / 100.0f);
	for (int32 Z = 0; Z < CellsZ; ++Z)
	{
		for (int32 X = 0; X < CellsX; ++X)
		{
			if (SolidMask[ToIndex(X, Z)] == 0)
			{
				continue;
			}

			const FVector RelativeLocation((X + 0.5f) * CellSize - Width * 0.5f, 0.0f, (Z + 0.5f) * CellSize);
			TerrainInstances->AddInstance(FTransform(FRotator::ZeroRotator, RelativeLocation, CellScale));
		}
	}
}

int32 AFortRogueDestructibleTerrain::ToIndex(int32 X, int32 Z) const
{
	return Z * CellsX + X;
}

bool AFortRogueDestructibleTerrain::WorldToCell(const FVector& WorldLocation, int32& OutX, int32& OutZ) const
{
	const FVector LocalLocation = WorldLocation - GetActorLocation();
	OutX = FMath::FloorToInt((LocalLocation.X + Width * 0.5f) / CellSize);
	OutZ = FMath::FloorToInt(LocalLocation.Z / CellSize);
	return OutX >= 0 && OutX < CellsX && OutZ >= 0 && OutZ < CellsZ;
}
