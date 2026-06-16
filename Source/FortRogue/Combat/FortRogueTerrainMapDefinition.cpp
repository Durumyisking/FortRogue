// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/FortRogueTerrainMapDefinition.h"

#include "Engine/Texture2D.h"

namespace
{
bool ReadTerrainImportPixels(UTexture2D* SourceTexture, TArray<FColor>& OutPixels, int32& OutWidth, int32& OutHeight)
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

				OutWidth = SourceWidth;
				OutHeight = SourceHeight;
				OutPixels.SetNumUninitialized(OutWidth * OutHeight);
				for (int32 SourceY = 0; SourceY < OutHeight; ++SourceY)
				{
					for (int32 X = 0; X < OutWidth; ++X)
					{
						const int64 SourceIndex = static_cast<int64>(SourceY) * OutWidth + X;
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

	OutWidth = Mip.SizeX;
	OutHeight = Mip.SizeY;
	OutPixels.SetNumUninitialized(OutWidth * OutHeight);
	FMemory::Memcpy(OutPixels.GetData(), TextureData, OutPixels.Num() * sizeof(FColor));
	Mip.BulkData.Unlock();
	return true;
}

bool TryGetAverageOpaqueTextureColor(UTexture2D* SourceTexture, FLinearColor& OutColor)
{
	TArray<FColor> Pixels;
	int32 TextureWidth = 0;
	int32 TextureHeight = 0;
	if (!ReadTerrainImportPixels(SourceTexture, Pixels, TextureWidth, TextureHeight))
	{
		return false;
	}

	FVector4f AccumulatedColor(0.0f, 0.0f, 0.0f, 0.0f);
	int32 SampleCount = 0;
	for (const FColor& Pixel : Pixels)
	{
		if (Pixel.A == 0)
		{
			continue;
		}

		AccumulatedColor.X += static_cast<float>(Pixel.R);
		AccumulatedColor.Y += static_cast<float>(Pixel.G);
		AccumulatedColor.Z += static_cast<float>(Pixel.B);
		AccumulatedColor.W += static_cast<float>(Pixel.A);
		++SampleCount;
	}

	if (SampleCount <= 0)
	{
		return false;
	}

	const float InvScale = 1.0f / (static_cast<float>(SampleCount) * 255.0f);
	OutColor = FLinearColor(AccumulatedColor.X * InvScale, AccumulatedColor.Y * InvScale, AccumulatedColor.Z * InvScale, AccumulatedColor.W * InvScale);
	OutColor.A = 1.0f;
	return true;
}

bool ClampImportRegion(int32 SourceWidth, int32 SourceHeight, int32 SourceMinX, int32 SourceMinY, int32 RegionWidth, int32 RegionHeight, FIntRect& OutRegion)
{
	if (SourceWidth <= 0 || SourceHeight <= 0)
	{
		return false;
	}

	const int32 MinX = FMath::Clamp(SourceMinX, 0, SourceWidth - 1);
	const int32 MinY = FMath::Clamp(SourceMinY, 0, SourceHeight - 1);
	const int32 Width = RegionWidth > 0 ? RegionWidth : SourceWidth - MinX;
	const int32 Height = RegionHeight > 0 ? RegionHeight : SourceHeight - MinY;
	const int32 MaxX = FMath::Clamp(MinX + Width, MinX + 1, SourceWidth);
	const int32 MaxY = FMath::Clamp(MinY + Height, MinY + 1, SourceHeight);
	OutRegion = FIntRect(MinX, MinY, MaxX, MaxY);
	return OutRegion.Width() > 0 && OutRegion.Height() > 0;
}

int32 GetNearestImportSourceX(int32 TargetX, int32 TargetWidth, const FIntRect& SourceRegion)
{
	return FMath::Clamp(SourceRegion.Min.X + FMath::FloorToInt((static_cast<float>(TargetX) + 0.5f) * SourceRegion.Width() / FMath::Max(1, TargetWidth)), SourceRegion.Min.X, SourceRegion.Max.X - 1);
}

int32 GetNearestImportSourceY(int32 TargetY, int32 TargetHeight, const FIntRect& SourceRegion)
{
	return FMath::Clamp(SourceRegion.Min.Y + FMath::FloorToInt((static_cast<float>(TargetY) + 0.5f) * SourceRegion.Height() / FMath::Max(1, TargetHeight)), SourceRegion.Min.Y, SourceRegion.Max.Y - 1);
}

float GetImportSourceCoordinate(int32 Target, int32 TargetSize, int32 SourceMin, int32 SourceSize)
{
	return SourceMin + (static_cast<float>(Target) + 0.5f) * SourceSize / FMath::Max(1, TargetSize) - 0.5f;
}

uint8 GetImportMaskSample(const TArray<FColor>& Pixels, int32 TextureWidth, const FIntRect& SourceRegion, int32 TargetX, int32 TargetY, int32 TargetWidth, int32 TargetHeight, bool bUseAlpha)
{
	const bool bUpscaling = TargetWidth > SourceRegion.Width() || TargetHeight > SourceRegion.Height();
	if (!bUpscaling)
	{
		const int32 TextureX = GetNearestImportSourceX(TargetX, TargetWidth, SourceRegion);
		const int32 TextureY = GetNearestImportSourceY(TargetY, TargetHeight, SourceRegion);
		const FColor Pixel = Pixels[TextureY * TextureWidth + TextureX];
		return bUseAlpha ? Pixel.A : static_cast<uint8>((static_cast<int32>(Pixel.R) + Pixel.G + Pixel.B) / 3);
	}

	const float SourceX = FMath::Clamp(GetImportSourceCoordinate(TargetX, TargetWidth, SourceRegion.Min.X, SourceRegion.Width()), static_cast<float>(SourceRegion.Min.X), static_cast<float>(SourceRegion.Max.X - 1));
	const float SourceY = FMath::Clamp(GetImportSourceCoordinate(TargetY, TargetHeight, SourceRegion.Min.Y, SourceRegion.Height()), static_cast<float>(SourceRegion.Min.Y), static_cast<float>(SourceRegion.Max.Y - 1));
	const int32 X0 = FMath::Clamp(FMath::FloorToInt(SourceX), SourceRegion.Min.X, SourceRegion.Max.X - 1);
	const int32 Y0 = FMath::Clamp(FMath::FloorToInt(SourceY), SourceRegion.Min.Y, SourceRegion.Max.Y - 1);
	const int32 X1 = FMath::Clamp(X0 + 1, SourceRegion.Min.X, SourceRegion.Max.X - 1);
	const int32 Y1 = FMath::Clamp(Y0 + 1, SourceRegion.Min.Y, SourceRegion.Max.Y - 1);
	const float LerpX = SourceX - X0;
	const float LerpY = SourceY - Y0;

	auto ReadSample = [&Pixels, TextureWidth, bUseAlpha](int32 X, int32 Y)
	{
		const FColor Pixel = Pixels[Y * TextureWidth + X];
		return static_cast<float>(bUseAlpha ? Pixel.A : static_cast<uint8>((static_cast<int32>(Pixel.R) + Pixel.G + Pixel.B) / 3));
	};

	const float Top = FMath::Lerp(ReadSample(X0, Y0), ReadSample(X1, Y0), LerpX);
	const float Bottom = FMath::Lerp(ReadSample(X0, Y1), ReadSample(X1, Y1), LerpX);
	return static_cast<uint8>(FMath::Clamp(FMath::RoundToInt(FMath::Lerp(Top, Bottom, LerpY)), 0, 255));
}

bool IsImportColorMatch(const FColor& Pixel, const FColor& Target, uint8 ToleranceByte)
{
	return FMath::Abs(static_cast<int32>(Pixel.R) - Target.R) <= ToleranceByte
		&& FMath::Abs(static_cast<int32>(Pixel.G) - Target.G) <= ToleranceByte
		&& FMath::Abs(static_cast<int32>(Pixel.B) - Target.B) <= ToleranceByte;
}

bool GetImportColorMatch(const TArray<FColor>& Pixels, int32 TextureWidth, const FIntRect& SourceRegion, int32 TargetX, int32 TargetY, int32 TargetWidth, int32 TargetHeight, const FColor& Target, uint8 ToleranceByte)
{
	const bool bUpscaling = TargetWidth > SourceRegion.Width() || TargetHeight > SourceRegion.Height();
	if (!bUpscaling)
	{
		const int32 TextureX = GetNearestImportSourceX(TargetX, TargetWidth, SourceRegion);
		const int32 TextureY = GetNearestImportSourceY(TargetY, TargetHeight, SourceRegion);
		return IsImportColorMatch(Pixels[TextureY * TextureWidth + TextureX], Target, ToleranceByte);
	}

	const float SourceX = FMath::Clamp(GetImportSourceCoordinate(TargetX, TargetWidth, SourceRegion.Min.X, SourceRegion.Width()), static_cast<float>(SourceRegion.Min.X), static_cast<float>(SourceRegion.Max.X - 1));
	const float SourceY = FMath::Clamp(GetImportSourceCoordinate(TargetY, TargetHeight, SourceRegion.Min.Y, SourceRegion.Height()), static_cast<float>(SourceRegion.Min.Y), static_cast<float>(SourceRegion.Max.Y - 1));
	const int32 X0 = FMath::Clamp(FMath::FloorToInt(SourceX), SourceRegion.Min.X, SourceRegion.Max.X - 1);
	const int32 Y0 = FMath::Clamp(FMath::FloorToInt(SourceY), SourceRegion.Min.Y, SourceRegion.Max.Y - 1);
	const int32 X1 = FMath::Clamp(X0 + 1, SourceRegion.Min.X, SourceRegion.Max.X - 1);
	const int32 Y1 = FMath::Clamp(Y0 + 1, SourceRegion.Min.Y, SourceRegion.Max.Y - 1);
	const float LerpX = SourceX - X0;
	const float LerpY = SourceY - Y0;

	auto MatchWeight = [&Pixels, TextureWidth, &Target, ToleranceByte](int32 X, int32 Y)
	{
		return IsImportColorMatch(Pixels[Y * TextureWidth + X], Target, ToleranceByte) ? 1.0f : 0.0f;
	};

	const float Top = FMath::Lerp(MatchWeight(X0, Y0), MatchWeight(X1, Y0), LerpX);
	const float Bottom = FMath::Lerp(MatchWeight(X0, Y1), MatchWeight(X1, Y1), LerpX);
	return FMath::Lerp(Top, Bottom, LerpY) >= 0.5f;
}

int32 GetNearestResampleSource(int32 Target, int32 TargetSize, int32 SourceSize)
{
	return FMath::Clamp(FMath::FloorToInt((static_cast<float>(Target) + 0.5f) * SourceSize / FMath::Max(1, TargetSize)), 0, SourceSize - 1);
}

uint8 GetResampledSolid(const TArray<uint8>& SourceMask, int32 SourceCellsX, int32 SourceCellsZ, int32 TargetX, int32 TargetZ, int32 TargetCellsX, int32 TargetCellsZ)
{
	const float SourceX = FMath::Clamp(GetImportSourceCoordinate(TargetX, TargetCellsX, 0, SourceCellsX), 0.0f, static_cast<float>(SourceCellsX - 1));
	const float SourceZ = FMath::Clamp(GetImportSourceCoordinate(TargetZ, TargetCellsZ, 0, SourceCellsZ), 0.0f, static_cast<float>(SourceCellsZ - 1));
	const int32 X0 = FMath::Clamp(FMath::FloorToInt(SourceX), 0, SourceCellsX - 1);
	const int32 Z0 = FMath::Clamp(FMath::FloorToInt(SourceZ), 0, SourceCellsZ - 1);
	const int32 X1 = FMath::Clamp(X0 + 1, 0, SourceCellsX - 1);
	const int32 Z1 = FMath::Clamp(Z0 + 1, 0, SourceCellsZ - 1);
	const float LerpX = SourceX - X0;
	const float LerpZ = SourceZ - Z0;

	auto ReadMask = [&SourceMask, SourceCellsX](int32 X, int32 Z)
	{
		return SourceMask[Z * SourceCellsX + X] != 0 ? 1.0f : 0.0f;
	};

	const float Bottom = FMath::Lerp(ReadMask(X0, Z0), ReadMask(X1, Z0), LerpX);
	const float Top = FMath::Lerp(ReadMask(X0, Z1), ReadMask(X1, Z1), LerpX);
	return FMath::Lerp(Bottom, Top, LerpZ) >= 0.5f ? 1 : 0;
}

FVector GetResampledSpawn(const FVector& SourceSpawn, float SourceWidth, float SourceHeight, float TargetWidth, float TargetHeight)
{
	FVector ResampledSpawn = SourceSpawn;
	const float SourceAlphaX = SourceWidth > 0.0f ? (SourceSpawn.X + SourceWidth * 0.5f) / SourceWidth : 0.5f;
	ResampledSpawn.X = FMath::Clamp(SourceAlphaX, 0.0f, 1.0f) * TargetWidth - TargetWidth * 0.5f;
	ResampledSpawn.Z = TargetHeight + (SourceSpawn.Z - SourceHeight);
	return ResampledSpawn;
}
}

UFortRogueTerrainMapDefinition::UFortRogueTerrainMapDefinition()
{
	Resize(CellsX, CellsZ);
	TextureLayers.SetNum(1);
	TextureLayers[0].LayerId = TEXT("Layer0");
	TextureLayers[0].FallbackColor = FLinearColor(0.43f, 0.34f, 0.21f, 1.0f);

	for (int32 X = 0; X < CellsX; ++X)
	{
		const float Alpha = CellsX > 1 ? static_cast<float>(X) / static_cast<float>(CellsX - 1) : 0.0f;
		const float Ridge = FMath::Sin(Alpha * 2.0f * PI * 1.7f + 0.35f) * 0.16f;
		const float Detail = FMath::Sin(Alpha * 2.0f * PI * 4.6f + 1.8f) * 0.055f;
		const float Bowl = -FMath::Square(Alpha - 0.5f) * 0.18f;
		const int32 SurfaceZ = FMath::Clamp(FMath::RoundToInt(CellsZ * (0.58f + Ridge + Detail + Bowl)), 1, CellsZ - 1);

		for (int32 Z = 0; Z < SurfaceZ; ++Z)
		{
			SolidMask[ToIndex(X, Z)] = 1;
		}
	}
}

void UFortRogueTerrainMapDefinition::PostLoad()
{
	Super::PostLoad();
	NormalizeMapData();
}

#if WITH_EDITOR
void UFortRogueTerrainMapDefinition::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	NormalizeMapData();
}
#endif

