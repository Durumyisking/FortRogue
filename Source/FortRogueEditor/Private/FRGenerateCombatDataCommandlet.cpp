// Copyright Epic Games, Inc. All Rights Reserved.

#include "FRGenerateCombatDataCommandlet.h"

#include "Characters/FRCharacterDefinition.h"
#include "Combat/FRBattleCharacter.h"
#include "Combat/FRTerrainMapDefinition.h"
#include "Combat/FRDestructibleTerrain.h"
#include "FRGameplayTags.h"
#include "Game/FRGameModeDataAsset.h"
#include "Blueprint/UserWidget.h"
#include "HAL/FileManager.h"
#include "Misc/CommandLine.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "PaperFlipbook.h"
#include "ProjectileEffects/FRProjectileEffect.h"
#include "Run/FRDefaultLoadoutDefinition.h"
#include "Run/FRStageRunDefinition.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "Weapons/FRWeaponDefinition.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Camera/CameraActor.h"
#include "Engine/World.h"

namespace
{
constexpr const TCHAR* WeaponPath = TEXT("/Game/FortRogue/Weapon");
constexpr const TCHAR* CharacterPath = TEXT("/Game/FortRogue/DataAsset/Character");
constexpr const TCHAR* LoadoutPath = TEXT("/Game/FortRogue/Loadout");
constexpr const TCHAR* MapPath = TEXT("/Game/FortRogue/Map");
constexpr const TCHAR* RunPath = TEXT("/Game/FortRogue/Run");
constexpr const TCHAR* ModePath = TEXT("/Game/FortRogue/DataAsset/Global");

template <typename AssetType>
AssetType* LoadOrCreateDataAsset(const TCHAR* PackagePath, const TCHAR* AssetName)
{
	const FString LongPackageName = FString::Printf(TEXT("%s/%s"), PackagePath, AssetName);
	const FString ObjectPath = FString::Printf(TEXT("%s.%s"), *LongPackageName, AssetName);
	if (AssetType* ExistingAsset = LoadObject<AssetType>(nullptr, *ObjectPath))
	{
		return ExistingAsset;
	}

	UPackage* Package = CreatePackage(*LongPackageName);
	if (!Package)
	{
		return nullptr;
	}

	AssetType* NewAsset = NewObject<AssetType>(Package, AssetType::StaticClass(), AssetName, RF_Public | RF_Standalone | RF_Transactional);
	if (NewAsset)
	{
		FAssetRegistryModule::AssetCreated(NewAsset);
		Package->MarkPackageDirty();
	}
	return NewAsset;
}

bool SaveDataAsset(UObject* Asset)
{
	if (!Asset)
	{
		return false;
	}

	UPackage* Package = Asset->GetOutermost();
	const FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(PackageFileName), true);

	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	SaveArgs.SaveFlags = SAVE_NoError;
	return UPackage::SavePackage(Package, Asset, *PackageFileName, SaveArgs);
}

FFRProjectileEffectSpec MakeDrillEffect(float RadiusBonus, float RadiusMultiplier)
{
	FFRProjectileEffectSpec Effect;
	Effect.EffectClass = UFRProjectileEffectDrill::StaticClass();
	FFRProjectileEffectDrillParams Params;
	Params.RadiusBonus = RadiusBonus;
	Params.RadiusMultiplier = RadiusMultiplier;
	Effect.Parameters.InitializeAs(FFRProjectileEffectDrillParams::StaticStruct(), reinterpret_cast<const uint8*>(&Params));
	return Effect;
}

FFRProjectileEffectSpec MakeTerrainColumnEffect(float Radius, float Height, float StepSpacing)
{
	FFRProjectileEffectSpec Effect;
	Effect.EffectClass = UFRProjectileEffectTerrainColumn::StaticClass();
	FFRProjectileEffectTerrainColumnParams Params;
	Params.Radius = Radius;
	Params.Height = Height;
	Params.StepSpacing = StepSpacing;
	Params.TextureLayer = 0;
	Effect.Parameters.InitializeAs(FFRProjectileEffectTerrainColumnParams::StaticStruct(), reinterpret_cast<const uint8*>(&Params));
	return Effect;
}

