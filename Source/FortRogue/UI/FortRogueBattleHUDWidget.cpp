// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FortRogueBattleHUDWidget.h"

#include "FortRogueGameMode.h"

AFortRogueGameMode* UFortRogueBattleHUDWidget::GetFortRogueGameMode() const
{
	return GetWorld() ? GetWorld()->GetAuthGameMode<AFortRogueGameMode>() : nullptr;
}
