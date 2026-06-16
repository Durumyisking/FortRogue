// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/FortRogueDestructibleTerrain.h"

#include "Combat/FortRogueTerrainMapDefinition.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
constexpr float MinimumSpawnClearance = 80.0f;

bool ReadLayerTexturePixels(UTexture2D* SourceTexture, TArray<FColor>& OutPixels, FIntPoint& OutSize)
{
	if (!SourceTexture)
	{
		return false;
	}

#if WITH_EDITOR
	if (SourceTexture->Source.IsValid())
	{
		TArray64<uint8> SourceMipData;
		if (SourceTexture->Source.GetMipData(SourceMipData, 0))
		{
			const int32 SourceWidth = static_cast<int32>(SourceTexture->Source.GetSizeX());
			const int32 SourceHeight = static_cast<int32>(SourceTexture->Source.GetSizeY());
			const ETextureSourceFormat SourceFormat = SourceTexture->Source.GetFormat();
			if (SourceWidth > 0 && SourceHeight > 0 && (SourceFormat == TSF_BGRA8 || SourceFormat == TSF_G8))
			{
				const int64 BytesPerPixel = SourceFormat == TSF_BGRA8 ? 4 : 1;
				if (SourceMipData.Num() < static_cast<int64>(SourceWidth) * SourceHeight * BytesPerPixel)
				{
					return false;
				}

				OutSize = FIntPoint(SourceWidth, SourceHeight);
				OutPixels.SetNumUninitialized(SourceWidth * SourceHeight);
				for (int32 SourceY = 0; SourceY < SourceHeight; ++SourceY)
				{
					for (int32 X = 0; X < SourceWidth; ++X)
					{
						const int64 SourceIndex = static_cast<int64>(SourceY) * SourceWidth + X;
						if (SourceFormat == TSF_BGRA8)
						{
							const uint8* PixelData = SourceMipData.GetData() + SourceIndex * 4;
							OutPixels[SourceIndex] = FColor(PixelData[2], PixelData[1], PixelData[0], PixelData[3]);
						}
						else
						{
							const uint8 Value = SourceMipData[SourceIndex];
							OutPixels[SourceIndex] = FColor(Value, Value, Value, Value);
						}
					}
				}
				return true;
			}
		}
	}
#endif

	if (!SourceTexture->GetPlatformData() || SourceTexture->GetPlatformData()->Mips.Num() == 0 || SourceTexture->GetPixelFormat() != PF_B8G8R8A8)
	{
		return false;
	}

	FTexture2DMipMap& Mip = SourceTexture->GetPlatformData()->Mips[0];
	if (Mip.SizeX <= 0 || Mip.SizeY <= 0)
	{
		return false;
	}

	const void* TextureData = Mip.BulkData.LockReadOnly();
	if (!TextureData)
	{
		Mip.BulkData.Unlock();
		return false;
	}

	OutSize = FIntPoint(Mip.SizeX, Mip.SizeY);
	OutPixels.SetNumUninitialized(Mip.SizeX * Mip.SizeY);
	FMemory::Memcpy(OutPixels.GetData(), TextureData, OutPixels.Num() * sizeof(FColor));
	Mip.BulkData.Unlock();
	return true;
}
}

AFortRogueDestructibleTerrain::AFortRogueDestructibleTerrain()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	TerrainInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("TerrainInstances"));
	TerrainInstances->SetupAttachment(Root);
	TerrainInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TerrainInstances->SetUsingAbsoluteRotation(true);
	TerrainInstances->SetUsingAbsoluteScale(true);

	TerrainTexturePlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TerrainTexturePlane"));
	TerrainTexturePlane->SetupAttachment(Root);
	TerrainTexturePlane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TerrainTexturePlane->SetUsingAbsoluteRotation(true);
	TerrainTexturePlane->SetUsingAbsoluteScale(true);
	ConfigureTexturePlane();

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		TerrainInstances->SetStaticMesh(CubeMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMesh.Succeeded())
	{
		TerrainTexturePlane->SetStaticMesh(PlaneMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> SpriteMaterial(TEXT("/Paper2D/MaskedUnlitSpriteMaterial.MaskedUnlitSpriteMaterial"));
	if (SpriteMaterial.Succeeded())
	{
		TextureTerrainMaterial = SpriteMaterial.Object;
	}
}

void AFortRogueDestructibleTerrain::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	NormalizeActorTransformForGameplayPlane();
	InitializeMask();
	InitializeRuntimeTexture();
	RebuildVisuals();
}