void ConfigureWeapon(UFRWeaponDefinition* WeaponDefinition, const FText& DisplayName, const FText& Description, FGameplayTag WeaponTag, float Damage, float BlastRadius, int32 ProjectilesPerShot, const TArray<FFRProjectileEffectSpec>& ProjectileEffects)
{
	if (!WeaponDefinition)
	{
		return;
	}

	WeaponDefinition->Modify();
	WeaponDefinition->Weapon = FFRWeaponSpec();
	WeaponDefinition->Weapon.DisplayName = DisplayName;
	WeaponDefinition->Weapon.Description = Description;
	WeaponDefinition->Weapon.WeaponTag = WeaponTag;
	WeaponDefinition->Weapon.Damage = Damage;
	WeaponDefinition->Weapon.BlastRadius = BlastRadius;
	WeaponDefinition->Weapon.ExplosionFullDamageRadius = 0.0f;
	WeaponDefinition->Weapon.TerrainDamage = BlastRadius;
	WeaponDefinition->Weapon.ProjectileSpeed = 1200.0f;
	WeaponDefinition->Weapon.Gravity = 980.0f;
	WeaponDefinition->Weapon.ProjectilesPerShot = FMath::Max(1, ProjectilesPerShot);
	WeaponDefinition->Weapon.ProjectileEffects = ProjectileEffects;
	WeaponDefinition->MarkPackageDirty();
}

UPaperFlipbook* LoadFlipbook(const TCHAR* AssetName)
{
	const FString ObjectPath = FString::Printf(TEXT("/Game/FortRogue/Character/GeneratedSprites/%s.%s"), AssetName, AssetName);
	return LoadObject<UPaperFlipbook>(nullptr, *ObjectPath);
}

void ConfigureCharacter(UFRCharacterDefinition* CharacterDefinition, const FText& DisplayName, UFRWeaponDefinition* BasicAttack, UFRWeaponDefinition* SpecialAttack, bool bCanUseSpecialAttack, float MaxHealth, float BonusDamage, float MaxMoveBudget, UPaperFlipbook* BodyFlipbook)
{
	if (!CharacterDefinition)
	{
		return;
	}

	CharacterDefinition->Modify();
	CharacterDefinition->DisplayName = DisplayName;
	CharacterDefinition->BodyFlipbook = BodyFlipbook;
	CharacterDefinition->MaxHealth = MaxHealth;
	CharacterDefinition->BonusDamage = BonusDamage;
	CharacterDefinition->MaxMoveBudget = MaxMoveBudget;
	CharacterDefinition->ShotPowerMultiplier = 1.0f;
	CharacterDefinition->BasicAttackDefinition = BasicAttack;
	CharacterDefinition->SpecialAttackDefinition = SpecialAttack;
	CharacterDefinition->bCanUseSpecialAttack = bCanUseSpecialAttack;
	CharacterDefinition->WeaponLoadout.Reset();
	CharacterDefinition->MarkPackageDirty();
}

