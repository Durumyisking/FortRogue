// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Commandlets/Commandlet.h"
#include "FRGenerateCommonUIWidgetsCommandlet.generated.h"

UCLASS()
class FORTROGUEEDITOR_API UFRGenerateCommonUIWidgetsCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UFRGenerateCommonUIWidgetsCommandlet();

	virtual int32 Main(const FString& Params) override;
};
