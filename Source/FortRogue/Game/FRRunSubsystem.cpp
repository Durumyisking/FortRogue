// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/FRRunSubsystem.h"

#include "Characters/FRCharacterDefinition.h"
#include "Perks/FRPerkDefinition.h"
#include "Run/FRStageRunDefinition.h"

namespace
{
constexpr int32 MaxRewardChoiceCount = 5;
}

void UFRRunSubsystem::StartRun(int32 Seed)
{
	bRunActive = true;
	RunSeed = Seed != 0 ? Seed : static_cast<int32>(FPlatformTime::Cycles());
	RunRandom.Initialize(RunSeed);
	CurrentStage = 1;
	ChosenRewardTags.Reset();
	EncounteredEnemyDefinitions.Reset();
	AcquiredPerks.Reset();
}

void UFRRunSubsystem::EndRun()
{
	bRunActive = false;
}

void UFRRunSubsystem::AdvanceStage()
{
	++CurrentStage;
}

float UFRRunSubsystem::RandRange(float Min, float Max)
{
	return RunRandom.FRandRange(Min, Max);
}

int32 UFRRunSubsystem::RandRangeInt(int32 Min, int32 Max)
{
	return RunRandom.RandRange(Min, Max);
}

void UFRRunSubsystem::RecordChosenRewardTag(const FGameplayTag& RewardTag)
{
	if (RewardTag.IsValid())
	{
		ChosenRewardTags.AddUnique(RewardTag);
	}
}

void UFRRunSubsystem::ClearChosenRewardTags()
{
	ChosenRewardTags.Reset();
}

FGameplayTagContainer UFRRunSubsystem::GetChosenRewardTags() const
{
	FGameplayTagContainer Result;
	for (const FGameplayTag& ChosenRewardTag : ChosenRewardTags)
	{
		if (ChosenRewardTag.IsValid())
		{
			Result.AddTag(ChosenRewardTag);
		}
	}
	return Result;
}

void UFRRunSubsystem::RecordAcquiredPerk(UFRPerkDefinition* PerkDefinition)
{
	if (PerkDefinition)
	{
		AcquiredPerks.Add(PerkDefinition);
	}
}

TArray<UFRPerkDefinition*> UFRRunSubsystem::GetAcquiredPerks() const
{
	TArray<UFRPerkDefinition*> Result;
	for (UFRPerkDefinition* Perk : AcquiredPerks)
	{
		if (Perk)
		{
			Result.Add(Perk);
		}
	}
	return Result;
}

UFRCharacterDefinition* UFRRunSubsystem::PickNextEnemyDefinition(const TArray<TObjectPtr<UFRCharacterDefinition>>& EnemyPool)
{
	TArray<UFRCharacterDefinition*> Candidates;
	for (UFRCharacterDefinition* Candidate : EnemyPool)
	{
		if (Candidate && !EncounteredEnemyDefinitions.Contains(Candidate))
		{
			Candidates.Add(Candidate);
		}
	}

	if (Candidates.Num() == 0 && EnemyPool.Num() > 0)
	{
		EncounteredEnemyDefinitions.Reset();
		for (UFRCharacterDefinition* Candidate : EnemyPool)
		{
			if (Candidate)
			{
				Candidates.Add(Candidate);
			}
		}
	}

	if (Candidates.Num() == 0)
	{
		return nullptr;
	}

	UFRCharacterDefinition* Picked = Candidates[RandRangeInt(0, Candidates.Num() - 1)];
	EncounteredEnemyDefinitions.AddUnique(Picked);
	return Picked;
}

TArray<FFRRewardChoice> UFRRunSubsystem::BuildRewardChoices(const UFRStageRunDefinition* StageRunDefinition, const UFRCharacterDefinition* PlayerDefinition)
{
	TArray<FFRRewardChoice> Choices;
	if (!StageRunDefinition || StageRunDefinition->RewardPool.Num() <= 0)
	{
		return Choices;
	}

	const FGameplayTagContainer ChosenRewardTagContainer = GetChosenRewardTags();
	TArray<FFRRewardChoice> CompatibleRewards;
	TArray<FFRRewardChoice> CandidateRewards;
	for (const FFRRewardChoice& Reward : StageRunDefinition->RewardPool)
	{
		if (PlayerDefinition && !Reward.MatchesPerkCategoryFilter(PlayerDefinition->RequiredPerkCategoryTags, PlayerDefinition->BlockedPerkCategoryTags))
		{
			continue;
		}
		if (!Reward.MeetsRewardTagConditions(ChosenRewardTagContainer))
		{
			continue;
		}

		CompatibleRewards.Add(Reward);
		const FGameplayTag ResolvedRewardTag = Reward.GetResolvedRewardTag();
		if (Reward.bOfferOncePerRun && ResolvedRewardTag.IsValid() && ChosenRewardTags.Contains(ResolvedRewardTag))
		{
			continue;
		}

		CandidateRewards.Add(Reward);
	}
	if (CandidateRewards.Num() <= 0)
	{
		CandidateRewards = CompatibleRewards;
	}
	if (CandidateRewards.Num() <= 0)
	{
		return Choices;
	}

	const int32 MaxAvailableChoices = FMath::Min(MaxRewardChoiceCount, CandidateRewards.Num());
	const int32 ChoiceCount = FMath::Clamp(StageRunDefinition->RewardChoiceCount, 1, MaxAvailableChoices);
	for (int32 ChoiceIndex = 0; ChoiceIndex < ChoiceCount; ++ChoiceIndex)
	{
		float TotalWeight = 0.0f;
		for (const FFRRewardChoice& CandidateReward : CandidateRewards)
		{
			TotalWeight += FMath::Max(0.0f, CandidateReward.RewardWeight);
		}

		int32 CandidateIndex = RandRangeInt(0, CandidateRewards.Num() - 1);
		if (TotalWeight > KINDA_SMALL_NUMBER)
		{
			float WeightRoll = RandRange(0.0f, TotalWeight);
			for (int32 Index = 0; Index < CandidateRewards.Num(); ++Index)
			{
				const float CandidateWeight = FMath::Max(0.0f, CandidateRewards[Index].RewardWeight);
				if (CandidateWeight <= 0.0f)
				{
					continue;
				}

				WeightRoll -= CandidateWeight;
				if (WeightRoll <= 0.0f)
				{
					CandidateIndex = Index;
					break;
				}
			}
		}

		Choices.Add(CandidateRewards[CandidateIndex]);
		CandidateRewards.RemoveAtSwap(CandidateIndex, 1, EAllowShrinking::No);
	}

	return Choices;
}
