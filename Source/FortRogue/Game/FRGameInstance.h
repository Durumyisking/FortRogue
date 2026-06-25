// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "FRGameInstance.generated.h"

UCLASS()
class FORTROGUE_API UFRGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void OnStart() override;
};