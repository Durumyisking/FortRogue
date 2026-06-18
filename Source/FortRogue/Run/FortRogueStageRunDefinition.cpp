// Copyright Epic Games, Inc. All Rights Reserved.

#include "Run/FortRogueStageRunDefinition.h"

UFortRogueStageRunDefinition::UFortRogueStageRunDefinition()
{
	NormalizeStageData();
}

void UFortRogueStageRunDefinition::PostLoad()
{
	Super::PostLoad();
	NormalizeStageData();
}

#if WITH_EDITOR
void UFortRogueStageRunDefinition::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	NormalizeStageData();
}
#endif

const FFortRogueStageDifficultyData& UFortRogueStageRunDefinition::GetStageDifficulty(int32 StageNumber) const
{
	const int32 StageIndex = FMath::Clamp(StageNumber - 1, 0, FMath::Max(0, StageDifficultyData.Num() - 1));
	if (StageDifficultyData.IsValidIndex(StageIndex))
	{
		return StageDifficultyData[StageIndex];
	}

	static const FFortRogueStageDifficultyData DefaultDifficulty;
	return DefaultDifficulty;
}

void UFortRogueStageRunDefinition::NormalizeStageData()
{
	StageCount = FMath::Max(1, StageCount);
	StageDifficultyData.SetNum(StageCount);
}
