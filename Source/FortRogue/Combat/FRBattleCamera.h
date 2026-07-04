// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Camera/CameraActor.h"
#include "CoreMinimal.h"
#include "FRBattleCamera.generated.h"

class AFRDestructibleTerrain;

UENUM()
enum class EFRBattleCameraControlMode : uint8
{
	Auto,
	Manual
};

/**
 * 전투 카메라 액터입니다. 자동 추적(턴/투사체 포커스)과 방향키 수동 이동을 스스로 처리하며,
 * 지형 경계를 벗어나지 않게 위치를 보정합니다. GameMode는 포커스 요청과 입력 전달만 합니다.
 */
UCLASS()
class FORTROGUE_API AFRBattleCamera : public ACameraActor
{
	GENERATED_BODY()

public:
	AFRBattleCamera(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void Tick(float DeltaSeconds) override;

	void ConfigureBattle(const FVector& InBaseLocation, float InOrthoWidth, float InFollowInterpSpeed, float InManualPanSpeed);
	void SetTerrainActor(AFRDestructibleTerrain* InTerrain);
	void RequestAutoFocus(AActor* FocusActor, const FVector& FocusLocation, float ZOffset);
	void HandleManualInput(const FVector2D& InputAxis, bool bAnyDirectionKeyDown, float DeltaSeconds);

	EFRBattleCameraControlMode GetControlMode() const { return ControlMode; }
	FVector GetManualLocation() const { return ManualLocation; }
	bool RequiresManualInputRelease() const { return bManualInputRequiresRelease; }
	static FRotator GetBattleRotation();

private:
	FVector GetDesiredLocation() const;
	FVector GetFocusLocation() const;
	FVector ClampToTerrainBounds(const FVector& DesiredLocation) const;
	float GetOrthoWidthSafe() const;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Camera", meta = (ToolTip = "카메라가 목표 위치로 따라가는 보간 속도입니다. 값이 클수록 빠르게 따라갑니다."))
	float FollowInterpSpeed = 4.5f;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Camera", meta = (ClampMin = "0.0", ToolTip = "방향키로 수동 카메라를 이동하는 초당 월드 거리입니다."))
	float ManualPanSpeed = 900.0f;

	TWeakObjectPtr<AFRDestructibleTerrain> Terrain;
	FVector BaseLocation = FVector(0.0f, 3000.0f, 860.0f);
	EFRBattleCameraControlMode ControlMode = EFRBattleCameraControlMode::Auto;
	TWeakObjectPtr<AActor> AutoFocusActor;
	FVector AutoFocusLocation = FVector::ZeroVector;
	float AutoFocusZOffset = 0.0f;
	FVector ManualLocation = FVector::ZeroVector;
	bool bManualInputHeld = false;
	bool bManualInputRequiresRelease = false;
};