void AFortRogueDestructibleTerrain::BeginPlay()
{
	Super::BeginPlay();

	NormalizeActorTransformForGameplayPlane();
	InitializeMask();
	InitializeRuntimeTexture();
	RebuildVisuals();
}

#if WITH_EDITOR
void AFortRogueDestructibleTerrain::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);

	NormalizeActorTransformForGameplayPlane();
	ConfigureTexturePlane();
}
#endif

bool AFortRogueDestructibleTerrain::IsSolidAtWorldLocation(const FVector& WorldLocation) const
{
	int32 X = 0;
	int32 Z = 0;
	return WorldToCell(WorldLocation, X, Z) && SolidMask.IsValidIndex(ToIndex(X, Z)) && SolidMask[ToIndex(X, Z)] != 0;
}

bool AFortRogueDestructibleTerrain::FindSurfaceZAtWorldX(float WorldX, float StartWorldZ, float SearchDistance, float& OutSurfaceZ) const
{
	int32 X = 0;
	if (!WorldXToCell(WorldX, X))
	{
		return false;
	}

	const float LocalStartZ = StartWorldZ - GetActorLocation().Z;
	const float LocalEndZ = LocalStartZ - FMath::Max(0.0f, SearchDistance);
	const int32 StartZ = FMath::Clamp(FMath::FloorToInt(LocalStartZ / CellSize), 0, CellsZ - 1);
	const int32 EndZ = FMath::Clamp(FMath::FloorToInt(LocalEndZ / CellSize), 0, CellsZ - 1);

	for (int32 Z = StartZ; Z >= EndZ; --Z)
	{
		const int32 Index = ToIndex(X, Z);
		const int32 AboveIndex = ToIndex(X, Z + 1);
		const bool bAboveIsSolid = Z + 1 < CellsZ && SolidMask.IsValidIndex(AboveIndex) && SolidMask[AboveIndex] != 0;
		const float CellTopWorldZ = GetActorLocation().Z + (Z + 1) * CellSize;
		if (SolidMask.IsValidIndex(Index) && SolidMask[Index] != 0 && !bAboveIsSolid && CellTopWorldZ <= StartWorldZ + KINDA_SMALL_NUMBER)
		{
			OutSurfaceZ = CellTopWorldZ;
			return true;
		}
	}

	return false;
}

bool AFortRogueDestructibleTerrain::FindFirstSolidAlongWorldSegment(const FVector& StartWorldLocation, const FVector& EndWorldLocation, FVector& OutImpactLocation) const
{
	const FVector Segment = EndWorldLocation - StartWorldLocation;
	const float SegmentLengthXZ = FVector2D(Segment.X, Segment.Z).Size();
	const float StepLength = FMath::Max(1.0f, CellSize * 0.5f);
	const int32 Steps = FMath::Max(1, FMath::CeilToInt(SegmentLengthXZ / StepLength));

	for (int32 Step = 0; Step <= Steps; ++Step)
	{
		const float Alpha = static_cast<float>(Step) / static_cast<float>(Steps);
		const FVector TestLocation = FMath::Lerp(StartWorldLocation, EndWorldLocation, Alpha);
		if (IsSolidAtWorldLocation(TestLocation))
		{
			OutImpactLocation = TestLocation;
			return true;
		}
	}

	return false;
}