FFRRewardChoice MakeWeaponReward(UFRWeaponDefinition* WeaponDefinition)
{
	FFRRewardChoice Reward;
	Reward.Type = EFRRewardType::Weapon;
	Reward.WeaponReward = WeaponDefinition;
	Reward.RewardWeight = 1.0f;
	Reward.bOfferOncePerRun = true;
	if (WeaponDefinition)
	{
		Reward.DisplayName = WeaponDefinition->Weapon.DisplayName;
		Reward.Description = WeaponDefinition->Weapon.Description;
		Reward.RewardTag = WeaponDefinition->Weapon.WeaponTag;
	}
	return Reward;
}
TArray<UObject*> ConfigureGameFlowModeDataAssets(UFRCharacterDefinition* Cannon, UFRStageRunDefinition* DefaultRun, UFRTerrainMapDefinition* TestMap)
{
	UFRGameModeDataAsset* MainMenuMode = LoadOrCreateDataAsset<UFRGameModeDataAsset>(ModePath, TEXT("DA_MainMenuMode"));
	if (MainMenuMode)
	{
		MainMenuMode->Modify();
		MainMenuMode->ModeName = TEXT("MainMenu");
		MainMenuMode->DisplayName = FText::FromString(TEXT("Main Menu"));
		MainMenuMode->Level = TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Game/FortRogue/Level/GameLevel.GameLevel")));
		MainMenuMode->HUDWidgetClass = TSoftClassPtr<UUserWidget>(FSoftObjectPath(TEXT("/Game/FortRogue/Widget/MainMenu/WBP_MainMenuHUD.WBP_MainMenuHUD_C")));
		MainMenuMode->InputMode = EFRGameFlowInputMode::UIOnly;
		MainMenuMode->bShowMouseCursor = true;
		MainMenuMode->StartMainGameButtonName = TEXT("GameStartButton");
		MainMenuMode->EntryPawnClass = nullptr;
		MainMenuMode->bStartBattleOnEnter = false;
		MainMenuMode->MarkPackageDirty();
	}

	UFRGameModeDataAsset* MainGameMode = LoadOrCreateDataAsset<UFRGameModeDataAsset>(ModePath, TEXT("DA_MainGameMode"));
	if (MainGameMode)
	{
		MainGameMode->Modify();
		MainGameMode->ModeName = TEXT("MainGame");
		MainGameMode->DisplayName = FText::FromString(TEXT("Main Game"));
		MainGameMode->Level = TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Game/FortRogue/Level/GameLevel.GameLevel")));
		MainGameMode->HUDWidgetClass = TSoftClassPtr<UUserWidget>(FSoftObjectPath(TEXT("/Game/FortRogue/Widget/MainGame/WBP_MainGameHUD.WBP_MainGameHUD_C")));
		MainGameMode->InputMode = EFRGameFlowInputMode::GameAndUI;
		MainGameMode->bShowMouseCursor = true;
		MainGameMode->StartMainGameButtonName = NAME_None;
		MainGameMode->EntryPawnClass = AFRBattleCharacter::StaticClass();
		MainGameMode->PlayerCharacterClass = AFRBattleCharacter::StaticClass();
		MainGameMode->EnemyCharacterClass = AFRBattleCharacter::StaticClass();
		MainGameMode->TerrainClass = AFRDestructibleTerrain::StaticClass();
		MainGameMode->CameraClass = ACameraActor::StaticClass();
		MainGameMode->PlayerDefinition = Cannon;
		MainGameMode->StageRunDefinition = DefaultRun;
		MainGameMode->TerrainMapDefinition = TestMap;
		MainGameMode->bStartBattleOnEnter = true;
		MainGameMode->MarkPackageDirty();
	}

	return { MainMenuMode, MainGameMode };
}

int32 GenerateGameFlowModeDataAssetsOnly()
{
	UFRCharacterDefinition* Cannon = LoadOrCreateDataAsset<UFRCharacterDefinition>(CharacterPath, TEXT("CD_Cannon"));
	UFRTerrainMapDefinition* TestMap = LoadOrCreateDataAsset<UFRTerrainMapDefinition>(MapPath, TEXT("TestMapDefinition"));
	UFRStageRunDefinition* DefaultRun = LoadOrCreateDataAsset<UFRStageRunDefinition>(RunPath, TEXT("DA_DefaultStageRun"));

	bool bSavedAll = true;
	for (UObject* Asset : ConfigureGameFlowModeDataAssets(Cannon, DefaultRun, TestMap))
	{
		bSavedAll &= SaveDataAsset(Asset);
	}

	UE_LOG(LogTemp, Display, TEXT("Generated FR game flow mode data assets. SavedAll=%s"), bSavedAll ? TEXT("true") : TEXT("false"));
	return bSavedAll ? 0 : 1;
}
}

UFRGenerateCombatDataCommandlet::UFRGenerateCombatDataCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

