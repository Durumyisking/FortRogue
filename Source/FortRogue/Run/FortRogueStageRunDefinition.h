// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Rewards/FortRogueRewardTypes.h"
#include "FortRogueStageRunDefinition.generated.h"

class UFortRogueCharacterDefinition;
class UFortRogueDefaultLoadoutDefinition;
class UFortRogueTerrainMapDefinition;

USTRUCT(BlueprintType)
struct FFortRogueStageDifficultyData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0"))
	float EnemyTurnDelaySeconds = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0"))
	float MinAimArcHeight = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	float AimHeightOffset = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float MinAimAngleDegrees = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float MaxAimAngleDegrees = 72.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "1.0"))
	float ShotPowerDistanceScale = 1450.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinShotPower = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxShotPower = 0.92f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0"))
	float AimAngleErrorDegrees = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0"))
	float ShotPowerError = 0.0f;
};

UCLASS(BlueprintType)
class FORTROGUE_API UFortRogueStageRunDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFortRogueStageRunDefinition();

	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	const FFortRogueStageDifficultyData& GetStageDifficulty(int32 StageNumber) const;
	void NormalizeStageData();

	UFUNCTION(BlueprintPure, Category = "FortRogue|Run")
	FText GetDataValidationSummary() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Run", meta = (ClampMin = "1"))
	int32 StageCount = 7;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Run")
	TObjectPtr<UFortRogueTerrainMapDefinition> DefaultTerrainMapDefinition;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Run")
	TArray<TObjectPtr<UFortRogueCharacterDefinition>> EnemyDefinitionPool;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Run")
	TObjectPtr<UFortRogueDefaultLoadoutDefinition> DefaultLoadoutDefinition;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rewards")
	TArray<FFortRogueRewardChoice> RewardPool;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rewards", meta = (ClampMin = "1", ClampMax = "5"))
	int32 RewardChoiceCount = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Difficulty")
	TArray<FFortRogueStageDifficultyData> StageDifficultyData;
};
