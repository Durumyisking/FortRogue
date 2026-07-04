// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/FRBattleCamera.h"

#include "Camera/CameraComponent.h"
#include "Combat/FRDestructibleTerrain.h"

namespace
{
constexpr float TerrainCameraPadding = 240.0f;
const FRotator BattleCameraRotation(0.0f, -90.0f, 0.0f);
}

AFRBattleCamera::AFRBattleCamera(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
}

FRotator AFRBattleCamera::GetBattleRotation()
{
	return BattleCameraRotation;
}

void AFRBattleCamera::ConfigureBattle(const FVector& InBaseLocation, float InOrthoWidth, float InFollowInterpSpeed, float InManualPanSpeed)
{
	BaseLocation = InBaseLocation;
	FollowInterpSpeed = InFollowInterpSpeed;
	ManualPanSpeed = InManualPanSpeed;
	SetActorRotation(BattleCameraRotation);
	if (UCameraComponent* Camera = GetCameraComponent())
	{
		Camera->ProjectionMode = ECameraProjectionMode::Orthographic;
		Camera->OrthoWidth = FMath::Max(1.0f, InOrthoWidth);
	}
}

void AFRBattleCamera::SetTerrainActor(AFRDestructibleTerrain* InTerrain)
{
	Terrain = InTerrain;
}

void AFRBattleCamera::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const FVector CurrentLocation = GetActorLocation();
	const FVector DesiredLocation = GetDesiredLocation();
	const FVector NewLocation = ControlMode == EFRBattleCameraControlMode::Manual
		? DesiredLocation
		: FMath::VInterpTo(CurrentLocation, DesiredLocation, DeltaSeconds, FollowInterpSpeed);
	SetActorLocation(NewLocation);
	SetActorRotation(BattleCameraRotation);
}

void AFRBattleCamera::RequestAutoFocus(AActor* FocusActor, const FVector& FocusLocation, float ZOffset)
{
	ControlMode = EFRBattleCameraControlMode::Auto;
	AutoFocusActor = FocusActor;
	AutoFocusLocation = FocusActor ? FocusActor->GetActorLocation() : FocusLocation;
	AutoFocusZOffset = ZOffset;
	bManualInputRequiresRelease = bManualInputHeld;

	SetActorLocation(GetDesiredLocation());
	SetActorRotation(BattleCameraRotation);
}

void AFRBattleCamera::HandleManualInput(const FVector2D& InputAxis, bool bAnyDirectionKeyDown, float DeltaSeconds)
{
	bManualInputHeld = bAnyDirectionKeyDown;
	if (!bAnyDirectionKeyDown)
	{
		bManualInputRequiresRelease = false;
		return;
	}

	if (bManualInputRequiresRelease || InputAxis.IsNearlyZero())
	{
		return;
	}

	if (ControlMode != EFRBattleCameraControlMode::Manual)
	{
		ControlMode = EFRBattleCameraControlMode::Manual;
		ManualLocation = GetActorLocation();
	}

	const FVector2D ClampedInput = InputAxis.GetClampedToMaxSize(1.0f);
	ManualLocation.X += ClampedInput.X * ManualPanSpeed * DeltaSeconds;
	ManualLocation.Z += ClampedInput.Y * ManualPanSpeed * DeltaSeconds;
	ManualLocation.Y = BaseLocation.Y;
	ManualLocation = ClampToTerrainBounds(ManualLocation);
}

FVector AFRBattleCamera::GetDesiredLocation() const
{
	if (ControlMode == EFRBattleCameraControlMode::Manual)
	{
		return ClampToTerrainBounds(ManualLocation);
	}

	FVector DesiredLocation = GetActorLocation();
	DesiredLocation.Y = BaseLocation.Y;

	const FVector FocusLocation = GetFocusLocation();
	DesiredLocation.X = FocusLocation.X;
	DesiredLocation.Z = FocusLocation.Z;
	return ClampToTerrainBounds(DesiredLocation);
}

FVector AFRBattleCamera::GetFocusLocation() const
{
	const FVector FocusLocation = AutoFocusActor.IsValid()
		? AutoFocusActor->GetActorLocation()
		: AutoFocusLocation;
	return FVector(FocusLocation.X, BaseLocation.Y, FocusLocation.Z + AutoFocusZOffset);
}

float AFRBattleCamera::GetOrthoWidthSafe() const
{
	if (const UCameraComponent* Camera = GetCameraComponent())
	{
		return FMath::Max(1.0f, Camera->OrthoWidth);
	}
	return 1.0f;
}

FVector AFRBattleCamera::ClampToTerrainBounds(const FVector& DesiredLocation) const
{
	const AFRDestructibleTerrain* TerrainActor = Terrain.Get();
	if (!TerrainActor)
	{
		return DesiredLocation;
	}

	const float ViewHalfWidth = GetOrthoWidthSafe() * 0.5f;
	const float TerrainHalfWidth = TerrainActor->Width * 0.5f;
	const float PaddingHalf = TerrainCameraPadding * 0.5f;
	const float TerrainCenterX = TerrainActor->GetActorLocation().X;

	FVector ClampedLocation = DesiredLocation;
	if (ViewHalfWidth >= TerrainHalfWidth + PaddingHalf)
	{
		ClampedLocation.X = TerrainCenterX;
	}
	else
	{
		const float MinCameraX = TerrainCenterX - TerrainHalfWidth - PaddingHalf + ViewHalfWidth;
		const float MaxCameraX = TerrainCenterX + TerrainHalfWidth + PaddingHalf - ViewHalfWidth;
		ClampedLocation.X = FMath::Clamp(static_cast<float>(DesiredLocation.X), MinCameraX, MaxCameraX);
	}

	return ClampedLocation;
}