int32 UFRGenerateCombatDataCommandlet::Main(const FString& Params)
{
	if (Params.Contains(TEXT("GameFlowOnly"), ESearchCase::IgnoreCase) || FParse::Param(FCommandLine::Get(), TEXT("GameFlowOnly")))
	{
		return GenerateGameFlowModeDataAssetsOnly();
	}
	UFRWeaponDefinition* BasicShell = LoadOrCreateDataAsset<UFRWeaponDefinition>(WeaponPath, TEXT("WP_BasicShell"));
	UFRWeaponDefinition* CannonSpecial = LoadOrCreateDataAsset<UFRWeaponDefinition>(WeaponPath, TEXT("WP_CannonSpecial"));
	UFRWeaponDefinition* BanditSpecial = LoadOrCreateDataAsset<UFRWeaponDefinition>(WeaponPath, TEXT("WP_BanditSpecial"));
	UFRWeaponDefinition* MinerSpecial = LoadOrCreateDataAsset<UFRWeaponDefinition>(WeaponPath, TEXT("WP_MinerSpecial"));
	UFRWeaponDefinition* EngineerSpecial = LoadOrCreateDataAsset<UFRWeaponDefinition>(WeaponPath, TEXT("WP_EngineerSpecial"));

	ConfigureWeapon(BasicShell, FText::FromString(TEXT("Basic Shell")), FText::FromString(TEXT("Shared arcing shell used by every class as the basic attack.")), FRGameplayTags::Weapon_Shell, 35.0f, 135.0f, 1, {});
	ConfigureWeapon(CannonSpecial, FText::FromString(TEXT("Pinpoint Cannon")), FText::FromString(TEXT("High damage shot with a tiny blast radius.")), FRGameplayTags::Weapon_Cannon_Special, 105.0f, 24.0f, 1, {});
	ConfigureWeapon(BanditSpecial, FText::FromString(TEXT("Nine Shot")), FText::FromString(TEXT("Fires nine weak projectiles at once.")), FRGameplayTags::Weapon_Bandit_Special, 9.0f, 32.0f, 9, {});
	ConfigureWeapon(MinerSpecial, FText::FromString(TEXT("Drill Shot")), FText::FromString(TEXT("Digs into terrain on impact.")), FRGameplayTags::Weapon_Miner_Special, 20.0f, 78.0f, 1, { MakeDrillEffect(95.0f, 1.0f) });
	ConfigureWeapon(EngineerSpecial, FText::FromString(TEXT("Pillar Shot")), FText::FromString(TEXT("Creates a vertical terrain column at the impact point.")), FRGameplayTags::Weapon_Engineer_Special, 10.0f, 18.0f, 1, { MakeTerrainColumnEffect(42.0f, 360.0f, 28.0f) });

	UFRCharacterDefinition* Cannon = LoadOrCreateDataAsset<UFRCharacterDefinition>(CharacterPath, TEXT("CD_Cannon"));
	UFRCharacterDefinition* Bandit = LoadOrCreateDataAsset<UFRCharacterDefinition>(CharacterPath, TEXT("CD_Bandit"));
	UFRCharacterDefinition* Miner = LoadOrCreateDataAsset<UFRCharacterDefinition>(CharacterPath, TEXT("CD_Miner"));
	UFRCharacterDefinition* Engineer = LoadOrCreateDataAsset<UFRCharacterDefinition>(CharacterPath, TEXT("CD_Engineer"));
	UFRCharacterDefinition* EnemyGrunt = LoadOrCreateDataAsset<UFRCharacterDefinition>(CharacterPath, TEXT("CD_EnemyGrunt"));
	UFRCharacterDefinition* EnemyMarauder = LoadOrCreateDataAsset<UFRCharacterDefinition>(CharacterPath, TEXT("CD_EnemyMarauder"));

	ConfigureCharacter(Cannon, FText::FromString(TEXT("Cannon")), BasicShell, CannonSpecial, true, 115.0f, 4.0f, 390.0f, LoadFlipbook(TEXT("cannon_tank_flipbook")));
	ConfigureCharacter(Bandit, FText::FromString(TEXT("Bandit")), BasicShell, BanditSpecial, true, 90.0f, 0.0f, 455.0f, LoadFlipbook(TEXT("multi_missile_tank_flipbook")));
	ConfigureCharacter(Miner, FText::FromString(TEXT("Miner")), BasicShell, MinerSpecial, true, 105.0f, 0.0f, 405.0f, LoadFlipbook(TEXT("missile_tank_flipbook")));
	ConfigureCharacter(Engineer, FText::FromString(TEXT("Engineer")), BasicShell, EngineerSpecial, true, 100.0f, 0.0f, 420.0f, LoadFlipbook(TEXT("laser_tank_flipbook")));
	ConfigureCharacter(EnemyGrunt, FText::FromString(TEXT("Enemy Grunt")), BasicShell, nullptr, false, 75.0f, 0.0f, 360.0f, LoadFlipbook(TEXT("crossbow_tank_flipbook")));
	ConfigureCharacter(EnemyMarauder, FText::FromString(TEXT("Enemy Marauder")), BasicShell, BanditSpecial, true, 85.0f, 0.0f, 430.0f, LoadFlipbook(TEXT("multi_missile_tank_flipbook")));

	UFRDefaultLoadoutDefinition* DefaultLoadout = LoadOrCreateDataAsset<UFRDefaultLoadoutDefinition>(LoadoutPath, TEXT("DA_DefaultLoadout"));
	if (DefaultLoadout)
	{
		DefaultLoadout->Modify();
		DefaultLoadout->WeaponDefinitions.Reset();
		DefaultLoadout->WeaponDefinitions.Add(BasicShell);
		DefaultLoadout->ItemDefinitions.Reset();
		DefaultLoadout->MarkPackageDirty();
	}

	UFRTerrainMapDefinition* TestMap = LoadOrCreateDataAsset<UFRTerrainMapDefinition>(MapPath, TEXT("TestMapDefinition"));
	if (TestMap)
	{
		TestMap->Modify();
		TestMap->PlayerSpawnLocal = FVector(-448.0f, 0.0f, TestMap->CellsZ * TestMap->CellSize + 80.0f);
		TestMap->EnemySpawnLocal = FVector(448.0f, 0.0f, TestMap->CellsZ * TestMap->CellSize + 80.0f);
		TestMap->EnemyPlacements.Reset();

		FFREnemyPlacement GruntPlacement;
		GruntPlacement.CharacterDefinition = EnemyGrunt;
		GruntPlacement.SpawnLocal = FVector(360.0f, 0.0f, TestMap->CellsZ * TestMap->CellSize + 80.0f);
		GruntPlacement.bUseSpecialAttack = false;
		TestMap->EnemyPlacements.Add(GruntPlacement);

		FFREnemyPlacement MarauderPlacement;
		MarauderPlacement.CharacterDefinition = EnemyMarauder;
		MarauderPlacement.SpawnLocal = FVector(620.0f, 0.0f, TestMap->CellsZ * TestMap->CellSize + 80.0f);
		MarauderPlacement.bUseSpecialAttack = true;
		TestMap->EnemyPlacements.Add(MarauderPlacement);
		TestMap->MarkPackageDirty();
	}

	UFRStageRunDefinition* DefaultRun = LoadOrCreateDataAsset<UFRStageRunDefinition>(RunPath, TEXT("DA_DefaultStageRun"));
	if (DefaultRun)
	{
		DefaultRun->Modify();
		DefaultRun->DefaultTerrainMapDefinition = TestMap;
		DefaultRun->DefaultLoadoutDefinition = DefaultLoadout;
		DefaultRun->EnemyDefinitionPool.Reset();
		DefaultRun->EnemyDefinitionPool.Add(EnemyGrunt);
		DefaultRun->EnemyDefinitionPool.Add(EnemyMarauder);
		DefaultRun->RewardPool.Reset();
		DefaultRun->RewardPool.Add(MakeWeaponReward(CannonSpecial));
		DefaultRun->RewardPool.Add(MakeWeaponReward(BanditSpecial));
		DefaultRun->RewardPool.Add(MakeWeaponReward(MinerSpecial));
		DefaultRun->RewardPool.Add(MakeWeaponReward(EngineerSpecial));
		DefaultRun->MarkPackageDirty();
	}

	TArray<UObject*> ModeAssets = ConfigureGameFlowModeDataAssets(Cannon, DefaultRun, TestMap);
	TArray<UObject*> AssetsToSave = {
		BasicShell, CannonSpecial, BanditSpecial, MinerSpecial, EngineerSpecial,
		Cannon, Bandit, Miner, Engineer, EnemyGrunt, EnemyMarauder,
		DefaultLoadout, TestMap, DefaultRun
	};
	AssetsToSave.Append(ModeAssets);

	bool bSavedAll = true;
	for (UObject* Asset : AssetsToSave)
	{
		bSavedAll &= SaveDataAsset(Asset);
	}

	UE_LOG(LogTemp, Display, TEXT("Generated FR combat data assets. SavedAll=%s"), bSavedAll ? TEXT("true") : TEXT("false"));
	return bSavedAll ? 0 : 1;
}