void UFortRogueTerrainMapDefinition::Resize(int32 NewCellsX, int32 NewCellsZ)
{
	CellsX = FMath::Max(1, NewCellsX);
	CellsZ = FMath::Max(1, NewCellsZ);
	SolidMask.SetNumZeroed(CellsX * CellsZ);
	TextureLayerMask.SetNumZeroed(CellsX * CellsZ);
}

void UFortRogueTerrainMapDefinition::NormalizeMapData()
{
	CellsX = FMath::Max(1, CellsX);
	CellsZ = FMath::Max(1, CellsZ);
	CellSize = FMath::Max(1.0f, CellSize);

	const int32 ExpectedCells = CellsX * CellsZ;
	SolidMask.SetNumZeroed(ExpectedCells);
	TextureLayerMask.SetNumZeroed(ExpectedCells);

	for (int32 Index = 0; Index < ExpectedCells; ++Index)
	{
		SolidMask[Index] = SolidMask[Index] != 0 ? 1 : 0;
		if (SolidMask[Index] == 0)
		{
			TextureLayerMask[Index] = 0;
		}
	}
}

void UFortRogueTerrainMapDefinition::ResizeResampled(int32 NewCellsX, int32 NewCellsZ)
{
	const int32 TargetCellsX = FMath::Max(1, NewCellsX);
	const int32 TargetCellsZ = FMath::Max(1, NewCellsZ);
	if (TargetCellsX == CellsX && TargetCellsZ == CellsZ)
	{
		return;
	}

	if (CellsX <= 0 || CellsZ <= 0 || SolidMask.Num() != CellsX * CellsZ || TextureLayerMask.Num() != CellsX * CellsZ)
	{
		Resize(TargetCellsX, TargetCellsZ);
		return;
	}

	const int32 SourceCellsX = CellsX;
	const int32 SourceCellsZ = CellsZ;
	const float SourceWidth = SourceCellsX * CellSize;
	const float SourceHeight = SourceCellsZ * CellSize;
	const TArray<uint8> SourceSolidMask = SolidMask;
	const TArray<uint8> SourceTextureLayerMask = TextureLayerMask;

	TArray<uint8> ResampledSolidMask;
	TArray<uint8> ResampledTextureLayerMask;
	ResampledSolidMask.SetNumZeroed(TargetCellsX * TargetCellsZ);
	ResampledTextureLayerMask.SetNumZeroed(TargetCellsX * TargetCellsZ);

	for (int32 Z = 0; Z < TargetCellsZ; ++Z)
	{
		for (int32 X = 0; X < TargetCellsX; ++X)
		{
			const int32 TargetIndex = Z * TargetCellsX + X;
			ResampledSolidMask[TargetIndex] = GetResampledSolid(SourceSolidMask, SourceCellsX, SourceCellsZ, X, Z, TargetCellsX, TargetCellsZ);
			if (ResampledSolidMask[TargetIndex] != 0)
			{
				const int32 SourceX = GetNearestResampleSource(X, TargetCellsX, SourceCellsX);
				const int32 SourceZ = GetNearestResampleSource(Z, TargetCellsZ, SourceCellsZ);
				ResampledTextureLayerMask[TargetIndex] = SourceTextureLayerMask[SourceZ * SourceCellsX + SourceX];
			}
		}
	}

	CellsX = TargetCellsX;
	CellsZ = TargetCellsZ;
	SolidMask = MoveTemp(ResampledSolidMask);
	TextureLayerMask = MoveTemp(ResampledTextureLayerMask);

	const float TargetWidth = CellsX * CellSize;
	const float TargetHeight = CellsZ * CellSize;
	PlayerSpawnLocal = GetResampledSpawn(PlayerSpawnLocal, SourceWidth, SourceHeight, TargetWidth, TargetHeight);
	EnemySpawnLocal = GetResampledSpawn(EnemySpawnLocal, SourceWidth, SourceHeight, TargetWidth, TargetHeight);
}

