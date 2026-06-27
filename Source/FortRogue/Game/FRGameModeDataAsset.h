// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameFramework/Pawn.h"
#include "FRGameModeDataAsset.generated.h"

class ACameraActor;
class AFRBattleCharacter;
class AFRDestructibleTerrain;
class UFRCharacterDefinition;
class UFRDefaultLoadoutDefinition;
class UFRStageRunDefinition;
class UFRTerrainMapDefinition;
class UUserWidget;
class UWorld;

UENUM(BlueprintType)
enum class EFRGameFlowInputMode : uint8
{
	GameOnly,
	UIOnly,
	GameAndUI
};

UCLASS(BlueprintType)
class FORTROGUE_API UFRGameModeDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FortRogue|Game Flow")
	FString GetLevelPackageName() const;

	TSubclassOf<UUserWidget> LoadHUDWidgetClass() const;
	TSubclassOf<APawn> LoadEntryPawnClass() const;
	TSubclassOf<AFRBattleCharacter> LoadPlayerCharacterClass() const;
	TSubclassOf<AFRBattleCharacter> LoadEnemyCharacterClass() const;
	TSubclassOf<AFRDestructibleTerrain> LoadTerrainClass() const;
	TSubclassOf<ACameraActor> LoadCameraClass() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mode")
	FName ModeName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mode")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mode", meta = (AllowedClasses = "/Script/Engine.World"))
	TSoftObjectPtr<UWorld> Level;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSoftClassPtr<UUserWidget> HUDWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	EFRGameFlowInputMode InputMode = EFRGameFlowInputMode::GameOnly;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	bool bShowMouseCursor = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	FName StartMainGameButtonName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn")
	TSoftClassPtr<APawn> EntryPawnClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	bool bStartBattleOnEnter = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	TSoftClassPtr<AFRBattleCharacter> PlayerCharacterClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	TSoftClassPtr<AFRBattleCharacter> EnemyCharacterClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	TSoftClassPtr<AFRDestructibleTerrain> TerrainClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	TSoftClassPtr<ACameraActor> CameraClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	TObjectPtr<UFRCharacterDefinition> PlayerDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	TObjectPtr<UFRStageRunDefinition> StageRunDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	TObjectPtr<UFRTerrainMapDefinition> TerrainMapDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	FVector TerrainLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	FVector PlayerSpawnOffset = FVector(-650.0f, 0.0f, 95.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	FVector EnemySpawnOffset = FVector(650.0f, 0.0f, 95.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	FVector CameraLocation = FVector(0.0f, 3000.0f, 860.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float CameraOrthoWidth = 2700.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float CameraFollowInterpSpeed = 4.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = "0.0"))
	float CameraManualPanSpeed = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float CameraProjectileZOffset = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float CameraTurnZOffset = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float ShotImpactCameraHoldSeconds = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	float MinWind = -180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	float MaxWind = 180.0f;
};
