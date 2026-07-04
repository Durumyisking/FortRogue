// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/FRTerrainMovementComponent.h"

#include "Combat/FRDestructibleTerrain.h"
#include "EngineUtils.h"
#include "FRGameMode.h"
#include "GameFramework/Actor.h"

namespace
{
constexpr float DefaultMaxCharacterSlopeDegrees = 45.0f;
constexpr float MaxUsableSlopeDegrees = 89.0f;
}

UFRTerrainMovementComponent::UFRTerrainMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFRTerrainMovementComponent::SetTerrain(AFRDestructibleTerrain* InTerrain)
{
	AssignedTerrain = InTerrain;
	const AActor* Owner = GetOwner();
	if (Owner && Owner->HasActorBegunPlay())
	{
		SnapToTerrain();
	}
}

AFRDestructibleTerrain* UFRTerrainMovementComponent::FindTerrain() const
{
	if (AssignedTerrain)
	{
		return AssignedTerrain;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<AFRDestructibleTerrain> It(World); It; ++It)
	{
		return *It;
	}

	return nullptr;
}

FVector UFRTerrainMovementComponent::GetOwnerLocation() const
{
	const AActor* Owner = GetOwner();
	return Owner ? Owner->GetActorLocation() : FVector::ZeroVector;
}

void UFRTerrainMovementComponent::SetOwnerLocation(const FVector& NewLocation)
{
	if (AActor* Owner = GetOwner())
	{
		Owner->SetActorLocation(NewLocation);
	}
}

bool UFRTerrainMovementComponent::IsSupportedByTerrain() const
{
	AFRDestructibleTerrain* Terrain = FindTerrain();
	if (!Terrain)
	{
		return true;
	}

	const FVector CurrentLocation = GetOwnerLocation();
	const float ClampedWorldX = ClampWorldXToTerrainBounds(*Terrain, CurrentLocation.X);
	const float CurrentFootZ = CurrentLocation.Z - FootOffsetZ;
	float SurfaceZ = 0.0f;
	return FindFootprintSurfaceZ(*Terrain, ClampedWorldX, CurrentFootZ + GroundSnapDistance, GroundSnapDistance + 1.0f, SurfaceZ);
}

float UFRTerrainMovementComponent::MoveHorizontal(float RequestedDelta)
{
	if (FMath::IsNearlyZero(RequestedDelta))
	{
		return 0.0f;
	}

	FVector NewLocation = GetOwnerLocation();
	float ActualDelta = 0.0f;
	bool bLostSupport = false;

	if (AFRDestructibleTerrain* Terrain = FindTerrain())
	{
		const float StepLength = FMath::Max(1.0f, Terrain->CellSize * 0.5f);
		const int32 StepCount = FMath::Max(1, FMath::CeilToInt(FMath::Abs(RequestedDelta) / StepLength));
		const float StepDelta = RequestedDelta / static_cast<float>(StepCount);

		for (int32 Step = 0; Step < StepCount; ++Step)
		{
			FVector StepLocation = NewLocation;
			StepLocation.X += StepDelta;
			StepLocation.X = ClampWorldXToTerrainBounds(*Terrain, StepLocation.X);
			const float ClampedStepDelta = StepLocation.X - NewLocation.X;
			if (FMath::IsNearlyZero(ClampedStepDelta))
			{
				break;
			}

			const float CurrentFootZ = NewLocation.Z - FootOffsetZ;
			float SurfaceZ = 0.0f;
			if (FindFootprintSurfaceZ(*Terrain, StepLocation.X, CurrentFootZ + MaxStepUp, MaxStepUp + MaxStepDown, SurfaceZ))
			{
				if (!IsSlopeTraversable(CurrentFootZ, SurfaceZ, ClampedStepDelta, Terrain->CellSize))
				{
					break;
				}

				StepLocation.Z = SurfaceZ + FootOffsetZ;
				if (!TryResolveFootprintBlock(*Terrain, StepLocation, SurfaceZ))
				{
					break;
				}
				VerticalVelocity = 0.0f;
			}
			else if (IsFootprintBlocked(*Terrain, StepLocation, CurrentFootZ))
			{
				break;
			}
			else
			{
				NewLocation = StepLocation;
				ActualDelta += ClampedStepDelta;
				bLostSupport = true;
				break;
			}

			NewLocation = StepLocation;
			ActualDelta += ClampedStepDelta;
		}
	}
	else
	{
		NewLocation.X += RequestedDelta;
		ActualDelta = RequestedDelta;
	}

	if (FMath::IsNearlyZero(ActualDelta))
	{
		return 0.0f;
	}

	SetOwnerLocation(NewLocation);
	if (bLostSupport)
	{
		ReevaluateSupport();
	}
	else
	{
		UpdateSurfaceAlignment(FindTerrain());
	}
	return ActualDelta;
}