void UFortRogueTerrainMapDefinition::SetCellSizePreservingSpawns(float NewCellSize)
{
	NormalizeMapData();
	const float TargetCellSize = FMath::Max(1.0f, NewCellSize);
	if (FMath::IsNearlyEqual(CellSize, TargetCellSize))
	{
		return;
	}

	const float SourceWidth = CellsX * CellSize;
	const float SourceHeight = CellsZ * CellSize;
	const float TargetWidth = CellsX * TargetCellSize;
	const float TargetHeight = CellsZ * TargetCellSize;
	PlayerSpawnLocal = GetResampledSpawn(PlayerSpawnLocal, SourceWidth, SourceHeight, TargetWidth, TargetHeight);
	EnemySpawnLocal = GetResampledSpawn(EnemySpawnLocal, SourceWidth, SourceHeight, TargetWidth, TargetHeight);
	CellSize = TargetCellSize;
}

void UFortRogueTerrainMapDefinition::Clear(bool bSolid)
{
	SolidMask.Init(bSolid ? 1 : 0, CellsX * CellsZ);
	TextureLayerMask.Init(0, CellsX * CellsZ);
}

void UFortRogueTerrainMapDefinition::FillRect(int32 MinX, int32 MinZ, int32 MaxX, int32 MaxZ, bool bSolid)
{
	const int32 ClampedMinX = FMath::Clamp(FMath::Min(MinX, MaxX), 0, CellsX - 1);
	const int32 ClampedMaxX = FMath::Clamp(FMath::Max(MinX, MaxX), 0, CellsX - 1);
	const int32 ClampedMinZ = FMath::Clamp(FMath::Min(MinZ, MaxZ), 0, CellsZ - 1);
	const int32 ClampedMaxZ = FMath::Clamp(FMath::Max(MinZ, MaxZ), 0, CellsZ - 1);

	for (int32 Z = ClampedMinZ; Z <= ClampedMaxZ; ++Z)
	{
		for (int32 X = ClampedMinX; X <= ClampedMaxX; ++X)
		{
			const int32 Index = ToIndex(X, Z);
			SolidMask[Index] = bSolid ? 1 : 0;
			TextureLayerMask[Index] = 0;
		}
	}
}

