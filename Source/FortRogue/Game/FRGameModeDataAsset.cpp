// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/FRGameModeDataAsset.h"

#include "Combat/FRBattleCamera.h"
#include "Combat/FRBattleCharacter.h"
#include "Combat/FRDestructibleTerrain.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"

FString UFRGameModeDataAsset::GetLevelPackageName() const
{
	const FSoftObjectPath LevelPath = Level.ToSoftObjectPath();
	return LevelPath.IsValid() ? LevelPath.GetLongPackageName() : FString();
}

TSubclassOf<UUserWidget> UFRGameModeDataAsset::LoadHUDWidgetClass() const
{
	return HUDWidgetClass.LoadSynchronous();
}

TSubclassOf<APawn> UFRGameModeDataAsset::LoadEntryPawnClass() const
{
	return EntryPawnClass.LoadSynchronous();
}

TSubclassOf<AFRBattleCharacter> UFRGameModeDataAsset::LoadPlayerCharacterClass() const
{
	if (TSubclassOf<AFRBattleCharacter> LoadedClass = PlayerCharacterClass.LoadSynchronous())
	{
		return LoadedClass;
	}

	const TSubclassOf<APawn> LoadedEntryPawnClass = LoadEntryPawnClass();
	if (LoadedEntryPawnClass && LoadedEntryPawnClass->IsChildOf(AFRBattleCharacter::StaticClass()))
	{
		return TSubclassOf<AFRBattleCharacter>(*LoadedEntryPawnClass);
	}

	return nullptr;
}

TSubclassOf<AFRBattleCharacter> UFRGameModeDataAsset::LoadEnemyCharacterClass() const
{
	return EnemyCharacterClass.LoadSynchronous();
}

TSubclassOf<AFRDestructibleTerrain> UFRGameModeDataAsset::LoadTerrainClass() const
{
	return TerrainClass.LoadSynchronous();
}

TSubclassOf<AFRBattleCamera> UFRGameModeDataAsset::LoadCameraClass() const
{
	return CameraClass.LoadSynchronous();
}