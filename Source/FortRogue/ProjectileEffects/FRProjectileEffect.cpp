// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectileEffects/FRProjectileEffect.h"

#include "Combat/FRDestructibleTerrain.h"
#include "Combat/FRShotSpec.h"
#include "EngineUtils.h"
#include "FRGameplayTags.h"

namespace
{
void AddProjectileEffectValidationIssue(TArray<FString>& Issues, const FString& Issue)
{
	if (!Issue.IsEmpty())
	{
		Issues.Add(Issue);
	}
}

void ApplyToImpactTerrain(const FFRProjectileEffectImpactContext& Context, TFunctionRef<void(AFRDestructibleTerrain&)> Apply)
{
	if (Context.AssignedTerrain)
	{
		Apply(*Context.AssignedTerrain);
		return;
	}

	if (!Context.World)
	{
		return;
	}

	for (TActorIterator<AFRDestructibleTerrain> It(Context.World); It; ++It)
	{
		Apply(**It);
	}
}
}

const UScriptStruct* UFRProjectileEffectBase::GetParameterStruct() const
{
	return FFRProjectileEffectParameters::StaticStruct();
}

void UFRProjectileEffectBase::ApplyToShotSpec(const FFRProjectileEffectSpec& EffectSpec, FFRShotSpec& ShotSpec) const
{
}

void UFRProjectileEffectBase::HandleImpact(const FFRProjectileEffectSpec& EffectSpec, const FFRProjectileEffectImpactContext& Context) const
{
}

bool UFRProjectileEffectBase::UsesCustomTerrainImpact(const FFRProjectileEffectSpec& EffectSpec) const
{
	return false;
}

void UFRProjectileEffectBase::AddDataValidationIssues(const FFRProjectileEffectSpec& EffectSpec, TArray<FString>& Issues) const
{
}

const UFRProjectileEffectBase* FFRProjectileEffectSpec::GetEffectCDO() const
{
	return EffectClass ? EffectClass->GetDefaultObject<UFRProjectileEffectBase>() : nullptr;
}

const UScriptStruct* FFRProjectileEffectSpec::GetExpectedParameterStruct() const
{
	const UFRProjectileEffectBase* EffectCDO = GetEffectCDO();
	return EffectCDO ? EffectCDO->GetParameterStruct() : nullptr;
}

FText FFRProjectileEffectSpec::GetEffectDisplayName() const
{
	if (!EffectClass)
	{
		return FText::GetEmpty();
	}

	const FText DisplayName = EffectClass->GetDisplayNameText();
	return DisplayName.IsEmpty() ? FText::FromString(EffectClass->GetName()) : DisplayName;
}

bool FFRProjectileEffectSpec::HasValidParameters() const
{
	const UScriptStruct* ExpectedStruct = GetExpectedParameterStruct();
	if (!ExpectedStruct)
	{
		return true;
	}

	return Parameters.IsValid() && Parameters.GetScriptStruct() == ExpectedStruct;
}

bool FFRProjectileEffectSpec::EnsureParametersMatchEffectClass()
{
	const UScriptStruct* ExpectedStruct = GetExpectedParameterStruct();
	if (!ExpectedStruct)
	{
		if (!Parameters.IsValid())
		{
			return false;
		}

		Parameters.Reset();
		return true;
	}

	if (Parameters.IsValid() && Parameters.GetScriptStruct() == ExpectedStruct)
	{
		return false;
	}

	Parameters.InitializeAs(ExpectedStruct);
	return true;
}

void FFRProjectileEffectSpec::ApplyToShotSpec(FFRShotSpec& ShotSpec) const
{
	if (const UFRProjectileEffectBase* EffectCDO = GetEffectCDO())
	{
		EffectCDO->ApplyToShotSpec(*this, ShotSpec);
	}
}

void FFRProjectileEffectSpec::HandleImpact(const FFRProjectileEffectImpactContext& Context) const
{
	if (const UFRProjectileEffectBase* EffectCDO = GetEffectCDO())
	{
		EffectCDO->HandleImpact(*this, Context);
	}
}