void UFortRogueTerrainMapDefinition::FillTexturedRect(int32 MinX, int32 MinZ, int32 MaxX, int32 MaxZ, int32 LayerIndex)
{
	FillRect(MinX, MinZ, MaxX, MaxZ, true);
	ApplyTextureRect(MinX, MinZ, MaxX, MaxZ, LayerIndex);
}

void UFortRogueTerrainMapDefinition::ApplyCircle(int32 CenterX, int32 CenterZ, int32 RadiusCells, bool bSolid)
{
	const int32 Radius = FMath::Max(0, RadiusCells);
	const int32 RadiusSq = Radius * Radius;
	const int32 MinX = FMath::Clamp(CenterX - Radius, 0, CellsX - 1);
	const int32 MaxX = FMath::Clamp(CenterX + Radius, 0, CellsX - 1);
	const int32 MinZ = FMath::Clamp(CenterZ - Radius, 0, CellsZ - 1);
	const int32 MaxZ = FMath::Clamp(CenterZ + Radius, 0, CellsZ - 1);

	for (int32 Z = MinZ; Z <= MaxZ; ++Z)
	{
		for (int32 X = MinX; X <= MaxX; ++X)
		{
			const int32 DX = X - CenterX;
			const int32 DZ = Z - CenterZ;
			if (DX * DX + DZ * DZ <= RadiusSq)
			{
				const int32 Index = ToIndex(X, Z);
				SolidMask[Index] = bSolid ? 1 : 0;
				TextureLayerMask[Index] = 0;
			}
		}
	}
}