bool AFortRogueDestructibleTerrain::CarveCircle(const FVector& WorldLocation, float Radius)
{
	if (CellsX <= 0 || CellsZ <= 0 || CellSize <= 0.0f || Radius < 0.0f)
	{
		return false;
	}

	bool bChanged = false;
	const float RadiusSq = FMath::Square(Radius);
	const FVector LocalLocation = WorldLocation - GetActorLocation();
	const float LocalCenterX = LocalLocation.X + Width * 0.5f;
	if (LocalCenterX + Radius < 0.0f || LocalCenterX - Radius >= Width || LocalLocation.Z + Radius < 0.0f || LocalLocation.Z - Radius >= Height)
	{
		return false;
	}

	const int32 MinX = FMath::Clamp(FMath::FloorToInt((LocalCenterX - Radius) / CellSize), 0, CellsX - 1);
	const int32 MaxX = FMath::Clamp(FMath::FloorToInt((LocalCenterX + Radius) / CellSize), 0, CellsX - 1);
	const int32 MinZ = FMath::Clamp(FMath::FloorToInt((LocalLocation.Z - Radius) / CellSize), 0, CellsZ - 1);
	const int32 MaxZ = FMath::Clamp(FMath::FloorToInt((LocalLocation.Z + Radius) / CellSize), 0, CellsZ - 1);

	if (MinX > MaxX || MinZ > MaxZ)
	{
		return false;
	}

	for (int32 Z = MinZ; Z <= MaxZ; ++Z)
	{
		for (int32 X = MinX; X <= MaxX; ++X)
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
				if (TextureLayerMask.IsValidIndex(Index))
				{
					TextureLayerMask[Index] = 0;
				}
				bChanged = true;
			}
		}
	}

	if (bChanged)
	{
		if (RuntimeTerrainMaterial)
		{
			ConfigureTexturePlane();
			TerrainTexturePlane->SetVisibility(true);
		}
		else
		{
			RebuildVisuals();
		}
		UpdateRuntimeTextureRegion(MinX, MinZ, MaxX, MaxZ);
	}

	return bChanged;
}

float AFortRogueDestructibleTerrain::GetSurfaceZ() const
{
	return GetActorLocation().Z + Height;
}

FVector AFortRogueDestructibleTerrain::GetPlayerSpawnWorldLocation() const
{
	return ResolveSpawnWorldLocation(MapDefinition ? MapDefinition->PlayerSpawnLocal : FVector::ZeroVector, FVector(-650.0f, 0.0f, Height + 95.0f));
}

FVector AFortRogueDestructibleTerrain::GetEnemySpawnWorldLocation() const
{
	return ResolveSpawnWorldLocation(MapDefinition ? MapDefinition->EnemySpawnLocal : FVector::ZeroVector, FVector(650.0f, 0.0f, Height + 95.0f));
}

UTexture2D* AFortRogueDestructibleTerrain::GetRuntimeTerrainTexture() const
{
	return RuntimeTerrainTexture.Get();
}

void AFortRogueDestructibleTerrain::InitializeMask()
{
	if (MapDefinition)
	{
		MapDefinition->NormalizeMapData();
	}

	if (MapDefinition && MapDefinition->CellsX > 0 && MapDefinition->CellsZ > 0 && MapDefinition->SolidMask.Num() == MapDefinition->CellsX * MapDefinition->CellsZ)
	{
		InitializeMaskFromDefinition();
		return;
	}

	InitializeGeneratedMask();
}

void AFortRogueDestructibleTerrain::InitializeGeneratedMask()
{
	CellSize = FMath::Max(1.0f, CellSize);
	Width = FMath::Max(CellSize, Width);
	Height = FMath::Max(CellSize, Height);
	CellsX = FMath::Max(1, FMath::RoundToInt(Width / CellSize));
	CellsZ = FMath::Max(1, FMath::RoundToInt(Height / CellSize));
	SolidMask.SetNumZeroed(CellsX * CellsZ);
	TextureLayerMask.SetNumZeroed(CellsX * CellsZ);

	for (int32 Z = 0; Z < CellsZ; ++Z)
	{
		for (int32 X = 0; X < CellsX; ++X)
		{
			const float CellTopZ = (Z + 1) * CellSize;
			SolidMask[ToIndex(X, Z)] = CellTopZ <= GetGeneratedSurfaceZ(X) ? 1 : 0;
		}
	}
}

void AFortRogueDestructibleTerrain::InitializeMaskFromDefinition()
{
	ApplyDefinitionDimensions();
	SolidMask = MapDefinition->SolidMask;
	TextureLayerMask = MapDefinition->TextureLayerMask;
	if (TextureLayerMask.Num() != SolidMask.Num())
	{
		TextureLayerMask.SetNumZeroed(SolidMask.Num());
	}
}

