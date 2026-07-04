// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "FRTerrainMovementComponent.generated.h"

class AFRDestructibleTerrain;

/**
 * 파괴 가능 지형 위에서의 캐릭터 이동 물리를 담당하는 컴포넌트입니다.
 * 발판 탐색, 경사 판정, 스텝 이동, 중력 낙하, 지형 스냅, 낙사 판정을 처리합니다.
 * 소유 액터의 시각적 반응(몸체 기울기, 차지 취소, 낙사 피해)은 델리게이트로 알립니다.
 */
UCLASS(ClassGroup = (FortRogue), meta = (BlueprintSpawnableComponent))
class FORTROGUE_API UFRTerrainMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFRTerrainMovementComponent();

	/** 지형 위 몸체 기울기가 갱신될 때 pitch(도)를 전달합니다. */
	TDelegate<void(float)> OnSurfacePitchChanged;
	/** 바인딩되어 있으면 중력 처리 전 확인합니다. false를 돌려주면 그 틱의 중력을 건너뜁니다(예: 사망 상태). */
	TDelegate<bool()> CanApplyGravity;
	/** 발밑 지지가 사라져 낙하가 시작될 때 호출됩니다. */
	FSimpleDelegate OnSupportLost;
	/** 지형 아래로 낙사 깊이를 넘었을 때 호출됩니다. */
	FSimpleDelegate OnFellToDeath;

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Terrain")
	void SetTerrain(AFRDestructibleTerrain* InTerrain);

	AFRDestructibleTerrain* FindTerrain() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Terrain")
	bool IsSupportedByTerrain() const;

	/** 지형 스텝 규칙을 적용해 수평 이동합니다. 실제로 움직인 거리를 돌려줍니다. */
	float MoveHorizontal(float RequestedDelta);

	/** 지지가 없으면 낙하를 진행합니다. 낙사 판정도 여기서 일어납니다. */
	void TickGravity(float DeltaSeconds);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Terrain")
	void SnapToTerrain();

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Terrain")
	void ReevaluateSupport();

	/** 폭발 지점 반대 방향으로 밀어낸 뒤 지지를 재평가합니다. */
	void ApplyKnockback(float HorizontalDistance, const FVector& ImpactLocation, const FVector& ImpactVelocity);

	float GetFootOffsetZ() const { return FootOffsetZ; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (ClampMin = "0.0", ToolTip = "턴 중 좌우 이동 속도입니다. 월드 단위/초 기준입니다."))
	float MoveSpeed = 260.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (ClampMin = "0.0", ToolTip = "캐릭터 위치에서 발 접지점을 찾을 때 아래로 내리는 Z 오프셋입니다."))
	float FootOffsetZ = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (ClampMin = "0.0", ToolTip = "발판 판정을 위해 캐릭터 중심 좌우로 검사할 반폭입니다."))
	float FootProbeHalfWidth = 22.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (ClampMin = "0.0", ToolTip = "한 번에 올라갈 수 있는 최대 지형 높이입니다. 이보다 높은 턱은 이동을 막습니다."))
	float MaxStepUp = 34.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (ClampMin = "0.0", ToolTip = "한 번에 내려갈 수 있는 최대 지형 높이입니다. 이보다 깊으면 낙하 상태로 처리됩니다."))
	float MaxStepDown = 56.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (ClampMin = "0.0", ToolTip = "접지 중 지형 표면에 붙이기 위해 허용하는 최대 보정 거리입니다."))
	float GroundSnapDistance = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (ClampMin = "0.0", ToolTip = "몸체 기울기를 계산할 때 캐릭터 중심 좌우로 검사할 반폭입니다."))
	float BodySlopeProbeHalfWidth = 28.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (ClampMin = "0.0", ToolTip = "시각적인 몸체 기울기에서 무시할 좌우 지형 높이 차이입니다. 작은 셀 단위 요철이 캐릭터 회전으로 보이지 않게 합니다."))
	float BodySlopeVisualDeadZoneHeight = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (ClampMin = "0.0", ToolTip = "지형 지지가 없을 때 적용할 낙하 가속도입니다."))
	float GravityAcceleration = 980.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (ClampMin = "0.0", ToolTip = "낙하 중 도달할 수 있는 최대 하강 속도입니다."))
	float MaxFallSpeed = 1600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (ClampMin = "0.0", ToolTip = "지형 아래로 이 깊이 이상 떨어지면 패배 처리할 기준 거리입니다."))
	float FallDeathDepth = 400.0f;

private:
	bool FindFootprintSurfaceZ(const AFRDestructibleTerrain& Terrain, float CenterWorldX, float StartWorldZ, float SearchDistance, float& OutSurfaceZ) const;
	bool IsFootprintBlocked(const AFRDestructibleTerrain& Terrain, const FVector& CenterLocation, float FootWorldZ) const;
	bool TryResolveFootprintBlock(const AFRDestructibleTerrain& Terrain, FVector& InOutCenterLocation, float& InOutFootWorldZ) const;
	bool IsSlopeTraversable(float CurrentFootWorldZ, float NextSurfaceWorldZ, float HorizontalDistance, float TerrainCellSize) const;
	float GetMaxCharacterSlopeDegrees() const;
	float ClampWorldXToTerrainBounds(const AFRDestructibleTerrain& Terrain, float WorldX) const;
	void UpdateSurfaceAlignment(const AFRDestructibleTerrain* Terrain);
	bool CheckFallDeath();
	FVector GetOwnerLocation() const;
	void SetOwnerLocation(const FVector& NewLocation);

	UPROPERTY()
	TObjectPtr<AFRDestructibleTerrain> AssignedTerrain;

	float VerticalVelocity = 0.0f;
};
