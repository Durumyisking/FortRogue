// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Combat/FRShotSpec.h"
#include "CoreMinimal.h"

/**
 * 적 AI가 목표를 맞추기 위한 조준 각도/파워 탐색기입니다.
 * 캐릭터 상태를 변경하지 않는 순수 탐색으로, 후보 샷 스펙과 발사 지오메트리는 콜백으로 받습니다.
 */
struct FFRShotAimSolverParams
{
	FVector TargetLocation = FVector::ZeroVector;
	float Wind = 0.0f;
	bool bFacingRight = true;
	float MinAimAngle = 0.0f;
	float MaxAimAngle = 90.0f;
	float MinShotPower = 0.0f;
	float MaxShotPower = 1.0f;
	int32 AimSamples = 24;
	int32 PowerSamples = 24;
	int32 SimulationSteps = 150;
	float SimulationDeltaSeconds = 1.0f / 30.0f;

	/** 후보 조준 각도/파워로 만든 샷 스펙을 돌려줍니다. modifier 조건이 각도에 반응하도록 후보마다 호출됩니다. */
	TFunction<FFRShotSpec(float AimAngle, float ShotPower)> BuildShotSpec;
	/** 후보 조준 각도의 발사 방향(단위 벡터)을 돌려줍니다. */
	TFunction<FVector(float AimAngle)> GetLaunchDirection;
	/** 발사 방향에 대한 투사체 스폰 위치를 돌려줍니다. */
	TFunction<FVector(const FVector& LaunchDirection)> GetSpawnLocation;
};

namespace FRShotAimSolver
{
	/** 탄도 시뮬레이션으로 목표에 가장 근접하는 조준을 찾습니다. 찾으면 true를 돌려줍니다. */
	FORTROGUE_API bool SolveBallisticAim(const FFRShotAimSolverParams& Params, float& OutAimAngle, float& OutShotPower);
}