void AFortRogueDestructibleTerrain::ApplyDefinitionDimensions()
{
	if (!MapDefinition || MapDefinition->CellsX <= 0 || MapDefinition->CellsZ <= 0)
	{
		return;
	}

	CellsX = MapDefinition->CellsX;
	CellsZ = MapDefinition->CellsZ;
	CellSize = MapDefinition->CellSize;
	Width = CellsX * CellSize;
	Height = CellsZ * CellSize;
}

void AFortRogueDestructibleTerrain::NormalizeActorTransformForGameplayPlane()
{
	if (!GetActorRotation().IsNearlyZero())
	{
		SetActorRotation(FRotator::ZeroRotator);
	}

	if (!GetActorScale3D().Equals(FVector::OneVector))
	{
		SetActorScale3D(FVector::OneVector);
	}
}

void AFortRogueDestructibleTerrain::ConfigureTexturePlane()
{
	if (!TerrainTexturePlane)
	{
		return;
	}

	TerrainTexturePlane->SetRelativeRotation(FRotator(0.0f, 0.0f, 90.0f));
	TerrainTexturePlane->SetRelativeLocation(FVector(0.0f, -6.0f, Height * 0.5f));
	TerrainTexturePlane->SetRelativeScale3D(FVector(Width / 100.0f, Height / 100.0f, 1.0f));
}

void AFortRogueDestructibleTerrain::RebuildVisuals()
{
	TerrainInstances->ClearInstances();
	ConfigureTexturePlane();
	TerrainTexturePlane->SetVisibility(RuntimeTerrainMaterial != nullptr);

	if (RuntimeTerrainMaterial)
	{
		return;
	}

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

void AFortRogueDestructibleTerrain::InitializeRuntimeTexture()
{
	CacheLayerTextures();
	RuntimeTexturePixels.SetNumZeroed(CellsX * CellsZ);
	RuntimeTerrainTexture = UTexture2D::CreateTransient(CellsX, CellsZ, PF_B8G8R8A8);
	if (RuntimeTerrainTexture)
	{
		RuntimeTerrainTexture->CompressionSettings = TC_VectorDisplacementmap;
		RuntimeTerrainTexture->Filter = TF_Bilinear;
		RuntimeTerrainTexture->SRGB = true;
		RuntimeTerrainTexture->NeverStream = true;
	}

	RuntimeTerrainMaterial = TextureTerrainMaterial ? UMaterialInstanceDynamic::Create(TextureTerrainMaterial, this) : nullptr;
	if (RuntimeTerrainMaterial && RuntimeTerrainTexture)
	{
		RuntimeTerrainMaterial->SetTextureParameterValue(TextureParameterName, RuntimeTerrainTexture);
		RuntimeTerrainMaterial->SetTextureParameterValue(TEXT("Texture"), RuntimeTerrainTexture);
		RuntimeTerrainMaterial->SetTextureParameterValue(TEXT("SpriteTexture"), RuntimeTerrainTexture);
		RuntimeTerrainMaterial->SetTextureParameterValue(TEXT("DiffuseTexture"), RuntimeTerrainTexture);
		RuntimeTerrainMaterial->SetTextureParameterValue(TEXT("BaseTexture"), RuntimeTerrainTexture);
		TerrainTexturePlane->SetMaterial(0, RuntimeTerrainMaterial);
	}

	UpdateRuntimeTexture();
}

void AFortRogueDestructibleTerrain::CacheLayerTextures()
{
	CachedLayerTexturePixels.Reset();
	CachedLayerTextureSizes.Reset();

	const int32 LayerCount = MapDefinition ? MapDefinition->TextureLayers.Num() : 0;
	CachedLayerTexturePixels.SetNum(LayerCount);
	CachedLayerTextureSizes.SetNumZeroed(LayerCount);

	for (int32 LayerIndex = 0; LayerIndex < LayerCount; ++LayerIndex)
	{
		UTexture2D* SourceTexture = MapDefinition->TextureLayers[LayerIndex].Texture;
		ReadLayerTexturePixels(SourceTexture, CachedLayerTexturePixels[LayerIndex], CachedLayerTextureSizes[LayerIndex]);
	}
}

FVector AFortRogueDestructibleTerrain::ResolveSpawnWorldLocation(const FVector& PreferredLocalLocation, const FVector& FallbackLocalLocation) const
{
	FVector LocalLocation = MapDefinition ? PreferredLocalLocation : FallbackLocalLocation;
	const float HalfWidth = Width * 0.5f;
	const float EdgePadding = FMath::Max(CellSize, 1.0f);
	if (HalfWidth > EdgePadding)
	{
		LocalLocation.X = FMath::Clamp(LocalLocation.X, -HalfWidth + EdgePadding, HalfWidth - EdgePadding);
	}

	const float SpawnClearance = FMath::Max(MinimumSpawnClearance, LocalLocation.Z - Height);
	float SurfaceZ = 0.0f;
	if (FindSurfaceZAtWorldX(GetActorLocation().X + LocalLocation.X, GetActorLocation().Z + Height, Height + CellSize, SurfaceZ))
	{
		LocalLocation.Z = SurfaceZ - GetActorLocation().Z + SpawnClearance;
	}
	else if (LocalLocation.Z <= Height)
	{
		LocalLocation.Z = Height + MinimumSpawnClearance;
	}

	return GetActorLocation() + LocalLocation;
}

void AFortRogueDestructibleTerrain::UpdateRuntimeTexture()
{
	if (!RuntimeTerrainTexture || !RuntimeTerrainTexture->GetPlatformData() || RuntimeTerrainTexture->GetPlatformData()->Mips.Num() == 0 || RuntimeTexturePixels.Num() != CellsX * CellsZ)
	{
		return;
	}

	for (int32 Z = 0; Z < CellsZ; ++Z)
	{
		for (int32 X = 0; X < CellsX; ++X)
		{
			const int32 TextureIndex = (CellsZ - 1 - Z) * CellsX + X;
			RuntimeTexturePixels[TextureIndex] = GetTerrainPixelColor(X, Z);
		}
	}

	FTexture2DMipMap& Mip = RuntimeTerrainTexture->GetPlatformData()->Mips[0];
	void* TextureData = Mip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureData, RuntimeTexturePixels.GetData(), RuntimeTexturePixels.Num() * sizeof(FColor));
	Mip.BulkData.Unlock();
	RuntimeTerrainTexture->UpdateResource();
}

