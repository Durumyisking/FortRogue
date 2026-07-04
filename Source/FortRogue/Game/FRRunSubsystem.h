// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Math/RandomStream.h"
#include "Rewards/FRRewardTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FRRunSubsystem.generated.h"

class UFRCharacterDefinition;
class UFRPerkDefinition;
class UFRStageRunDefinition;

/**
 * 한 번의 로그라이크 런 동안 유지되는 상태를 소유합니다.
 * 스테이지 진행, 선택한 보상 태그, 만난 적, 획득한 퍽, 그리고 런 전용 시드 RNG가 여기 있습니다.
 * GameMode는 전투 연출만 담당하고 런 상태는 레벨 전환과 무관하게 이 서브시스템에 남습니다.
 */
UCLASS()
class FORTROGUE_API UFRRunSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FortRogue|Run", meta = (ToolTip = "새 런을 시작합니다. Seed가 0이면 무작위 시드를 사용합니다."))
	void StartRun(int32 Seed = 0);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Run")
	void EndRun();

	UFUNCTION(BlueprintPure, Category = "FortRogue|Run")
	bool IsRunActive() const { return bRunActive; }

	UFUNCTION(BlueprintPure, Category = "FortRogue|Run", meta = (ToolTip = "현재 런의 시드입니다. 같은 시드는 같은 적/보상/바람 순서를 만듭니다."))
	int32 GetRunSeed() const { return RunSeed; }

	UFUNCTION(BlueprintPure, Category = "FortRogue|Run")
	int32 GetCurrentStage() const { return CurrentStage; }

	void AdvanceStage();

	/** 런 전용 RNG입니다. 런 도중의 모든 무작위 판정은 여기서만 뽑아야 시드 재현이 유지됩니다. */
	FRandomStream& GetRandomStream() { return RunRandom; }
	float RandRange(float Min, float Max);
	int32 RandRangeInt(int32 Min, int32 Max);

	void RecordChosenRewardTag(const FGameplayTag& RewardTag);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Run")
	void ClearChosenRewardTags();

	UFUNCTION(BlueprintPure, Category = "FortRogue|Run")
	FGameplayTagContainer GetChosenRewardTags() const;

	void RecordAcquiredPerk(UFRPerkDefinition* PerkDefinition);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Run")
	TArray<UFRPerkDefinition*> GetAcquiredPerks() const;

	/** 풀에서 아직 만나지 않은 적을 우선해 하나 뽑습니다. 전부 만났으면 기록을 비우고 다시 뽑습니다. */
	UFRCharacterDefinition* PickNextEnemyDefinition(const TArray<TObjectPtr<UFRCharacterDefinition>>& EnemyPool);

	/** 조건과 가중치를 반영해 이번 스테이지 보상 선택지를 뽑습니다. */
	TArray<FFRRewardChoice> BuildRewardChoices(const UFRStageRunDefinition* StageRunDefinition, const UFRCharacterDefinition* PlayerDefinition);

private:
	bool bRunActive = false;
	int32 RunSeed = 0;
	int32 CurrentStage = 1;
	FRandomStream RunRandom;

	UPROPERTY(Transient)
	TArray<FGameplayTag> ChosenRewardTags;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UFRCharacterDefinition>> EncounteredEnemyDefinitions;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UFRPerkDefinition>> AcquiredPerks;
};