void UFortRogueTerrainMapDefinition::ApplyTexturedCircle(int32 CenterX, int32 CenterZ, int32 RadiusCells, int32 LayerIndex)
{
	ApplyCircle(CenterX, CenterZ, RadiusCells, true);
	ApplyTextureCircle(CenterX, CenterZ, RadiusCells, LayerIndex);
}

void UFortRogueTerrainMapDefinition::ApplyCircleStroke(int32 StartX, int32 StartZ, int32 EndX, int32 EndZ, int32 RadiusCells, bool bSolid)
{
	const int32 StepCount = FMath::Max(FMath::Abs(EndX - StartX), FMath::Abs(EndZ - StartZ));
	if (StepCount <= 0)
	{
		ApplyCircle(StartX, StartZ, RadiusCells, bSolid);
		return;
	}

	for (int32 Step = 0; Step <= StepCount; ++Step)
	{
		const float Alpha = static_cast<float>(Step) / static_cast<float>(StepCount);
		const int32 CenterX = FMath::RoundToInt(FMath::Lerp(static_cast<float>(StartX), static_cast<float>(EndX), Alpha));
		const int32 CenterZ = FMath::RoundToInt(FMath::Lerp(static_cast<float>(StartZ), static_cast<float>(EndZ), Alpha));
		ApplyCircle(CenterX, CenterZ, RadiusCells, bSolid);
	}
}