void AFortRogueDestructibleTerrain::UpdateRuntimeTextureRegion(int32 MinX, int32 MinZ, int32 MaxX, int32 MaxZ)
{
	if (!RuntimeTerrainTexture || RuntimeTexturePixels.Num() != CellsX * CellsZ)
	{
		return;
	}

	MinX = FMath::Clamp(MinX, 0, CellsX - 1);
	MaxX = FMath::Clamp(MaxX, 0, CellsX - 1);
	MinZ = FMath::Clamp(MinZ, 0, CellsZ - 1);
	MaxZ = FMath::Clamp(MaxZ, 0, CellsZ - 1);
	if (MinX > MaxX || MinZ > MaxZ)
	{
		return;
	}

	for (int32 Z = MinZ; Z <= MaxZ; ++Z)
	{
		for (int32 X = MinX; X <= MaxX; ++X)
		{
			const int32 TextureIndex = (CellsZ - 1 - Z) * CellsX + X;
			RuntimeTexturePixels[TextureIndex] = GetTerrainPixelColor(X, Z);
		}
	}

	const int32 RegionWidth = MaxX - MinX + 1;
	const int32 RegionHeight = MaxZ - MinZ + 1;
	const int32 TextureMinY = CellsZ - 1 - MaxZ;
	FColor* RegionPixels = static_cast<FColor*>(FMemory::Malloc(RegionWidth * RegionHeight * sizeof(FColor)));
	if (!RegionPixels)
	{
		return;
	}

	for (int32 Row = 0; Row < RegionHeight; ++Row)
	{
		const int32 TextureY = TextureMinY + Row;
		const int32 TextureIndex = TextureY * CellsX + MinX;
		FMemory::Memcpy(RegionPixels + Row * RegionWidth, RuntimeTexturePixels.GetData() + TextureIndex, RegionWidth * sizeof(FColor));
	}

	FUpdateTextureRegion2D* Region = new FUpdateTextureRegion2D(MinX, TextureMinY, 0, 0, RegionWidth, RegionHeight);
	RuntimeTerrainTexture->UpdateTextureRegions(
		0,
		1,
		Region,
		RegionWidth * sizeof(FColor),
		sizeof(FColor),
		reinterpret_cast<uint8*>(RegionPixels),
		[](uint8* SrcData, const FUpdateTextureRegion2D* Regions)
		{
			FMemory::Free(SrcData);
			delete Regions;
		});
}