void UFRTerrainMovementComponent::ReevaluateSupport()
{
	AFRDestructibleTerrain* Terrain = FindTerrain();
	if (!Terrain)
	{
		return;
	}

	const FVector CurrentLocation = GetOwnerLocation();
	const float ClampedWorldX = ClampWorldXToTerrainBounds(*Terrain, CurrentLocation.X);
	const float CurrentFootZ = CurrentLocation.Z - FootOffsetZ;
	float SurfaceZ = 0.0f;
	if (FindFootprintSurfaceZ(*Terrain, ClampedWorldX, CurrentFootZ + GroundSnapDistance, GroundSnapDistance + MaxStepDown, SurfaceZ))
	{
		SetOwnerLocation(FVector(ClampedWorldX, CurrentLocation.Y, SurfaceZ + FootOffsetZ));
		VerticalVelocity = 0.0f;
		UpdateSurfaceAlignment(Terrain);
		return;
	}

	if (!FMath::IsNearlyEqual(ClampedWorldX, static_cast<float>(CurrentLocation.X)))
	{
		SetOwnerLocation(FVector(ClampedWorldX, CurrentLocation.Y, CurrentLocation.Z));
	}
	OnSupportLost.ExecuteIfBound();
	TickGravity(1.0f / 60.0f);
}

void UFRTerrainMovementComponent::ApplyKnockback(float HorizontalDistance, const FVector& ImpactLocation, const FVector& ImpactVelocity)
{
	if (HorizontalDistance <= 0.0f)
	{
		return;
	}

	FVector NewLocation = GetOwnerLocation();
	float Direction = FMath::Sign(NewLocation.X - ImpactLocation.X);
	if (FMath::IsNearlyZero(Direction))
	{
		Direction = ImpactVelocity.X >= 0.0f ? 1.0f : -1.0f;
	}
	NewLocation.X += Direction * HorizontalDistance;
	if (AFRDestructibleTerrain* Terrain = FindTerrain())
	{
		NewLocation.X = ClampWorldXToTerrainBounds(*Terrain, NewLocation.X);
	}
	SetOwnerLocation(NewLocation);
	ReevaluateSupport();
}

void UFRTerrainMovementComponent::TickGravity(float DeltaSeconds)
{
	if (CanApplyGravity.IsBound() && !CanApplyGravity.Execute())
	{
		return;
	}

	AFRDestructibleTerrain* Terrain = FindTerrain();
	if (!Terrain)
	{
		return;
	}

	const FVector CurrentLocation = GetOwnerLocation();
	const float ClampedWorldX = ClampWorldXToTerrainBounds(*Terrain, CurrentLocation.X);
	const float CurrentFootZ = CurrentLocation.Z - FootOffsetZ;
	float SurfaceZ = 0.0f;
	if (FindFootprintSurfaceZ(*Terrain, ClampedWorldX, CurrentFootZ + GroundSnapDistance, GroundSnapDistance + 1.0f, SurfaceZ))
	{
		SetOwnerLocation(FVector(ClampedWorldX, CurrentLocation.Y, SurfaceZ + FootOffsetZ));
		VerticalVelocity = 0.0f;
		UpdateSurfaceAlignment(Terrain);
		return;
	}

	OnSupportLost.ExecuteIfBound();
	VerticalVelocity = FMath::Max(VerticalVelocity - GravityAcceleration * DeltaSeconds, -MaxFallSpeed);
	const float FallDistance = FMath::Abs(VerticalVelocity * DeltaSeconds);
	if (FindFootprintSurfaceZ(*Terrain, ClampedWorldX, CurrentFootZ, FallDistance + GroundSnapDistance, SurfaceZ))
	{
		SetOwnerLocation(FVector(ClampedWorldX, CurrentLocation.Y, SurfaceZ + FootOffsetZ));
		VerticalVelocity = 0.0f;
		UpdateSurfaceAlignment(Terrain);
		return;
	}

	SetOwnerLocation(FVector(ClampedWorldX, CurrentLocation.Y, CurrentLocation.Z + VerticalVelocity * DeltaSeconds));
	UpdateSurfaceAlignment(nullptr);
	CheckFallDeath();
}

