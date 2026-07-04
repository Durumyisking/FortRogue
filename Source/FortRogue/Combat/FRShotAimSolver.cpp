// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/FRShotAimSolver.h"

namespace
{
float GetTrajectorySegmentAlphaXZ(const FVector& StartLocation, const FVector& EndLocation, const FVector& TestLocation)
{
	const FVector2D Segment(EndLocation.X - StartLocation.X, EndLocation.Z - StartLocation.Z);
	const FVector2D ToTest(TestLocation.X - StartLocation.X, TestLocation.Z - StartLocation.Z);
	const float SegmentLengthSq = Segment.SizeSquared();
	if (SegmentLengthSq <= KINDA_SMALL_NUMBER)
	{
		return 0.0f;
	}

	return FMath::Clamp(FVector2D::DotProduct(ToTest, Segment) / SegmentLengthSq, 0.0f, 1.0f);
}

FVector GetClosestPointOnTrajectorySegmentXZ(const FVector& StartLocation, const FVector& EndLocation, const FVector& TestLocation)
{
	return FMath::Lerp(StartLocation, EndLocation, GetTrajectorySegmentAlphaXZ(StartLocation, EndLocation, TestLocation));
}

float GetTrajectoryDistanceSquaredXZ(const FVector& First, const FVector& Second)
{
	return FVector2D(First.X - Second.X, First.Z - Second.Z).SizeSquared();
}
}

bool FRShotAimSolver::SolveBallisticAim(const FFRShotAimSolverParams& Params, float& OutAimAngle, float& OutShotPower)
{
	if (!Params.BuildShotSpec || !Params.GetLaunchDirection || !Params.GetSpawnLocation)
	{
		return false;
	}

	float BestScore = MAX_flt;
	const int32 AimSamples = FMath::Max(1, Params.AimSamples);
	const int32 PowerSamples = FMath::Max(1, Params.PowerSamples);

	for (int32 AimSampleIndex = 0; AimSampleIndex <= AimSamples; ++AimSampleIndex)
	{
		const float AimAlpha = static_cast<float>(AimSampleIndex) / static_cast<float>(AimSamples);
		const float CandidateAimAngle = FMath::Lerp(Params.MinAimAngle, Params.MaxAimAngle, AimAlpha);
		for (int32 PowerSampleIndex = 0; PowerSampleIndex <= PowerSamples; ++PowerSampleIndex)
		{
			const float PowerAlpha = static_cast<float>(PowerSampleIndex) / static_cast<float>(PowerSamples);
			const float CandidateShotPower = FMath::Lerp(Params.MinShotPower, Params.MaxShotPower, PowerAlpha);

			const FFRShotSpec CandidateShotSpec = Params.BuildShotSpec(CandidateAimAngle, CandidateShotPower);
			if (CandidateShotSpec.LaunchSpeed <= KINDA_SMALL_NUMBER)
			{
				continue;
			}

			const FVector LaunchDirection = Params.GetLaunchDirection(CandidateAimAngle);
			FVector SimulatedLocation = Params.GetSpawnLocation(LaunchDirection);
			FVector SimulatedVelocity = LaunchDirection * CandidateShotSpec.LaunchSpeed;
			float CandidateScore = GetTrajectoryDistanceSquaredXZ(SimulatedLocation, Params.TargetLocation);
			for (int32 StepIndex = 0; StepIndex < Params.SimulationSteps; ++StepIndex)
			{
				const FVector PreviousLocation = SimulatedLocation;
				SimulatedVelocity += FVector(Params.Wind, 0.0f, -CandidateShotSpec.Gravity) * Params.SimulationDeltaSeconds;
				SimulatedLocation += SimulatedVelocity * Params.SimulationDeltaSeconds;
				const FVector ClosestPoint = GetClosestPointOnTrajectorySegmentXZ(PreviousLocation, SimulatedLocation, Params.TargetLocation);
				CandidateScore = FMath::Min(CandidateScore, GetTrajectoryDistanceSquaredXZ(ClosestPoint, Params.TargetLocation));

				const float HorizontalOvershoot = Params.bFacingRight ? SimulatedLocation.X - Params.TargetLocation.X : Params.TargetLocation.X - SimulatedLocation.X;
				const bool bMovingPastTarget = Params.bFacingRight ? SimulatedVelocity.X > 0.0f : SimulatedVelocity.X < 0.0f;
				if ((SimulatedLocation.Z < Params.TargetLocation.Z - 2000.0f && SimulatedVelocity.Z < 0.0f) || (HorizontalOvershoot > 1400.0f && bMovingPastTarget))
				{
					break;
				}
			}

			if (CandidateScore < BestScore)
			{
				BestScore = CandidateScore;
				OutAimAngle = CandidateAimAngle;
				OutShotPower = CandidateShotPower;
			}
		}
	}

	return BestScore < MAX_flt;
}