void UFortRogueTerrainMapDefinition::ApplyTexturedCircleStroke(int32 StartX, int32 StartZ, int32 EndX, int32 EndZ, int32 RadiusCells, int32 LayerIndex)
{
	const int32 StepCount = FMath::Max(FMath::Abs(EndX - StartX), FMath::Abs(EndZ - StartZ));
	if (StepCount <= 0)
	{
		ApplyTexturedCircle(StartX, StartZ, RadiusCells, LayerIndex);
		return;
	}

	for (int32 Step = 0; Step <= StepCount; ++Step)
	{
		const float Alpha = static_cast<float>(Step) / static_cast<float>(StepCount);
		const int32 CenterX = FMath::RoundToInt(FMath::Lerp(static_cast<float>(StartX), static_cast<float>(EndX), Alpha));
		const int32 CenterZ = FMath::RoundToInt(FMath::Lerp(static_cast<float>(StartZ), static_cast<float>(EndZ), Alpha));
		ApplyTexturedCircle(CenterX, CenterZ, RadiusCells, LayerIndex);
	}
}

void UFortRogueTerrainMapDefinition::SetTextureLayer(int32 LayerIndex, UTexture2D* Texture)
{
	if (LayerIndex < 0)
	{
		return;
	}

	const int32 ClampedLayerIndex = FMath::Clamp(LayerIndex, 0, 255);
	TextureLayers.SetNum(FMath::Max(TextureLayers.Num(), ClampedLayerIndex + 1));
	FFortRogueTerrainTextureLayer& Layer = TextureLayers[ClampedLayerIndex];
	Layer.LayerId = FName(*FString::Printf(TEXT("Layer%d"), ClampedLayerIndex));
	Layer.Texture = Texture;
	FLinearColor AverageTextureColor;
	if (TryGetAverageOpaqueTextureColor(Texture, AverageTextureColor))
	{
		Layer.FallbackColor = AverageTextureColor;
	}
}