bool FFRProjectileEffectSpec::UsesCustomTerrainImpact() const
{
	const UFRProjectileEffectBase* EffectCDO = GetEffectCDO();
	return EffectCDO && EffectCDO->UsesCustomTerrainImpact(*this);
}

FText FFRProjectileEffectSpec::GetDataValidationSummary() const
{
	TArray<FString> Issues;
	if (!EffectClass)
	{
		AddProjectileEffectValidationIssue(Issues, TEXT("missing projectile effect class"));
	}
	else if (!HasValidParameters())
	{
		AddProjectileEffectValidationIssue(Issues, TEXT("projectile effect parameters do not match effect class"));
	}
	else if (const UFRProjectileEffectBase* EffectCDO = GetEffectCDO())
	{
		EffectCDO->AddDataValidationIssues(*this, Issues);
	}

	return Issues.Num() > 0 ? FText::FromString(FString::Join(Issues, TEXT(" | "))) : FText::GetEmpty();
}

const UScriptStruct* UFRProjectileEffectDrill::GetParameterStruct() const
{
	return FFRProjectileEffectDrillParams::StaticStruct();
}

void UFRProjectileEffectDrill::ApplyToShotSpec(const FFRProjectileEffectSpec& EffectSpec, FFRShotSpec& ShotSpec) const
{
	const FFRProjectileEffectDrillParams& Params = EffectSpec.GetParametersOrDefault<FFRProjectileEffectDrillParams>();
	ShotSpec.EffectTags.AddTag(FRGameplayTags::ShotEffect_Drill);
	ShotSpec.TerrainCarveRadius = FMath::Max(0.0f, (ShotSpec.TerrainCarveRadius + Params.RadiusBonus) * Params.RadiusMultiplier);
}

void UFRProjectileEffectDrill::HandleImpact(const FFRProjectileEffectSpec& EffectSpec, const FFRProjectileEffectImpactContext& Context) const
{
	if (Context.TerrainCarveRadius <= 0.0f)
	{
		return;
	}

	ApplyToImpactTerrain(Context, [&Context](AFRDestructibleTerrain& Terrain)
	{
		Terrain.CarveCircle(Context.ImpactLocation, Context.TerrainCarveRadius);
	});
}

bool UFRProjectileEffectDrill::UsesCustomTerrainImpact(const FFRProjectileEffectSpec& EffectSpec) const
{
	return true;
}

void UFRProjectileEffectDrill::AddDataValidationIssues(const FFRProjectileEffectSpec& EffectSpec, TArray<FString>& Issues) const
{
	const FFRProjectileEffectDrillParams& Params = EffectSpec.GetParametersOrDefault<FFRProjectileEffectDrillParams>();
	if (Params.RadiusBonus < 0.0f)
	{
		AddProjectileEffectValidationIssue(Issues, TEXT("drill radius bonus must be non-negative"));
	}
	if (Params.RadiusMultiplier < 0.0f)
	{
		AddProjectileEffectValidationIssue(Issues, TEXT("drill radius multiplier must be non-negative"));
	}
}

const UScriptStruct* UFRProjectileEffectTerrainCreate::GetParameterStruct() const
{
	return FFRProjectileEffectTerrainCreateParams::StaticStruct();
}

void UFRProjectileEffectTerrainCreate::ApplyToShotSpec(const FFRProjectileEffectSpec& EffectSpec, FFRShotSpec& ShotSpec) const
{
	const FFRProjectileEffectTerrainCreateParams& Params = EffectSpec.GetParametersOrDefault<FFRProjectileEffectTerrainCreateParams>();
	ShotSpec.EffectTags.AddTag(FRGameplayTags::ShotEffect_TerrainCreate);
	ShotSpec.TerrainFillRadius = FMath::Max(0.0f, (ShotSpec.TerrainFillRadius + Params.RadiusBonus) * Params.RadiusMultiplier);
}