bool UFRTerrainMovementComponent::CheckFallDeath()
{
	AFRDestructibleTerrain* Terrain = FindTerrain();
	if (!Terrain)
	{
		return false;
	}

	if (GetOwnerLocation().Z >= Terrain->GetActorLocation().Z - FallDeathDepth)
	{
		return false;
	}

	VerticalVelocity = 0.0f;
	OnFellToDeath.ExecuteIfBound();
	return true;
}

void UFRTerrainMovementComponent::SnapToTerrain()
{
	AFRDestructibleTerrain* Terrain = FindTerrain();
	if (!Terrain)
	{
		return;
	}

	const FVector CurrentLocation = GetOwnerLocation();
	const float ClampedWorldX = ClampWorldXToTerrainBounds(*Terrain, CurrentLocation.X);
	float SurfaceZ = 0.0f;
	if (FindFootprintSurfaceZ(*Terrain, ClampedWorldX, CurrentLocation.Z, 2000.0f, SurfaceZ))
	{
		SetOwnerLocation(FVector(ClampedWorldX, CurrentLocation.Y, SurfaceZ + FootOffsetZ));
		VerticalVelocity = 0.0f;
		UpdateSurfaceAlignment(Terrain);
	}
}

bool UFRTerrainMovementComponent::FindFootprintSurfaceZ(const AFRDestructibleTerrain& Terrain, float CenterWorldX, float StartWorldZ, float SearchDistance, float& OutSurfaceZ) const
{
	bool bFoundSurface = false;
	float BestSurfaceZ = -TNumericLimits<float>::Max();
	const float SampleOffsets[] = { -FootProbeHalfWidth, 0.0f, FootProbeHalfWidth };
	for (const float SampleOffset : SampleOffsets)
	{
		float SampleSurfaceZ = 0.0f;
		if (Terrain.FindSurfaceZAtWorldX(CenterWorldX + SampleOffset, StartWorldZ, SearchDistance, SampleSurfaceZ))
		{
			BestSurfaceZ = FMath::Max(BestSurfaceZ, SampleSurfaceZ);
			bFoundSurface = true;
		}
	}

	if (bFoundSurface)
	{
		OutSurfaceZ = BestSurfaceZ;
	}

	return bFoundSurface;
}

bool UFRTerrainMovementComponent::IsFootprintBlocked(const AFRDestructibleTerrain& Terrain, const FVector& CenterLocation, float FootWorldZ) const
{
	const float SampleOffsets[] = { -FootProbeHalfWidth, 0.0f, FootProbeHalfWidth };
	for (const float SampleOffset : SampleOffsets)
	{
		const FVector FootProbe(CenterLocation.X + SampleOffset, CenterLocation.Y, FootWorldZ + 1.0f);
		const FVector BodyProbe(CenterLocation.X + SampleOffset, CenterLocation.Y, FootWorldZ + FootOffsetZ * 0.5f);
		if (Terrain.IsSolidAtWorldLocation(FootProbe) || Terrain.IsSolidAtWorldLocation(BodyProbe))
		{
			return true;
		}
	}

	return false;
}

bool UFRTerrainMovementComponent::TryResolveFootprintBlock(const AFRDestructibleTerrain& Terrain, FVector& InOutCenterLocation, float& InOutFootWorldZ) const
{
	if (!IsFootprintBlocked(Terrain, InOutCenterLocation, InOutFootWorldZ))
	{
		return true;
	}

	const float LiftStep = FMath::Max(1.0f, Terrain.CellSize);
	for (float Lift = LiftStep; Lift <= MaxStepUp + KINDA_SMALL_NUMBER; Lift += LiftStep)
	{
		const float CandidateFootWorldZ = InOutFootWorldZ + Lift;
		FVector CandidateLocation = InOutCenterLocation;
		CandidateLocation.Z = CandidateFootWorldZ + FootOffsetZ;
		if (!IsFootprintBlocked(Terrain, CandidateLocation, CandidateFootWorldZ))
		{
			InOutCenterLocation = CandidateLocation;
			InOutFootWorldZ = CandidateFootWorldZ;
			return true;
		}
	}

	return false;
}

