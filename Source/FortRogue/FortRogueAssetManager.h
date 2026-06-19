// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "FortRogueAssetManager.generated.h"

UCLASS()
class FORTROGUE_API UFortRogueAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	virtual void StartInitialLoading() override;

private:
	void HandleCompletedInitialScan();
	void DumpInitialScanState() const;
	static bool ShouldDumpInitialScanState();
};
