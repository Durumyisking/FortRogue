// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Rewards/FRRewardTypes.h"
#include "FRStageRunDefinition.generated.h"

class UFRCharacterDefinition;
class UFRDefaultLoadoutDefinition;
class UFRTerrainMapDefinition;

USTRUCT(BlueprintType)
struct FFRStageDifficultyData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0", ToolTip = "적 턴이 시작된 뒤 실제로 발사하기 전까지 기다리는 시간입니다. 0이면 바로 행동합니다."))
	float EnemyTurnDelaySeconds = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0", ToolTip = "적이 고각 포물선을 계산할 때 목표보다 최소로 띄우려는 탄도 높이입니다. 값이 클수록 더 높은 궤적을 시도합니다."))
	float MinAimArcHeight = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ToolTip = "적 조준 계산에서 목표 위치 위로 더해지는 높이 보정입니다. 양수면 더 높게, 음수면 더 낮게 겨냥합니다."))
	float AimHeightOffset = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0", ClampMax = "90.0", ToolTip = "적이 선택할 수 있는 최소 발사 각도입니다. 0~90도 값을 사용하세요."))
	float MinAimAngleDegrees = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0", ClampMax = "90.0", ToolTip = "적이 선택할 수 있는 최대 발사 각도입니다. Min보다 작으면 데이터 검수 경고가 발생합니다."))
	float MaxAimAngleDegrees = 72.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "1.0", ToolTip = "목표까지의 거리를 샷 파워로 환산할 때 나누는 기준 거리입니다. 값이 작을수록 같은 거리에서 더 강하게 쏩니다."))
	float ShotPowerDistanceScale = 1450.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0", ClampMax = "1.0", ToolTip = "적 샷 파워의 최소값입니다. 0~1 범위를 사용하세요."))
	float MinShotPower = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0", ClampMax = "1.0", ToolTip = "적 샷 파워의 최대값입니다. Min보다 작으면 데이터 검수 경고가 발생합니다."))
	float MaxShotPower = 0.92f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0", ToolTip = "적 조준 각도에 더할 무작위 오차의 최대 절댓값입니다. 0이면 각도 오차가 없습니다."))
	float AimAngleErrorDegrees = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0", ToolTip = "적 샷 파워에 더할 무작위 오차의 최대 절댓값입니다. 0이면 파워 오차가 없습니다."))
	float ShotPowerError = 0.0f;
};

UCLASS(BlueprintType)
class FORTROGUE_API UFRStageRunDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFRStageRunDefinition();

	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	const FFRStageDifficultyData& GetStageDifficulty(int32 StageNumber) const;
	void NormalizeStageData();

	UFUNCTION(BlueprintPure, Category = "FortRogue|Run")
	FText GetDataValidationSummary() const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Run", meta = (ClampMin = "1", ToolTip = "한 런에서 진행할 총 스테이지 수입니다. 1 이상이어야 합니다."))
	int32 StageCount = 7;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Run", meta = (ToolTip = "스테이지 생성 시 기본으로 사용할 지형 맵 데이터입니다. 비워두면 런 시작에 필요한 지형이 없을 수 있습니다."))
	TObjectPtr<UFRTerrainMapDefinition> DefaultTerrainMapDefinition;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Run", meta = (ToolTip = "스테이지마다 선택될 수 있는 적 캐릭터 데이터 풀입니다. 여러 개를 넣으면 런 중 적 구성이 변합니다."))
	TArray<TObjectPtr<UFRCharacterDefinition>> EnemyDefinitionPool;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Run", meta = (ToolTip = "플레이어가 런을 시작할 때 받을 기본 무기와 아이템 구성입니다."))
	TObjectPtr<UFRDefaultLoadoutDefinition> DefaultLoadoutDefinition;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rewards", meta = (ToolTip = "스테이지 클리어 후 제시될 수 있는 보상 전체 목록입니다. 조건과 가중치에 따라 선택지가 뽑힙니다."))
	TArray<FFRRewardChoice> RewardPool;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rewards", meta = (ClampMin = "1", ClampMax = "5", ToolTip = "보상 화면에 동시에 보여줄 선택지 수입니다. 현재 UI 기준 1~5개 값을 사용하세요."))
	int32 RewardChoiceCount = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Difficulty", meta = (ToolTip = "스테이지 번호별 적 AI 난이도 설정입니다. StageCount에 맞춰 자동으로 개수가 보정됩니다."))
	TArray<FFRStageDifficultyData> StageDifficultyData;
};
