// Copyright Epic Games, Inc. All Rights Reserved.

#include "Weapons/FortRogueWeaponDefinition.h"

#include "Combat/FortRogueShotSpec.h"

bool FFortRogueShotModifierSpec::MeetsShotConditions(const FFortRogueShotSpec& CurrentShotSpec, float CurrentAimAngle, float Wind, bool bShotFacingRight) const
{
	if (bUseAimAngleRange)
	{
		const float MinAngle = FMath::Min(MinAimAngle, MaxAimAngle);
		const float MaxAngle = FMath::Max(MinAimAngle, MaxAimAngle);
		if (CurrentAimAngle < MinAngle || CurrentAimAngle > MaxAngle)
		{
			return false;
		}
	}
	if (bRequireWindAligned)
	{
		const float ShotDirection = bShotFacingRight ? 1.0f : -1.0f;
		if (FMath::Abs(Wind) < MinWindMagnitude || Wind * ShotDirection <= 0.0f)
		{
			return false;
		}
	}
	if (!RequiredShotTags.IsEmpty() && !CurrentShotSpec.EffectTags.HasAny(RequiredShotTags))
	{
		return false;
	}
	if (!BlockedShotTags.IsEmpty() && CurrentShotSpec.EffectTags.HasAny(BlockedShotTags))
	{
		return false;
	}
	return true;
}

FText FFortRogueShotModifierSpec::GetShotConditionFailureSummary(const FFortRogueShotSpec& CurrentShotSpec, float CurrentAimAngle, float Wind, bool bShotFacingRight) const
{
	if (bUseAimAngleRange)
	{
		const float MinAngle = FMath::Min(MinAimAngle, MaxAimAngle);
		const float MaxAngle = FMath::Max(MinAimAngle, MaxAimAngle);
		if (CurrentAimAngle < MinAngle || CurrentAimAngle > MaxAngle)
		{
			return FText::FromString(FString::Printf(TEXT("requires aim %.0f-%.0f deg"), MinAngle, MaxAngle));
		}
	}
	if (bRequireWindAligned)
	{
		const float ShotDirection = bShotFacingRight ? 1.0f : -1.0f;
		if (FMath::Abs(Wind) < MinWindMagnitude || Wind * ShotDirection <= 0.0f)
		{
			return FText::FromString(MinWindMagnitude > 0.0f ? FString::Printf(TEXT("requires aligned wind %.0f+"), MinWindMagnitude) : FString(TEXT("requires aligned wind")));
		}
	}
	if (!RequiredShotTags.IsEmpty() && !CurrentShotSpec.EffectTags.HasAny(RequiredShotTags))
	{
		return FText::FromString(FString::Printf(TEXT("requires shot tag %s"), *RequiredShotTags.ToStringSimple()));
	}
	if (!BlockedShotTags.IsEmpty() && CurrentShotSpec.EffectTags.HasAny(BlockedShotTags))
	{
		return FText::FromString(FString::Printf(TEXT("blocked by shot tag %s"), *BlockedShotTags.ToStringSimple()));
	}
	return FText::GetEmpty();
}