void UFRProjectileEffectTerrainCreate::HandleImpact(const FFRProjectileEffectSpec& EffectSpec, const FFRProjectileEffectImpactContext& Context) const
{
	if (Context.TerrainFillRadius <= 0.0f)
	{
		return;
	}

	ApplyToImpactTerrain(Context, [&Context](AFRDestructibleTerrain& Terrain)
	{
		Terrain.FillCircle(Context.ImpactLocation, Context.TerrainFillRadius);
	});
}

bool UFRProjectileEffectTerrainCreate::UsesCustomTerrainImpact(const FFRProjectileEffectSpec& EffectSpec) const
{
	return true;
}

void UFRProjectileEffectTerrainCreate::AddDataValidationIssues(const FFRProjectileEffectSpec& EffectSpec, TArray<FString>& Issues) const
{
	const FFRProjectileEffectTerrainCreateParams& Params = EffectSpec.GetParametersOrDefault<FFRProjectileEffectTerrainCreateParams>();
	if (Params.RadiusBonus < 0.0f)
	{
		AddProjectileEffectValidationIssue(Issues, TEXT("terrain create radius bonus must be non-negative"));
	}
	if (Params.RadiusMultiplier < 0.0f)
	{
		AddProjectileEffectValidationIssue(Issues, TEXT("terrain create radius multiplier must be non-negative"));
	}
}

const UScriptStruct* UFRProjectileEffectTerrainColumn::GetParameterStruct() const
{
	return FFRProjectileEffectTerrainColumnParams::StaticStruct();
}

void UFRProjectileEffectTerrainColumn::ApplyToShotSpec(const FFRProjectileEffectSpec& EffectSpec, FFRShotSpec& ShotSpec) const
{
	ShotSpec.EffectTags.AddTag(FRGameplayTags::ShotEffect_TerrainColumn);
}

void UFRProjectileEffectTerrainColumn::HandleImpact(const FFRProjectileEffectSpec& EffectSpec, const FFRProjectileEffectImpactContext& Context) const
{
	const FFRProjectileEffectTerrainColumnParams& Params = EffectSpec.GetParametersOrDefault<FFRProjectileEffectTerrainColumnParams>();
	if (Params.Radius <= 0.0f || Params.Height <= 0.0f)
	{
		return;
	}

	const float StepSpacing = FMath::Max(1.0f, Params.StepSpacing);
	ApplyToImpactTerrain(Context, [&Context, &Params, StepSpacing](AFRDestructibleTerrain& Terrain)
	{
		const int32 StepCount = FMath::Max(1, FMath::CeilToInt(Params.Height / StepSpacing));
		for (int32 Step = 0; Step <= StepCount; ++Step)
		{
			const float Alpha = static_cast<float>(Step) / static_cast<float>(StepCount);
			const FVector FillLocation = Context.ImpactLocation + FVector(0.0f, 0.0f, Params.Height * Alpha);
			Terrain.FillCircle(FillLocation, Params.Radius, Params.TextureLayer);
		}
	});
}

bool UFRProjectileEffectTerrainColumn::UsesCustomTerrainImpact(const FFRProjectileEffectSpec& EffectSpec) const
{
	return true;
}

void UFRProjectileEffectTerrainColumn::AddDataValidationIssues(const FFRProjectileEffectSpec& EffectSpec, TArray<FString>& Issues) const
{
	const FFRProjectileEffectTerrainColumnParams& Params = EffectSpec.GetParametersOrDefault<FFRProjectileEffectTerrainColumnParams>();
	if (Params.Radius <= 0.0f)
	{
		AddProjectileEffectValidationIssue(Issues, TEXT("terrain column radius must be positive"));
	}
	if (Params.Height <= 0.0f)
	{
		AddProjectileEffectValidationIssue(Issues, TEXT("terrain column height must be positive"));
	}
	if (Params.StepSpacing <= 0.0f)
	{
		AddProjectileEffectValidationIssue(Issues, TEXT("terrain column step spacing must be positive"));
	}
}