bool UFRTerrainMovementComponent::IsSlopeTraversable(float CurrentFootWorldZ, float NextSurfaceWorldZ, float HorizontalDistance, float TerrainCellSize) const
{
	if (NextSurfaceWorldZ <= CurrentFootWorldZ)
	{
		return true;
	}

	const float HorizontalSpan = FMath::Max(FMath::Abs(HorizontalDistance), FMath::Max(TerrainCellSize, 1.0f));
	const float MaxVerticalDelta = FMath::Tan(FMath::DegreesToRadians(GetMaxCharacterSlopeDegrees())) * HorizontalSpan;
	return NextSurfaceWorldZ - CurrentFootWorldZ <= MaxVerticalDelta + KINDA_SMALL_NUMBER;
}

float UFRTerrainMovementComponent::GetMaxCharacterSlopeDegrees() const
{
	const AFRGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFRGameMode>() : nullptr;
	const float MaxSlopeDegrees = GameMode ? GameMode->GetMaxCharacterSlopeDegrees() : DefaultMaxCharacterSlopeDegrees;
	return FMath::Clamp(MaxSlopeDegrees, 0.0f, MaxUsableSlopeDegrees);
}

float UFRTerrainMovementComponent::ClampWorldXToTerrainBounds(const AFRDestructibleTerrain& Terrain, float WorldX) const
{
	const float HalfWidth = Terrain.Width * 0.5f;
	const float EdgePadding = FMath::Clamp(FootProbeHalfWidth, 0.0f, HalfWidth);
	const float MinX = Terrain.GetActorLocation().X - HalfWidth + EdgePadding;
	const float MaxX = Terrain.GetActorLocation().X + HalfWidth - EdgePadding;
	if (MinX > MaxX)
	{
		return Terrain.GetActorLocation().X;
	}

	return FMath::Clamp(WorldX, MinX, MaxX);
}

void UFRTerrainMovementComponent::UpdateSurfaceAlignment(const AFRDestructibleTerrain* Terrain)
{
	if (!Terrain)
	{
		OnSurfacePitchChanged.ExecuteIfBound(0.0f);
		return;
	}

	const FVector CurrentLocation = GetOwnerLocation();
	const float CurrentFootZ = CurrentLocation.Z - FootOffsetZ;
	const float ProbeHalfWidth = FMath::Max(BodySlopeProbeHalfWidth, Terrain->CellSize);
	float LeftSurfaceZ = 0.0f;
	float RightSurfaceZ = 0.0f;
	const bool bLeftSurface = Terrain->FindSurfaceZAtWorldX(CurrentLocation.X - ProbeHalfWidth, CurrentFootZ + GroundSnapDistance, GroundSnapDistance + MaxStepDown, LeftSurfaceZ);
	const bool bRightSurface = Terrain->FindSurfaceZAtWorldX(CurrentLocation.X + ProbeHalfWidth, CurrentFootZ + GroundSnapDistance, GroundSnapDistance + MaxStepDown, RightSurfaceZ);
	if (!bLeftSurface || !bRightSurface)
	{
		OnSurfacePitchChanged.ExecuteIfBound(0.0f);
		return;
	}

	const float SurfaceDeltaZ = RightSurfaceZ - LeftSurfaceZ;
	if (FMath::Abs(SurfaceDeltaZ) <= BodySlopeVisualDeadZoneHeight)
	{
		OnSurfacePitchChanged.ExecuteIfBound(0.0f);
		return;
	}

	const float SlopeAngleDegrees = FMath::RadiansToDegrees(FMath::Atan2(SurfaceDeltaZ, ProbeHalfWidth * 2.0f));
	const float MaxSlopeDegrees = GetMaxCharacterSlopeDegrees();
	OnSurfacePitchChanged.ExecuteIfBound(FMath::Clamp(SlopeAngleDegrees, -MaxSlopeDegrees, MaxSlopeDegrees));
}