FColor AFortRogueDestructibleTerrain::GetTerrainPixelColor(int32 X, int32 Z) const
{
	const int32 Index = ToIndex(X, Z);
	if (!SolidMask.IsValidIndex(Index) || SolidMask[Index] == 0)
	{
		return FColor(0, 0, 0, 0);
	}

	const uint8 LayerIndex = TextureLayerMask.IsValidIndex(Index) ? TextureLayerMask[Index] : 0;
	FColor TextureColor;
	if (TryGetLayerTextureColor(LayerIndex, X, Z, TextureColor))
	{
		return TextureColor;
	}

	if (MapDefinition && MapDefinition->TextureLayers.IsValidIndex(LayerIndex))
	{
		return MapDefinition->TextureLayers[LayerIndex].FallbackColor.ToFColor(true);
	}

	static const FColor FallbackColors[] = {
		FColor(110, 87, 54, 255),
		FColor(64, 107, 71, 255),
		FColor(115, 107, 97, 255),
		FColor(133, 79, 61, 255),
		FColor(61, 92, 128, 255),
		FColor(148, 128, 56, 255)
	};

	return FallbackColors[LayerIndex % UE_ARRAY_COUNT(FallbackColors)];
}

bool AFortRogueDestructibleTerrain::TryGetLayerTextureColor(uint8 LayerIndex, int32 X, int32 Z, FColor& OutColor) const
{
	if (!CachedLayerTexturePixels.IsValidIndex(LayerIndex) || !CachedLayerTextureSizes.IsValidIndex(LayerIndex))
	{
		return false;
	}

	const FIntPoint TextureSize = CachedLayerTextureSizes[LayerIndex];
	const TArray<FColor>& TexturePixels = CachedLayerTexturePixels[LayerIndex];
	if (TextureSize.X <= 0 || TextureSize.Y <= 0 || TexturePixels.Num() != TextureSize.X * TextureSize.Y)
	{
		return false;
	}

	const int32 TextureX = FMath::Clamp(FMath::FloorToInt((static_cast<float>(X) + 0.5f) * TextureSize.X / FMath::Max(1, CellsX)), 0, TextureSize.X - 1);
	const int32 TextureY = FMath::Clamp(FMath::FloorToInt((static_cast<float>(CellsZ - 1 - Z) + 0.5f) * TextureSize.Y / FMath::Max(1, CellsZ)), 0, TextureSize.Y - 1);
	OutColor = TexturePixels[TextureY * TextureSize.X + TextureX];
	OutColor.A = 255;
	return true;
}

int32 AFortRogueDestructibleTerrain::ToIndex(int32 X, int32 Z) const
{
	return Z * CellsX + X;
}

float AFortRogueDestructibleTerrain::GetGeneratedSurfaceZ(int32 X) const
{
	const float Alpha = CellsX > 1 ? static_cast<float>(X) / static_cast<float>(CellsX - 1) : 0.0f;
	const float Ridge = FMath::Sin(Alpha * 2.0f * PI * 1.7f + 0.35f) * 0.16f;
	const float Detail = FMath::Sin(Alpha * 2.0f * PI * 4.6f + 1.8f) * 0.055f;
	const float Bowl = -FMath::Square(Alpha - 0.5f) * 0.18f;
	return Height * FMath::Clamp(0.58f + Ridge + Detail + Bowl, 0.32f, 0.86f);
}

bool AFortRogueDestructibleTerrain::WorldToCell(const FVector& WorldLocation, int32& OutX, int32& OutZ) const
{
	const FVector LocalLocation = WorldLocation - GetActorLocation();
	OutX = FMath::FloorToInt((LocalLocation.X + Width * 0.5f) / CellSize);
	OutZ = FMath::FloorToInt(LocalLocation.Z / CellSize);
	return OutX >= 0 && OutX < CellsX && OutZ >= 0 && OutZ < CellsZ;
}

bool AFortRogueDestructibleTerrain::WorldXToCell(float WorldX, int32& OutX) const
{
	OutX = FMath::FloorToInt(((WorldX - GetActorLocation().X) + Width * 0.5f) / CellSize);
	return OutX >= 0 && OutX < CellsX;
}
