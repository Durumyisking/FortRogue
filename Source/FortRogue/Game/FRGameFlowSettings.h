// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "FRGameFlowSettings.generated.h"

class UFRBattleCharacterAimIndicatorWidget;
class UFRBattleCharacterStatusWidget;
class UFRGameModeDataAsset;

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "FortRogue Game Flow"))
class FORTROGUE_API UFRGameFlowSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game Flow")
	TSoftObjectPtr<UFRGameModeDataAsset> StartupModeData;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game Flow")
	TSoftObjectPtr<UFRGameModeDataAsset> MainMenuModeData;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Game Flow")
	TSoftObjectPtr<UFRGameModeDataAsset> MainGameModeData;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Combat UI", meta = (ToolTip = "전투 캐릭터의 조준 각도 인디케이터 위젯 기본 클래스입니다. 캐릭터에 별도 지정이 없을 때 사용합니다."))
	TSoftClassPtr<UFRBattleCharacterAimIndicatorWidget> DefaultAimIndicatorWidgetClass;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Combat UI", meta = (ToolTip = "전투 캐릭터의 체력 표시 위젯 기본 클래스입니다. 캐릭터에 별도 지정이 없을 때 사용합니다."))
	TSoftClassPtr<UFRBattleCharacterStatusWidget> DefaultCharacterStatusWidgetClass;
};