void UFortRogueTerrainMapDefinition::ApplyTextureRect(int32 MinX, int32 MinZ, int32 MaxX, int32 MaxZ, int32 LayerIndex)
{
	const uint8 ClampedLayer = static_cast<uint8>(FMath::Clamp(LayerIndex, 0, 255));
	const int32 ClampedMinX = FMath::Clamp(FMath::Min(MinX, MaxX), 0, CellsX - 1);
	const int32 ClampedMaxX = FMath::Clamp(FMath::Max(MinX, MaxX), 0, CellsX - 1);
	const int32 ClampedMinZ = FMath::Clamp(FMath::Min(MinZ, MaxZ), 0, CellsZ - 1);
	const int32 ClampedMaxZ = FMath::Clamp(FMath::Max(MinZ, MaxZ), 0, CellsZ - 1);

	for (int32 Z = ClampedMinZ; Z <= ClampedMaxZ; ++Z)
	{
		for (int32 X = ClampedMinX; X <= ClampedMaxX; ++X)
		{
			const int32 Index = ToIndex(X, Z);
			if (SolidMask.IsValidIndex(Index) && SolidMask[Index] != 0)
			{
				TextureLayerMask[Index] = ClampedLayer;
			}
		}
	}
}

void UFortRogueTerrainMapDefinition::ApplyTextureCircle(int32 CenterX, int32 CenterZ, int32 RadiusCells, int32 LayerIndex)
{
	const uint8 ClampedLayer = static_cast<uint8>(FMath::Clamp(LayerIndex, 0, 255));
	const int32 Radius = FMath::Max(0, RadiusCells);
	const int32 RadiusSq = Radius * Radius;
	const int32 MinX = FMath::Clamp(CenterX - Radius, 0, CellsX - 1);
	const int32 MaxX = FMath::Clamp(CenterX + Radius, 0, CellsX - 1);
	const int32 MinZ = FMath::Clamp(CenterZ - Radius, 0, CellsZ - 1);
	const int32 MaxZ = FMath::Clamp(CenterZ + Radius, 0, CellsZ - 1);

	for (int32 Z = MinZ; Z <= MaxZ; ++Z)
	{
		for (int32 X = MinX; X <= MaxX; ++X)
		{
			const int32 DX = X - CenterX;
			const int32 DZ = Z - CenterZ;
			const int32 Index = ToIndex(X, Z);
			if (DX * DX + DZ * DZ <= RadiusSq && SolidMask.IsValidIndex(Index) && SolidMask[Index] != 0)
			{
				TextureLayerMask[Index] = ClampedLayer;
			}
		}
	}
}

void UFortRogueTerrainMapDefinition::ApplyTextureCircleStroke(int32 StartX, int32 StartZ, int32 EndX, int32 EndZ, int32 RadiusCells, int32 LayerIndex)
{
	const int32 StepCount = FMath::Max(FMath::Abs(EndX - StartX), FMath::Abs(EndZ - StartZ));
	if (StepCount <= 0)
	{
		ApplyTextureCircle(StartX, StartZ, RadiusCells, LayerIndex);
		return;
	}

	for (int32 Step = 0; Step <= StepCount; ++Step)
	{
		const float Alpha = static_cast<float>(Step) / static_cast<float>(StepCount);
		const int32 CenterX = FMath::RoundToInt(FMath::Lerp(static_cast<float>(StartX), static_cast<float>(EndX), Alpha));
		const int32 CenterZ = FMath::RoundToInt(FMath::Lerp(static_cast<float>(StartZ), static_cast<float>(EndZ), Alpha));
		ApplyTextureCircle(CenterX, CenterZ, RadiusCells, LayerIndex);
	}
}

bool UFortRogueTerrainMapDefinition::ImportSolidMaskFromTexture(UTexture2D* SourceTexture, bool bUseAlpha, float Threshold, int32 LayerIndex, bool bResizeToTexture)
{
	return ImportSolidMaskFromTextureRegion(SourceTexture, 0, 0, 0, 0, bUseAlpha, Threshold, LayerIndex, bResizeToTexture);
}

