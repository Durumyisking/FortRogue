// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "FRAssetManager.generated.h"

UCLASS()
class FORTROGUE_API UFRAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	virtual void StartInitialLoading() override;

private:
	void HandleCompletedInitialScan();
	void DumpInitialScanState() const;
	static bool ShouldDumpInitialScanState();
};
