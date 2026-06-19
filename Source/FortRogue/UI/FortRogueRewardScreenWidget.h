// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonActivatableWidget.h"
#include "Rewards/FortRogueRewardTypes.h"
#include "FortRogueRewardScreenWidget.generated.h"

UCLASS(Abstract, Blueprintable)
class FORTROGUE_API UFortRogueRewardScreenWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FortRogue|UI")
	class AFortRogueGameMode* GetFortRogueGameMode() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	TArray<FFortRogueRewardChoice> GetRewardChoices() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	int32 GetRewardChoiceCount() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FFortRogueRewardChoice GetRewardChoice(int32 ChoiceIndex) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FText GetRewardChoiceSummary(int32 ChoiceIndex) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FText GetRewardChoiceConditionFailureSummary(int32 ChoiceIndex) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	FGameplayTagContainer GetChosenRewardTags() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|UI")
	bool CanChooseReward(int32 ChoiceIndex) const;

	UFUNCTION(BlueprintCallable, Category = "FortRogue|UI")
	void ChooseReward(int32 ChoiceIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = "FortRogue|UI")
	void RefreshRewardScreen();
};