bool UFortRogueTerrainMapDefinition::ImportSolidMaskFromTextureByColor(UTexture2D* SourceTexture, FLinearColor TargetColor, float Tolerance, int32 LayerIndex, bool bResizeToTexture)
{
	return ImportSolidMaskFromTextureRegionByColor(SourceTexture, 0, 0, 0, 0, TargetColor, Tolerance, LayerIndex, bResizeToTexture);
}

bool UFortRogueTerrainMapDefinition::ImportSolidMaskFromTextureRegion(UTexture2D* SourceTexture, int32 SourceMinX, int32 SourceMinY, int32 SourceWidth, int32 SourceHeight, bool bUseAlpha, float Threshold, int32 LayerIndex, bool bResizeToRegion)
{
	TArray<FColor> Pixels;
	int32 TextureWidth = 0;
	int32 TextureHeight = 0;
	if (!ReadTerrainImportPixels(SourceTexture, Pixels, TextureWidth, TextureHeight))
	{
		return false;
	}

	FIntRect SourceRegion;
	if (!ClampImportRegion(TextureWidth, TextureHeight, SourceMinX, SourceMinY, SourceWidth, SourceHeight, SourceRegion))
	{
		return false;
	}

	const uint8 ClampedLayer = static_cast<uint8>(FMath::Clamp(LayerIndex, 0, 255));
	const uint8 ThresholdByte = static_cast<uint8>(FMath::Clamp(Threshold, 0.0f, 1.0f) * 255.0f);

	if (bResizeToRegion)
	{
		Resize(SourceRegion.Width(), SourceRegion.Height());
	}

	for (int32 TargetY = 0; TargetY < CellsZ; ++TargetY)
	{
		for (int32 X = 0; X < CellsX; ++X)
		{
			const uint8 Sample = GetImportMaskSample(Pixels, TextureWidth, SourceRegion, X, TargetY, CellsX, CellsZ, bUseAlpha);
			const bool bSolid = Sample >= ThresholdByte;
			const int32 Index = ToIndex(X, CellsZ - 1 - TargetY);
			SolidMask[Index] = bSolid ? 1 : 0;
			TextureLayerMask[Index] = bSolid ? ClampedLayer : 0;
		}
	}

	return true;
}

bool UFortRogueTerrainMapDefinition::ImportSolidMaskFromTextureRegionByColor(UTexture2D* SourceTexture, int32 SourceMinX, int32 SourceMinY, int32 SourceWidth, int32 SourceHeight, FLinearColor TargetColor, float Tolerance, int32 LayerIndex, bool bResizeToRegion)
{
	TArray<FColor> Pixels;
	int32 TextureWidth = 0;
	int32 TextureHeight = 0;
	if (!ReadTerrainImportPixels(SourceTexture, Pixels, TextureWidth, TextureHeight))
	{
		return false;
	}

	FIntRect SourceRegion;
	if (!ClampImportRegion(TextureWidth, TextureHeight, SourceMinX, SourceMinY, SourceWidth, SourceHeight, SourceRegion))
	{
		return false;
	}

	const uint8 ClampedLayer = static_cast<uint8>(FMath::Clamp(LayerIndex, 0, 255));
	const FColor Target = TargetColor.ToFColor(true);
	const uint8 ToleranceByte = static_cast<uint8>(FMath::Clamp(Tolerance, 0.0f, 1.0f) * 255.0f);

	if (bResizeToRegion)
	{
		Resize(SourceRegion.Width(), SourceRegion.Height());
	}

	for (int32 TargetY = 0; TargetY < CellsZ; ++TargetY)
	{
		for (int32 X = 0; X < CellsX; ++X)
		{
			const bool bSolid = GetImportColorMatch(Pixels, TextureWidth, SourceRegion, X, TargetY, CellsX, CellsZ, Target, ToleranceByte);
			const int32 Index = ToIndex(X, CellsZ - 1 - TargetY);
			SolidMask[Index] = bSolid ? 1 : 0;
			TextureLayerMask[Index] = bSolid ? ClampedLayer : 0;
		}
	}

	return true;
}

bool UFortRogueTerrainMapDefinition::IsValidCell(int32 X, int32 Z) const
{
	return X >= 0 && X < CellsX && Z >= 0 && Z < CellsZ;
}

int32 UFortRogueTerrainMapDefinition::ToIndex(int32 X, int32 Z) const
{
	return Z * CellsX + X;
}
