// Copyright Epic Games, Inc. All Rights Reserved.

#include "Run/FRStageRunDefinition.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

namespace
{
void AddStageRunValidationIssue(TArray<FString>& Issues, const FString& Issue)
{
	if (!Issue.IsEmpty())
	{
		Issues.Add(Issue);
	}
}
}

UFRStageRunDefinition::UFRStageRunDefinition()
{
	NormalizeStageData();
}

void UFRStageRunDefinition::PostLoad()
{
	Super::PostLoad();
	NormalizeStageData();
}

#if WITH_EDITOR
void UFRStageRunDefinition::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	NormalizeStageData();
}
#endif

const FFRStageDifficultyData& UFRStageRunDefinition::GetStageDifficulty(int32 StageNumber) const
{
	const int32 StageIndex = FMath::Clamp(StageNumber - 1, 0, FMath::Max(0, StageDifficultyData.Num() - 1));
	if (StageDifficultyData.IsValidIndex(StageIndex))
	{
		return StageDifficultyData[StageIndex];
	}

	static const FFRStageDifficultyData DefaultDifficulty;
	return DefaultDifficulty;
}

void UFRStageRunDefinition::NormalizeStageData()
{
	StageCount = FMath::Max(1, StageCount);
	RewardChoiceCount = FMath::Clamp(RewardChoiceCount, 1, 5);
	StageDifficultyData.SetNum(StageCount);
}

FText UFRStageRunDefinition::GetDataValidationSummary() const
{
	TArray<FString> Issues;
	if (StageCount <= 0)
	{
		AddStageRunValidationIssue(Issues, TEXT("stage count must be greater than 0"));
	}
	if (RewardChoiceCount <= 0 || RewardChoiceCount > 5)
	{
		AddStageRunValidationIssue(Issues, TEXT("reward choice count must be 1-5"));
	}
	if (StageDifficultyData.Num() != StageCount)
	{
		AddStageRunValidationIssue(Issues, TEXT("stage difficulty rows must match stage count"));
	}
	if (RewardPool.Num() <= 0)
	{
		AddStageRunValidationIssue(Issues, TEXT("reward pool is empty"));
	}
	else if (RewardChoiceCount > RewardPool.Num())
	{
		AddStageRunValidationIssue(Issues, TEXT("reward pool has fewer entries than reward choice count"));
	}

	bool bHasRewardDataWarning = false;
	bool bHasStartingReward = false;
	const FGameplayTagContainer EmptyChosenRewardTags;
	for (const FFRRewardChoice& Reward : RewardPool)
	{
		if (!Reward.GetDataValidationSummary().IsEmpty())
		{
			bHasRewardDataWarning = true;
		}
		if (Reward.MeetsRewardTagConditions(EmptyChosenRewardTags))
		{
			bHasStartingReward = true;
		}
	}
	if (bHasRewardDataWarning)
	{
		AddStageRunValidationIssue(Issues, TEXT("reward pool data has warnings"));
	}
	if (RewardPool.Num() > 0 && !bHasStartingReward)
	{
		AddStageRunValidationIssue(Issues, TEXT("no reward can appear at run start"));
	}

	for (const FFRStageDifficultyData& DifficultyData : StageDifficultyData)
	{
		if (DifficultyData.MinAimAngleDegrees > DifficultyData.MaxAimAngleDegrees)
		{
			AddStageRunValidationIssue(Issues, TEXT("stage difficulty aim range min is greater than max"));
			break;
		}
		if (DifficultyData.MinShotPower > DifficultyData.MaxShotPower)
		{
			AddStageRunValidationIssue(Issues, TEXT("stage difficulty shot power min is greater than max"));
			break;
		}
	}

	return Issues.Num() > 0 ? FText::FromString(FString::Join(Issues, TEXT(" | "))) : FText::GetEmpty();
}

#if WITH_EDITOR
EDataValidationResult UFRStageRunDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	const FText ValidationSummary = GetDataValidationSummary();
	if (!ValidationSummary.IsEmpty())
	{
		Context.AddWarning(ValidationSummary);
	}
	return Result;
}
#endif
