// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "AbilitySystem/Attributes/FRCombatSet.h"
#include "AbilitySystem/FRAbilitySet.h"
#include "Characters/FRCharacterDefinition.h"
#include "Combat/FRBattleCamera.h"
#include "Combat/FRBattleCharacter.h"
#include "Combat/FRCharacterSpriteAnimator.h"
#include "Combat/FRDestructibleTerrain.h"
#include "Combat/FRProjectile.h"
#include "Combat/FRTerrainMapDefinition.h"
#include "Combat/FRTerrainMovementComponent.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FRGameMode.h"
#include "FRGameplayTags.h"
#include "Game/FRRunSubsystem.h"
#include "Items/FRItemDefinition.h"
#include "Items/FRItemEffect.h"
#include "Kismet/GameplayStatics.h"
#include "PaperFlipbook.h"
#include "PaperFlipbookComponent.h"
#include "Perks/FRPerkDefinition.h"
#include "ProjectileEffects/FRProjectileEffect.h"
#include "ProjectileEffects/FRProjectileSpecEffects.h"
#include "ProjectileEffects/FRProjectileSplitEffect.h"
#include "Rewards/FRRewardBlueprintLibrary.h"
#include "Rewards/FRRewardGrant.h"
#include "Rewards/FRRewardTypes.h"
#include "Run/FRDefaultLoadoutDefinition.h"
#include "Run/FRStageRunDefinition.h"
#include "Weapons/FRWeaponDefinition.h"
#include "UObject/UnrealType.h"

namespace
{
UFRWeaponDefinition* CreateTestWeaponDefinition(UObject* Outer)
{
	UFRWeaponDefinition* WeaponDefinition = NewObject<UFRWeaponDefinition>(Outer);
	WeaponDefinition->Weapon.DisplayName = FText::FromString(TEXT("Test Shell"));
	WeaponDefinition->Weapon.HitDamage = 0.0f;
	WeaponDefinition->Weapon.Damage = 35.0f;
	WeaponDefinition->Weapon.BlastRadius = 145.0f;
	WeaponDefinition->Weapon.ExplosionFullDamageRadius = 0.0f;
	WeaponDefinition->Weapon.TerrainDamage = 145.0f;
	WeaponDefinition->Weapon.ProjectileSpeed = 1180.0f;
	WeaponDefinition->Weapon.ProjectilesPerShot = 1;
	return WeaponDefinition;
}

UFRDefaultLoadoutDefinition* CreateTestDefaultLoadout(UObject* Outer)
{
	UFRDefaultLoadoutDefinition* LoadoutDefinition = NewObject<UFRDefaultLoadoutDefinition>(Outer);
	LoadoutDefinition->WeaponDefinitions.Add(CreateTestWeaponDefinition(LoadoutDefinition));
	return LoadoutDefinition;
}

void AddTestPerkGrant(FFRRewardChoice& Reward, UFRPerkDefinition* PerkDefinition)
{
	UFRRewardGrant_Perk* Grant = NewObject<UFRRewardGrant_Perk>(PerkDefinition ? static_cast<UObject*>(PerkDefinition) : GetTransientPackage());
	Grant->PerkDefinition = PerkDefinition;
	Reward.Grants.Add(Grant);
}

void AddTestItemGrant(FFRRewardChoice& Reward, UFRItemDefinition* ItemDefinition)
{
	UFRRewardGrant_Item* Grant = NewObject<UFRRewardGrant_Item>(ItemDefinition ? static_cast<UObject*>(ItemDefinition) : GetTransientPackage());
	Grant->ItemDefinition = ItemDefinition;
	Reward.Grants.Add(Grant);
}

void AddTestWeaponGrant(FFRRewardChoice& Reward, UFRWeaponDefinition* WeaponDefinition)
{
	UFRRewardGrant_Weapon* Grant = NewObject<UFRRewardGrant_Weapon>(WeaponDefinition ? static_cast<UObject*>(WeaponDefinition) : GetTransientPackage());
	Grant->WeaponDefinition = WeaponDefinition;
	Reward.Grants.Add(Grant);
}

void AddTestHealEffect(UFRItemDefinition* ItemDefinition, float HealAmount)
{
	UFRItemEffect_Heal* HealEffect = NewObject<UFRItemEffect_Heal>(ItemDefinition);
	HealEffect->HealAmount = HealAmount;
	ItemDefinition->UseEffects.Add(HealEffect);
}

void AddTestAttackMultiplierEffect(UFRItemDefinition* ItemDefinition, float Multiplier)
{
	UFRItemEffect_AttackMultiplier* AttackEffect = NewObject<UFRItemEffect_AttackMultiplier>(ItemDefinition);
	AttackEffect->AttackMultiplier = Multiplier;
	ItemDefinition->UseEffects.Add(AttackEffect);
}

bool TestGameplayTagCategories(FAutomationTestBase& Test, const UStruct* Struct, FName PropertyName, const TCHAR* ExpectedCategories)
{
	const FString StructName = Struct ? Struct->GetName() : TEXT("<null>");
	const FProperty* Property = Struct ? Struct->FindPropertyByName(PropertyName) : nullptr;
	Test.TestNotNull(FString::Printf(TEXT("%s.%s exists"), *StructName, *PropertyName.ToString()), Property);
	if (!Property)
	{
		return false;
	}

	return Test.TestEqual(
		FString::Printf(TEXT("%s.%s gameplay tag categories"), *StructName, *PropertyName.ToString()),
		Property->GetMetaData(TEXT("Categories")),
		FString(ExpectedCategories));
}

bool TestPropertyMetaData(FAutomationTestBase& Test, const UStruct* Struct, FName PropertyName, const TCHAR* MetaDataKey, const TCHAR* ExpectedValue)
{
	const FString StructName = Struct ? Struct->GetName() : TEXT("<null>");
	const FProperty* Property = Struct ? Struct->FindPropertyByName(PropertyName) : nullptr;
	Test.TestNotNull(FString::Printf(TEXT("%s.%s exists"), *StructName, *PropertyName.ToString()), Property);
	if (!Property)
	{
		return false;
	}

	return Test.TestEqual(
		FString::Printf(TEXT("%s.%s %s metadata"), *StructName, *PropertyName.ToString(), MetaDataKey),
		Property->GetMetaData(MetaDataKey),
		FString(ExpectedValue));
}

bool TestPropertyAdvancedDisplay(FAutomationTestBase& Test, const UStruct* Struct, FName PropertyName)
{
	const FString StructName = Struct ? Struct->GetName() : TEXT("<null>");
	const FProperty* Property = Struct ? Struct->FindPropertyByName(PropertyName) : nullptr;
	Test.TestNotNull(FString::Printf(TEXT("%s.%s exists"), *StructName, *PropertyName.ToString()), Property);
	if (!Property)
	{
		return false;
	}

	return Test.TestTrue(
		FString::Printf(TEXT("%s.%s is advanced display"), *StructName, *PropertyName.ToString()),
		Property->HasAnyPropertyFlags(CPF_AdvancedDisplay));
}

bool TestFunctionParamGameplayTagCategories(FAutomationTestBase& Test, const UClass* Class, FName FunctionName, FName ParamName, const TCHAR* ExpectedCategories)
{
	const FString ClassName = Class ? Class->GetName() : TEXT("<null>");
	const UFunction* Function = Class ? Class->FindFunctionByName(FunctionName) : nullptr;
	Test.TestNotNull(FString::Printf(TEXT("%s.%s function exists"), *ClassName, *FunctionName.ToString()), Function);
	if (!Function)
	{
		return false;
	}

	const FProperty* ParamProperty = Function->FindPropertyByName(ParamName);
	Test.TestNotNull(FString::Printf(TEXT("%s.%s.%s param exists"), *ClassName, *FunctionName.ToString(), *ParamName.ToString()), ParamProperty);
	if (!ParamProperty)
	{
		return false;
	}

	return Test.TestEqual(
		FString::Printf(TEXT("%s.%s.%s gameplay tag categories"), *ClassName, *FunctionName.ToString(), *ParamName.ToString()),
		ParamProperty->GetMetaData(TEXT("Categories")),
		FString(ExpectedCategories));
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFRCharacterAttackSlotDefinitionTest, "FortRogue.Character.AttackSlots.UseSharedWeaponDefinition", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFRCharacterAttackSlotDefinitionTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>(GEngine);
	TestNotNull(TEXT("Transient game instance is created"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("Transient test world is created"), World);
	if (!World)
	{
		return false;
	}

	World->SetShouldTick(false);
	World->AddToRoot();

	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.OwningGameInstance = GameInstance;
	World->SetGameInstance(GameInstance);
	WorldContext.SetCurrentWorld(World);
	GameInstance->Init();

	auto CleanupWorld = [&World, &GameInstance]()
	{
		World->RemoveFromRoot();
		GameInstance->Shutdown();
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	};

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AFRBattleCharacter* Character = World->SpawnActor<AFRBattleCharacter>(AFRBattleCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Battle character is spawned"), Character);
	if (!Character)
	{
		CleanupWorld();
		return false;
	}
	FURL URL;
	World->InitializeActorsForPlay(URL);

	UFRWeaponDefinition* BasicAttack = CreateTestWeaponDefinition(Character);
	BasicAttack->Weapon.WeaponTag = FRGameplayTags::Weapon_Shell;

	UFRWeaponDefinition* SpecialAttack = CreateTestWeaponDefinition(Character);
	SpecialAttack->Weapon.DisplayName = FText::FromString(TEXT("Special"));
	SpecialAttack->Weapon.WeaponTag = FRGameplayTags::Weapon_Cannon_Special;

	UFRCharacterDefinition* CharacterDefinition = NewObject<UFRCharacterDefinition>(Character);
	CharacterDefinition->BasicAttackDefinition = BasicAttack;
	CharacterDefinition->SpecialAttackDefinition = SpecialAttack;
	CharacterDefinition->bCanUseSpecialAttack = false;

	Character->InitializeFromDefinition(CharacterDefinition);
	TestEqual(TEXT("Character loads basic and special attacks from CharacterDefinition"), Character->GetWeaponLoadout().Num(), 2);
	TestTrue(TEXT("Basic attack slot remains selectable"), Character->CanSelectWeapon(0));
	TestFalse(TEXT("Disabled special attack slot is not selectable"), Character->CanSelectSpecialAttack());
	TestFalse(TEXT("Generic weapon selection also respects disabled special slot"), Character->CanSelectWeapon(1));

	Character->SetSpecialAttackEnabled(true);
	TestTrue(TEXT("Special attack can be enabled per placement/runtime"), Character->CanSelectSpecialAttack());
	TestTrue(TEXT("Special attack selection uses the same weapon slot data"), Character->SelectSpecialAttack());
	TestEqual(TEXT("Special attack is stored in slot 1"), Character->GetSelectedWeaponIndex(), 1);

	Character->SetSpecialAttackEnabled(false);
	TestEqual(TEXT("Disabling special attack falls back to basic attack"), Character->GetSelectedWeaponIndex(), 0);

	CleanupWorld();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFRGameModeEnemyTurnContinuationTest, "FortRogue.GameMode.EnemyTurnContinuation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFRGameModeEnemyTurnContinuationTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>(GEngine);
	TestNotNull(TEXT("Transient game instance is created"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("Transient test world is created"), World);
	if (!World)
	{
		return false;
	}

	World->SetShouldTick(false);
	World->AddToRoot();

	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.OwningGameInstance = GameInstance;
	World->SetGameInstance(GameInstance);
	WorldContext.SetCurrentWorld(World);
	GameInstance->Init();

	auto CleanupWorld = [&World, &GameInstance]()
	{
		if (World->HasBegunPlay())
		{
			World->BeginTearingDown();
			World->EndPlay(EEndPlayReason::Quit);
		}

		World->RemoveFromRoot();
		GameInstance->Shutdown();
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	};

	UFRTerrainMapDefinition* Map = NewObject<UFRTerrainMapDefinition>();
	Map->Resize(20, 6);
	Map->CellSize = 10.0f;
	Map->Clear(false);
	Map->FillRect(0, 0, 19, 0, true);
	Map->PlayerSpawnLocal = FVector(-40.0f, 0.0f, 140.0f);
	Map->EnemySpawnLocal = FVector(40.0f, 0.0f, 140.0f);

	FURL URL;
	World->SetGameMode(URL);
	AFRGameMode* GameMode = World->GetAuthGameMode<AFRGameMode>();
	TestNotNull(TEXT("FortRogue game mode is created"), GameMode);
	if (!GameMode)
	{
		CleanupWorld();
		return false;
	}

	FObjectProperty* TerrainMapProperty = FindFProperty<FObjectProperty>(AFRGameMode::StaticClass(), TEXT("TerrainMapDefinition"));
	TestNotNull(TEXT("Game mode exposes a terrain map definition property"), TerrainMapProperty);
	if (!TerrainMapProperty)
	{
		CleanupWorld();
		return false;
	}
	TerrainMapProperty->SetObjectPropertyValue_InContainer(GameMode, Map);

	UFRStageRunDefinition* StageRunDefinition = NewObject<UFRStageRunDefinition>(GameMode);
	StageRunDefinition->StageCount = 1;
	StageRunDefinition->NormalizeStageData();
	UFRCharacterDefinition* FirstEnemyDefinition = NewObject<UFRCharacterDefinition>(StageRunDefinition);
	FirstEnemyDefinition->BasicAttackDefinition = CreateTestWeaponDefinition(FirstEnemyDefinition);
	StageRunDefinition->EnemyDefinitionPool = { FirstEnemyDefinition };
	GameMode->StageRunDefinition = StageRunDefinition;

	World->InitializeActorsForPlay(URL);
	World->BeginPlay();

	AFRBattleCharacter* FirstEnemy = GameMode->GetEnemyCharacter();
	TestNotNull(TEXT("Game mode spawns the first enemy"), FirstEnemy);
	if (!FirstEnemy)
	{
		CleanupWorld();
		return false;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AFRBattleCharacter* SecondEnemy = World->SpawnActor<AFRBattleCharacter>(AFRBattleCharacter::StaticClass(), FirstEnemy->GetActorLocation() + FVector(-30.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Second enemy is spawned for enemy turn continuation"), SecondEnemy);
	if (!SecondEnemy)
	{
		CleanupWorld();
		return false;
	}

	SecondEnemy->ConfigureAsEnemy(true);
	SecondEnemy->SetTerrain(GameMode->Terrain);
	SecondEnemy->AddWeaponDefinition(CreateTestWeaponDefinition(SecondEnemy));
	SecondEnemy->BeginTurn();
	GameMode->EnemyCharacters.Add(SecondEnemy);
	AFRTurnBasedGameState* TurnState = GameMode->GetTurnBasedGameState();
	TestNotNull(TEXT("Turn-based game state exists"), TurnState);
	if (!TurnState)
	{
		CleanupWorld();
		return false;
	}
	TurnState->StartEnemyTurn(GameMode->GetEnemyCharacters(), GameMode->GetWind(), FText::FromString(TEXT("Enemy turn")));
	TurnState->AdvanceToNextEnemyTurnCharacter(GameMode->GetEnemyCharacters());
	TurnState->EnterShotResolution(FirstEnemy, 1, FText::FromString(TEXT("Enemy shell incoming")));

	GameMode->FinishShotResolution();

	TestEqual(TEXT("Enemy shot resolution resumes enemy turn before selecting the next enemy"), GameMode->GetBattleState(), EFRBattleState::ResolvingShot);
	TestTrue(TEXT("Second enemy fires after the first enemy shot resolves"), SecondEnemy->HasFiredThisTurn());
	TestFalse(TEXT("Second enemy turn ends after firing"), SecondEnemy->IsActiveTurn());
	TestEqual(TEXT("Second enemy becomes the last shooter"), TurnState->GetLastShooter(), SecondEnemy);

	CleanupWorld();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFRTerrainMapEnemyPlacementTest, "FortRogue.Terrain.MapDefinition.EnemyPlacements", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFRTerrainMapEnemyPlacementTest::RunTest(const FString& Parameters)
{
	UFRTerrainMapDefinition* Map = NewObject<UFRTerrainMapDefinition>();
	TestNotNull(TEXT("Map asset object is created"), Map);
	if (!Map)
	{
		return false;
	}

	Map->Resize(100, 50);
	Map->CellSize = 2.0f;

	FFREnemyPlacement EnemyPlacement;
	EnemyPlacement.CharacterDefinition = NewObject<UFRCharacterDefinition>(Map);
	EnemyPlacement.SpawnLocal = FVector(0.0f, 0.0f, 180.0f);
	EnemyPlacement.bUseSpecialAttack = true;
	Map->EnemyPlacements.Add(EnemyPlacement);

	Map->ResizeResampled(200, 100);
	TestEqual(TEXT("Enemy placement count survives map resize"), Map->EnemyPlacements.Num(), 1);
	TestTrue(TEXT("Enemy placement keeps horizontal center after resize"), FMath::IsNearlyEqual(Map->EnemyPlacements[0].SpawnLocal.X, 0.0f));
	TestTrue(TEXT("Enemy placement keeps height clearance after resize"), FMath::IsNearlyEqual(Map->EnemyPlacements[0].SpawnLocal.Z, 280.0f));
	TestTrue(TEXT("Enemy placement keeps special attack flag after resize"), Map->EnemyPlacements[0].bUseSpecialAttack);

	Map->SetCellSizePreservingSpawns(1.0f);
	TestTrue(TEXT("Enemy placement keeps horizontal center after cell size change"), FMath::IsNearlyEqual(Map->EnemyPlacements[0].SpawnLocal.X, 0.0f));
	TestTrue(TEXT("Enemy placement keeps height clearance after cell size change"), FMath::IsNearlyEqual(Map->EnemyPlacements[0].SpawnLocal.Z, 180.0f));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFRTerrainMapDefinitionEditTest, "FortRogue.Terrain.MapDefinition.Edits", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFRTerrainMapDefinitionEditTest::RunTest(const FString& Parameters)
{
	UFRTerrainMapDefinition* Map = NewObject<UFRTerrainMapDefinition>();
	TestNotNull(TEXT("Map asset object is created"), Map);
	TestEqual(TEXT("Default map width uses 4x Fortress screen pixels"), Map->CellsX, 1280);
	TestEqual(TEXT("Default map height uses 4x Fortress screen pixels"), Map->CellsZ, 960);
	TestEqual(TEXT("Default map uses one-unit world cells"), Map->CellSize, 1.0f);
	const float HalfMapWidth = Map->CellsX * Map->CellSize * 0.5f;
	TestTrue(TEXT("Default player spawn is inside the map width"), FMath::Abs(Map->PlayerSpawnLocal.X) < HalfMapWidth);
	TestTrue(TEXT("Default enemy spawn is inside the map width"), FMath::Abs(Map->EnemySpawnLocal.X) < HalfMapWidth);
	TestTrue(TEXT("Default spawns start above the map"), Map->PlayerSpawnLocal.Z > Map->CellsZ * Map->CellSize && Map->EnemySpawnLocal.Z > Map->CellsZ * Map->CellSize);

	UFRStageRunDefinition* StageRun = NewObject<UFRStageRunDefinition>();
	TestNotNull(TEXT("Stage run definition object is created"), StageRun);
	if (StageRun)
	{
		TestEqual(TEXT("Stage run starts with seven difficulty rows"), StageRun->StageDifficultyData.Num(), 7);
		StageRun->StageCount = 3;
		StageRun->NormalizeStageData();
		TestEqual(TEXT("Stage run difficulty rows follow the stage count"), StageRun->StageDifficultyData.Num(), 3);
		StageRun->RewardChoiceCount = 9;
		StageRun->NormalizeStageData();
		TestEqual(TEXT("Stage run clamps reward choice count maximum"), StageRun->RewardChoiceCount, 5);
		StageRun->RewardChoiceCount = 0;
		StageRun->NormalizeStageData();
		TestEqual(TEXT("Stage run clamps reward choice count minimum"), StageRun->RewardChoiceCount, 1);
		StageRun->StageDifficultyData[2].EnemyTurnDelaySeconds = 0.25f;
		TestEqual(TEXT("Stage difficulty lookup uses one-based stage numbers"), StageRun->GetStageDifficulty(3).EnemyTurnDelaySeconds, 0.25f);
	}

	UFRStageRunDefinition* InvalidStageRunData = NewObject<UFRStageRunDefinition>();
	InvalidStageRunData->StageCount = 0;
	InvalidStageRunData->RewardChoiceCount = 0;
	InvalidStageRunData->StageDifficultyData.Reset();
	InvalidStageRunData->StageDifficultyData.AddDefaulted();
	FFRRewardChoice InvalidStageReward;
	InvalidStageRunData->RewardPool.Add(InvalidStageReward);
	const FString InvalidStageRunDataSummary = InvalidStageRunData->GetDataValidationSummary().ToString();
	TestTrue(TEXT("Stage run data validation reports invalid stage counts"), InvalidStageRunDataSummary.Contains(TEXT("stage count")));
	TestTrue(TEXT("Stage run data validation reports invalid reward choice counts"), InvalidStageRunDataSummary.Contains(TEXT("reward choice count")));
	TestTrue(TEXT("Stage run data validation reports difficulty row mismatches"), InvalidStageRunDataSummary.Contains(TEXT("stage difficulty rows")));
	TestTrue(TEXT("Stage run data validation reports nested reward warnings"), InvalidStageRunDataSummary.Contains(TEXT("reward pool data")));
	UFRStageRunDefinition* LockedStageRunData = NewObject<UFRStageRunDefinition>();
	UFRPerkDefinition* LockedRewardPerk = NewObject<UFRPerkDefinition>(LockedStageRunData);
	LockedRewardPerk->DisplayName = FText::FromString(TEXT("Locked Reward Perk"));
	LockedRewardPerk->PerkTag = FRGameplayTags::Trait_Damage;
	LockedRewardPerk->DamageBonus = 1.0f;
	FFRRewardChoice LockedReward;
	LockedReward.DisplayName = FText::FromString(TEXT("Locked Reward"));
	AddTestPerkGrant(LockedReward, LockedRewardPerk);
	LockedReward.RequiredRewardTags.AddTag(FRGameplayTags::Trait_Damage);
	LockedStageRunData->RewardPool.Add(LockedReward);
	LockedStageRunData->RewardChoiceCount = 1;
	TestTrue(TEXT("Stage run data validation reports missing starting rewards"), LockedStageRunData->GetDataValidationSummary().ToString().Contains(TEXT("run start")));
	TestTrue(TEXT("Blueprint helper reports stage run data validation"), UFRRewardBlueprintLibrary::GetStageRunDataValidationSummary(InvalidStageRunData).ToString().Contains(TEXT("stage count")));
	TestTrue(TEXT("Blueprint helper reports missing stage run assets"), UFRRewardBlueprintLibrary::GetStageRunDataValidationSummary(nullptr).ToString().Contains(TEXT("missing stage run")));
	UFRStageRunDefinition* ValidStageRunData = NewObject<UFRStageRunDefinition>();
	UFRPerkDefinition* ValidStageRewardPerk = NewObject<UFRPerkDefinition>(ValidStageRunData);
	ValidStageRewardPerk->DisplayName = FText::FromString(TEXT("Valid Stage Perk"));
	ValidStageRewardPerk->PerkTag = FRGameplayTags::Trait_Damage;
	ValidStageRewardPerk->DamageBonus = 1.0f;
	FFRRewardChoice ValidStageReward;
	ValidStageReward.DisplayName = FText::FromString(TEXT("Valid Stage Reward"));
	AddTestPerkGrant(ValidStageReward, ValidStageRewardPerk);
	ValidStageRunData->RewardPool.Add(ValidStageReward);
	ValidStageRunData->RewardChoiceCount = 1;
	TestTrue(TEXT("Stage run data validation is empty for valid run data"), ValidStageRunData->GetDataValidationSummary().ToString().IsEmpty());

	UFRDefaultLoadoutDefinition* InvalidLoadoutData = NewObject<UFRDefaultLoadoutDefinition>();
	InvalidLoadoutData->WeaponDefinitions.Add(nullptr);
	FFRItemStack InvalidItemStack;
	InvalidItemStack.Charges = 0;
	InvalidLoadoutData->ItemDefinitions.Add(InvalidItemStack);
	const FString InvalidLoadoutDataSummary = InvalidLoadoutData->GetDataValidationSummary().ToString();
	TestTrue(TEXT("Default loadout data validation reports missing weapon entries"), InvalidLoadoutDataSummary.Contains(TEXT("missing weapon entry")));
	TestTrue(TEXT("Default loadout data validation reports missing item entries"), InvalidLoadoutDataSummary.Contains(TEXT("missing item entry")));
	TestTrue(TEXT("Default loadout data validation reports invalid item charges"), InvalidLoadoutDataSummary.Contains(TEXT("item charges")));
	UFRDefaultLoadoutDefinition* EmptyLoadoutData = NewObject<UFRDefaultLoadoutDefinition>();
	TestTrue(TEXT("Default loadout data validation reports empty weapon lists"), EmptyLoadoutData->GetDataValidationSummary().ToString().Contains(TEXT("weapon definitions are empty")));
	UFRDefaultLoadoutDefinition* NestedInvalidLoadoutData = NewObject<UFRDefaultLoadoutDefinition>();
	NestedInvalidLoadoutData->WeaponDefinitions.Add(NewObject<UFRWeaponDefinition>(NestedInvalidLoadoutData));
	FFRItemStack NestedInvalidItemStack;
	NestedInvalidItemStack.ItemDefinition = NewObject<UFRItemDefinition>(NestedInvalidLoadoutData);
	NestedInvalidLoadoutData->ItemDefinitions.Add(NestedInvalidItemStack);
	const FString NestedInvalidLoadoutDataSummary = NestedInvalidLoadoutData->GetDataValidationSummary().ToString();
	TestTrue(TEXT("Default loadout data validation reports nested weapon warnings"), NestedInvalidLoadoutDataSummary.Contains(TEXT("weapon data")));
	TestTrue(TEXT("Default loadout data validation reports nested item warnings"), NestedInvalidLoadoutDataSummary.Contains(TEXT("item data")));
	TestTrue(TEXT("Blueprint helper reports default loadout data validation"), UFRRewardBlueprintLibrary::GetDefaultLoadoutDataValidationSummary(InvalidLoadoutData).ToString().Contains(TEXT("missing weapon entry")));
	TestTrue(TEXT("Blueprint helper reports missing default loadout assets"), UFRRewardBlueprintLibrary::GetDefaultLoadoutDataValidationSummary(nullptr).ToString().Contains(TEXT("missing loadout")));
	UFRDefaultLoadoutDefinition* ValidLoadoutData = NewObject<UFRDefaultLoadoutDefinition>();
	UFRWeaponDefinition* ValidLoadoutWeapon = NewObject<UFRWeaponDefinition>(ValidLoadoutData);
	ValidLoadoutWeapon->Weapon.DisplayName = FText::FromString(TEXT("Valid Loadout Shell"));
	ValidLoadoutWeapon->Weapon.WeaponTag = FRGameplayTags::Weapon_Shell;
	ValidLoadoutWeapon->Weapon.Damage = 10.0f;
	ValidLoadoutWeapon->Weapon.BlastRadius = 100.0f;
	ValidLoadoutWeapon->Weapon.ProjectileSpeed = 1000.0f;
	ValidLoadoutWeapon->Weapon.ProjectilesPerShot = 1;
	ValidLoadoutData->WeaponDefinitions.Add(ValidLoadoutWeapon);
	TestTrue(TEXT("Default loadout data validation is empty for valid loadout data"), ValidLoadoutData->GetDataValidationSummary().ToString().IsEmpty());

	TestGameplayTagCategories(*this, FFRShotModifierSpec::StaticStruct(), GET_MEMBER_NAME_CHECKED(FFRShotModifierSpec, EffectTags), TEXT("ShotEffect"));
	TestGameplayTagCategories(*this, FFRShotModifierSpec::StaticStruct(), GET_MEMBER_NAME_CHECKED(FFRShotModifierSpec, ModifierTag), TEXT("ShotEffect"));
	TestGameplayTagCategories(*this, FFRShotModifierSpec::StaticStruct(), GET_MEMBER_NAME_CHECKED(FFRShotModifierSpec, RequiredShotTags), TEXT("Weapon,ShotEffect"));
	TestGameplayTagCategories(*this, FFRShotModifierSpec::StaticStruct(), GET_MEMBER_NAME_CHECKED(FFRShotModifierSpec, BlockedShotTags), TEXT("Weapon,ShotEffect"));
	TestGameplayTagCategories(*this, FFRWeaponSpec::StaticStruct(), GET_MEMBER_NAME_CHECKED(FFRWeaponSpec, WeaponTag), TEXT("Weapon"));
	TestGameplayTagCategories(*this, FFRWeaponSpec::StaticStruct(), GET_MEMBER_NAME_CHECKED(FFRWeaponSpec, ShotEffectTags), TEXT("ShotEffect"));
	TestGameplayTagCategories(*this, UFRItemDefinition::StaticClass(), GET_MEMBER_NAME_CHECKED(UFRItemDefinition, ItemTag), TEXT("Item"));
	TestGameplayTagCategories(*this, FFRRewardChoice::StaticStruct(), GET_MEMBER_NAME_CHECKED(FFRRewardChoice, RewardTag), TEXT("Weapon,Item,Trait"));
	TestGameplayTagCategories(*this, FFRRewardChoice::StaticStruct(), GET_MEMBER_NAME_CHECKED(FFRRewardChoice, RequiredRewardTags), TEXT("Weapon,Item,Trait"));
	TestGameplayTagCategories(*this, FFRRewardChoice::StaticStruct(), GET_MEMBER_NAME_CHECKED(FFRRewardChoice, BlockedRewardTags), TEXT("Weapon,Item,Trait"));
	TestGameplayTagCategories(*this, FFRShotSpec::StaticStruct(), GET_MEMBER_NAME_CHECKED(FFRShotSpec, WeaponTag), TEXT("Weapon"));
	TestGameplayTagCategories(*this, FFRShotSpec::StaticStruct(), GET_MEMBER_NAME_CHECKED(FFRShotSpec, EffectTags), TEXT("Weapon,ShotEffect"));
	TestGameplayTagCategories(*this, UFRPerkDefinition::StaticClass(), GET_MEMBER_NAME_CHECKED(UFRPerkDefinition, PerkTag), TEXT("Trait"));
	TestGameplayTagCategories(*this, UFRPerkDefinition::StaticClass(), GET_MEMBER_NAME_CHECKED(UFRPerkDefinition, PerkCategoryTags), TEXT("Trait.Category"));
	TestGameplayTagCategories(*this, UFRCharacterDefinition::StaticClass(), GET_MEMBER_NAME_CHECKED(UFRCharacterDefinition, RequiredPerkCategoryTags), TEXT("Trait.Category"));
	TestGameplayTagCategories(*this, UFRCharacterDefinition::StaticClass(), GET_MEMBER_NAME_CHECKED(UFRCharacterDefinition, BlockedPerkCategoryTags), TEXT("Trait.Category"));
	TestGameplayTagCategories(*this, FFRProjectileEffectSplitParams::StaticStruct(), GET_MEMBER_NAME_CHECKED(FFRProjectileEffectSplitParams, ChildEffectTags), TEXT("ShotEffect"));
	TestGameplayTagCategories(*this, FFRAbilitySet_GameplayAbility::StaticStruct(), GET_MEMBER_NAME_CHECKED(FFRAbilitySet_GameplayAbility, InputTag), TEXT("InputTag"));
	TestGameplayTagCategories(*this, UFRAbilitySet::StaticClass(), GET_MEMBER_NAME_CHECKED(UFRAbilitySet, AbilitySetTag), TEXT("Trait"));
	TestPropertyMetaData(*this, FFRProjectileEffectSpec::StaticStruct(), GET_MEMBER_NAME_CHECKED(FFRProjectileEffectSpec, EffectClass), TEXT("AllowAbstract"), TEXT("false"));
	TestFunctionParamGameplayTagCategories(*this, AFRBattleCharacter::StaticClass(), GET_FUNCTION_NAME_CHECKED(AFRBattleCharacter, TryGetCombatAttributeValueByTag), TEXT("AttributeTag"), TEXT("Attribute"));
	TestFunctionParamGameplayTagCategories(*this, AFRBattleCharacter::StaticClass(), GET_FUNCTION_NAME_CHECKED(AFRBattleCharacter, SelectWeaponByTag), TEXT("WeaponTag"), TEXT("Weapon"));
	TestNull(TEXT("Game mode no longer exposes a fallback enemy definition property"), FindFProperty<FObjectProperty>(AFRGameMode::StaticClass(), TEXT("EnemyDefinition")));
	TestNull(TEXT("Character definition no longer exposes a map definition property"), FindFProperty<FObjectProperty>(UFRCharacterDefinition::StaticClass(), TEXT("BattleMapDefinition")));

	UFRAbilitySet* NamedAbilitySet = NewObject<UFRAbilitySet>();
	NamedAbilitySet->DisplayName = FText::FromString(TEXT("Wind Split"));
	NamedAbilitySet->Description = FText::FromString(TEXT("Adds wind-aware split behavior."));
	TestTrue(TEXT("Ability set summary includes display name"), NamedAbilitySet->GetEffectSummary().ToString().Contains(TEXT("Wind Split")));
	TestTrue(TEXT("Ability set summary includes descriptions"), NamedAbilitySet->GetEffectSummary().ToString().Contains(TEXT("Adds wind-aware split behavior.")));
	TestTrue(TEXT("Blueprint helper summarizes ability set assets"), UFRRewardBlueprintLibrary::GetAbilitySetEffectSummary(NamedAbilitySet).ToString().Contains(TEXT("Wind Split")));
	UFRAbilitySet* InvalidAbilitySetData = NewObject<UFRAbilitySet>();
	InvalidAbilitySetData->DisplayName = FText::GetEmpty();
	const FString InvalidAbilitySetDataSummary = InvalidAbilitySetData->GetDataValidationSummary().ToString();
	TestTrue(TEXT("Ability set data validation reports missing display names"), InvalidAbilitySetDataSummary.Contains(TEXT("missing display name")));
	TestTrue(TEXT("Ability set data validation reports missing grants"), InvalidAbilitySetDataSummary.Contains(TEXT("missing granted ability")));
	TestTrue(TEXT("Blueprint helper reports ability set data validation"), UFRRewardBlueprintLibrary::GetAbilitySetDataValidationSummary(InvalidAbilitySetData).ToString().Contains(TEXT("missing display name")));
	TestTrue(TEXT("Blueprint helper reports missing ability set assets"), UFRRewardBlueprintLibrary::GetAbilitySetDataValidationSummary(nullptr).ToString().Contains(TEXT("missing ability set")));
	FFRShotModifierSpec SummaryModifier;
	SummaryModifier.ModifierTag = FRGameplayTags::ShotEffect_Projectiles;
	SummaryModifier.EffectTags.AddTag(FRGameplayTags::ShotEffect_SplitOnImpact);
	SummaryModifier.ProjectileCountBonus = 2;
	TArray<FFRShotModifierSpec> SummaryModifiers = { SummaryModifier };
	TestTrue(TEXT("Blueprint helper summarizes standalone shot modifiers"), UFRRewardBlueprintLibrary::GetShotModifierEffectSummary(SummaryModifiers).ToString().Contains(TEXT("projectiles +2")));
	TestTrue(TEXT("Blueprint helper summarizes shot modifier tags"), UFRRewardBlueprintLibrary::GetShotModifierEffectSummary(SummaryModifiers).ToString().Contains(TEXT("modifier tag ShotEffect.Projectiles")));
	TestTrue(TEXT("Blueprint helper summarizes shot effect tags"), UFRRewardBlueprintLibrary::GetShotModifierEffectSummary(SummaryModifiers).ToString().Contains(TEXT("ShotEffect.SplitOnImpact")));
	FFRProjectileEffectDrillParams DrillParams;
	DrillParams.RadiusBonus = 25.0f;
	FFRProjectileEffectSpec DrillEffect;
	DrillEffect.EffectClass = UFRProjectileEffectDrill::StaticClass();
	DrillEffect.Parameters = FInstancedStruct::Make(DrillParams);
	TestEqual(TEXT("Projectile effect display name uses class display name"), DrillEffect.GetEffectDisplayName().ToString(), UFRProjectileEffectDrill::StaticClass()->GetDisplayNameText().ToString());
	FFRProjectileEffectSpec MissingParamsEffect;
	MissingParamsEffect.EffectClass = UFRProjectileEffectDrill::StaticClass();
	TestFalse(TEXT("Projectile effect detects missing parameter structs"), MissingParamsEffect.HasValidParameters());
	TestTrue(TEXT("Projectile effect data validation reports missing parameter structs"), MissingParamsEffect.GetDataValidationSummary().ToString().Contains(TEXT("parameters do not match")));
	TestTrue(TEXT("Projectile effect spec creates parameters for its effect class"), MissingParamsEffect.EnsureParametersMatchEffectClass());
	TestTrue(TEXT("Projectile effect spec has valid parameters after sync"), MissingParamsEffect.HasValidParameters());
	TestTrue(TEXT("Projectile effect spec uses the effect class parameter struct"), MissingParamsEffect.Parameters.GetScriptStruct() == FFRProjectileEffectDrillParams::StaticStruct());
	FFRProjectileEffectSpec EmptyClassEffect;
	EmptyClassEffect.Parameters = FInstancedStruct::Make(DrillParams);
	TestTrue(TEXT("Projectile effect spec removes parameters when effect class is empty"), EmptyClassEffect.EnsureParametersMatchEffectClass());
	TestFalse(TEXT("Projectile effect spec clears orphaned parameters"), EmptyClassEffect.Parameters.IsValid());
	FFRProjectileEffectTerrainCreateParams TerrainCreateParams;
	TerrainCreateParams.RadiusBonus = 60.0f;
	FFRProjectileEffectSpec TerrainCreateEffect;
	TerrainCreateEffect.EffectClass = UFRProjectileEffectTerrainCreate::StaticClass();
	TerrainCreateEffect.Parameters = FInstancedStruct::Make(TerrainCreateParams);
	FFRShotSpec EffectShotSpec;
	EffectShotSpec.TerrainDamage = 100.0f;
	DrillEffect.ApplyToShotSpec(EffectShotSpec);
	TerrainCreateEffect.ApplyToShotSpec(EffectShotSpec);
	TestTrue(TEXT("Projectile effect CDO adds drill tags"), EffectShotSpec.EffectTags.HasTagExact(FRGameplayTags::ShotEffect_Drill));
	TestTrue(TEXT("Projectile effect CDO adds terrain create tags"), EffectShotSpec.EffectTags.HasTagExact(FRGameplayTags::ShotEffect_TerrainCreate));
	TestEqual(TEXT("Projectile drill effect updates terrain damage radius"), EffectShotSpec.TerrainDamage, 125.0f);
	TestEqual(TEXT("Projectile terrain create effect updates terrain fill radius"), EffectShotSpec.TerrainFillRadius, 60.0f);
	FFRProjectileEffectDirectHitDamageParams DirectHitDamageParams;
	DirectHitDamageParams.BonusDamage = 10.0f;
	DirectHitDamageParams.DamageMultiplier = 2.0f;
	FFRProjectileEffectSpec DirectHitDamageEffect;
	DirectHitDamageEffect.EffectClass = UFRProjectileEffectDirectHitDamage::StaticClass();
	DirectHitDamageEffect.Parameters = FInstancedStruct::Make(DirectHitDamageParams);
	FFRProjectileEffectExplosionPayloadParams ExplosionPayloadParams;
	ExplosionPayloadParams.DamageMultiplier = 1.5f;
	ExplosionPayloadParams.RadiusMultiplier = 1.25f;
	FFRProjectileEffectSpec ExplosionPayloadEffect;
	ExplosionPayloadEffect.EffectClass = UFRProjectileEffectExplosionPayload::StaticClass();
	ExplosionPayloadEffect.Parameters = FInstancedStruct::Make(ExplosionPayloadParams);
	FFRProjectileEffectSalvoParams SalvoParams;
	SalvoParams.CountBonus = 2;
	SalvoParams.SalvoInterval = 0.2f;
	FFRProjectileEffectSpec SalvoEffect;
	SalvoEffect.EffectClass = UFRProjectileEffectSalvo::StaticClass();
	SalvoEffect.Parameters = FInstancedStruct::Make(SalvoParams);
	FFRShotModifierSpec ShellEffectModifier;
	ShellEffectModifier.ProjectileEffects = { DirectHitDamageEffect, ExplosionPayloadEffect, SalvoEffect };
	FFRShotSpec ShellEffectShotSpec;
	ShellEffectShotSpec.HitDamage = 20.0f;
	ShellEffectShotSpec.Damage = 40.0f;
	ShellEffectShotSpec.BlastRadius = 100.0f;
	ShellEffectModifier.ApplyToShotSpec(ShellEffectShotSpec);
	TestEqual(TEXT("Direct hit ShellEffect applies bonus then multiplier"), ShellEffectShotSpec.HitDamage, 60.0f);
	TestEqual(TEXT("Explosion payload ShellEffect changes damage"), ShellEffectShotSpec.Damage, 60.0f);
	TestEqual(TEXT("Explosion payload ShellEffect changes radius"), ShellEffectShotSpec.BlastRadius, 125.0f);
	TestEqual(TEXT("Salvo ShellEffect changes repeat count"), ShellEffectShotSpec.SalvoCount, 3);
	TestEqual(TEXT("Salvo ShellEffect changes repeat interval"), ShellEffectShotSpec.SalvoInterval, 0.2f);
	TestEqual(TEXT("Spec-only ShellEffects are not attached to projectiles"), ShellEffectShotSpec.ProjectileEffects.Num(), 0);
	FFRProjectileEffectKnockbackParams KnockbackParams;
	FFRProjectileEffectSpec KnockbackEffect;
	KnockbackEffect.EffectClass = UFRProjectileEffectKnockback::StaticClass();
	KnockbackEffect.Parameters = FInstancedStruct::Make(KnockbackParams);
	TestTrue(TEXT("Knockback remains attached for post-impact execution"), KnockbackEffect.RequiresProjectileRuntime());
	FFRShotModifierSpec EffectModifierData;
	EffectModifierData.ProjectileEffects.Add(DrillEffect);
	EffectModifierData.ProjectileEffects.Add(TerrainCreateEffect);
	FFRProjectileEffectSplitParams SplitParams;
	SplitParams.ProjectileCount = 2;
	SplitParams.ChildShotModifiers.Add(EffectModifierData);
	FFRProjectileEffectSpec SplitEffect;
	SplitEffect.EffectClass = UFRProjectileEffectSplit::StaticClass();
	SplitEffect.Parameters = FInstancedStruct::Make(SplitParams);
	SplitEffect.ApplyToShotSpec(EffectShotSpec);
	TestTrue(TEXT("Projectile split effect CDO adds split tags"), EffectShotSpec.EffectTags.HasTagExact(FRGameplayTags::ShotEffect_SplitOnImpact));
	FFRShotSpec SplitModifierShotSpec;
	SplitModifierShotSpec.ProjectileEffects.Add(SplitEffect);
	FFRProjectileEffectSplitModifierParams SplitModifierParams;
	SplitModifierParams.ChildDamageMultiplier = 1.5f;
	SplitModifierParams.ChildTerrainCarveMultiplier = 2.0f;
	SplitModifierParams.bInheritParentRuntimeEffects = true;
	FFRProjectileEffectSpec SplitModifierEffect;
	SplitModifierEffect.EffectClass = UFRProjectileEffectSplitModifier::StaticClass();
	SplitModifierEffect.Parameters = FInstancedStruct::Make(SplitModifierParams);
	SplitModifierEffect.ApplyToShotSpec(SplitModifierShotSpec);
	const FFRProjectileEffectSplitParams* ModifiedSplitParams = SplitModifierShotSpec.ProjectileEffects[0].Parameters.GetPtr<FFRProjectileEffectSplitParams>();
	TestNotNull(TEXT("Split modifier preserves typed split parameters"), ModifiedSplitParams);
	if (ModifiedSplitParams)
	{
		TestEqual(TEXT("Split modifier changes child damage"), ModifiedSplitParams->DamageMultiplier, 0.75f);
		TestEqual(TEXT("Split modifier changes child terrain carve"), ModifiedSplitParams->TerrainCarveRadiusMultiplier, 1.0f);
		TestTrue(TEXT("Split modifier enables runtime effect inheritance"), ModifiedSplitParams->bInheritParentRuntimeEffects);
	}
	EffectModifierData.ProjectileEffects.Add(SplitEffect);
	TestTrue(TEXT("Shot modifier data validation accepts projectile effects"), EffectModifierData.GetDataValidationSummary().ToString().IsEmpty());
	TArray<FFRShotModifierSpec> EffectModifierSummaryData = { EffectModifierData };
	const FString EffectModifierSummary = UFRRewardBlueprintLibrary::GetShotModifierEffectSummary(EffectModifierSummaryData).ToString();
	TestTrue(TEXT("Shot modifier summary counts projectile effects"), EffectModifierSummary.Contains(TEXT("projectile effects 3")));
	TestTrue(TEXT("Shot modifier summary names drill projectile effects"), EffectModifierSummary.Contains(UFRProjectileEffectDrill::StaticClass()->GetDisplayNameText().ToString()));
	TestTrue(TEXT("Shot modifier summary names terrain create projectile effects"), EffectModifierSummary.Contains(UFRProjectileEffectTerrainCreate::StaticClass()->GetDisplayNameText().ToString()));
	TestTrue(TEXT("Shot modifier summary names split projectile effects"), EffectModifierSummary.Contains(UFRProjectileEffectSplit::StaticClass()->GetDisplayNameText().ToString()));
	TestTrue(TEXT("Shot modifier gameplay effect helper accepts projectile effects"), EffectModifierData.HasGameplayEffect());
	FFRShotModifierSpec EmptyProjectileEffectSummaryModifier;
	EmptyProjectileEffectSummaryModifier.ProjectileEffects.AddDefaulted();
	TArray<FFRShotModifierSpec> EmptyProjectileEffectSummaryData = { EmptyProjectileEffectSummaryModifier };
	TestFalse(TEXT("Shot modifier summary ignores empty projectile effect placeholders"), UFRRewardBlueprintLibrary::GetShotModifierEffectSummary(EmptyProjectileEffectSummaryData).ToString().Contains(TEXT("projectile effects")));
	FFRProjectileEffectDrillParams InvalidDrillParams;
	InvalidDrillParams.RadiusBonus = -1.0f;
	FFRProjectileEffectSpec InvalidDrillEffect;
	InvalidDrillEffect.EffectClass = UFRProjectileEffectDrill::StaticClass();
	InvalidDrillEffect.Parameters = FInstancedStruct::Make(InvalidDrillParams);
	TestTrue(TEXT("Projectile effect data validation reports class-owned drill warnings"), InvalidDrillEffect.GetDataValidationSummary().ToString().Contains(TEXT("drill radius bonus")));
	FFRProjectileEffectSplitParams InvalidSplitParams;
	InvalidSplitParams.ProjectileCount = 0;
	InvalidSplitParams.LaunchSpeed = 0.0f;
	InvalidSplitParams.ChildShotModifiers.AddDefaulted();
	FFRShotModifierSpec InvalidSplitChildCountModifier;
	InvalidSplitChildCountModifier.ProjectileCountBonus = 1;
	InvalidSplitParams.ChildShotModifiers.Add(InvalidSplitChildCountModifier);
	FFRProjectileEffectSpec InvalidSplitEffect;
	InvalidSplitEffect.EffectClass = UFRProjectileEffectSplit::StaticClass();
	InvalidSplitEffect.Parameters = FInstancedStruct::Make(InvalidSplitParams);
	const FString InvalidSplitEffectSummary = InvalidSplitEffect.GetDataValidationSummary().ToString();
	TestTrue(TEXT("Projectile effect data validation reports class-owned split warnings"), InvalidSplitEffectSummary.Contains(TEXT("split projectile count")));
	TestTrue(TEXT("Projectile effect data validation reports invalid split child modifiers"), InvalidSplitEffectSummary.Contains(TEXT("split child shot modifier")));
	TestTrue(TEXT("Projectile effect data validation reports ignored split child projectile bonuses"), InvalidSplitEffectSummary.Contains(TEXT("projectile count bonus")));
	FFRShotModifierSpec InvalidShotModifierData;
	InvalidShotModifierData.bUseAimAngleRange = true;
	InvalidShotModifierData.MinAimAngle = 80.0f;
	InvalidShotModifierData.MaxAimAngle = 20.0f;
	InvalidShotModifierData.RequiredShotTags.AddTag(FRGameplayTags::Weapon_Shell);
	InvalidShotModifierData.BlockedShotTags.AddTag(FRGameplayTags::Weapon_Shell);
	InvalidShotModifierData.ProjectileEffects.AddDefaulted();
	const FString InvalidShotModifierDataSummary = InvalidShotModifierData.GetDataValidationSummary().ToString();
	TestTrue(TEXT("Shot modifier data validation reports missing effects"), InvalidShotModifierDataSummary.Contains(TEXT("missing shot effect")));
	TestTrue(TEXT("Shot modifier data validation reports inverted aim ranges"), InvalidShotModifierDataSummary.Contains(TEXT("aim range")));
	TestTrue(TEXT("Shot modifier data validation reports overlapping shot tags"), InvalidShotModifierDataSummary.Contains(TEXT("overlap")));
	TestTrue(TEXT("Shot modifier data validation reports invalid projectile effects"), InvalidShotModifierDataSummary.Contains(TEXT("projectile effect")));
	TestTrue(TEXT("Blueprint helper reports shot modifier data validation"), UFRRewardBlueprintLibrary::GetShotModifierDataValidationSummary(InvalidShotModifierData).ToString().Contains(TEXT("missing shot effect")));
	TestFalse(TEXT("Shot modifier gameplay effect helper rejects empty placeholders"), InvalidShotModifierData.HasGameplayEffect());
	FFRShotModifierSpec ValidShotModifierData;
	ValidShotModifierData.DamageBonus = 5.0f;
	TestTrue(TEXT("Shot modifier data validation is empty for valid modifier data"), ValidShotModifierData.GetDataValidationSummary().ToString().IsEmpty());
	FFRShotSpec ConditionShotSpec;
	ConditionShotSpec.WeaponTag = FRGameplayTags::Weapon_Shell;
	FFRShotModifierSpec ConditionModifier;
	ConditionModifier.RequiredShotTags.AddTag(FRGameplayTags::Weapon_Shell);
	TestTrue(TEXT("Shot modifier condition helper accepts required weapon tags"), ConditionModifier.MeetsShotConditions(ConditionShotSpec, 45.0f, 0.0f, true));
	TestTrue(TEXT("Blueprint shot modifier condition helper accepts required shot tags"), UFRRewardBlueprintLibrary::DoesShotModifierMeetShotConditions(ConditionModifier, ConditionShotSpec, 45.0f, 0.0f, true));
	ConditionModifier.RequiredShotTags.Reset();
	ConditionShotSpec.EffectTags.AddTag(FRGameplayTags::ShotEffect_Drill);
	ConditionModifier.RequiredShotTags.AddTag(FRGameplayTags::ShotEffect_Drill);
	TestTrue(TEXT("Shot modifier condition helper accepts required effect tags"), ConditionModifier.MeetsShotConditions(ConditionShotSpec, 45.0f, 0.0f, true));
	ConditionModifier.RequiredShotTags.Reset();
	ConditionModifier.RequiredShotTags.AddTag(FRGameplayTags::Weapon_Shell);
	ConditionModifier.RequiredShotTags.AddTag(FRGameplayTags::ShotEffect_TerrainCreate);
	TestFalse(TEXT("Shot modifier condition helper requires all required shot tags"), ConditionModifier.MeetsShotConditions(ConditionShotSpec, 45.0f, 0.0f, true));
	ConditionShotSpec.EffectTags.AddTag(FRGameplayTags::ShotEffect_TerrainCreate);
	TestTrue(TEXT("Shot modifier condition helper accepts all required shot tags"), ConditionModifier.MeetsShotConditions(ConditionShotSpec, 45.0f, 0.0f, true));
	ConditionModifier.RequiredShotTags.Reset();
	ConditionModifier.RequiredShotTags.AddTag(FRGameplayTags::Weapon_Cluster);
	TestFalse(TEXT("Shot modifier condition helper rejects missing required shot tags"), ConditionModifier.MeetsShotConditions(ConditionShotSpec, 45.0f, 0.0f, true));
	TestTrue(TEXT("Shot modifier condition failure summary names required shot tags"), ConditionModifier.GetShotConditionFailureSummary(ConditionShotSpec, 45.0f, 0.0f, true).ToString().Contains(TEXT("requires shot tag")));
	ConditionModifier.RequiredShotTags.Reset();
	ConditionModifier.BlockedShotTags.AddTag(FRGameplayTags::Weapon_Shell);
	TestFalse(TEXT("Shot modifier condition helper rejects blocked weapon tags"), ConditionModifier.MeetsShotConditions(ConditionShotSpec, 45.0f, 0.0f, true));
	TestTrue(TEXT("Blueprint shot modifier condition failure summary names blocked shot tags"), UFRRewardBlueprintLibrary::GetShotModifierConditionFailureSummary(ConditionModifier, ConditionShotSpec, 45.0f, 0.0f, true).ToString().Contains(TEXT("blocked by shot tag")));
	ConditionModifier.BlockedShotTags.Reset();
	ConditionModifier.bUseAimAngleRange = true;
	ConditionModifier.MinAimAngle = 100.0f;
	ConditionModifier.MaxAimAngle = 180.0f;
	TestFalse(TEXT("Shot modifier condition helper rejects out-of-range aim"), ConditionModifier.MeetsShotConditions(ConditionShotSpec, 45.0f, 0.0f, true));
	TestTrue(TEXT("Shot modifier condition helper accepts backward aim range"), ConditionModifier.MeetsShotConditions(ConditionShotSpec, 135.0f, 0.0f, true));
	TestTrue(TEXT("Shot modifier condition failure summary names aim range"), ConditionModifier.GetShotConditionFailureSummary(ConditionShotSpec, 45.0f, 0.0f, true).ToString().Contains(TEXT("requires aim")));
	ConditionModifier.bUseAimAngleRange = false;
	ConditionModifier.bRequireWindAligned = true;
	ConditionModifier.MinWindMagnitude = 10.0f;
	TestTrue(TEXT("Shot modifier condition helper accepts aligned wind"), ConditionModifier.MeetsShotConditions(ConditionShotSpec, 45.0f, 15.0f, true));
	TestFalse(TEXT("Shot modifier condition helper rejects opposite wind"), ConditionModifier.MeetsShotConditions(ConditionShotSpec, 45.0f, -15.0f, true));
	TestTrue(TEXT("Shot modifier condition failure summary names aligned wind"), ConditionModifier.GetShotConditionFailureSummary(ConditionShotSpec, 45.0f, -15.0f, true).ToString().Contains(TEXT("requires aligned wind")));
	ConditionModifier.bRequireWindAligned = false;
	ConditionModifier.bRequireWindOpposed = true;
	TestTrue(TEXT("Shot modifier condition helper accepts opposed wind"), ConditionModifier.MeetsShotConditions(ConditionShotSpec, 45.0f, -15.0f, true));
	TestFalse(TEXT("Shot modifier condition helper rejects aligned wind for opposed condition"), ConditionModifier.MeetsShotConditions(ConditionShotSpec, 45.0f, 15.0f, true));
	TestTrue(TEXT("Shot modifier condition failure summary names opposed wind"), ConditionModifier.GetShotConditionFailureSummary(ConditionShotSpec, 45.0f, 15.0f, true).ToString().Contains(TEXT("requires opposed wind")));
	UFRPerkDefinition* AbilitySetPerk = NewObject<UFRPerkDefinition>();
	AbilitySetPerk->DisplayName = FText::FromString(TEXT("Wind Split Perk"));
	AbilitySetPerk->PerkTag = FRGameplayTags::Trait_ShotModifier;
	AbilitySetPerk->GrantedAbilitySet = NamedAbilitySet;
	FFRRewardChoice AbilitySetReward;
	AddTestPerkGrant(AbilitySetReward, AbilitySetPerk);
	TestTrue(TEXT("Reward summary names perk ability set"), AbilitySetReward.GetEffectSummary().ToString().Contains(TEXT("ability set Wind Split")));
	FFRRewardChoice InvalidRewardData;
	InvalidRewardData.bOfferOncePerRun = true;
	InvalidRewardData.RewardWeight = 0.0f;
	InvalidRewardData.RequiredRewardTags.AddTag(FRGameplayTags::Trait_Damage);
	InvalidRewardData.BlockedRewardTags.AddTag(FRGameplayTags::Trait_Damage);
	const FString InvalidRewardDataSummary = InvalidRewardData.GetDataValidationSummary().ToString();
	TestTrue(TEXT("Reward data validation reports missing display names"), InvalidRewardDataSummary.Contains(TEXT("missing display name")));
	TestTrue(TEXT("Reward data validation reports invalid weights"), InvalidRewardDataSummary.Contains(TEXT("greater than 0")));
	TestTrue(TEXT("Reward data validation reports missing once-per-run tags"), InvalidRewardDataSummary.Contains(TEXT("RewardTag")));
	TestTrue(TEXT("Reward data validation reports missing effects"), InvalidRewardDataSummary.Contains(TEXT("missing gameplay effect")));
	TestTrue(TEXT("Reward data validation reports overlapping condition tags"), InvalidRewardDataSummary.Contains(TEXT("overlap")));
	TestTrue(TEXT("Blueprint helper reports reward data validation"), UFRRewardBlueprintLibrary::GetRewardDataValidationSummary(InvalidRewardData).ToString().Contains(TEXT("missing display name")));
	UFRPerkDefinition* ValidRewardPerk = NewObject<UFRPerkDefinition>();
	ValidRewardPerk->DisplayName = FText::FromString(TEXT("Valid Reward Perk"));
	ValidRewardPerk->PerkTag = FRGameplayTags::Trait_Damage;
	ValidRewardPerk->DamageBonus = 3.0f;
	FFRRewardChoice ValidRewardData;
	ValidRewardData.DisplayName = FText::FromString(TEXT("Valid Reward"));
	ValidRewardData.RewardTag = FRGameplayTags::Trait_Damage;
	ValidRewardData.bOfferOncePerRun = true;
	AddTestPerkGrant(ValidRewardData, ValidRewardPerk);
	TestTrue(TEXT("Reward data validation is empty for valid reward data"), ValidRewardData.GetDataValidationSummary().ToString().IsEmpty());
	const FGameplayTag DrillCategoryTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Trait.Category.Drill")));
	const FGameplayTag CannonCategoryTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Trait.Category.Cannon")));
	ValidRewardPerk->PerkCategoryTags.AddTag(DrillCategoryTag);
	FGameplayTagContainer RequiredDrillCategory;
	RequiredDrillCategory.AddTag(DrillCategoryTag);
	FGameplayTagContainer BlockedCannonCategory;
	BlockedCannonCategory.AddTag(CannonCategoryTag);
	TestTrue(TEXT("Perk reports required category tags"), ValidRewardPerk->HasAllCategoryTags(RequiredDrillCategory));
	TestTrue(TEXT("Reward category filter accepts matching Perks"), ValidRewardData.MatchesPerkCategoryFilter(RequiredDrillCategory, BlockedCannonCategory));
	BlockedCannonCategory.Reset();
	BlockedCannonCategory.AddTag(DrillCategoryTag);
	TestFalse(TEXT("Reward category filter rejects blocked Perks"), ValidRewardData.MatchesPerkCategoryFilter(FGameplayTagContainer(), BlockedCannonCategory));
	FFRRewardChoice WeaponCategoryFilterReward;
	AddTestWeaponGrant(WeaponCategoryFilterReward, CreateTestWeaponDefinition(GetTransientPackage()));
	TestTrue(TEXT("Perk category filter leaves non-Perk rewards available"), WeaponCategoryFilterReward.MatchesPerkCategoryFilter(RequiredDrillCategory, BlockedCannonCategory));

	UFRWeaponDefinition* SummaryWeapon = NewObject<UFRWeaponDefinition>();
	SummaryWeapon->Weapon.DisplayName = FText::FromString(TEXT("Fork Shell"));
	SummaryWeapon->Weapon.Description = FText::FromString(TEXT("Splits the battlefield."));
	SummaryWeapon->Weapon.WeaponTag = FRGameplayTags::Weapon_Shell;
	SummaryWeapon->Weapon.ProjectileEffects.Add(TerrainCreateEffect);
	TestTrue(TEXT("Blueprint helper summarizes weapon assets"), UFRRewardBlueprintLibrary::GetWeaponEffectSummary(SummaryWeapon).ToString().Contains(TEXT("weapon Fork Shell")));
	TestTrue(TEXT("Blueprint helper summarizes weapon descriptions"), UFRRewardBlueprintLibrary::GetWeaponEffectSummary(SummaryWeapon).ToString().Contains(TEXT("Splits the battlefield.")));
	TestTrue(TEXT("Blueprint helper summarizes weapon tags"), UFRRewardBlueprintLibrary::GetWeaponEffectSummary(SummaryWeapon).ToString().Contains(TEXT("tag Weapon.Shell")));
	TestTrue(TEXT("Blueprint helper summarizes weapon base damage"), UFRRewardBlueprintLibrary::GetWeaponEffectSummary(SummaryWeapon).ToString().Contains(TEXT("explosion damage 35")));
	TestTrue(TEXT("Blueprint helper summarizes weapon blast radius"), UFRRewardBlueprintLibrary::GetWeaponEffectSummary(SummaryWeapon).ToString().Contains(TEXT("blast 150")));
	TestTrue(TEXT("Blueprint helper summarizes weapon terrain damage"), UFRRewardBlueprintLibrary::GetWeaponEffectSummary(SummaryWeapon).ToString().Contains(TEXT("terrain damage 150")));
	TestTrue(TEXT("Blueprint helper summarizes weapon projectile effects"), UFRRewardBlueprintLibrary::GetWeaponEffectSummary(SummaryWeapon).ToString().Contains(TEXT("projectile effect")));
	UFRWeaponDefinition* InvalidWeaponData = NewObject<UFRWeaponDefinition>();
	InvalidWeaponData->Weapon.DisplayName = FText::GetEmpty();
	InvalidWeaponData->Weapon.HitDamage = 0.0f;
	InvalidWeaponData->Weapon.Damage = 0.0f;
	InvalidWeaponData->Weapon.BlastRadius = 0.0f;
	InvalidWeaponData->Weapon.TerrainDamage = 0.0f;
	InvalidWeaponData->Weapon.ProjectileSpeed = 0.0f;
	InvalidWeaponData->Weapon.ProjectilesPerShot = 0;
	InvalidWeaponData->Weapon.ProjectileEffects.AddDefaulted();
	const FString InvalidWeaponDataSummary = InvalidWeaponData->GetDataValidationSummary().ToString();
	TestTrue(TEXT("Weapon data validation reports missing display names"), InvalidWeaponDataSummary.Contains(TEXT("missing display name")));
	TestTrue(TEXT("Weapon data validation reports missing tags"), InvalidWeaponDataSummary.Contains(TEXT("WeaponTag")));
	TestTrue(TEXT("Weapon data validation reports missing effects"), InvalidWeaponDataSummary.Contains(TEXT("missing weapon effect")));
	TestTrue(TEXT("Weapon data validation reports invalid projectile speed"), InvalidWeaponDataSummary.Contains(TEXT("projectile speed")));
	TestTrue(TEXT("Weapon data validation reports invalid projectile counts"), InvalidWeaponDataSummary.Contains(TEXT("projectiles per shot")));
	TestTrue(TEXT("Weapon data validation reports invalid projectile effects"), InvalidWeaponDataSummary.Contains(TEXT("projectile effect")));
	TestTrue(TEXT("Blueprint helper reports weapon data validation"), UFRRewardBlueprintLibrary::GetWeaponDataValidationSummary(InvalidWeaponData).ToString().Contains(TEXT("missing display name")));
	TestTrue(TEXT("Blueprint helper reports missing weapon assets"), UFRRewardBlueprintLibrary::GetWeaponDataValidationSummary(nullptr).ToString().Contains(TEXT("missing weapon")));
	TestTrue(TEXT("Weapon data validation is empty for valid weapon data"), SummaryWeapon->GetDataValidationSummary().ToString().IsEmpty());

	UFRItemDefinition* AbilityItem = NewObject<UFRItemDefinition>();
	AbilityItem->DisplayName = FText::FromString(TEXT("Storm Capsule"));
	AbilityItem->Description = FText::FromString(TEXT("Empowers the next shot."));
	AbilityItem->ItemTag = FRGameplayTags::Item_NextShot;
	AbilityItem->InitialCharges = 2;
	AbilityItem->UseAbilitySet = NamedAbilitySet;
	FFRRewardChoice ItemAbilityReward;
	AddTestItemGrant(ItemAbilityReward, AbilityItem);
	TestTrue(TEXT("Reward summary names item ability set"), ItemAbilityReward.GetEffectSummary().ToString().Contains(TEXT("ability set Wind Split")));
	TestTrue(TEXT("Reward summary includes item descriptions"), ItemAbilityReward.GetEffectSummary().ToString().Contains(TEXT("Empowers the next shot.")));
	TestTrue(TEXT("Reward summary names item tags"), ItemAbilityReward.GetEffectSummary().ToString().Contains(TEXT("tag Item.NextShot")));
	TestTrue(TEXT("Reward summary names item initial charges"), ItemAbilityReward.GetEffectSummary().ToString().Contains(TEXT("charges 2")));
	TestTrue(TEXT("Blueprint helper summarizes item assets"), UFRRewardBlueprintLibrary::GetItemEffectSummary(AbilityItem).ToString().Contains(TEXT("ability set Wind Split")));
	UFRItemDefinition* HealSummaryItem = NewObject<UFRItemDefinition>();
	HealSummaryItem->ItemType = EFRItemType::Heal;
	AddTestHealEffect(HealSummaryItem, 42.0f);
	TestTrue(TEXT("Reward summary names heal item amount"), UFRRewardBlueprintLibrary::GetItemEffectSummary(HealSummaryItem).ToString().Contains(TEXT("heal +42")));
	UFRItemDefinition* AttackSummaryItem = NewObject<UFRItemDefinition>();
	AttackSummaryItem->ItemType = EFRItemType::AttackMultiplier;
	AddTestAttackMultiplierEffect(AttackSummaryItem, 2.0f);
	TestTrue(TEXT("Reward summary names attack multiplier item amount"), UFRRewardBlueprintLibrary::GetItemEffectSummary(AttackSummaryItem).ToString().Contains(TEXT("next shot attack x2")));
	UFRItemDefinition* InvalidItemData = NewObject<UFRItemDefinition>();
	InvalidItemData->DisplayName = FText::GetEmpty();
	InvalidItemData->ItemType = EFRItemType::AttackMultiplier;
	InvalidItemData->InitialCharges = 0;
	AddTestAttackMultiplierEffect(InvalidItemData, 1.0f);
	const FString InvalidItemDataSummary = InvalidItemData->GetDataValidationSummary().ToString();
	TestTrue(TEXT("Item data validation reports missing display names"), InvalidItemDataSummary.Contains(TEXT("missing display name")));
	TestTrue(TEXT("Item data validation reports missing tags"), InvalidItemDataSummary.Contains(TEXT("ItemTag")));
	TestTrue(TEXT("Item data validation reports invalid charges"), InvalidItemDataSummary.Contains(TEXT("initial charges")));
	TestTrue(TEXT("Item data validation reports missing effects"), InvalidItemDataSummary.Contains(TEXT("missing item effect")));
	TestTrue(TEXT("Item data validation reports invalid attack multipliers"), InvalidItemDataSummary.Contains(TEXT("attack multiplier")));
	UFRItemDefinition* EmptyModifierItemData = NewObject<UFRItemDefinition>();
	EmptyModifierItemData->DisplayName = FText::FromString(TEXT("Empty Modifier Item"));
	EmptyModifierItemData->ItemTag = FRGameplayTags::Item_NextShot;
	EmptyModifierItemData->ItemType = EFRItemType::AbilitySet;
	EmptyModifierItemData->UseShotModifiers.AddDefaulted();
	const FString EmptyModifierItemDataSummary = EmptyModifierItemData->GetDataValidationSummary().ToString();
	TestTrue(TEXT("Item data validation ignores empty shot modifiers as gameplay effects"), EmptyModifierItemDataSummary.Contains(TEXT("missing item effect")));
	TestTrue(TEXT("Item data validation reports nested empty shot modifiers"), EmptyModifierItemDataSummary.Contains(TEXT("shot modifier data")));
	UFRItemDefinition* NestedInvalidItemData = NewObject<UFRItemDefinition>();
	NestedInvalidItemData->DisplayName = FText::FromString(TEXT("Nested Invalid Item"));
	NestedInvalidItemData->ItemTag = FRGameplayTags::Item_NextShot;
	NestedInvalidItemData->ItemType = EFRItemType::AbilitySet;
	NestedInvalidItemData->UseAbilitySet = InvalidAbilitySetData;
	NestedInvalidItemData->UseShotModifiers.Add(InvalidShotModifierData);
	const FString NestedInvalidItemDataSummary = NestedInvalidItemData->GetDataValidationSummary().ToString();
	TestTrue(TEXT("Item data validation reports nested ability set warnings"), NestedInvalidItemDataSummary.Contains(TEXT("ability set data")));
	TestTrue(TEXT("Item data validation reports nested shot modifier warnings"), NestedInvalidItemDataSummary.Contains(TEXT("shot modifier data")));
	TestTrue(TEXT("Blueprint helper reports item data validation"), UFRRewardBlueprintLibrary::GetItemDataValidationSummary(InvalidItemData).ToString().Contains(TEXT("missing display name")));
	TestTrue(TEXT("Blueprint helper reports missing item assets"), UFRRewardBlueprintLibrary::GetItemDataValidationSummary(nullptr).ToString().Contains(TEXT("missing item")));
	UFRItemDefinition* ValidItemData = NewObject<UFRItemDefinition>();
	ValidItemData->DisplayName = FText::FromString(TEXT("Valid Heal"));
	ValidItemData->ItemTag = FRGameplayTags::Item_Repair;
	ValidItemData->ItemType = EFRItemType::Heal;
	ValidItemData->InitialCharges = 1;
	AddTestHealEffect(ValidItemData, 10.0f);
	TestTrue(TEXT("Item data validation is empty for valid item data"), ValidItemData->GetDataValidationSummary().ToString().IsEmpty());

	UFRPerkDefinition* AbilityPerk = NewObject<UFRPerkDefinition>();
	AbilityPerk->DisplayName = FText::FromString(TEXT("Storm Training"));
	AbilityPerk->Description = FText::FromString(TEXT("Every run shot bends harder."));
	AbilityPerk->PerkTag = FRGameplayTags::Trait_ShotModifier;
	AbilityPerk->Rarity = EFRPerkRarity::Epic;
	AbilityPerk->GrantedAbilitySet = NamedAbilitySet;
	AbilityPerk->DamageBonus = -2.0f;
	AbilityPerk->MaxHealthBonus = -8.0f;
	AbilityPerk->MaxMoveBudgetBonus = -1.0f;
	AbilityPerk->ProjectileBonus = -1;
	AbilityPerk->ShotPowerMultiplierBonus = -0.2f;
	FFRRewardChoice PerkAbilityReward;
	AddTestPerkGrant(PerkAbilityReward, AbilityPerk);
	TestTrue(TEXT("Reward summary names perk ability set"), PerkAbilityReward.GetEffectSummary().ToString().Contains(TEXT("ability set Wind Split")));
	TestTrue(TEXT("Reward summary includes perk descriptions"), PerkAbilityReward.GetEffectSummary().ToString().Contains(TEXT("Every run shot bends harder.")));
	TestTrue(TEXT("Reward summary names perk tags"), PerkAbilityReward.GetEffectSummary().ToString().Contains(TEXT("tag Trait.ShotModifier")));
	TestTrue(TEXT("Reward summary names perk rarity"), PerkAbilityReward.GetEffectSummary().ToString().Contains(TEXT("rarity Epic")));
	TestTrue(TEXT("Reward summary names negative perk damage bonuses"), PerkAbilityReward.GetEffectSummary().ToString().Contains(TEXT("damage -2")));
	TestTrue(TEXT("Reward summary names negative perk max health bonuses"), PerkAbilityReward.GetEffectSummary().ToString().Contains(TEXT("max HP -8")));
	TestTrue(TEXT("Reward summary names negative perk move bonuses"), PerkAbilityReward.GetEffectSummary().ToString().Contains(TEXT("move -1")));
	TestTrue(TEXT("Reward summary names negative perk projectile bonuses"), PerkAbilityReward.GetEffectSummary().ToString().Contains(TEXT("projectiles -1")));
	TestTrue(TEXT("Reward summary names negative perk shot power bonuses"), PerkAbilityReward.GetEffectSummary().ToString().Contains(TEXT("shot power -0.2")));
	TestTrue(TEXT("Blueprint helper summarizes perk assets"), UFRRewardBlueprintLibrary::GetPerkEffectSummary(AbilityPerk).ToString().Contains(TEXT("ability set Wind Split")));
	TestTrue(TEXT("Blueprint helper summarizes perk rarity"), UFRRewardBlueprintLibrary::GetPerkEffectSummary(AbilityPerk).ToString().Contains(TEXT("rarity Epic")));
	UFRPerkDefinition* InvalidPerkData = NewObject<UFRPerkDefinition>();
	InvalidPerkData->DisplayName = FText::GetEmpty();
	const FString InvalidPerkDataSummary = InvalidPerkData->GetDataValidationSummary().ToString();
	TestTrue(TEXT("Perk data validation reports missing display names"), InvalidPerkDataSummary.Contains(TEXT("missing display name")));
	TestTrue(TEXT("Perk data validation reports missing tags"), InvalidPerkDataSummary.Contains(TEXT("PerkTag")));
	TestTrue(TEXT("Perk data validation reports missing effects"), InvalidPerkDataSummary.Contains(TEXT("missing perk effect")));
	UFRPerkDefinition* EmptyModifierPerkData = NewObject<UFRPerkDefinition>();
	EmptyModifierPerkData->DisplayName = FText::FromString(TEXT("Empty Modifier Perk"));
	EmptyModifierPerkData->PerkTag = FRGameplayTags::Trait_ShotModifier;
	EmptyModifierPerkData->ShotModifiers.AddDefaulted();
	const FString EmptyModifierPerkDataSummary = EmptyModifierPerkData->GetDataValidationSummary().ToString();
	TestTrue(TEXT("Perk data validation ignores empty shot modifiers as gameplay effects"), EmptyModifierPerkDataSummary.Contains(TEXT("missing perk effect")));
	TestTrue(TEXT("Perk data validation reports nested empty shot modifiers"), EmptyModifierPerkDataSummary.Contains(TEXT("shot modifier data")));
	UFRPerkDefinition* NestedInvalidPerkData = NewObject<UFRPerkDefinition>();
	NestedInvalidPerkData->DisplayName = FText::FromString(TEXT("Nested Invalid Perk"));
	NestedInvalidPerkData->PerkTag = FRGameplayTags::Trait_ShotModifier;
	NestedInvalidPerkData->GrantedAbilitySet = InvalidAbilitySetData;
	NestedInvalidPerkData->ShotModifiers.Add(InvalidShotModifierData);
	const FString NestedInvalidPerkDataSummary = NestedInvalidPerkData->GetDataValidationSummary().ToString();
	TestTrue(TEXT("Perk data validation reports nested ability set warnings"), NestedInvalidPerkDataSummary.Contains(TEXT("ability set data")));
	TestTrue(TEXT("Perk data validation reports nested shot modifier warnings"), NestedInvalidPerkDataSummary.Contains(TEXT("shot modifier data")));
	TestTrue(TEXT("Blueprint helper reports perk data validation"), UFRRewardBlueprintLibrary::GetPerkDataValidationSummary(InvalidPerkData).ToString().Contains(TEXT("missing display name")));
	TestTrue(TEXT("Blueprint helper reports missing perk assets"), UFRRewardBlueprintLibrary::GetPerkDataValidationSummary(nullptr).ToString().Contains(TEXT("missing perk")));
	UFRPerkDefinition* ValidPerkData = NewObject<UFRPerkDefinition>();
	ValidPerkData->DisplayName = FText::FromString(TEXT("Valid Perk"));
	ValidPerkData->PerkTag = FRGameplayTags::Trait_Damage;
	ValidPerkData->DamageBonus = 1.0f;
	TestTrue(TEXT("Perk data validation is empty for valid perk data"), ValidPerkData->GetDataValidationSummary().ToString().IsEmpty());
	UFRPerkDefinition* RiskPerk = NewObject<UFRPerkDefinition>();
	RiskPerk->DisplayName = FText::FromString(TEXT("Glass Cannon Perk"));
	RiskPerk->PerkTag = FRGameplayTags::Trait_Damage;
	RiskPerk->DamageBonus = -5.0f;
	RiskPerk->MaxHealthBonus = -20.0f;
	RiskPerk->MaxMoveBudgetBonus = -3.0f;
	RiskPerk->ProjectileBonus = -1;
	RiskPerk->ShotPowerMultiplierBonus = -0.25f;
	FFRRewardChoice RiskReward;
	RiskReward.DisplayName = FText::FromString(TEXT("Glass Cannon"));
	RiskReward.Description = FText::FromString(TEXT("Trades survivability for tempo."));
	AddTestPerkGrant(RiskReward, RiskPerk);
	const FString RiskRewardSummary = RiskReward.GetEffectSummary().ToString();
	TestTrue(TEXT("Reward summary includes reward display names"), RiskRewardSummary.Contains(TEXT("reward Glass Cannon")));
	TestTrue(TEXT("Reward summary includes reward descriptions"), RiskRewardSummary.Contains(TEXT("Trades survivability for tempo.")));
	TestTrue(TEXT("Reward summary names negative damage bonuses"), RiskRewardSummary.Contains(TEXT("damage -5")));
	TestTrue(TEXT("Reward summary names negative max health bonuses"), RiskRewardSummary.Contains(TEXT("max HP -20")));
	TestTrue(TEXT("Reward summary names negative move bonuses"), RiskRewardSummary.Contains(TEXT("move -3")));
	TestTrue(TEXT("Reward summary names negative projectile bonuses"), RiskRewardSummary.Contains(TEXT("projectiles -1")));
	TestTrue(TEXT("Reward summary names negative shot power bonuses"), RiskRewardSummary.Contains(TEXT("shot power -0.25")));
	FFRRewardChoice TaggedReward;
	TaggedReward.RewardTag = FRGameplayTags::Trait_Damage;
	TestTrue(TEXT("Reward summary names reward tags"), TaggedReward.GetEffectSummary().ToString().Contains(TEXT("reward tag Trait.Damage")));

	UFRTerrainMapDefinition* CorruptMap = NewObject<UFRTerrainMapDefinition>();
	TestNotNull(TEXT("Corrupt map asset object is created"), CorruptMap);
	CorruptMap->CellsX = 3;
	CorruptMap->CellsZ = 2;
	CorruptMap->CellSize = 0.25f;
	CorruptMap->SolidMask = { 9, 0 };
	CorruptMap->TextureLayerMask = { 4, 7 };
	CorruptMap->NormalizeMapData();
	TestEqual(TEXT("Normalize clamps map cell size"), CorruptMap->CellSize, 1.0f);
	TestEqual(TEXT("Normalize repairs solid mask size"), CorruptMap->SolidMask.Num(), 6);
	TestEqual(TEXT("Normalize repairs texture layer mask size"), CorruptMap->TextureLayerMask.Num(), 6);
	TestEqual(TEXT("Normalize coerces nonzero solid values to one"), CorruptMap->SolidMask[0], static_cast<uint8>(1));
	TestEqual(TEXT("Normalize preserves texture layer on solid cells"), CorruptMap->TextureLayerMask[0], static_cast<uint8>(4));
	TestEqual(TEXT("Normalize clears texture layer on empty cells"), CorruptMap->TextureLayerMask[1], static_cast<uint8>(0));

	Map->Resize(8, 4);
	Map->Clear(false);
	TestEqual(TEXT("Resize updates SolidMask size"), Map->SolidMask.Num(), 32);
	TestEqual(TEXT("Resize updates TextureLayerMask size"), Map->TextureLayerMask.Num(), 32);

	Map->FillRect(2, 1, 4, 2, true);
	TestEqual(TEXT("Filled rect sets minimum cell solid"), Map->SolidMask[Map->ToIndex(2, 1)], static_cast<uint8>(1));
	TestEqual(TEXT("Filled rect sets maximum cell solid"), Map->SolidMask[Map->ToIndex(4, 2)], static_cast<uint8>(1));
	TestEqual(TEXT("Outside rect stays empty"), Map->SolidMask[Map->ToIndex(1, 1)], static_cast<uint8>(0));

	Map->FillTexturedRect(0, 0, 1, 0, 2);
	TestEqual(TEXT("Textured rect fills empty terrain"), Map->SolidMask[Map->ToIndex(0, 0)], static_cast<uint8>(1));
	TestEqual(TEXT("Textured rect assigns the requested layer"), Map->TextureLayerMask[Map->ToIndex(0, 0)], static_cast<uint8>(2));

	Map->ApplyTextureRect(2, 1, 4, 2, 3);
	TestEqual(TEXT("Texture rect paints existing solid cell"), Map->TextureLayerMask[Map->ToIndex(3, 1)], static_cast<uint8>(3));
	TestEqual(TEXT("Texture rect does not paint empty cell"), Map->TextureLayerMask[Map->ToIndex(1, 1)], static_cast<uint8>(0));

	Map->ApplyCircle(3, 1, 1, false);
	TestEqual(TEXT("Erase circle clears center solid"), Map->SolidMask[Map->ToIndex(3, 1)], static_cast<uint8>(0));
	TestEqual(TEXT("Erase circle clears center texture layer"), Map->TextureLayerMask[Map->ToIndex(3, 1)], static_cast<uint8>(0));

	Map->ApplyCircle(6, 2, 1, true);
	Map->ApplyTextureCircle(6, 2, 1, 5);
	TestEqual(TEXT("Texture circle paints solid center"), Map->TextureLayerMask[Map->ToIndex(6, 2)], static_cast<uint8>(5));
	Map->ApplyTexturedCircle(7, 0, 0, 4);
	TestEqual(TEXT("Textured circle fills its center"), Map->SolidMask[Map->ToIndex(7, 0)], static_cast<uint8>(1));
	TestEqual(TEXT("Textured circle assigns the requested layer"), Map->TextureLayerMask[Map->ToIndex(7, 0)], static_cast<uint8>(4));
	Map->SetTextureLayer(300, nullptr);
	TestEqual(TEXT("Texture layer setup clamps to the byte-sized layer mask range"), Map->TextureLayers.Num(), 256);
	TestEqual(TEXT("Texture layer setup stores the clamped layer id"), Map->TextureLayers[255].LayerId, FName(TEXT("Layer255")));
	Map->ApplyTexturedCircle(5, 0, 0, 300);
	TestEqual(TEXT("Textured painting clamps layer indices to the byte-sized layer mask range"), Map->TextureLayerMask[Map->ToIndex(5, 0)], static_cast<uint8>(255));

	UTexture2D* LayerFallbackTexture = UTexture2D::CreateTransient(2, 1, PF_B8G8R8A8);
	TestNotNull(TEXT("Layer fallback texture is created"), LayerFallbackTexture);
	if (LayerFallbackTexture && LayerFallbackTexture->GetPlatformData() && LayerFallbackTexture->GetPlatformData()->Mips.Num() > 0)
	{
		FTexture2DMipMap& LayerFallbackMip = LayerFallbackTexture->GetPlatformData()->Mips[0];
		void* LayerFallbackData = LayerFallbackMip.BulkData.Lock(LOCK_READ_WRITE);
		TestNotNull(TEXT("Layer fallback texture mip locks"), LayerFallbackData);
		if (LayerFallbackData)
		{
			FColor* LayerFallbackPixels = static_cast<FColor*>(LayerFallbackData);
			LayerFallbackPixels[0] = FColor::Red;
			LayerFallbackPixels[1] = FColor::Green;
			LayerFallbackMip.BulkData.Unlock();

			Map->SetTextureLayer(1, LayerFallbackTexture);
			TestEqual(TEXT("Texture layer fallback color averages source texture red"), Map->TextureLayers[1].FallbackColor.R, 0.5f);
			TestEqual(TEXT("Texture layer fallback color averages source texture green"), Map->TextureLayers[1].FallbackColor.G, 0.5f);
			TestEqual(TEXT("Texture layer fallback color averages source texture blue"), Map->TextureLayers[1].FallbackColor.B, 0.0f);
			TestEqual(TEXT("Texture layer fallback color stays opaque"), Map->TextureLayers[1].FallbackColor.A, 1.0f);
		}
		else
		{
			LayerFallbackMip.BulkData.Unlock();
		}
	}

	UFRTerrainMapDefinition* StrokeMap = NewObject<UFRTerrainMapDefinition>();
	TestNotNull(TEXT("Stroke map asset object is created"), StrokeMap);
	StrokeMap->Resize(10, 3);
	StrokeMap->Clear(false);
	StrokeMap->ApplyCircleStroke(1, 1, 8, 1, 0, true);
	TestEqual(TEXT("Circle stroke paints the start cell"), StrokeMap->SolidMask[StrokeMap->ToIndex(1, 1)], static_cast<uint8>(1));
	TestEqual(TEXT("Circle stroke fills skipped drag cells"), StrokeMap->SolidMask[StrokeMap->ToIndex(5, 1)], static_cast<uint8>(1));
	TestEqual(TEXT("Circle stroke paints the end cell"), StrokeMap->SolidMask[StrokeMap->ToIndex(8, 1)], static_cast<uint8>(1));
	StrokeMap->ApplyTextureCircleStroke(1, 1, 8, 1, 0, 4);
	TestEqual(TEXT("Texture circle stroke paints skipped solid drag cells"), StrokeMap->TextureLayerMask[StrokeMap->ToIndex(5, 1)], static_cast<uint8>(4));
	StrokeMap->ApplyCircleStroke(4, 1, 6, 1, 0, false);
	TestEqual(TEXT("Erase circle stroke clears skipped drag cells"), StrokeMap->SolidMask[StrokeMap->ToIndex(5, 1)], static_cast<uint8>(0));
	TestEqual(TEXT("Erase circle stroke clears skipped texture cells"), StrokeMap->TextureLayerMask[StrokeMap->ToIndex(5, 1)], static_cast<uint8>(0));
	StrokeMap->ApplyTexturedCircleStroke(1, 2, 8, 2, 0, 6);
	TestEqual(TEXT("Textured circle stroke fills skipped drag cells"), StrokeMap->SolidMask[StrokeMap->ToIndex(5, 2)], static_cast<uint8>(1));
	TestEqual(TEXT("Textured circle stroke assigns requested layer to skipped drag cells"), StrokeMap->TextureLayerMask[StrokeMap->ToIndex(5, 2)], static_cast<uint8>(6));

	UFRTerrainMapDefinition* ResizedMap = NewObject<UFRTerrainMapDefinition>();
	TestNotNull(TEXT("Resample resize map asset object is created"), ResizedMap);
	ResizedMap->Resize(4, 2);
	ResizedMap->CellSize = 10.0f;
	ResizedMap->Clear(false);
	ResizedMap->FillRect(1, 0, 1, 1, true);
	ResizedMap->ApplyTextureRect(1, 0, 1, 1, 6);
	ResizedMap->PlayerSpawnLocal = FVector(-10.0f, 0.0f, 35.0f);
	ResizedMap->EnemySpawnLocal = FVector(10.0f, 0.0f, 35.0f);
	ResizedMap->ResizeResampled(8, 4);
	TestEqual(TEXT("Resample resize updates map width"), ResizedMap->CellsX, 8);
	TestEqual(TEXT("Resample resize updates map height"), ResizedMap->CellsZ, 4);
	TestEqual(TEXT("Resample resize preserves scaled solid terrain"), ResizedMap->SolidMask[ResizedMap->ToIndex(2, 1)], static_cast<uint8>(1));
	TestEqual(TEXT("Resample resize preserves scaled texture layer"), ResizedMap->TextureLayerMask[ResizedMap->ToIndex(2, 1)], static_cast<uint8>(6));
	TestEqual(TEXT("Resample resize keeps empty areas empty"), ResizedMap->SolidMask[ResizedMap->ToIndex(6, 1)], static_cast<uint8>(0));
	TestEqual(TEXT("Resample resize preserves player spawn horizontal ratio"), ResizedMap->PlayerSpawnLocal.X, -20.0);
	TestEqual(TEXT("Resample resize preserves enemy spawn horizontal ratio"), ResizedMap->EnemySpawnLocal.X, 20.0);
	TestEqual(TEXT("Resample resize preserves spawn clearance above the map"), ResizedMap->PlayerSpawnLocal.Z, 55.0);

	ResizedMap->SetCellSizePreservingSpawns(20.0f);
	TestEqual(TEXT("Cell size update changes the map world cell size"), ResizedMap->CellSize, 20.0f);
	TestEqual(TEXT("Cell size update preserves player spawn horizontal ratio"), ResizedMap->PlayerSpawnLocal.X, -40.0);
	TestEqual(TEXT("Cell size update preserves enemy spawn horizontal ratio"), ResizedMap->EnemySpawnLocal.X, 40.0);
	TestEqual(TEXT("Cell size update preserves spawn clearance above the resized map"), ResizedMap->PlayerSpawnLocal.Z, 95.0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFRTerrainMapDefinitionImportTest, "FortRogue.Terrain.MapDefinition.ImportTextureMask", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFRTerrainMapDefinitionImportTest::RunTest(const FString& Parameters)
{
	UTexture2D* Texture = UTexture2D::CreateTransient(4, 3, PF_B8G8R8A8);
	TestNotNull(TEXT("Transient mask texture is created"), Texture);
	if (!Texture || !Texture->GetPlatformData() || Texture->GetPlatformData()->Mips.Num() == 0)
	{
		return false;
	}

	FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
	void* TextureData = Mip.BulkData.Lock(LOCK_READ_WRITE);
	TestNotNull(TEXT("Transient mask texture mip locks"), TextureData);
	if (!TextureData)
	{
		Mip.BulkData.Unlock();
		return false;
	}

	FColor* Pixels = static_cast<FColor*>(TextureData);
	for (int32 Index = 0; Index < 12; ++Index)
	{
		Pixels[Index] = FColor(0, 0, 0, 0);
	}

	Pixels[0] = FColor(255, 255, 255, 255);
	Pixels[5] = FColor(255, 255, 255, 255);
	Pixels[11] = FColor(255, 255, 255, 255);
	Mip.BulkData.Unlock();

	UFRTerrainMapDefinition* Map = NewObject<UFRTerrainMapDefinition>();
	TestNotNull(TEXT("Map asset object is created"), Map);
	const bool bImported = Map->ImportSolidMaskFromTexture(Texture, true, 0.5f, 7);
	TestTrue(TEXT("Texture alpha mask imports"), bImported);
	TestEqual(TEXT("Import updates map width"), Map->CellsX, 4);
	TestEqual(TEXT("Import updates map height"), Map->CellsZ, 3);

	TestEqual(TEXT("Top-left source pixel maps to top map cell"), Map->SolidMask[Map->ToIndex(0, 2)], static_cast<uint8>(1));
	TestEqual(TEXT("Middle source pixel maps to middle map cell"), Map->SolidMask[Map->ToIndex(1, 1)], static_cast<uint8>(1));
	TestEqual(TEXT("Bottom-right source pixel maps to bottom map cell"), Map->SolidMask[Map->ToIndex(3, 0)], static_cast<uint8>(1));
	TestEqual(TEXT("Imported solid cells receive requested texture layer"), Map->TextureLayerMask[Map->ToIndex(1, 1)], static_cast<uint8>(7));
	TestEqual(TEXT("Transparent cells remain empty"), Map->SolidMask[Map->ToIndex(2, 2)], static_cast<uint8>(0));

	UFRTerrainMapDefinition* ColorMap = NewObject<UFRTerrainMapDefinition>();
	TestNotNull(TEXT("Color import map asset object is created"), ColorMap);
	const bool bColorImported = ColorMap->ImportSolidMaskFromTextureByColor(Texture, FLinearColor::White, 0.01f, 2);
	TestTrue(TEXT("Texture color mask imports"), bColorImported);
	TestEqual(TEXT("Color import updates map width"), ColorMap->CellsX, 4);
	TestEqual(TEXT("Color import updates map height"), ColorMap->CellsZ, 3);
	TestEqual(TEXT("Color-matched source pixel maps to solid terrain"), ColorMap->SolidMask[ColorMap->ToIndex(0, 2)], static_cast<uint8>(1));
	TestEqual(TEXT("Color-matched solid cells receive requested texture layer"), ColorMap->TextureLayerMask[ColorMap->ToIndex(1, 1)], static_cast<uint8>(2));
	TestEqual(TEXT("Nonmatching color cells remain empty"), ColorMap->SolidMask[ColorMap->ToIndex(2, 2)], static_cast<uint8>(0));

	UFRTerrainMapDefinition* ResampledMap = NewObject<UFRTerrainMapDefinition>();
	TestNotNull(TEXT("Resampled import map asset object is created"), ResampledMap);
	ResampledMap->Resize(2, 2);
	const bool bResampledImported = ResampledMap->ImportSolidMaskFromTextureByColor(Texture, FLinearColor::White, 0.01f, 4, false);
	TestTrue(TEXT("Texture color mask imports while keeping current map size"), bResampledImported);
	TestEqual(TEXT("Keep-size import preserves map width"), ResampledMap->CellsX, 2);
	TestEqual(TEXT("Keep-size import preserves map height"), ResampledMap->CellsZ, 2);
	TestEqual(TEXT("Keep-size import samples matching source color"), ResampledMap->SolidMask[ResampledMap->ToIndex(1, 0)], static_cast<uint8>(1));
	TestEqual(TEXT("Keep-size import assigns requested texture layer"), ResampledMap->TextureLayerMask[ResampledMap->ToIndex(1, 0)], static_cast<uint8>(4));

	UFRTerrainMapDefinition* HighResolutionMap = NewObject<UFRTerrainMapDefinition>();
	TestNotNull(TEXT("High-resolution import map asset object is created"), HighResolutionMap);
	const bool bHighResolutionImported = HighResolutionMap->ImportSolidMaskFromTexture(Texture, true, 0.5f, 8, false);
	TestTrue(TEXT("Small source texture imports into the current high-resolution map size"), bHighResolutionImported);
	TestEqual(TEXT("High-resolution keep-size import preserves default map width"), HighResolutionMap->CellsX, 1280);
	TestEqual(TEXT("High-resolution keep-size import preserves default map height"), HighResolutionMap->CellsZ, 960);
	TestEqual(TEXT("High-resolution import samples top-left source pixel"), HighResolutionMap->SolidMask[HighResolutionMap->ToIndex(100, 959)], static_cast<uint8>(1));
	TestEqual(TEXT("High-resolution import samples middle source pixel"), HighResolutionMap->SolidMask[HighResolutionMap->ToIndex(400, 479)], static_cast<uint8>(1));
	TestEqual(TEXT("High-resolution import samples bottom-right source pixel"), HighResolutionMap->SolidMask[HighResolutionMap->ToIndex(1100, 159)], static_cast<uint8>(1));
	TestEqual(TEXT("High-resolution import preserves empty source areas"), HighResolutionMap->SolidMask[HighResolutionMap->ToIndex(800, 959)], static_cast<uint8>(0));
	TestEqual(TEXT("High-resolution imported solid cells receive requested texture layer"), HighResolutionMap->TextureLayerMask[HighResolutionMap->ToIndex(400, 479)], static_cast<uint8>(8));

	UTexture2D* EdgeTexture = UTexture2D::CreateTransient(2, 1, PF_B8G8R8A8);
	TestNotNull(TEXT("Transient edge texture is created"), EdgeTexture);
	if (!EdgeTexture || !EdgeTexture->GetPlatformData() || EdgeTexture->GetPlatformData()->Mips.Num() == 0)
	{
		return false;
	}

	FTexture2DMipMap& EdgeMip = EdgeTexture->GetPlatformData()->Mips[0];
	void* EdgeTextureData = EdgeMip.BulkData.Lock(LOCK_READ_WRITE);
	TestNotNull(TEXT("Transient edge texture mip locks"), EdgeTextureData);
	if (!EdgeTextureData)
	{
		EdgeMip.BulkData.Unlock();
		return false;
	}

	FColor* EdgePixels = static_cast<FColor*>(EdgeTextureData);
	EdgePixels[0] = FColor(0, 0, 0, 0);
	EdgePixels[1] = FColor(255, 255, 255, 255);
	EdgeMip.BulkData.Unlock();

	UFRTerrainMapDefinition* UpscaledEdgeMap = NewObject<UFRTerrainMapDefinition>();
	TestNotNull(TEXT("Upscaled edge import map asset object is created"), UpscaledEdgeMap);
	UpscaledEdgeMap->Resize(4, 1);
	const bool bUpscaledEdgeImported = UpscaledEdgeMap->ImportSolidMaskFromTexture(EdgeTexture, true, 0.2f, 3, false);
	TestTrue(TEXT("Low-resolution edge mask imports into a larger map"), bUpscaledEdgeImported);
	TestEqual(TEXT("Upscaled import keeps the first edge cell empty"), UpscaledEdgeMap->SolidMask[UpscaledEdgeMap->ToIndex(0, 0)], static_cast<uint8>(0));
	TestEqual(TEXT("Upscaled import interpolates the edge instead of nearest-only sampling"), UpscaledEdgeMap->SolidMask[UpscaledEdgeMap->ToIndex(1, 0)], static_cast<uint8>(1));
	TestEqual(TEXT("Upscaled import keeps the solid side filled"), UpscaledEdgeMap->SolidMask[UpscaledEdgeMap->ToIndex(2, 0)], static_cast<uint8>(1));
	TestEqual(TEXT("Upscaled import assigns the requested layer on interpolated cells"), UpscaledEdgeMap->TextureLayerMask[UpscaledEdgeMap->ToIndex(1, 0)], static_cast<uint8>(3));

	UFRTerrainMapDefinition* RegionMap = NewObject<UFRTerrainMapDefinition>();
	TestNotNull(TEXT("Region import map asset object is created"), RegionMap);
	const bool bRegionImported = RegionMap->ImportSolidMaskFromTextureRegion(Texture, 1, 1, 2, 2, true, 0.5f, 5);
	TestTrue(TEXT("Texture region alpha mask imports"), bRegionImported);
	TestEqual(TEXT("Region import updates map width to source region width"), RegionMap->CellsX, 2);
	TestEqual(TEXT("Region import updates map height to source region height"), RegionMap->CellsZ, 2);
	TestEqual(TEXT("Region top-left source pixel maps to top map cell"), RegionMap->SolidMask[RegionMap->ToIndex(0, 1)], static_cast<uint8>(1));
	TestEqual(TEXT("Region nonmatching source pixel remains empty"), RegionMap->SolidMask[RegionMap->ToIndex(1, 1)], static_cast<uint8>(0));
	TestEqual(TEXT("Region imported solid cell receives requested texture layer"), RegionMap->TextureLayerMask[RegionMap->ToIndex(0, 1)], static_cast<uint8>(5));

	UFRTerrainMapDefinition* RegionColorMap = NewObject<UFRTerrainMapDefinition>();
	TestNotNull(TEXT("Region color import map asset object is created"), RegionColorMap);
	RegionColorMap->Resize(2, 2);
	const bool bRegionColorImported = RegionColorMap->ImportSolidMaskFromTextureRegionByColor(Texture, 1, 1, 2, 2, FLinearColor::White, 0.01f, 6, false);
	TestTrue(TEXT("Texture region color mask imports while keeping current map size"), bRegionColorImported);
	TestEqual(TEXT("Region color import preserves map width"), RegionColorMap->CellsX, 2);
	TestEqual(TEXT("Region color import preserves map height"), RegionColorMap->CellsZ, 2);
	TestEqual(TEXT("Region color import samples matching source color"), RegionColorMap->SolidMask[RegionColorMap->ToIndex(0, 1)], static_cast<uint8>(1));
	TestEqual(TEXT("Region color import assigns requested texture layer"), RegionColorMap->TextureLayerMask[RegionColorMap->ToIndex(0, 1)], static_cast<uint8>(6));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFRGameModeCameraControlTest, "FortRogue.GameMode.CameraControl", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFRGameModeCameraControlTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>(GEngine);
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("Camera test game instance is created"), GameInstance);
	TestNotNull(TEXT("Camera test world is created"), World);
	if (!GameInstance || !World)
	{
		return false;
	}

	World->SetShouldTick(false);
	World->AddToRoot();
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.OwningGameInstance = GameInstance;
	World->SetGameInstance(GameInstance);
	WorldContext.SetCurrentWorld(World);
	GameInstance->Init();

	auto CleanupWorld = [&World, &GameInstance]()
	{
		World->RemoveFromRoot();
		GameInstance->Shutdown();
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	};

	FURL URL;
	World->SetGameMode(URL);
	AFRGameMode* GameMode = World->GetAuthGameMode<AFRGameMode>();
	TestNotNull(TEXT("Camera test game mode is created"), GameMode);
	if (!GameMode)
	{
		CleanupWorld();
		return false;
	}

	GameMode->Terrain = World->SpawnActor<AFRDestructibleTerrain>();
	GameMode->BattleCamera = World->SpawnActor<AFRBattleCamera>(
		AFRBattleCamera::StaticClass(),
		GameMode->CameraLocation,
		AFRBattleCamera::GetBattleRotation());
	TestNotNull(TEXT("Camera test terrain is spawned"), GameMode->Terrain.Get());
	TestNotNull(TEXT("Camera test battle camera is spawned"), GameMode->BattleCamera.Get());
	if (!GameMode->Terrain || !GameMode->BattleCamera)
	{
		CleanupWorld();
		return false;
	}

	AFRBattleCamera* BattleCamera = GameMode->BattleCamera;
	BattleCamera->ConfigureBattle(GameMode->CameraLocation, GameMode->CameraOrthoWidth, GameMode->CameraFollowInterpSpeed, GameMode->CameraManualPanSpeed);
	BattleCamera->SetTerrainActor(GameMode->Terrain);

	GameMode->Terrain->Width = 5000.0f;
	GameMode->Terrain->Height = 2000.0f;
	TestEqual(TEXT("Battle camera zoom stays independent from terrain size"), BattleCamera->GetCameraComponent()->OrthoWidth, GameMode->CameraOrthoWidth);

	GameMode->HandleManualCameraInput(FVector2D(0.0f, 1.0f), true, 0.1f);
	TestEqual(TEXT("Fresh direction input switches the camera to manual"), BattleCamera->GetControlMode(), EFRBattleCameraControlMode::Manual);
	BattleCamera->Tick(0.001f);
	TestEqual(TEXT("Manual camera movement does not interpolate"), BattleCamera->GetActorLocation(), BattleCamera->GetManualLocation());
	GameMode->RequestAutoCameraFocus(nullptr, FVector(0.0f, 0.0f, 700.0f), 0.0f);
	TestEqual(TEXT("Auto focus interrupts manual camera control"), BattleCamera->GetControlMode(), EFRBattleCameraControlMode::Auto);
	TestTrue(TEXT("Auto focus blocks the direction input that was already held"), BattleCamera->RequiresManualInputRelease());
	TestEqual(TEXT("Auto focus snaps to the requested height"), static_cast<float>(BattleCamera->GetActorLocation().Z), 700.0f);

	GameMode->HandleManualCameraInput(FVector2D(0.0f, 1.0f), true, 0.1f);
	TestEqual(TEXT("Held direction input cannot interrupt auto focus"), BattleCamera->GetControlMode(), EFRBattleCameraControlMode::Auto);
	GameMode->HandleManualCameraInput(FVector2D::ZeroVector, false, 0.0f);
	GameMode->HandleManualCameraInput(FVector2D(0.0f, 1.0f), true, 0.1f);
	TestEqual(TEXT("Direction input can return to manual after release and press"), BattleCamera->GetControlMode(), EFRBattleCameraControlMode::Manual);

	CleanupWorld();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFRTerrainGameModeMapDefinitionTest, "FortRogue.Terrain.GameMode.UsesMapDefinition", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFRTerrainGameModeMapDefinitionTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>(GEngine);
	TestNotNull(TEXT("Transient game instance is created"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("Transient test world is created"), World);
	if (!World)
	{
		return false;
	}

	World->SetShouldTick(false);
	World->AddToRoot();

	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.OwningGameInstance = GameInstance;
	World->SetGameInstance(GameInstance);
	WorldContext.SetCurrentWorld(World);
	GameInstance->Init();

	auto CleanupWorld = [&World, &GameInstance]()
	{
		if (World->HasBegunPlay())
		{
			World->BeginTearingDown();
			World->EndPlay(EEndPlayReason::Quit);
		}

		World->RemoveFromRoot();
		GameInstance->Shutdown();
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	};

	UFRTerrainMapDefinition* Map = NewObject<UFRTerrainMapDefinition>();
	Map->Resize(20, 6);
	Map->CellSize = 10.0f;
	Map->Clear(false);
	Map->FillRect(0, 0, 19, 0, true);
	Map->PlayerSpawnLocal = FVector(-40.0f, 0.0f, 140.0f);
	Map->EnemySpawnLocal = FVector(40.0f, 0.0f, 140.0f);

	FURL URL;
	World->SetGameMode(URL);
	AFRGameMode* GameMode = World->GetAuthGameMode<AFRGameMode>();
	TestNotNull(TEXT("FortRogue game mode is created"), GameMode);
	if (!GameMode)
	{
		CleanupWorld();
		return false;
	}

	FObjectProperty* TerrainMapProperty = FindFProperty<FObjectProperty>(AFRGameMode::StaticClass(), TEXT("TerrainMapDefinition"));
	TestNotNull(TEXT("Game mode exposes a terrain map definition property"), TerrainMapProperty);
	if (!TerrainMapProperty)
	{
		CleanupWorld();
		return false;
	}
	TerrainMapProperty->SetObjectPropertyValue_InContainer(GameMode, Map);

	UFRStageRunDefinition* TestStageRunDefinition = NewObject<UFRStageRunDefinition>(GameMode);
	TestStageRunDefinition->StageCount = 2;
	TestStageRunDefinition->NormalizeStageData();
	UFRCharacterDefinition* TestEnemyDefinition = NewObject<UFRCharacterDefinition>(TestStageRunDefinition);
	TestEnemyDefinition->BasicAttackDefinition = CreateTestWeaponDefinition(TestEnemyDefinition);
	TestStageRunDefinition->DefaultTerrainMapDefinition = Map;
	TestStageRunDefinition->EnemyDefinitionPool = { TestEnemyDefinition };
	GameMode->StageRunDefinition = TestStageRunDefinition;

	if (FFloatProperty* MinWindProperty = FindFProperty<FFloatProperty>(AFRGameMode::StaticClass(), TEXT("MinWind")))
	{
		MinWindProperty->SetPropertyValue_InContainer(GameMode, 120.0f);
	}
	if (FFloatProperty* MaxWindProperty = FindFProperty<FFloatProperty>(AFRGameMode::StaticClass(), TEXT("MaxWind")))
	{
		MaxWindProperty->SetPropertyValue_InContainer(GameMode, 120.0f);
	}

	World->InitializeActorsForPlay(URL);
	World->BeginPlay();

	AFRDestructibleTerrain* SpawnedTerrain = nullptr;
	for (TActorIterator<AFRDestructibleTerrain> It(World); It; ++It)
	{
		if (It->MapDefinition == Map)
		{
			SpawnedTerrain = *It;
			break;
		}
	}

	TestNotNull(TEXT("Game mode spawns destructible terrain using the configured map definition"), SpawnedTerrain);
	if (SpawnedTerrain)
	{
		TestEqual(TEXT("Spawned terrain width follows the configured map definition"), SpawnedTerrain->Width, 200.0f);
		TestEqual(TEXT("Spawned terrain height follows the configured map definition"), SpawnedTerrain->Height, 60.0f);
		TestNotNull(TEXT("Spawned terrain creates its runtime texture"), SpawnedTerrain->GetRuntimeTerrainTexture());
	}
	TestNotNull(TEXT("Game mode spawns the player character"), GameMode->GetPlayerCharacter());
	TestNotNull(TEXT("Game mode spawns the enemy character"), GameMode->GetEnemyCharacter());
	const int32 EnemyCountBeforeNullDefinitionSpawn = GameMode->GetEnemyCharacters().Num();
	TestNull(TEXT("Game mode skips enemy spawns without a character definition"), GameMode->SpawnEnemyCharacter(World, FVector::ZeroVector, nullptr, false, false));
	TestEqual(TEXT("Null enemy definitions do not add placeholder cube enemies"), GameMode->GetEnemyCharacters().Num(), EnemyCountBeforeNullDefinitionSpawn);
	TestEqual(TEXT("Game mode turn wind can be fixed for deterministic projectile tests"), GameMode->GetWind(), 120.0f);
	TestTrue(TEXT("Game mode wind summary includes signed wind"), GameMode->GetWindSummary().ToString().Contains(TEXT("Wind +120")));
	TestTrue(TEXT("Game mode run progress summary includes stage progress"), GameMode->GetRunProgressSummary().ToString().Contains(TEXT("Stage 1/2")));
	TestFalse(TEXT("Game mode run progress summary keeps status text separate"), GameMode->GetRunProgressSummary().ToString().Contains(GameMode->GetStatusText().ToString()));
	if (AFRBattleCharacter* PlayerCharacter = GameMode->GetPlayerCharacter())
	{
		const float BaseDamageBonus = PlayerCharacter->GetDamageBonus();
		PlayerCharacter->ApplyRewardDamage(10.0f);
		UFRPerkDefinition* NegativeDamagePerk = NewObject<UFRPerkDefinition>(GameMode);
		NegativeDamagePerk->DamageBonus = -4.0f;
		FFRRewardChoice NegativeDamageReward;
		AddTestPerkGrant(NegativeDamageReward, NegativeDamagePerk);
		GameMode->ApplyRewardToPlayer(NegativeDamageReward);
		TestEqual(TEXT("Game mode applies negative perk reward deltas"), PlayerCharacter->GetDamageBonus(), BaseDamageBonus + 6.0f);
	}

	auto HasRewardChoiceTag = [](const TArray<FFRRewardChoice>& RewardChoices, FGameplayTag RewardTag)
	{
		for (const FFRRewardChoice& RewardChoice : RewardChoices)
		{
			if (RewardChoice.RewardTag.MatchesTagExact(RewardTag))
			{
				return true;
			}
		}
		return false;
	};

	FFRRewardChoice BaseReward;
	BaseReward.DisplayName = FText::FromString(TEXT("Damage Seed"));
	BaseReward.RewardTag = FRGameplayTags::Trait_Damage;
	FFRRewardChoice RequiredReward;
	RequiredReward.DisplayName = FText::FromString(TEXT("Projectile Branch"));
	RequiredReward.RewardTag = FRGameplayTags::Trait_Projectiles;
	RequiredReward.RequiredRewardTags.AddTag(FRGameplayTags::Trait_Damage);
	FFRRewardChoice BlockedReward;
	BlockedReward.DisplayName = FText::FromString(TEXT("Health Branch"));
	BlockedReward.RewardTag = FRGameplayTags::Trait_Health;
	BlockedReward.BlockedRewardTags.AddTag(FRGameplayTags::Trait_Damage);
	FFRRewardChoice OpenReward;
	OpenReward.DisplayName = FText::FromString(TEXT("Open Branch"));
	OpenReward.RewardTag = FRGameplayTags::Trait_ShotModifier;
	TestStageRunDefinition->RewardChoiceCount = 5;
	TestStageRunDefinition->RewardPool = { BaseReward, RequiredReward, BlockedReward, OpenReward };

	UFRRunSubsystem* RunSubsystem = GameInstance->GetSubsystem<UFRRunSubsystem>();
	TestNotNull(TEXT("Run subsystem exists for reward tag tracking"), RunSubsystem);
	if (!RunSubsystem)
	{
		CleanupWorld();
		return false;
	}
	RunSubsystem->ClearChosenRewardTags();
	GameMode->BuildRewardChoices();
	FGameplayTagContainer EmptyChosenRewardTags;
	TestFalse(TEXT("Reward condition helper rejects missing required tags"), RequiredReward.MeetsRewardTagConditions(EmptyChosenRewardTags));
	TestFalse(TEXT("Blueprint reward condition helper rejects missing required tags"), UFRRewardBlueprintLibrary::DoesRewardMeetTagConditions(RequiredReward, EmptyChosenRewardTags));
	TestTrue(TEXT("Reward condition failure summary names missing required tags"), RequiredReward.GetRewardTagConditionFailureSummary(EmptyChosenRewardTags).ToString().Contains(TEXT("requires reward")));
	TestFalse(TEXT("Reward choices hide rewards before required reward tags are chosen"), HasRewardChoiceTag(GameMode->GetRewardChoices(), FRGameplayTags::Trait_Projectiles));
	TestTrue(TEXT("Reward choices keep rewards whose blocked tags are absent"), HasRewardChoiceTag(GameMode->GetRewardChoices(), FRGameplayTags::Trait_Health));
	GameMode->RewardChoices = { RequiredReward, BlockedReward };
	TestTrue(TEXT("Game mode reward choice failure summary names missing required tags"), GameMode->GetRewardChoiceConditionFailureSummary(0).ToString().Contains(TEXT("requires reward")));
	TestTrue(TEXT("Game mode reward choice failure summary is empty for invalid choices"), GameMode->GetRewardChoiceConditionFailureSummary(GameMode->GetRewardChoices().Num()).ToString().IsEmpty());
	GameMode->GetTurnBasedGameState()->EnterRewardState(FText::GetEmpty());
	TestFalse(TEXT("Game mode rejects reward choices with unmet required tags"), GameMode->CanApplyRewardChoice(0));
	GameMode->GetTurnBasedGameState()->StartPlayerTurn(GameMode->GetPlayerCharacter(), GameMode->GetWind(), FText::FromString(TEXT("Player turn")));

	RunSubsystem->ClearChosenRewardTags();
	RunSubsystem->RecordChosenRewardTag(FRGameplayTags::Trait_Damage);
	TestTrue(TEXT("Game mode exposes chosen reward tags for UI"), GameMode->GetChosenRewardTags().HasTagExact(FRGameplayTags::Trait_Damage));
	TestTrue(TEXT("Reward condition helper accepts satisfied required tags"), RequiredReward.MeetsRewardTagConditions(GameMode->GetChosenRewardTags()));
	TestFalse(TEXT("Reward condition helper rejects blocked chosen tags"), BlockedReward.MeetsRewardTagConditions(GameMode->GetChosenRewardTags()));
	TestTrue(TEXT("Blueprint reward condition failure summary names blocked tags"), UFRRewardBlueprintLibrary::GetRewardTagConditionFailureSummary(BlockedReward, GameMode->GetChosenRewardTags()).ToString().Contains(TEXT("blocked by reward")));
	TestTrue(TEXT("Reward condition failure summary is empty when conditions pass"), RequiredReward.GetRewardTagConditionFailureSummary(GameMode->GetChosenRewardTags()).ToString().IsEmpty());
	TestTrue(TEXT("Game mode reward choice failure summary names blocked tags"), GameMode->GetRewardChoiceConditionFailureSummary(1).ToString().Contains(TEXT("blocked by reward")));
	TestTrue(TEXT("Game mode reward choice failure summary is empty when conditions pass"), GameMode->GetRewardChoiceConditionFailureSummary(0).ToString().IsEmpty());
	GameMode->GetTurnBasedGameState()->EnterRewardState(FText::GetEmpty());
	TestTrue(TEXT("Game mode accepts reward choices with satisfied required tags"), GameMode->CanApplyRewardChoice(0));
	TestFalse(TEXT("Game mode rejects reward choices with blocked tags"), GameMode->CanApplyRewardChoice(1));
	GameMode->GetTurnBasedGameState()->StartPlayerTurn(GameMode->GetPlayerCharacter(), GameMode->GetWind(), FText::FromString(TEXT("Player turn")));
	GameMode->BuildRewardChoices();
	TestTrue(TEXT("Reward choices include rewards after required reward tags are chosen"), HasRewardChoiceTag(GameMode->GetRewardChoices(), FRGameplayTags::Trait_Projectiles));
	TestFalse(TEXT("Reward choices hide rewards blocked by chosen reward tags"), HasRewardChoiceTag(GameMode->GetRewardChoices(), FRGameplayTags::Trait_Health));
	TestEqual(TEXT("Game mode reward choice count matches current choices"), GameMode->GetRewardChoiceCount(), GameMode->GetRewardChoices().Num());
	TestTrue(TEXT("Game mode returns reward choices by index"), GameMode->GetRewardChoice(0).RewardTag.IsValid());
	TestFalse(TEXT("Game mode returns default reward choices for invalid indexes"), GameMode->GetRewardChoice(GameMode->GetRewardChoiceCount()).RewardTag.IsValid());
	TestFalse(TEXT("Game mode rejects reward choice outside reward state"), GameMode->CanApplyRewardChoice(0));
	TestFalse(TEXT("Game mode reward choice summary is available for valid choices"), GameMode->GetRewardChoiceSummary(0).ToString().IsEmpty());
	TestTrue(TEXT("Game mode reward choice summary is empty for invalid choices"), GameMode->GetRewardChoiceSummary(GameMode->GetRewardChoices().Num()).ToString().IsEmpty());
	GameMode->GetTurnBasedGameState()->EnterRewardState(FText::GetEmpty());
	TestTrue(TEXT("Game mode reports valid reward choices selectable"), GameMode->CanApplyRewardChoice(0));
	TestFalse(TEXT("Game mode rejects negative reward choice indexes"), GameMode->CanApplyRewardChoice(-1));
	TestFalse(TEXT("Game mode rejects reward choice indexes past the end"), GameMode->CanApplyRewardChoice(GameMode->GetRewardChoices().Num()));
	const int32 StageBeforeRewardChoice = GameMode->GetCurrentStage();
	const FGameplayTag AppliedRewardTag = GameMode->GetRewardChoice(0).RewardTag;
	GameMode->ApplyRewardChoice(0);
	TestTrue(TEXT("Game mode records applied reward tags"), AppliedRewardTag.IsValid() && GameMode->GetChosenRewardTags().HasTagExact(AppliedRewardTag));
	TestEqual(TEXT("Game mode advances to the next stage after reward choice"), GameMode->GetCurrentStage(), StageBeforeRewardChoice + 1);
	TestEqual(TEXT("Game mode clears reward choices after applying one"), GameMode->GetRewardChoiceCount(), 0);
	TestEqual(TEXT("Game mode returns to player turn after reward choice"), GameMode->GetBattleState(), EFRBattleState::PlayerTurn);
	GameMode->GetTurnBasedGameState()->StartPlayerTurn(GameMode->GetPlayerCharacter(), GameMode->GetWind(), FText::FromString(TEXT("Player turn")));
	TestStageRunDefinition->RewardPool.Reset();
	GameMode->RewardChoices.Reset();
	RunSubsystem->ClearChosenRewardTags();

	AFRProjectile* WindProjectile = World->SpawnActor<AFRProjectile>(AFRProjectile::StaticClass(), FVector(0.0f, 0.0f, 500.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Wind test projectile is spawned"), WindProjectile);
	if (WindProjectile)
	{
		WindProjectile->InitializeProjectile(nullptr, nullptr, FVector::ZeroVector, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		WindProjectile->Tick(0.5f);
		TestEqual(TEXT("Projectile drift follows the current wind acceleration"), static_cast<float>(WindProjectile->GetActorLocation().X), 30.0f);
	}

	ACameraActor* BattleCamera = nullptr;
	for (TActorIterator<ACameraActor> It(World); It; ++It)
	{
		BattleCamera = *It;
		break;
	}

	TestNotNull(TEXT("Game mode spawns a battle camera"), BattleCamera);
	if (BattleCamera)
	{
		TestEqual(TEXT("Battle camera faces the X/Z gameplay plane without requiring terrain rotation"), BattleCamera->GetActorRotation(), FRotator(0.0f, -90.0f, 0.0f));
		TestTrue(TEXT("Battle camera screen right maps to positive world X"), BattleCamera->GetActorRightVector().X > 0.99f);
		TestEqual(TEXT("Battle camera views the gameplay plane from positive Y"), static_cast<float>(BattleCamera->GetActorLocation().Y), 3000.0f);
		TestEqual(TEXT("Battle camera X stays centered when the full terrain fits in view"), static_cast<float>(BattleCamera->GetActorLocation().X), 0.0f);
		TestNotNull(TEXT("Battle camera has a camera component"), BattleCamera->GetCameraComponent());
		if (BattleCamera->GetCameraComponent())
		{
			TestEqual(TEXT("Battle camera uses orthographic projection"), BattleCamera->GetCameraComponent()->ProjectionMode, ECameraProjectionMode::Orthographic);
		}
	}

	if (SpawnedTerrain && GameMode->GetPlayerCharacter() && GameMode->GetEnemyCharacter())
	{
		const FVector EnemyLocation = GameMode->GetEnemyCharacter()->GetActorLocation();
		SpawnedTerrain->CarveCircle(FVector(EnemyLocation.X, EnemyLocation.Y, 5.0f), 40.0f);
		GameMode->GetEnemyCharacter()->SetActorLocation(EnemyLocation + FVector(0.0f, 0.0f, 500.0f));
		GameMode->GetEnemyCharacter()->ReevaluateTerrainSupport();

		GameMode->StartEnemyTurn();
		TestEqual(TEXT("Game mode can enter the enemy turn for an unsupported enemy"), GameMode->GetBattleState(), EFRBattleState::EnemyTurn);
		GameMode->RunEnemyTurn();
		TestEqual(TEXT("Unsupported enemy that cannot fire yields the turn back to the player"), GameMode->GetBattleState(), EFRBattleState::PlayerTurn);
	}

	AFRProjectile* StrayProjectile = World->SpawnActor<AFRProjectile>(AFRProjectile::StaticClass(), FVector(25.0f, 0.0f, 500.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Stray projectile is spawned"), StrayProjectile);
	if (StrayProjectile)
	{
		TestEqual(TEXT("Game mode starts on the player turn before stray projectile resolution"), GameMode->GetBattleState(), EFRBattleState::PlayerTurn);
		GameMode->NotifyProjectileResolved(StrayProjectile);
		World->Tick(ELevelTick::LEVELTICK_All, 1.0f);
		TestEqual(TEXT("Stray projectile resolution outside a shot does not advance the turn"), GameMode->GetBattleState(), EFRBattleState::PlayerTurn);
	}

	UFRTerrainMapDefinition* NextStageMap = NewObject<UFRTerrainMapDefinition>();
	NextStageMap->Resize(16, 8);
	NextStageMap->CellSize = 12.0f;
	NextStageMap->Clear(false);
	NextStageMap->FillRect(0, 0, 15, 1, true);
	NextStageMap->PlayerSpawnLocal = FVector(-36.0f, 0.0f, 140.0f);
	NextStageMap->EnemySpawnLocal = FVector(36.0f, 0.0f, 140.0f);

	UFRCharacterDefinition* NextStageEnemyDefinition = NewObject<UFRCharacterDefinition>();
	NextStageEnemyDefinition->DisplayName = FText::FromString(TEXT("Map Carrier"));
	NextStageEnemyDefinition->BasicAttackDefinition = CreateTestWeaponDefinition(NextStageEnemyDefinition);

	TestStageRunDefinition->DefaultTerrainMapDefinition = NextStageMap;
	TestStageRunDefinition->EnemyDefinitionPool = { NextStageEnemyDefinition };
	const int32 StageBeforeEnemyDefeat = GameMode->GetCurrentStage();
	TestStageRunDefinition->StageCount = StageBeforeEnemyDefeat + 1;
	TestStageRunDefinition->NormalizeStageData();
	TestEqual(TEXT("Game mode has one more stage before enemy defeat transition"), GameMode->GetMaxStages(), StageBeforeEnemyDefeat + 1);
	TestNotNull(TEXT("Game mode has an enemy before stage transition"), GameMode->GetEnemyCharacter());
	if (GameMode->GetEnemyCharacter())
	{
		GameMode->GetEnemyCharacter()->ApplyDamage(100000.0f);
		GameMode->CheckTurnDefeatState();
		TestEqual(TEXT("Defeating an enemy advances to the next stage without entering rewards"), GameMode->GetCurrentStage(), StageBeforeEnemyDefeat + 1);
		TestEqual(TEXT("Next stage starts on the player turn"), GameMode->GetBattleState(), EFRBattleState::PlayerTurn);
		TestTrue(TEXT("Next stage uses the encountered enemy character definition"), GameMode->CurrentEnemyDefinition == NextStageEnemyDefinition);
		TestNotNull(TEXT("Next stage respawns terrain"), GameMode->Terrain.Get());
		if (GameMode->Terrain)
		{
			TestEqual(TEXT("Next stage terrain follows the stage run map definition"), GameMode->Terrain->MapDefinition.Get(), NextStageMap);
			TestEqual(TEXT("Next stage terrain width follows the stage run map definition"), GameMode->Terrain->Width, 192.0f);
		}
	}

	CleanupWorld();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFRBattleCharacterSpriteLocalRotationTest, "FortRogue.Character.SpriteLocalRotation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFRBattleCharacterSpriteLocalRotationTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>(GEngine);
	TestNotNull(TEXT("Transient game instance is created"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("Transient test world is created"), World);
	if (!World)
	{
		return false;
	}

	World->SetShouldTick(false);
	World->AddToRoot();

	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.OwningGameInstance = GameInstance;
	World->SetGameInstance(GameInstance);
	WorldContext.SetCurrentWorld(World);
	GameInstance->Init();

	auto CleanupWorld = [&World, &GameInstance]()
	{
		World->RemoveFromRoot();
		GameInstance->Shutdown();
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	};

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AFRBattleCharacter* Character = World->SpawnActor<AFRBattleCharacter>(AFRBattleCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Battle character is spawned"), Character);
	if (!Character)
	{
		CleanupWorld();
		return false;
	}

	UPaperFlipbookComponent* BodySprite = Cast<UPaperFlipbookComponent>(Character->GetDefaultSubobjectByName(TEXT("BodySprite")));
	TestNotNull(TEXT("Battle character sprite component exists"), BodySprite);
	if (!BodySprite)
	{
		CleanupWorld();
		return false;
	}

	BodySprite->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	Character->ConfigureAsEnemy(true);
	TestEqual(TEXT("Battle character actor root turns left"), static_cast<float>(FMath::Abs(FRotator::NormalizeAxis(Character->GetActorRotation().Yaw))), 180.0f);
	TestEqual(TEXT("Battle character sprite local rotation is normalized while actor turns left"), BodySprite->GetRelativeRotation(), FRotator::ZeroRotator);

	BodySprite->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	Character->ConfigureAsEnemy(false);
	TestEqual(TEXT("Battle character actor root turns right"), static_cast<float>(Character->GetActorRotation().Yaw), 0.0f);
	TestEqual(TEXT("Battle character sprite local rotation is normalized while actor turns right"), BodySprite->GetRelativeRotation(), FRotator::ZeroRotator);

	CleanupWorld();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFRCharacterSpriteAnimatorStateTest, "FortRogue.Character.SpriteAnimator.StateTransitions", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFRCharacterSpriteAnimatorStateTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>(GEngine);
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!GameInstance || !World)
	{
		TestNotNull(TEXT("Transient test world is created"), World);
		return false;
	}

	World->SetShouldTick(false);
	World->AddToRoot();
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.OwningGameInstance = GameInstance;
	World->SetGameInstance(GameInstance);
	WorldContext.SetCurrentWorld(World);
	GameInstance->Init();

	auto CleanupWorld = [&World, &GameInstance]()
	{
		World->RemoveFromRoot();
		GameInstance->Shutdown();
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	};

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AFRBattleCharacter* Character = World->SpawnActor<AFRBattleCharacter>(AFRBattleCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	UPaperFlipbookComponent* BodySprite = Character ? Cast<UPaperFlipbookComponent>(Character->GetDefaultSubobjectByName(TEXT("BodySprite"))) : nullptr;
	UFRCharacterSpriteAnimator* Animator = Character ? Character->FindComponentByClass<UFRCharacterSpriteAnimator>() : nullptr;
	TestNotNull(TEXT("Battle character sprite animator exists"), Animator);
	if (!Character || !BodySprite || !Animator)
	{
		CleanupWorld();
		return false;
	}

	FFRCharacterAnimationSet AnimationSet;
	AnimationSet.Idle = NewObject<UPaperFlipbook>(GetTransientPackage());
	AnimationSet.Move = NewObject<UPaperFlipbook>(GetTransientPackage());
	AnimationSet.Shoot = NewObject<UPaperFlipbook>(GetTransientPackage());
	AnimationSet.Special = NewObject<UPaperFlipbook>(GetTransientPackage());
	AnimationSet.Hurt = NewObject<UPaperFlipbook>(GetTransientPackage());

	Animator->Initialize(BodySprite, AnimationSet, nullptr);
	TestEqual(TEXT("Animator starts in idle"), Animator->GetAnimState(), EFRCharacterAnimState::Idle);
	TestTrue(TEXT("Idle flipbook is applied"), BodySprite->GetFlipbook() == AnimationSet.Idle);

	Animator->NotifyMoving();
	TestEqual(TEXT("Moving switches to move state"), Animator->GetAnimState(), EFRCharacterAnimState::Move);
	TestTrue(TEXT("Move flipbook is applied"), BodySprite->GetFlipbook() == AnimationSet.Move);

	Animator->NotifyShoot(false);
	TestEqual(TEXT("Shooting switches to shoot state"), Animator->GetAnimState(), EFRCharacterAnimState::Shoot);
	TestTrue(TEXT("Shoot flipbook is applied"), BodySprite->GetFlipbook() == AnimationSet.Shoot);

	BodySprite->OnFinishedPlaying.Broadcast();
	TestEqual(TEXT("Shoot one-shot reverts to idle"), Animator->GetAnimState(), EFRCharacterAnimState::Idle);
	TestTrue(TEXT("Idle flipbook is restored"), BodySprite->GetFlipbook() == AnimationSet.Idle);

	Animator->NotifyShoot(true);
	TestEqual(TEXT("Special shot switches to special state"), Animator->GetAnimState(), EFRCharacterAnimState::Special);
	TestTrue(TEXT("Special flipbook is applied"), BodySprite->GetFlipbook() == AnimationSet.Special);

	Animator->NotifyHurt();
	TestEqual(TEXT("Damage interrupts into hurt state"), Animator->GetAnimState(), EFRCharacterAnimState::Hurt);
	TestTrue(TEXT("Hurt flipbook is applied"), BodySprite->GetFlipbook() == AnimationSet.Hurt);

	BodySprite->OnFinishedPlaying.Broadcast();
	TestEqual(TEXT("Hurt one-shot reverts to idle"), Animator->GetAnimState(), EFRCharacterAnimState::Idle);

	// 상태별 플립북이 비어 있으면 대체 규칙을 따릅니다: Special→Shoot, Move→Idle 아트 유지.
	FFRCharacterAnimationSet PartialSet;
	PartialSet.Idle = NewObject<UPaperFlipbook>(GetTransientPackage());
	PartialSet.Shoot = NewObject<UPaperFlipbook>(GetTransientPackage());
	Animator->Initialize(BodySprite, PartialSet, nullptr);
	Animator->NotifyShoot(true);
	TestEqual(TEXT("Special without special flipbook falls back to shoot"), Animator->GetAnimState(), EFRCharacterAnimState::Shoot);
	TestTrue(TEXT("Fallback shoot flipbook is applied"), BodySprite->GetFlipbook() == PartialSet.Shoot);

	CleanupWorld();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFRDestructibleTerrainRuntimeTest, "FortRogue.Terrain.DestructibleTerrain.RuntimeQueries", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFRDestructibleTerrainRuntimeTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>(GEngine);
	TestNotNull(TEXT("Transient game instance is created"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("Transient test world is created"), World);
	if (!World)
	{
		return false;
	}

	World->SetShouldTick(false);
	World->AddToRoot();

	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.OwningGameInstance = GameInstance;
	World->SetGameInstance(GameInstance);
	WorldContext.SetCurrentWorld(World);
	GameInstance->Init();

	auto CleanupWorld = [&World, &GameInstance]()
	{
		if (World->HasBegunPlay())
		{
			World->BeginTearingDown();
			World->EndPlay(EEndPlayReason::Quit);
		}

		World->RemoveFromRoot();
		GameInstance->Shutdown();
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	};

	UFRTerrainMapDefinition* Map = NewObject<UFRTerrainMapDefinition>();
	Map->Resize(20, 6);
	Map->CellSize = 10.0f;
	Map->Clear(false);
	Map->FillRect(0, 0, 19, 0, true);
	Map->FillRect(3, 1, 3, 1, true);
	Map->FillRect(4, 1, 4, 2, true);
	Map->FillRect(7, 1, 7, 3, true);
	Map->FillRect(10, 0, 10, 5, true);
	Map->PlayerSpawnLocal = FVector(-999.0f, 0.0f, 5.0f);
	Map->EnemySpawnLocal = FVector(999.0f, 0.0f, 5.0f);

	UTexture2D* LayerTexture = nullptr;
#if WITH_EDITOR
	LayerTexture = NewObject<UTexture2D>();
#else
	LayerTexture = UTexture2D::CreateTransient(2, 2, PF_B8G8R8A8);
#endif
	TestNotNull(TEXT("Layer source texture is created"), LayerTexture);
	if (LayerTexture)
	{
#if WITH_EDITOR
		const uint8 SourcePixels[] = {
			0, 0, 255, 255,
			0, 255, 0, 255,
			255, 0, 0, 255,
			0, 255, 255, 255
		};
		LayerTexture->Source.Init(2, 2, 1, 1, TSF_BGRA8, SourcePixels);
#else
		FTexture2DMipMap& LayerMip = LayerTexture->GetPlatformData()->Mips[0];
		void* LayerData = LayerMip.BulkData.Lock(LOCK_READ_WRITE);
		TestNotNull(TEXT("Layer source texture mip locks"), LayerData);
		if (LayerData)
		{
			FColor* LayerPixels = static_cast<FColor*>(LayerData);
			LayerPixels[0] = FColor::Red;
			LayerPixels[1] = FColor::Green;
			LayerPixels[2] = FColor::Blue;
			LayerPixels[3] = FColor::Yellow;
		}
		LayerMip.BulkData.Unlock();
#endif
		Map->SetTextureLayer(0, LayerTexture);
	}

	UFRTerrainMapDefinition* OverlappingMap = NewObject<UFRTerrainMapDefinition>();
	OverlappingMap->Resize(20, 6);
	OverlappingMap->CellSize = 10.0f;
	OverlappingMap->Clear(false);
	OverlappingMap->FillRect(0, 0, 19, 0, true);

	UFRTerrainMapDefinition* FastProjectileMap = NewObject<UFRTerrainMapDefinition>();
	FastProjectileMap->Resize(10, 6);
	FastProjectileMap->CellSize = 10.0f;
	FastProjectileMap->Clear(false);
	FastProjectileMap->FillRect(5, 3, 5, 3, true);

	UFRTerrainMapDefinition* GapMap = NewObject<UFRTerrainMapDefinition>();
	GapMap->Resize(12, 4);
	GapMap->CellSize = 10.0f;
	GapMap->Clear(false);
	GapMap->FillRect(0, 0, 4, 0, true);
	GapMap->FillRect(7, 0, 11, 0, true);

	FActorSpawnParameters SpawnParams;
	AFRDestructibleTerrain* Terrain = World->SpawnActor<AFRDestructibleTerrain>(AFRDestructibleTerrain::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Terrain actor is spawned"), Terrain);
	if (!Terrain)
	{
		CleanupWorld();
		return false;
	}

	TestEqual(TEXT("Fallback terrain uses high-resolution cells"), Terrain->CellSize, 1.0f);
	UStaticMeshComponent* TexturePlane = Cast<UStaticMeshComponent>(Terrain->GetDefaultSubobjectByName(TEXT("TerrainTexturePlane")));
	TestNotNull(TEXT("Terrain texture plane exists"), TexturePlane);
	if (TexturePlane)
	{
		TestEqual(TEXT("Terrain texture plane is aligned to the X/Z gameplay plane"), TexturePlane->GetRelativeRotation(), FRotator(0.0f, 0.0f, 90.0f));
		TestEqual(TEXT("Terrain texture plane does not use Unreal collision"), TexturePlane->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
	}
	UInstancedStaticMeshComponent* TextureTerrainInstances = Cast<UInstancedStaticMeshComponent>(Terrain->GetDefaultSubobjectByName(TEXT("TerrainInstances")));
	if (TextureTerrainInstances)
	{
		TestEqual(TEXT("Terrain instances do not use Unreal collision"), TextureTerrainInstances->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
	}

	Terrain->MapDefinition = Map;

	AFRDestructibleTerrain* OverlappingTerrain = World->SpawnActor<AFRDestructibleTerrain>(AFRDestructibleTerrain::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Overlapping terrain actor is spawned"), OverlappingTerrain);
	if (OverlappingTerrain)
	{
		OverlappingTerrain->MapDefinition = OverlappingMap;
	}

	AFRDestructibleTerrain* FastProjectileTerrain = World->SpawnActor<AFRDestructibleTerrain>(AFRDestructibleTerrain::StaticClass(), FVector(300.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Fast projectile test terrain actor is spawned"), FastProjectileTerrain);
	if (FastProjectileTerrain)
	{
		FastProjectileTerrain->MapDefinition = FastProjectileMap;
	}

	AFRDestructibleTerrain* GapTerrain = World->SpawnActor<AFRDestructibleTerrain>(AFRDestructibleTerrain::StaticClass(), FVector(500.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Gap movement test terrain actor is spawned"), GapTerrain);
	if (GapTerrain)
	{
		GapTerrain->MapDefinition = GapMap;
	}

	UFRTerrainMapDefinition* TransformMap = NewObject<UFRTerrainMapDefinition>();
	TransformMap->Resize(4, 2);
	TransformMap->CellSize = 10.0f;
	TransformMap->Clear(true);

	AFRDestructibleTerrain* TransformTerrain = World->SpawnActorDeferred<AFRDestructibleTerrain>(
		AFRDestructibleTerrain::StaticClass(),
		FTransform(FRotator(0.0f, 90.0f, 90.0f), FVector(900.0f, 0.0f, 0.0f), FVector(2.0f, 3.0f, 4.0f)));
	TestNotNull(TEXT("Transform-normalized terrain actor is spawned deferred"), TransformTerrain);
	if (TransformTerrain)
	{
		TransformTerrain->MapDefinition = TransformMap;
		UGameplayStatics::FinishSpawningActor(
			TransformTerrain,
			FTransform(FRotator(0.0f, 90.0f, 90.0f), FVector(900.0f, 0.0f, 0.0f), FVector(2.0f, 3.0f, 4.0f)));
		TestEqual(TEXT("Terrain actor clears accidental gameplay rotation"), TransformTerrain->GetActorRotation(), FRotator::ZeroRotator);
		TestEqual(TEXT("Terrain actor clears accidental gameplay scale"), TransformTerrain->GetActorScale3D(), FVector::OneVector);
		TestTrue(TEXT("Transform-normalized terrain still queries solid cells on world X/Z"), TransformTerrain->IsSolidAtWorldLocation(FVector(885.0f, 0.0f, 5.0f)));
		if (UStaticMeshComponent* TransformTexturePlane = Cast<UStaticMeshComponent>(TransformTerrain->GetDefaultSubobjectByName(TEXT("TerrainTexturePlane"))))
		{
			TestEqual(TEXT("Transform-normalized texture plane keeps X/Z rotation"), TransformTexturePlane->GetComponentRotation(), FRotator(0.0f, 0.0f, 90.0f));
			TestEqual(TEXT("Transform-normalized texture plane keeps X/Z centered world location"), TransformTexturePlane->GetComponentLocation(), FVector(900.0f, -6.0f, 10.0f));
			TestEqual(TEXT("Transform-normalized texture plane keeps map width scale"), static_cast<float>(TransformTexturePlane->GetComponentScale().X), 0.4f);
			TestEqual(TEXT("Transform-normalized texture plane keeps map height scale"), static_cast<float>(TransformTexturePlane->GetComponentScale().Y), 0.2f);
		}
	}

	UFRTerrainMapDefinition* RuntimeCorruptMap = NewObject<UFRTerrainMapDefinition>();
	RuntimeCorruptMap->CellsX = 3;
	RuntimeCorruptMap->CellsZ = 2;
	RuntimeCorruptMap->CellSize = 0.0f;
	RuntimeCorruptMap->SolidMask = { 2, 0, 1 };
	RuntimeCorruptMap->TextureLayerMask = { 4, 7 };

	AFRDestructibleTerrain* RuntimeCorruptTerrain = World->SpawnActorDeferred<AFRDestructibleTerrain>(
		AFRDestructibleTerrain::StaticClass(),
		FTransform(FRotator::ZeroRotator, FVector(1000.0f, 0.0f, 0.0f)));
	TestNotNull(TEXT("Runtime-normalized terrain actor is spawned deferred"), RuntimeCorruptTerrain);
	if (RuntimeCorruptTerrain)
	{
		RuntimeCorruptTerrain->MapDefinition = RuntimeCorruptMap;
		UGameplayStatics::FinishSpawningActor(RuntimeCorruptTerrain, FTransform(FRotator::ZeroRotator, FVector(1000.0f, 0.0f, 0.0f)));
		TestEqual(TEXT("Runtime terrain normalizes invalid map cell size"), RuntimeCorruptTerrain->CellSize, 1.0f);
		TestEqual(TEXT("Runtime terrain repairs corrupt map solid mask size"), RuntimeCorruptMap->SolidMask.Num(), 6);
		TestEqual(TEXT("Runtime terrain repairs corrupt map texture layer mask size"), RuntimeCorruptMap->TextureLayerMask.Num(), 6);
		TestTrue(TEXT("Runtime terrain uses normalized solid cells from the map"), RuntimeCorruptTerrain->IsSolidAtWorldLocation(FVector(998.6f, 0.0f, 0.5f)));
		TestFalse(TEXT("Runtime terrain uses normalized empty cells from the map"), RuntimeCorruptTerrain->IsSolidAtWorldLocation(FVector(999.5f, 0.0f, 0.5f)));
	}

	UFRTerrainMapDefinition* PreviewMap = NewObject<UFRTerrainMapDefinition>();
	PreviewMap->Resize(6, 3);
	PreviewMap->CellSize = 10.0f;
	PreviewMap->Clear(false);
	PreviewMap->FillTexturedRect(2, 1, 2, 1, 4);

	AFRDestructibleTerrain* PreviewTerrain = World->SpawnActorDeferred<AFRDestructibleTerrain>(AFRDestructibleTerrain::StaticClass(), FTransform(FRotator::ZeroRotator, FVector(700.0f, 0.0f, 0.0f)));
	TestNotNull(TEXT("Construction preview terrain actor is spawned deferred"), PreviewTerrain);
	if (PreviewTerrain)
	{
		PreviewTerrain->MapDefinition = PreviewMap;
		UGameplayStatics::FinishSpawningActor(PreviewTerrain, FTransform(FRotator::ZeroRotator, FVector(700.0f, 0.0f, 0.0f)));
		TestTrue(TEXT("Construction preview terrain uses its map mask before BeginPlay"), PreviewTerrain->IsSolidAtWorldLocation(FVector(695.0f, 0.0f, 15.0f)));
		TestNotNull(TEXT("Construction preview terrain creates its runtime texture before BeginPlay"), PreviewTerrain->GetRuntimeTerrainTexture());
		if (UStaticMeshComponent* PreviewTexturePlane = Cast<UStaticMeshComponent>(PreviewTerrain->GetDefaultSubobjectByName(TEXT("TerrainTexturePlane"))))
		{
			TestEqual(TEXT("Construction preview texture plane width follows the map definition"), static_cast<float>(PreviewTexturePlane->GetRelativeScale3D().X), 0.6f);
			TestEqual(TEXT("Construction preview texture plane height follows the map definition"), static_cast<float>(PreviewTexturePlane->GetRelativeScale3D().Y), 0.3f);
		}
	}

	FURL URL;
	World->SetGameMode(URL);
	World->InitializeActorsForPlay(URL);
	World->BeginPlay();
	AFRGameMode* RuntimeGameMode = World->GetAuthGameMode<AFRGameMode>();

	UFRTerrainMapDefinition* FillRuntimeMap = NewObject<UFRTerrainMapDefinition>();
	FillRuntimeMap->Resize(4, 3);
	FillRuntimeMap->CellSize = 10.0f;
	FillRuntimeMap->Clear(false);
	AFRDestructibleTerrain* FillRuntimeTerrain = World->SpawnActorDeferred<AFRDestructibleTerrain>(AFRDestructibleTerrain::StaticClass(), FTransform(FRotator::ZeroRotator, FVector(1200.0f, 0.0f, 0.0f)));
	TestNotNull(TEXT("Runtime fill terrain actor is spawned deferred"), FillRuntimeTerrain);
	if (FillRuntimeTerrain)
	{
		FillRuntimeTerrain->MapDefinition = FillRuntimeMap;
		UGameplayStatics::FinishSpawningActor(FillRuntimeTerrain, FTransform(FRotator::ZeroRotator, FVector(1200.0f, 0.0f, 0.0f)));
		TestFalse(TEXT("Runtime fill target starts empty"), FillRuntimeTerrain->IsSolidAtWorldLocation(FVector(1200.0f, 0.0f, 15.0f)));
		TestTrue(TEXT("Runtime fill circle reports changed terrain"), FillRuntimeTerrain->FillCircle(FVector(1200.0f, 0.0f, 15.0f), 8.0f));
		TestTrue(TEXT("Runtime fill circle creates solid terrain"), FillRuntimeTerrain->IsSolidAtWorldLocation(FVector(1200.0f, 0.0f, 15.0f)));
	}

	UTexture2D* RuntimeTexture = Terrain->GetRuntimeTerrainTexture();
	TestNotNull(TEXT("Runtime terrain texture is created"), RuntimeTexture);
	if (RuntimeTexture)
	{
		TestTrue(TEXT("Runtime terrain texture uses bilinear filtering to avoid chunky upscale"), RuntimeTexture->Filter == TF_Bilinear);
		TestTrue(TEXT("Runtime terrain texture clamps horizontally to avoid edge bleeding"), RuntimeTexture->AddressX == TA_Clamp);
		TestTrue(TEXT("Runtime terrain texture clamps vertically to avoid edge bleeding"), RuntimeTexture->AddressY == TA_Clamp);
		TestTrue(TEXT("Runtime terrain texture does not stream"), RuntimeTexture->NeverStream);
		FTexture2DMipMap& RuntimeMip = RuntimeTexture->GetPlatformData()->Mips[0];
		const FColor* RuntimePixels = static_cast<const FColor*>(RuntimeMip.BulkData.LockReadOnly());
		TestNotNull(TEXT("Runtime terrain texture mip locks"), RuntimePixels);
		if (RuntimePixels)
		{
			TestEqual(TEXT("Empty terrain cells render transparent"), RuntimePixels[0 * 20 + 0], FColor(0, 0, 0, 0));
			TestEqual(TEXT("Layer texture maps across bottom-left terrain cells"), RuntimePixels[5 * 20 + 2], FColor::Blue);
			TestEqual(TEXT("Layer texture maps across bottom-right terrain cells"), RuntimePixels[5 * 20 + 17], FColor::Yellow);
			TestEqual(TEXT("Layer texture maps across top-right terrain cells"), RuntimePixels[0 * 20 + 10], FColor::Green);
		}
		RuntimeMip.BulkData.Unlock();
	}
	if (TexturePlane)
	{
		TestEqual(TEXT("Terrain texture plane width follows the map definition"), static_cast<float>(TexturePlane->GetRelativeScale3D().X), 2.0f);
		TestEqual(TEXT("Terrain texture plane height follows the map definition"), static_cast<float>(TexturePlane->GetRelativeScale3D().Y), 0.6f);
		if (TexturePlane->IsVisible() && TextureTerrainInstances)
		{
			TestEqual(TEXT("Texture-rendered terrain does not build fallback instances"), TextureTerrainInstances->GetInstanceCount(), 0);
		}
	}

	const FVector PlayerSpawn = Terrain->GetPlayerSpawnWorldLocation();
	const FVector EnemySpawn = Terrain->GetEnemySpawnWorldLocation();
	TestTrue(TEXT("Player spawn is clamped inside terrain width"), PlayerSpawn.X > -100.0f && PlayerSpawn.X < 100.0f);
	TestTrue(TEXT("Enemy spawn is clamped inside terrain width"), EnemySpawn.X > -100.0f && EnemySpawn.X < 100.0f);
	TestTrue(TEXT("Spawns below map top are lifted above terrain height"), PlayerSpawn.Z > 60.0f && EnemySpawn.Z > 60.0f);
	TestEqual(TEXT("Player spawn height resolves from the terrain surface at its clamped X"), PlayerSpawn.Z, 90.0);
	TestEqual(TEXT("Enemy spawn height resolves from the terrain surface at its clamped X"), EnemySpawn.Z, 90.0);

	TestTrue(TEXT("Bottom cell is solid in world space"), Terrain->IsSolidAtWorldLocation(FVector(-15.0f, 0.0f, 5.0f)));
	TestFalse(TEXT("Empty air cell is not solid in world space"), Terrain->IsSolidAtWorldLocation(FVector(-15.0f, 0.0f, 15.0f)));
	TestFalse(TEXT("Carve circle outside the terrain reports no change"), Terrain->CarveCircle(FVector(500.0f, 0.0f, 5.0f), 20.0f));
	TestTrue(TEXT("Out-of-bounds carve leaves terrain unchanged"), Terrain->IsSolidAtWorldLocation(FVector(-15.0f, 0.0f, 5.0f)));

	float SurfaceZ = 0.0f;
	TestTrue(TEXT("Surface can be found above a solid column"), Terrain->FindSurfaceZAtWorldX(-15.0f, 30.0f, 40.0f, SurfaceZ));
	TestEqual(TEXT("Surface Z matches the top of the solid cell"), SurfaceZ, 10.0f);
	TestFalse(TEXT("Buried wall cells are not treated as walkable surfaces"), Terrain->FindSurfaceZAtWorldX(5.0f, 44.0f, 90.0f, SurfaceZ));
	TestTrue(TEXT("Exposed wall top is still found from above"), Terrain->FindSurfaceZAtWorldX(5.0f, 65.0f, 90.0f, SurfaceZ));
	TestEqual(TEXT("Exposed wall top surface matches the column height"), SurfaceZ, 60.0f);

	AFRBattleCharacter* Character = World->SpawnActor<AFRBattleCharacter>(AFRBattleCharacter::StaticClass(), FVector(-5.0f, 0.0f, 55.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Battle character is spawned"), Character);
	if (Character)
	{
		TestEqual(TEXT("Battle character actor starts facing right"), Character->GetActorRotation(), FRotator::ZeroRotator);
		if (UPrimitiveComponent* Body = Cast<UPrimitiveComponent>(Character->GetDefaultSubobjectByName(TEXT("Body"))))
		{
			TestEqual(TEXT("Battle character body does not use Unreal collision"), Body->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
			TestEqual(TEXT("Battle character body is offset above the foot pivot"), static_cast<float>(Body->GetRelativeLocation().Z), 45.0f);
			TestEqual(TEXT("Battle character body uses unit relative scale"), Body->GetRelativeScale3D(), FVector::OneVector);
		}
		USceneComponent* BodyFrame = Cast<USceneComponent>(Character->GetDefaultSubobjectByName(TEXT("BodyFrame")));
		TestNotNull(TEXT("Battle character body frame exists"), BodyFrame);
		if (BodyFrame)
		{
			TestEqual(TEXT("Battle character body frame is aligned to the character foot offset"), static_cast<float>(BodyFrame->GetRelativeLocation().Z), -45.0f);
		}
		UBoxComponent* Hurtbox = Cast<UBoxComponent>(Character->GetDefaultSubobjectByName(TEXT("Hurtbox")));
		TestNotNull(TEXT("Battle character hurtbox exists"), Hurtbox);
		if (Hurtbox)
		{
			TestTrue(TEXT("Battle character hurtbox is attached to the slope frame"), Hurtbox->GetAttachParent() == BodyFrame);
			TestEqual(TEXT("Battle character hurtbox only participates in queries"), Hurtbox->GetCollisionEnabled(), ECollisionEnabled::QueryOnly);
			TestEqual(TEXT("Battle character hurtbox is centered above the foot pivot"), static_cast<float>(Hurtbox->GetRelativeLocation().Z), 45.0f);
			TestEqual(TEXT("Battle character hurtbox has the default inset extent"), Hurtbox->GetUnscaledBoxExtent(), FVector(28.0f, 16.0f, 42.0f));
			TestEqual(TEXT("Battle character hurtbox uses unit relative scale"), Hurtbox->GetRelativeScale3D(), FVector::OneVector);
		}
		UPaperFlipbookComponent* BodySprite = Cast<UPaperFlipbookComponent>(Character->GetDefaultSubobjectByName(TEXT("BodySprite")));
		TestNotNull(TEXT("Battle character sprite component exists"), BodySprite);
		if (BodySprite)
		{
			TestEqual(TEXT("Battle character sprite does not use Unreal collision"), BodySprite->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
			TestEqual(TEXT("Battle character sprite stays local under the body frame"), BodySprite->GetRelativeRotation(), FRotator::ZeroRotator);
			TestEqual(TEXT("Battle character sprite is attached at the foot pivot"), static_cast<float>(BodySprite->GetRelativeLocation().Z), 0.0f);
			TestEqual(TEXT("Battle character sprite uses unit relative scale"), BodySprite->GetRelativeScale3D(), FVector::OneVector);
		}
		USceneComponent* Muzzle = Cast<USceneComponent>(Character->GetDefaultSubobjectByName(TEXT("Muzzle")));
		TestNotNull(TEXT("Battle character muzzle component exists"), Muzzle);
		if (Muzzle && BodyFrame)
		{
			TestTrue(TEXT("Battle character muzzle is attached to the body frame"), Muzzle->GetAttachParent() == BodyFrame);
			TestEqual(TEXT("Battle character muzzle uses unit relative scale"), Muzzle->GetRelativeScale3D(), FVector::OneVector);
			const FVector ExpectedMuzzleLocalLocation(FMath::Cos(FMath::DegreesToRadians(45.0f)) * 70.0f, 0.0f, 80.0f + FMath::Sin(FMath::DegreesToRadians(45.0f)) * 70.0f);
			TestTrue(TEXT("Battle character muzzle starts at the default aim tip"), Muzzle->GetRelativeLocation().Equals(ExpectedMuzzleLocalLocation, 0.1f));
			const FVector MuzzleLocalBeforeScale = Muzzle->GetRelativeLocation();
			BodyFrame->SetRelativeScale3D(FVector(1.2f));
			TestTrue(TEXT("Battle character muzzle world offset follows BodyFrame scale"), Muzzle->GetComponentLocation().Equals(BodyFrame->GetComponentTransform().TransformPosition(MuzzleLocalBeforeScale), 0.1f));
			BodyFrame->SetRelativeScale3D(FVector::OneVector);
		}
		UWidgetComponent* AngleIndicatorWidget = Cast<UWidgetComponent>(Character->GetDefaultSubobjectByName(TEXT("AngleIndicatorWidget")));
		TestNotNull(TEXT("Battle character angle indicator widget exists"), AngleIndicatorWidget);
		if (AngleIndicatorWidget && BodyFrame)
		{
			TestTrue(TEXT("Battle character angle indicator is attached to the body frame"), AngleIndicatorWidget->GetAttachParent() == BodyFrame);
			TestEqual(TEXT("Battle character angle indicator uses world widget space"), AngleIndicatorWidget->GetWidgetSpace(), EWidgetSpace::World);
			TestEqual(TEXT("Battle character angle indicator uses a square draw size"), AngleIndicatorWidget->GetDrawSize(), FVector2D(128.0f, 128.0f));
			TestEqual(TEXT("Battle character angle indicator pivots at its center"), AngleIndicatorWidget->GetPivot(), FVector2D(0.5f, 0.5f));
			TestTrue(TEXT("Battle character angle indicator is offset toward the battle camera"), AngleIndicatorWidget->GetRelativeLocation().Y > 0.0f);
			TestTrue(TEXT("Battle character angle indicator is two-sided for left/right facing"), AngleIndicatorWidget->GetTwoSided());
		}
		UWidgetComponent* HpWidget = Cast<UWidgetComponent>(Character->GetDefaultSubobjectByName(TEXT("HpWidget")));
		TestNotNull(TEXT("Battle character HP widget exists"), HpWidget);
		if (HpWidget)
		{
			TestTrue(TEXT("Battle character HP widget is attached outside the body frame"), HpWidget->GetAttachParent() == Character->GetRootComponent());
			TestEqual(TEXT("Battle character HP widget uses screen widget space"), HpWidget->GetWidgetSpace(), EWidgetSpace::Screen);
			TestTrue(TEXT("Battle character HP widget is below the sprite foot pivot"), HpWidget->GetRelativeLocation().Z < -45.0f);
		}
		Character->SetTerrain(Terrain);
		Character->BeginTurn();
		Character->MoveHorizontal(1.0f, 0.05f);
		TestTrue(TEXT("Battle character advances until a steep terrain wall blocks it"), Character->GetActorLocation().X > -5.0f);
		TestTrue(TEXT("Battle character does not tunnel into the steep terrain wall"), Character->GetActorLocation().X < 0.0f);
	}

	auto CountProjectiles = [&World]()
	{
		int32 Count = 0;
		for (TActorIterator<AFRProjectile> It(World); It; ++It)
		{
			++Count;
		}
		return Count;
	};


	FFRProjectileEffectDrillParams RuntimeSplitChildDrillParams;
	RuntimeSplitChildDrillParams.RadiusBonus = 28.0f;
	FFRProjectileEffectSpec RuntimeSplitChildDrillEffect;
	RuntimeSplitChildDrillEffect.EffectClass = UFRProjectileEffectDrill::StaticClass();
	RuntimeSplitChildDrillEffect.Parameters = FInstancedStruct::Make(RuntimeSplitChildDrillParams);
	FFRProjectileEffectTerrainCreateParams RuntimeSplitChildTerrainParams;
	RuntimeSplitChildTerrainParams.RadiusBonus = 12.0f;
	FFRProjectileEffectSpec RuntimeSplitChildTerrainEffect;
	RuntimeSplitChildTerrainEffect.EffectClass = UFRProjectileEffectTerrainCreate::StaticClass();
	RuntimeSplitChildTerrainEffect.Parameters = FInstancedStruct::Make(RuntimeSplitChildTerrainParams);
	FFRShotModifierSpec RuntimeSplitChildModifier;
	RuntimeSplitChildModifier.ProjectileEffects.Add(RuntimeSplitChildDrillEffect);
	RuntimeSplitChildModifier.ProjectileEffects.Add(RuntimeSplitChildTerrainEffect);
	FFRProjectileEffectSplitParams RuntimeBlockedNestedSplitParams;
	RuntimeBlockedNestedSplitParams.ProjectileCount = 1;
	RuntimeBlockedNestedSplitParams.LaunchSpeed = 180.0f;
	FFRProjectileEffectSpec RuntimeBlockedNestedSplitEffect;
	RuntimeBlockedNestedSplitEffect.EffectClass = UFRProjectileEffectSplit::StaticClass();
	RuntimeBlockedNestedSplitEffect.Parameters = FInstancedStruct::Make(RuntimeBlockedNestedSplitParams);
	FFRShotModifierSpec RuntimeBlockedSplitChildModifier;
	RuntimeBlockedSplitChildModifier.BlockedShotTags.AddTag(FRGameplayTags::ShotEffect_Drill);
	RuntimeBlockedSplitChildModifier.ProjectileEffects.Add(RuntimeBlockedNestedSplitEffect);
	FFRProjectileEffectSplitParams RuntimeSplitParams;
	RuntimeSplitParams.ProjectileCount = 1;
	RuntimeSplitParams.SpreadDegrees = 0.0f;
	RuntimeSplitParams.LaunchSpeed = 220.0f;
	RuntimeSplitParams.ChildShotModifiers.Add(RuntimeSplitChildModifier);
	RuntimeSplitParams.ChildShotModifiers.Add(RuntimeBlockedSplitChildModifier);
	FFRProjectileEffectSpec RuntimeSplitEffect;
	RuntimeSplitEffect.EffectClass = UFRProjectileEffectSplit::StaticClass();
	RuntimeSplitEffect.Parameters = FInstancedStruct::Make(RuntimeSplitParams);
	FFRProjectileEffectImpactContext RuntimeSplitContext;
	RuntimeSplitContext.World = World;
	RuntimeSplitContext.OwnerCharacter = Character;
	RuntimeSplitContext.AssignedTerrain = Terrain;
	RuntimeSplitContext.ImpactLocation = FVector(75.0f, 0.0f, 30.0f);
	RuntimeSplitContext.Velocity = FVector(0.0f, 0.0f, -1.0f);
	const int32 ProjectileCountBeforeSplit = CountProjectiles();
	RuntimeSplitEffect.HandleImpact(RuntimeSplitContext);
	TestEqual(TEXT("Runtime split effect spawns one child projectile"), CountProjectiles(), ProjectileCountBeforeSplit + 1);
	AFRProjectile* RuntimeSplitChildProjectile = nullptr;
	for (TActorIterator<AFRProjectile> It(World); It; ++It)
	{
		if (It->GetActorLocation().Equals(FVector(75.0f, 0.0f, 12.0f), 0.1))
		{
			RuntimeSplitChildProjectile = *It;
			break;
		}
	}
	TestNotNull(TEXT("Runtime split child projectile can be found at its spawn location"), RuntimeSplitChildProjectile);
	if (RuntimeSplitChildProjectile)
	{
		TestEqual(TEXT("Runtime split child projectile keeps child modifier effects"), RuntimeSplitChildProjectile->GetProjectileEffectCount(), 2);
		TestTrue(TEXT("Runtime split child projectile keeps drill effect class"), RuntimeSplitChildProjectile->HasProjectileEffectClass(UFRProjectileEffectDrill::StaticClass()));
		TestTrue(TEXT("Runtime split child projectile keeps terrain create effect class"), RuntimeSplitChildProjectile->HasProjectileEffectClass(UFRProjectileEffectTerrainCreate::StaticClass()));
		TestFalse(TEXT("Runtime split child projectile skips blocked child modifiers"), RuntimeSplitChildProjectile->HasProjectileEffectClass(UFRProjectileEffectSplit::StaticClass()));
		TestFalse(TEXT("Runtime split child terrain create target starts empty"), Terrain->IsSolidAtWorldLocation(FVector(75.0f, 0.0f, 15.0f)));
		TestTrue(TEXT("Runtime split child drill target starts solid"), Terrain->IsSolidAtWorldLocation(FVector(95.0f, 0.0f, 5.0f)));
		RuntimeSplitChildProjectile->Tick(0.1f);
		TestTrue(TEXT("Runtime split child terrain create effect fills terrain"), Terrain->IsSolidAtWorldLocation(FVector(75.0f, 0.0f, 15.0f)));
		TestFalse(TEXT("Runtime split child drill effect carves terrain outside fill radius"), Terrain->IsSolidAtWorldLocation(FVector(95.0f, 0.0f, 5.0f)));
	}

	auto SetFloatProperty = [](UObject* Object, const FName PropertyName, float Value)
	{
		if (FFloatProperty* Property = FindFProperty<FFloatProperty>(Object->GetClass(), PropertyName))
		{
			Property->SetPropertyValue_InContainer(Object, Value);
		}
	};

	auto GetFloatProperty = [](UObject* Object, const FName PropertyName, float FallbackValue)
	{
		if (FFloatProperty* Property = FindFProperty<FFloatProperty>(Object->GetClass(), PropertyName))
		{
			return Property->GetPropertyValue_InContainer(Object);
		}

		return FallbackValue;
	};

	auto GetCombatSet = [](AFRBattleCharacter* BattleCharacter)
	{
		if (!BattleCharacter)
		{
			return static_cast<UFRCombatSet*>(nullptr);
		}

		if (FObjectProperty* Property = FindFProperty<FObjectProperty>(BattleCharacter->GetClass(), TEXT("CombatSet")))
		{
			return Cast<UFRCombatSet>(Property->GetObjectPropertyValue_InContainer(BattleCharacter));
		}

		return static_cast<UFRCombatSet*>(nullptr);
	};

	AFRBattleCharacter* StatCharacter = World->SpawnActor<AFRBattleCharacter>(AFRBattleCharacter::StaticClass(), FVector(-15.0f, 0.0f, 55.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Stat battle character is spawned"), StatCharacter);
	if (StatCharacter)
	{
		UFRCharacterDefinition* CharacterDefinition = NewObject<UFRCharacterDefinition>();
		CharacterDefinition->MaxHealth = 175.0f;
		CharacterDefinition->BonusDamage = 12.0f;
		CharacterDefinition->MaxMoveBudget = 25.0f;
		CharacterDefinition->ShotPowerMultiplier = 1.8f;
		CharacterDefinition->MinAimAngle = 10.0f;
		CharacterDefinition->MaxAimAngle = 80.0f;
		StatCharacter->InitializeFromDefinition(CharacterDefinition);
		if (UFRCombatSet* CombatSet = GetCombatSet(StatCharacter))
		{
			TestEqual(TEXT("Character definition controls max health"), CombatSet->GetMaxHealth(), 175.0f);
			TestEqual(TEXT("Character definition controls turn movement budget"), CombatSet->GetMaxMoveBudget(), 25.0f);
			TestEqual(TEXT("Character definition controls shot power multiplier"), CombatSet->GetShotPowerMultiplier(), 1.8f);
			TestEqual(TEXT("Character definition controls min aim angle"), CombatSet->GetMinAimAngle(), 10.0f);
			TestEqual(TEXT("Character definition controls max aim angle"), CombatSet->GetMaxAimAngle(), 80.0f);
		}
		TestEqual(TEXT("Battle character getter exposes max move budget"), StatCharacter->GetMaxMoveBudget(), 25.0f);
		TestEqual(TEXT("Battle character getter exposes damage bonus"), StatCharacter->GetDamageBonus(), 12.0f);
		TestEqual(TEXT("Battle character getter exposes shot power multiplier"), StatCharacter->GetShotPowerMultiplier(), 1.8f);
		TestEqual(TEXT("Battle character getter exposes base projectile count"), StatCharacter->GetProjectileCount(), 1.0f);
		TestEqual(TEXT("Battle character getter exposes min aim angle"), StatCharacter->GetMinAimAngle(), 10.0f);
		TestEqual(TEXT("Battle character getter exposes max aim angle"), StatCharacter->GetMaxAimAngle(), 80.0f);
		float TaggedAttributeValue = -1.0f;
		TestTrue(TEXT("Battle character reads health by attribute tag"), StatCharacter->TryGetCombatAttributeValueByTag(FRGameplayTags::Attribute_Health, TaggedAttributeValue));
		TestEqual(TEXT("Tagged health value matches getter"), TaggedAttributeValue, 175.0f);
		TestTrue(TEXT("Battle character reads max health by attribute tag"), StatCharacter->TryGetCombatAttributeValueByTag(FRGameplayTags::Attribute_MaxHealth, TaggedAttributeValue));
		TestEqual(TEXT("Tagged max health value matches getter"), TaggedAttributeValue, 175.0f);
		TestTrue(TEXT("Battle character reads move budget by attribute tag"), StatCharacter->TryGetCombatAttributeValueByTag(FRGameplayTags::Attribute_MoveBudget, TaggedAttributeValue));
		TestEqual(TEXT("Tagged move budget value matches getter"), TaggedAttributeValue, 25.0f);
		TestTrue(TEXT("Battle character reads max move budget by attribute tag"), StatCharacter->TryGetCombatAttributeValueByTag(FRGameplayTags::Attribute_MaxMoveBudget, TaggedAttributeValue));
		TestEqual(TEXT("Tagged max move budget value matches getter"), TaggedAttributeValue, 25.0f);
		TestTrue(TEXT("Battle character reads damage by attribute tag"), StatCharacter->TryGetCombatAttributeValueByTag(FRGameplayTags::Attribute_Damage, TaggedAttributeValue));
		TestEqual(TEXT("Tagged damage value matches getter"), TaggedAttributeValue, 12.0f);
		TestTrue(TEXT("Battle character reads shot power multiplier by attribute tag"), StatCharacter->TryGetCombatAttributeValueByTag(FRGameplayTags::Attribute_ShotPowerMultiplier, TaggedAttributeValue));
		TestEqual(TEXT("Tagged shot power multiplier value matches getter"), TaggedAttributeValue, 1.8f);
		TestTrue(TEXT("Battle character reads projectile count by attribute tag"), StatCharacter->TryGetCombatAttributeValueByTag(FRGameplayTags::Attribute_ProjectileCount, TaggedAttributeValue));
		TestEqual(TEXT("Tagged projectile count value matches getter"), TaggedAttributeValue, 1.0f);
		TestTrue(TEXT("Battle character reads min aim angle by attribute tag"), StatCharacter->TryGetCombatAttributeValueByTag(FRGameplayTags::Attribute_MinAimAngle, TaggedAttributeValue));
		TestEqual(TEXT("Tagged min aim angle value matches getter"), TaggedAttributeValue, 10.0f);
		TestTrue(TEXT("Battle character reads max aim angle by attribute tag"), StatCharacter->TryGetCombatAttributeValueByTag(FRGameplayTags::Attribute_MaxAimAngle, TaggedAttributeValue));
		TestEqual(TEXT("Tagged max aim angle value matches getter"), TaggedAttributeValue, 80.0f);
		TestFalse(TEXT("Battle character rejects invalid attribute tags"), StatCharacter->TryGetCombatAttributeValueByTag(FGameplayTag(), TaggedAttributeValue));
		TestEqual(TEXT("Rejected attribute tag resets the out value"), TaggedAttributeValue, 0.0f);
		StatCharacter->ApplyRewardProjectiles(2);
		TestEqual(TEXT("Battle character getter exposes projectile count rewards"), StatCharacter->GetProjectileCount(), 3.0f);
		const FString CombatStatsSummary = StatCharacter->GetCombatStatsSummary().ToString();
		TestTrue(TEXT("Battle character stats summary includes health"), CombatStatsSummary.Contains(TEXT("HP 175/175")));
		TestTrue(TEXT("Battle character stats summary includes move budget"), CombatStatsSummary.Contains(TEXT("move 25/25")));
		TestTrue(TEXT("Battle character stats summary includes damage bonus"), CombatStatsSummary.Contains(TEXT("damage +12")));
		TestTrue(TEXT("Battle character stats summary includes shot power multiplier"), CombatStatsSummary.Contains(TEXT("shot power x1.8")));
		TestTrue(TEXT("Battle character stats summary includes projectile count"), CombatStatsSummary.Contains(TEXT("projectiles 3")));
		TestTrue(TEXT("Battle character stats summary includes aim angle range"), CombatStatsSummary.Contains(TEXT("aim 10-80")));
		TestTrue(TEXT("Battle character applies health deltas by attribute tag"), StatCharacter->TryApplyCombatAttributeDeltaByTag(FRGameplayTags::Attribute_Health, -25.0f));
		TestEqual(TEXT("Tagged health delta damages current health"), StatCharacter->GetHealth(), 150.0f);
		TestTrue(TEXT("Battle character applies healing deltas by attribute tag"), StatCharacter->TryApplyCombatAttributeDeltaByTag(FRGameplayTags::Attribute_Health, 1000.0f));
		TestEqual(TEXT("Tagged healing delta clamps to max health"), StatCharacter->GetHealth(), 175.0f);
		TestTrue(TEXT("Battle character applies max health deltas by attribute tag"), StatCharacter->TryApplyCombatAttributeDeltaByTag(FRGameplayTags::Attribute_MaxHealth, -1000.0f));
		TestEqual(TEXT("Tagged max health delta clamps to its minimum"), StatCharacter->GetMaxHealth(), 1.0f);
		TestTrue(TEXT("Battle character restores max health by attribute tag"), StatCharacter->TryApplyCombatAttributeDeltaByTag(FRGameplayTags::Attribute_MaxHealth, 174.0f));
		TestEqual(TEXT("Tagged max health restore updates max health"), StatCharacter->GetMaxHealth(), 175.0f);
		TestTrue(TEXT("Battle character applies move budget deltas by attribute tag"), StatCharacter->TryApplyCombatAttributeDeltaByTag(FRGameplayTags::Attribute_MoveBudget, -500.0f));
		TestEqual(TEXT("Tagged move budget delta clamps to zero"), StatCharacter->GetMoveBudget(), 0.0f);
		TestTrue(TEXT("Battle character restores move budget by attribute tag"), StatCharacter->TryApplyCombatAttributeDeltaByTag(FRGameplayTags::Attribute_MoveBudget, 500.0f));
		TestEqual(TEXT("Tagged move budget restore clamps to max move"), StatCharacter->GetMoveBudget(), 25.0f);
		TestTrue(TEXT("Battle character applies max move deltas by attribute tag"), StatCharacter->TryApplyCombatAttributeDeltaByTag(FRGameplayTags::Attribute_MaxMoveBudget, -500.0f));
		TestEqual(TEXT("Tagged max move delta clamps to zero"), StatCharacter->GetMaxMoveBudget(), 0.0f);
		TestTrue(TEXT("Battle character restores max move by attribute tag"), StatCharacter->TryApplyCombatAttributeDeltaByTag(FRGameplayTags::Attribute_MaxMoveBudget, 25.0f));
		TestEqual(TEXT("Tagged max move restore updates move budget"), StatCharacter->GetMoveBudget(), 25.0f);
		TestTrue(TEXT("Battle character applies damage deltas by attribute tag"), StatCharacter->TryApplyCombatAttributeDeltaByTag(FRGameplayTags::Attribute_Damage, -1000.0f));
		TestEqual(TEXT("Tagged damage delta clamps to zero"), StatCharacter->GetDamageBonus(), 0.0f);
		TestTrue(TEXT("Battle character restores damage by attribute tag"), StatCharacter->TryApplyCombatAttributeDeltaByTag(FRGameplayTags::Attribute_Damage, 12.0f));
		TestEqual(TEXT("Tagged damage restore updates damage"), StatCharacter->GetDamageBonus(), 12.0f);
		TestTrue(TEXT("Battle character applies shot power deltas by attribute tag"), StatCharacter->TryApplyCombatAttributeDeltaByTag(FRGameplayTags::Attribute_ShotPowerMultiplier, -1000.0f));
		TestEqual(TEXT("Tagged shot power delta clamps to zero"), StatCharacter->GetShotPowerMultiplier(), 0.0f);
		TestTrue(TEXT("Battle character restores shot power by attribute tag"), StatCharacter->TryApplyCombatAttributeDeltaByTag(FRGameplayTags::Attribute_ShotPowerMultiplier, 1.8f));
		TestEqual(TEXT("Tagged shot power restore updates multiplier"), StatCharacter->GetShotPowerMultiplier(), 1.8f);
		TestTrue(TEXT("Battle character applies projectile deltas by attribute tag"), StatCharacter->TryApplyCombatAttributeDeltaByTag(FRGameplayTags::Attribute_ProjectileCount, -1000.0f));
		TestEqual(TEXT("Tagged projectile delta clamps to one"), StatCharacter->GetProjectileCount(), 1.0f);
		TestTrue(TEXT("Battle character restores projectile count by attribute tag"), StatCharacter->TryApplyCombatAttributeDeltaByTag(FRGameplayTags::Attribute_ProjectileCount, 2.0f));
		TestEqual(TEXT("Tagged projectile restore updates count"), StatCharacter->GetProjectileCount(), 3.0f);
		TestTrue(TEXT("Battle character applies min aim angle deltas by attribute tag"), StatCharacter->TryApplyCombatAttributeDeltaByTag(FRGameplayTags::Attribute_MinAimAngle, -1000.0f));
		TestEqual(TEXT("Tagged min aim angle delta clamps to negative ninety"), StatCharacter->GetMinAimAngle(), -90.0f);
		TestTrue(TEXT("Battle character restores min aim angle by attribute tag"), StatCharacter->TryApplyCombatAttributeDeltaByTag(FRGameplayTags::Attribute_MinAimAngle, 100.0f));
		TestEqual(TEXT("Tagged min aim angle restore updates min aim"), StatCharacter->GetMinAimAngle(), 10.0f);
		TestTrue(TEXT("Battle character applies max aim angle deltas by attribute tag"), StatCharacter->TryApplyCombatAttributeDeltaByTag(FRGameplayTags::Attribute_MaxAimAngle, 1000.0f));
		TestEqual(TEXT("Tagged max aim angle delta clamps to one eighty"), StatCharacter->GetMaxAimAngle(), 180.0f);
		TestTrue(TEXT("Battle character restores max aim angle by attribute tag"), StatCharacter->TryApplyCombatAttributeDeltaByTag(FRGameplayTags::Attribute_MaxAimAngle, -100.0f));
		TestEqual(TEXT("Tagged max aim angle restore updates max aim"), StatCharacter->GetMaxAimAngle(), 80.0f);
		TestFalse(TEXT("Battle character rejects invalid attribute delta tags"), StatCharacter->TryApplyCombatAttributeDeltaByTag(FGameplayTag(), 1.0f));
		StatCharacter->ApplyRewardDamage(8.0f);
		StatCharacter->ApplyRewardDamage(-5.0f);
		TestEqual(TEXT("Battle character applies negative damage reward deltas"), StatCharacter->GetDamageBonus(), 15.0f);
		StatCharacter->ApplyRewardHealth(25.0f);
		StatCharacter->ApplyDamage(10.0f);
		StatCharacter->ApplyRewardHealth(-50.0f);
		TestEqual(TEXT("Battle character applies negative max health reward deltas"), StatCharacter->GetMaxHealth(), 150.0f);
		TestEqual(TEXT("Battle character clamps health after max health loss"), StatCharacter->GetHealth(), 150.0f);
		StatCharacter->ApplyRewardMoveBudget(10.0f);
		StatCharacter->ApplyRewardMoveBudget(-15.0f);
		TestEqual(TEXT("Battle character applies negative move budget reward deltas"), StatCharacter->GetMaxMoveBudget(), 20.0f);
		TestEqual(TEXT("Battle character clamps move budget after max move loss"), StatCharacter->GetMoveBudget(), 20.0f);
		StatCharacter->ApplyRewardShotPowerMultiplier(0.4f);
		StatCharacter->ApplyRewardShotPowerMultiplier(-0.7f);
		TestTrue(TEXT("Battle character applies negative shot power reward deltas"), FMath::IsNearlyEqual(StatCharacter->GetShotPowerMultiplier(), 1.5f));
		StatCharacter->ApplyRewardProjectiles(-1);
		TestEqual(TEXT("Battle character applies negative projectile reward deltas"), StatCharacter->GetProjectileCount(), 2.0f);
		UFRAbilitySet* EmptyAbilitySet = NewObject<UFRAbilitySet>(StatCharacter);
		EmptyAbilitySet->DisplayName = FText::FromString(TEXT("Empty Ability Set"));
		TestEqual(TEXT("Battle character ability set count starts empty"), StatCharacter->GetGrantedAbilitySetCount(EmptyAbilitySet), 0);
		StatCharacter->GrantAbilitySet(EmptyAbilitySet);
		StatCharacter->GrantAbilitySet(EmptyAbilitySet);
		TestEqual(TEXT("Battle character ability set count tracks repeated grants"), StatCharacter->GetGrantedAbilitySetCount(EmptyAbilitySet), 2);
		TestEqual(TEXT("Battle character exposes granted ability set list"), StatCharacter->GetGrantedAbilitySetsForBlueprint().Num(), 2);
		TestTrue(TEXT("Battle character summarizes granted ability sets"), StatCharacter->GetGrantedAbilitySetsSummary().ToString().Contains(TEXT("Empty Ability Set")));
		TestTrue(TEXT("Battle character removes one granted ability set entry"), StatCharacter->RemoveAbilitySet(EmptyAbilitySet));
		TestEqual(TEXT("Battle character ability set count updates after removal"), StatCharacter->GetGrantedAbilitySetCount(EmptyAbilitySet), 1);
		UFRAbilitySet* TaggedAbilitySet = NewObject<UFRAbilitySet>(StatCharacter);
		TaggedAbilitySet->AbilitySetTag = FRGameplayTags::Trait_ShotModifier;
		StatCharacter->GrantAbilitySet(TaggedAbilitySet);
		StatCharacter->GrantAbilitySet(TaggedAbilitySet);
		TestEqual(TEXT("Battle character ability set tag count tracks repeated grants"), StatCharacter->GetGrantedAbilitySetCountByTag(FRGameplayTags::Trait_ShotModifier), 2);
		TestTrue(TEXT("Battle character reports granted ability sets by tag"), StatCharacter->HasGrantedAbilitySetByTag(FRGameplayTags::Trait_ShotModifier));
		TestEqual(TEXT("Battle character removes granted ability sets by tag"), StatCharacter->RemoveAbilitySetsByTag(FRGameplayTags::Trait_ShotModifier), 2);
		TestEqual(TEXT("Battle character ability set tag count updates after removal"), StatCharacter->GetGrantedAbilitySetCountByTag(FRGameplayTags::Trait_ShotModifier), 0);
		TestFalse(TEXT("Battle character reports missing ability sets by tag"), StatCharacter->HasGrantedAbilitySetByTag(FRGameplayTags::Trait_ShotModifier));
		UFRWeaponDefinition* ShellWeapon = CreateTestWeaponDefinition(StatCharacter);
		ShellWeapon->Weapon.WeaponTag = FRGameplayTags::Weapon_Shell;
		UFRWeaponDefinition* ClusterWeapon = CreateTestWeaponDefinition(StatCharacter);
		ClusterWeapon->Weapon.WeaponTag = FRGameplayTags::Weapon_Cluster;
		StatCharacter->AddWeaponDefinition(ShellWeapon);
		StatCharacter->AddWeaponDefinition(ClusterWeapon);
		TestEqual(TEXT("Battle character finds weapon index by tag"), StatCharacter->GetWeaponIndexByTag(FRGameplayTags::Weapon_Cluster), 1);
		TestEqual(TEXT("Battle character returns INDEX_NONE for invalid weapon tags"), StatCharacter->GetWeaponIndexByTag(FGameplayTag()), INDEX_NONE);
		TestTrue(TEXT("Battle character reports valid weapon index selectable"), StatCharacter->CanSelectWeapon(1));
		TestFalse(TEXT("Battle character rejects invalid weapon index selection queries"), StatCharacter->CanSelectWeapon(-1));
		TestTrue(TEXT("Battle character reports valid weapon tag selectable"), StatCharacter->CanSelectWeaponByTag(FRGameplayTags::Weapon_Cluster));
		TestFalse(TEXT("Battle character rejects invalid weapon tag selection queries"), StatCharacter->CanSelectWeaponByTag(FGameplayTag()));
		TestFalse(TEXT("Battle character rejects invalid weapon selection tags"), StatCharacter->SelectWeaponByTag(FGameplayTag()));
		TestTrue(TEXT("Battle character selects a weapon by tag"), StatCharacter->SelectWeaponByTag(FRGameplayTags::Weapon_Cluster));
		TestTrue(TEXT("Battle character selection by tag updates the current weapon"), StatCharacter->GetCurrentWeapon().WeaponTag.MatchesTagExact(FRGameplayTags::Weapon_Cluster));
		UFRWeaponDefinition* DrillWeapon = CreateTestWeaponDefinition(StatCharacter);
		DrillWeapon->Weapon.WeaponTag = FRGameplayTags::Weapon_Shell;
		FFRProjectileEffectSpec WeaponDrillEffect;
		WeaponDrillEffect.EffectClass = UFRProjectileEffectDrill::StaticClass();
		FFRProjectileEffectDrillParams WeaponDrillParams;
		WeaponDrillParams.RadiusBonus = 25.0f;
		WeaponDrillEffect.Parameters = FInstancedStruct::Make(WeaponDrillParams);
		DrillWeapon->Weapon.ProjectileEffects.Add(WeaponDrillEffect);
		StatCharacter->AddWeaponDefinition(DrillWeapon);
		StatCharacter->SelectWeapon(StatCharacter->GetWeaponLoadout().Num() - 1);
		const FFRShotSpec DrillWeaponShotSpec = StatCharacter->GetCurrentShotSpec();
		TestTrue(TEXT("Weapon projectile effects add shot tags"), DrillWeaponShotSpec.EffectTags.HasTagExact(FRGameplayTags::ShotEffect_Drill));
		TestEqual(TEXT("Weapon projectile effects are carried into shot spec"), DrillWeaponShotSpec.ProjectileEffects.Num(), 1);
		TestEqual(TEXT("Weapon projectile effects can adjust terrain damage radius"), DrillWeaponShotSpec.TerrainDamage, 170.0f);
		UFRWeaponDefinition* UnsafeWeapon = CreateTestWeaponDefinition(StatCharacter);
		UnsafeWeapon->Weapon.HitDamage = -10.0f;
		UnsafeWeapon->Weapon.Damage = -100.0f;
		UnsafeWeapon->Weapon.BlastRadius = -20.0f;
		UnsafeWeapon->Weapon.ExplosionFullDamageRadius = -5.0f;
		UnsafeWeapon->Weapon.TerrainDamage = -30.0f;
		UnsafeWeapon->Weapon.ProjectileSpeed = -500.0f;
		UnsafeWeapon->Weapon.Gravity = -980.0f;
		StatCharacter->AddWeaponDefinition(UnsafeWeapon);
		StatCharacter->SelectWeapon(StatCharacter->GetWeaponLoadout().Num() - 1);
		const FFRShotSpec UnsafeShotSpec = StatCharacter->GetCurrentShotSpec();
		TestEqual(TEXT("Shot spec clamps negative hit damage"), UnsafeShotSpec.HitDamage, 0.0f);
		TestEqual(TEXT("Shot spec clamps negative weapon explosion damage"), UnsafeShotSpec.Damage, 0.0f);
		TestEqual(TEXT("Shot spec clamps negative blast radius"), UnsafeShotSpec.BlastRadius, 0.0f);
		TestEqual(TEXT("Shot spec clamps negative explosion full damage radius"), UnsafeShotSpec.ExplosionFullDamageRadius, 0.0f);
		TestEqual(TEXT("Shot spec clamps negative terrain damage"), UnsafeShotSpec.TerrainDamage, 0.0f);
		TestEqual(TEXT("Shot spec clamps negative launch speed"), UnsafeShotSpec.LaunchSpeed, 0.0f);
		TestEqual(TEXT("Shot spec clamps negative gravity"), UnsafeShotSpec.Gravity, 0.0f);
		const FString CurrentShotSummary = StatCharacter->GetCurrentShotSummary().ToString();
		TestTrue(TEXT("Battle character shot summary includes hit damage"), CurrentShotSummary.Contains(TEXT("Hit 0")));
		TestTrue(TEXT("Battle character shot summary includes explosion damage"), CurrentShotSummary.Contains(TEXT("Blast Dmg 0")));
		TestTrue(TEXT("Battle character shot summary includes blast radius"), CurrentShotSummary.Contains(TEXT("Blast 0")));
		TestTrue(TEXT("Battle character shot summary includes projectile count"), CurrentShotSummary.Contains(TEXT("Projectiles 2")));
		UFRItemDefinition* PendingModifierItem = NewObject<UFRItemDefinition>(StatCharacter);
		PendingModifierItem->ItemType = EFRItemType::AbilitySet;
		FFRShotModifierSpec PendingModifier;
		PendingModifier.ModifierTag = FRGameplayTags::ShotEffect_Damage;
		PendingModifier.DamageBonus = 5.0f;
		TArray<FFRShotModifierSpec> GrantedModifierTest = { PendingModifier };
		StatCharacter->GrantShotModifiers(GrantedModifierTest);
		TestTrue(TEXT("Battle character reports granted shot modifiers by tag"), StatCharacter->HasGrantedShotModifierByTag(FRGameplayTags::ShotEffect_Damage));
		TestEqual(TEXT("Battle character exposes granted shot modifier list"), StatCharacter->GetGrantedShotModifiersForBlueprint().Num(), 1);
		TestTrue(TEXT("Battle character summarizes granted shot modifiers"), StatCharacter->GetGrantedShotModifiersSummary().ToString().Contains(TEXT("modifier tag")));
		TestEqual(TEXT("Battle character removes granted shot modifiers by tag"), StatCharacter->RemoveGrantedShotModifiersByTag(FRGameplayTags::ShotEffect_Damage), 1);
		TestFalse(TEXT("Battle character reports missing granted shot modifiers by tag"), StatCharacter->HasGrantedShotModifierByTag(FRGameplayTags::ShotEffect_Damage));
		PendingModifierItem->UseShotModifiers.Add(PendingModifier);
		StatCharacter->AddItemDefinition(PendingModifierItem, 1);
		const int32 PendingModifierItemIndex = StatCharacter->GetItemLoadout().Num() - 1;
		StatCharacter->BeginTurn();
		TestTrue(TEXT("Battle character uses item that grants pending shot modifier"), StatCharacter->UseItemByIndex(PendingModifierItemIndex));
		TestEqual(TEXT("Battle character counts pending shot modifiers by tag"), StatCharacter->GetPendingShotModifierCountByTag(FRGameplayTags::ShotEffect_Damage), 1);
		TestTrue(TEXT("Battle character reports pending shot modifiers by tag"), StatCharacter->HasPendingShotModifierByTag(FRGameplayTags::ShotEffect_Damage));
		TestEqual(TEXT("Battle character exposes pending shot modifier list"), StatCharacter->GetPendingShotModifiersForBlueprint().Num(), 1);
		TestTrue(TEXT("Battle character summarizes pending shot modifiers"), StatCharacter->GetPendingShotModifiersSummary().ToString().Contains(TEXT("modifier tag")));
		TestEqual(TEXT("Battle character removes pending shot modifiers by tag"), StatCharacter->RemovePendingShotModifiersByTag(FRGameplayTags::ShotEffect_Damage), 1);
		TestEqual(TEXT("Battle character pending shot modifier count updates after removal"), StatCharacter->GetPendingShotModifierCountByTag(FRGameplayTags::ShotEffect_Damage), 0);
		TestFalse(TEXT("Battle character reports missing pending shot modifiers by tag"), StatCharacter->HasPendingShotModifierByTag(FRGameplayTags::ShotEffect_Damage));
	}

	AFRBattleCharacter* ItemSlotCharacter = World->SpawnActor<AFRBattleCharacter>(AFRBattleCharacter::StaticClass(), FVector(-15.0f, 0.0f, 55.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Item slot battle character is spawned"), ItemSlotCharacter);
	if (ItemSlotCharacter)
	{
		UFRItemDefinition* HealItem = NewObject<UFRItemDefinition>(ItemSlotCharacter);
		HealItem->DisplayName = FText::FromString(TEXT("Test Repair"));
		HealItem->ItemType = EFRItemType::Heal;
		HealItem->ItemTag = FRGameplayTags::Item_Repair;
		HealItem->InitialCharges = 2;
		AddTestHealEffect(HealItem, 25.0f);
		ItemSlotCharacter->SetTerrain(Terrain);
		ItemSlotCharacter->AddItemDefinition(HealItem, 2);
		ItemSlotCharacter->BeginTurn();
		ItemSlotCharacter->ApplyDamage(40.0f);
		const float HealthBeforeItem = ItemSlotCharacter->GetHealth();
		TestTrue(TEXT("Battle character can report usable item by loadout index"), ItemSlotCharacter->CanUseItemByIndex(0));
		TestTrue(TEXT("Battle character can report usable item by type"), ItemSlotCharacter->CanUseItemByType(EFRItemType::Heal));
		TestTrue(TEXT("Battle character can report usable item by tag"), ItemSlotCharacter->CanUseItemByTag(FRGameplayTags::Item_Repair));
		TestEqual(TEXT("Battle character finds item index by tag"), ItemSlotCharacter->GetItemIndexByTag(FRGameplayTags::Item_Repair), 0);
		TestEqual(TEXT("Battle character returns INDEX_NONE for invalid item tags"), ItemSlotCharacter->GetItemIndexByTag(FGameplayTag()), INDEX_NONE);
		TestFalse(TEXT("Battle character rejects negative item loadout index"), ItemSlotCharacter->UseItemByIndex(-1));
		TestFalse(TEXT("Battle character cannot use negative item loadout index"), ItemSlotCharacter->CanUseItemByIndex(-1));
		TestFalse(TEXT("Battle character rejects item loadout index past the end"), ItemSlotCharacter->UseItemByIndex(1));
		TestEqual(TEXT("Rejected item slot use keeps item charges"), ItemSlotCharacter->GetItemCharges(EFRItemType::Heal), 2);
		TestTrue(TEXT("Battle character uses an item by loadout index"), ItemSlotCharacter->UseItemByIndex(0));
		TestEqual(TEXT("Item slot use consumes one charge"), ItemSlotCharacter->GetItemCharges(EFRItemType::Heal), 1);
		TestTrue(TEXT("Item slot use applies the item effect"), ItemSlotCharacter->GetHealth() > HealthBeforeItem);
		TestTrue(TEXT("Battle character still reports usable item with remaining charges"), ItemSlotCharacter->CanUseItemByIndex(0));
		TestTrue(TEXT("Battle character consumes final item charge"), ItemSlotCharacter->UseItemByIndex(0));
		TestFalse(TEXT("Battle character reports unusable item after charges are spent"), ItemSlotCharacter->CanUseItemByIndex(0));
	}

	AFRBattleCharacter* ExhaustedTurnCharacter = World->SpawnActor<AFRBattleCharacter>(AFRBattleCharacter::StaticClass(), FVector(45.0f, 0.0f, 55.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Exhausted turn battle character is spawned"), ExhaustedTurnCharacter);
	if (ExhaustedTurnCharacter)
	{
		ExhaustedTurnCharacter->AddWeaponDefinition(CreateTestWeaponDefinition(ExhaustedTurnCharacter));
		ExhaustedTurnCharacter->SetTerrain(Terrain);
		ExhaustedTurnCharacter->BeginTurn();
		if (UFRCombatSet* CombatSet = GetCombatSet(ExhaustedTurnCharacter))
		{
			CombatSet->SetMaxMoveBudget(0.0f);
			CombatSet->SetMoveBudget(0.0f);
		}
		const float ExhaustedCharacterX = ExhaustedTurnCharacter->GetActorLocation().X;
		ExhaustedTurnCharacter->MoveHorizontal(-1.0f, 0.1f);
		TestEqual(TEXT("Battle character actor turns left when facing changes without movement"), static_cast<float>(FMath::Abs(FRotator::NormalizeAxis(ExhaustedTurnCharacter->GetActorRotation().Yaw))), 180.0f);
		if (UPaperFlipbookComponent* ExhaustedTurnSprite = Cast<UPaperFlipbookComponent>(ExhaustedTurnCharacter->GetDefaultSubobjectByName(TEXT("BodySprite"))))
		{
			TestEqual(TEXT("Battle character sprite stays local while actor turns left"), ExhaustedTurnSprite->GetRelativeRotation(), FRotator::ZeroRotator);
		}
		TestEqual(TEXT("Exhausted movement budget does not move the character"), static_cast<float>(ExhaustedTurnCharacter->GetActorLocation().X), ExhaustedCharacterX);
		if (const UWidgetComponent* ExhaustedAngleIndicator = Cast<UWidgetComponent>(ExhaustedTurnCharacter->GetDefaultSubobjectByName(TEXT("AngleIndicatorWidget"))))
		{
			TestTrue(TEXT("Left-facing angle indicator remains offset toward the battle camera"), ExhaustedAngleIndicator->GetComponentLocation().Y > ExhaustedTurnCharacter->GetActorLocation().Y);
		}
		TestEqual(TEXT("Exhausted movement budget still leaves the budget at zero"), ExhaustedTurnCharacter->GetMoveBudget(), 0.0f);
		TestTrue(TEXT("Battle character reports selected weapon fireable before firing"), ExhaustedTurnCharacter->CanFireSelectedWeapon());
		TestTrue(TEXT("Battle character reports shot charge can begin before firing"), ExhaustedTurnCharacter->CanBeginShotCharge());
		TestEqual(TEXT("Shot power gauge starts empty at the beginning of a turn"), ExhaustedTurnCharacter->GetShotPower(), 0.0f);
		TestEqual(TEXT("Shot charge alpha starts empty at the beginning of a turn"), ExhaustedTurnCharacter->GetShotChargeAlpha(), 0.0f);
		TestEqual(TEXT("Empty shot power gauge still uses the minimum launch speed"), ExhaustedTurnCharacter->GetCurrentShotSpec().LaunchSpeed, 295.0f);
		ExhaustedTurnCharacter->BeginShotCharge();
		TestTrue(TEXT("Shot charge can start from an empty gauge"), ExhaustedTurnCharacter->IsChargingShot());
		TestEqual(TEXT("Shot charge begins at zero percent"), ExhaustedTurnCharacter->GetShotChargeAlpha(), 0.0f);
		ExhaustedTurnCharacter->UpdateShotCharge(10.0f);
		TestEqual(TEXT("Shot charge can fill to one hundred percent"), ExhaustedTurnCharacter->GetShotChargeAlpha(), 1.0f);
		TestEqual(TEXT("Facing change with exhausted movement budget fires to the requested side"), ExhaustedTurnCharacter->FireSelectedWeapon(), 1);
		TestFalse(TEXT("Battle character reports selected weapon not fireable after firing"), ExhaustedTurnCharacter->CanFireSelectedWeapon());
		AFRProjectile* ExhaustedTurnProjectile = nullptr;
		for (TActorIterator<AFRProjectile> It(World); It; ++It)
		{
			if (It->GetOwner() == ExhaustedTurnCharacter)
			{
				ExhaustedTurnProjectile = *It;
				break;
			}
		}
		TestNotNull(TEXT("Facing change with exhausted movement budget spawns a projectile"), ExhaustedTurnProjectile);
		if (ExhaustedTurnProjectile)
		{
			TestTrue(TEXT("Facing change with exhausted movement budget launches left"), ExhaustedTurnProjectile->GetActorLocation().X < ExhaustedCharacterX);
			const USceneComponent* ExhaustedMuzzle = Cast<USceneComponent>(ExhaustedTurnCharacter->GetDefaultSubobjectByName(TEXT("Muzzle")));
			TestNotNull(TEXT("Facing change projectile owner has a muzzle"), ExhaustedMuzzle);
			const FVector ExpectedProjectileSpawnLocation = ExhaustedMuzzle ? ExhaustedMuzzle->GetComponentLocation() : ExhaustedTurnCharacter->GetActorLocation();
			TestTrue(TEXT("Projectile and trajectory start from the muzzle"), ExhaustedTurnProjectile->GetActorLocation().Equals(ExpectedProjectileSpawnLocation, 0.1));
			ExhaustedTurnProjectile->Destroy();
		}
	}

	AFRBattleCharacter* AimFacingCharacter = World->SpawnActor<AFRBattleCharacter>(AFRBattleCharacter::StaticClass(), FVector(15.0f, 0.0f, 55.0f), FRotator::ZeroRotator, SpawnParams);
	AFRBattleCharacter* LeftAimTarget = World->SpawnActor<AFRBattleCharacter>(AFRBattleCharacter::StaticClass(), FVector(-45.0f, 0.0f, 55.0f), FRotator::ZeroRotator, SpawnParams);
	AFRBattleCharacter* RightAimTarget = World->SpawnActor<AFRBattleCharacter>(AFRBattleCharacter::StaticClass(), FVector(45.0f, 0.0f, 55.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Aim-facing battle character is spawned"), AimFacingCharacter);
	TestNotNull(TEXT("Left aim target is spawned"), LeftAimTarget);
	TestNotNull(TEXT("Right aim target is spawned"), RightAimTarget);
	if (AimFacingCharacter && LeftAimTarget && RightAimTarget)
	{
		UPaperFlipbookComponent* AimFacingSprite = Cast<UPaperFlipbookComponent>(AimFacingCharacter->GetDefaultSubobjectByName(TEXT("BodySprite")));
		TestNotNull(TEXT("Aim-facing sprite component exists"), AimFacingSprite);
		FFRStageDifficultyData AimFacingDifficulty;
		AimFacingCharacter->FireAtTarget(LeftAimTarget, AimFacingDifficulty);
		TestEqual(TEXT("Battle character actor turns left when aim target is left"), static_cast<float>(FMath::Abs(FRotator::NormalizeAxis(AimFacingCharacter->GetActorRotation().Yaw))), 180.0f);
		if (AimFacingSprite)
		{
			TestEqual(TEXT("Battle character sprite stays local when actor turns left"), AimFacingSprite->GetRelativeRotation(), FRotator::ZeroRotator);
		}
		AimFacingCharacter->FireAtTarget(RightAimTarget, AimFacingDifficulty);
		TestEqual(TEXT("Battle character actor turns right when aim target is right"), static_cast<float>(AimFacingCharacter->GetActorRotation().Yaw), 0.0f);
		if (AimFacingSprite)
		{
			TestEqual(TEXT("Battle character sprite stays local when actor turns right"), AimFacingSprite->GetRelativeRotation(), FRotator::ZeroRotator);
		}
	}

	AFRBattleCharacter* BoundaryCharacter = World->SpawnActor<AFRBattleCharacter>(AFRBattleCharacter::StaticClass(), FVector(500.0f, 0.0f, 200.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Boundary battle character is spawned"), BoundaryCharacter);
	if (BoundaryCharacter)
	{
		BoundaryCharacter->SetActorLocation(FVector(500.0f, 0.0f, 200.0f));
		BoundaryCharacter->SetTerrain(Terrain);
		const float FootProbeHalfWidth = BoundaryCharacter->GetTerrainMovement()->FootProbeHalfWidth;
		const float MaxCharacterX = Terrain->GetActorLocation().X + Terrain->Width * 0.5f - FootProbeHalfWidth;
		TestEqual(TEXT("Assigning terrain clamps character inside terrain width"), static_cast<float>(BoundaryCharacter->GetActorLocation().X), MaxCharacterX);
		BoundaryCharacter->BeginTurn();
		BoundaryCharacter->MoveHorizontal(1.0f, 1.0f);
		TestEqual(TEXT("Battle character movement stays inside terrain width"), static_cast<float>(BoundaryCharacter->GetActorLocation().X), MaxCharacterX);
	}

	AFRBattleCharacter* LateBoundCharacter = World->SpawnActor<AFRBattleCharacter>(AFRBattleCharacter::StaticClass(), FVector(-15.0f, 0.0f, 200.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Late-bound terrain battle character is spawned"), LateBoundCharacter);
	if (LateBoundCharacter)
	{
		LateBoundCharacter->SetActorLocation(FVector(-15.0f, 0.0f, 200.0f));
		LateBoundCharacter->SetTerrain(Terrain);
		const float FootOffsetZ = LateBoundCharacter->GetTerrainMovement()->GetFootOffsetZ();
		TestEqual(TEXT("Assigning terrain after BeginPlay snaps character feet onto the highest footprint surface"), static_cast<float>(LateBoundCharacter->GetActorLocation().Z), 60.0f + FootOffsetZ);
	}

	AFRBattleCharacter* RampCharacter = World->SpawnActor<AFRBattleCharacter>(AFRBattleCharacter::StaticClass(), FVector(-95.0f, 0.0f, 55.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Ramp battle character is spawned"), RampCharacter);
	if (RampCharacter)
	{
		RampCharacter->AddWeaponDefinition(CreateTestWeaponDefinition(RampCharacter));
		RampCharacter->SetTerrain(Terrain);
		RampCharacter->GetTerrainMovement()->FootProbeHalfWidth = 0.0f;
		RampCharacter->GetTerrainMovement()->BodySlopeProbeHalfWidth = 5.0f;
		RampCharacter->SetActorLocation(FVector(-95.0f, 0.0f, 55.0f));
		RampCharacter->BeginTurn();
		RampCharacter->MoveHorizontal(1.0f, 0.12f);
		TestTrue(TEXT("Battle character climbs a traversable gentle slope"), RampCharacter->GetActorLocation().X > -70.0f && RampCharacter->GetActorLocation().Z >= 65.0f);
		UStaticMeshComponent* RampBody = Cast<UStaticMeshComponent>(RampCharacter->GetDefaultSubobjectByName(TEXT("Body")));
		TestNotNull(TEXT("Ramp battle character body exists"), RampBody);
		USceneComponent* RampBodyFrame = Cast<USceneComponent>(RampCharacter->GetDefaultSubobjectByName(TEXT("BodyFrame")));
		TestNotNull(TEXT("Ramp battle character body frame exists"), RampBodyFrame);
		TestEqual(TEXT("Battle character actor stays level on terrain slopes"), static_cast<float>(RampCharacter->GetActorRotation().Pitch), 0.0f);
		const float MaxCharacterSlopeDegrees = RuntimeGameMode ? RuntimeGameMode->GetMaxCharacterSlopeDegrees() : 45.0f;
		const float RampPitch = RampBodyFrame ? RampBodyFrame->GetRelativeRotation().Pitch : 0.0f;
		TestTrue(TEXT("Battle character body frame follows traversable terrain slope"), RampPitch > 0.0f && RampPitch <= MaxCharacterSlopeDegrees + 0.1f);
		if (RampBody)
		{
			TestEqual(TEXT("Battle character body stays local under the slope frame"), RampBody->GetRelativeRotation(), FRotator::ZeroRotator);
		}
		TestEqual(TEXT("Slope-aligned battle character fires from body frame pitch"), RampCharacter->FireSelectedWeapon(), 1);
		AFRProjectile* RampProjectile = nullptr;
		for (TActorIterator<AFRProjectile> It(World); It; ++It)
		{
			if (It->GetOwner() == RampCharacter)
			{
				RampProjectile = *It;
				break;
			}
		}
		TestNotNull(TEXT("Slope-aligned battle character spawns a projectile"), RampProjectile);
		if (RampProjectile)
		{
			const USceneComponent* RampMuzzle = Cast<USceneComponent>(RampCharacter->GetDefaultSubobjectByName(TEXT("Muzzle")));
			TestNotNull(TEXT("Slope-aligned projectile owner has a muzzle"), RampMuzzle);
			const FVector ExpectedRampProjectileSpawnLocation = RampMuzzle ? RampMuzzle->GetComponentLocation() : RampCharacter->GetActorLocation();
			TestTrue(TEXT("Slope-aligned projectile starts from the muzzle"), RampProjectile->GetActorLocation().Equals(ExpectedRampProjectileSpawnLocation, 0.1));
			RampProjectile->Destroy();
		}
	}

	AFRBattleCharacter* SteepSlopeCharacter = World->SpawnActor<AFRBattleCharacter>(AFRBattleCharacter::StaticClass(), FVector(-45.0f, 0.0f, 55.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Steep slope battle character is spawned"), SteepSlopeCharacter);
	if (SteepSlopeCharacter)
	{
		SteepSlopeCharacter->SetTerrain(Terrain);
		SteepSlopeCharacter->GetTerrainMovement()->FootProbeHalfWidth = 0.0f;
		SteepSlopeCharacter->SetActorLocation(FVector(-45.0f, 0.0f, 55.0f));
		SteepSlopeCharacter->BeginTurn();
		SteepSlopeCharacter->MoveHorizontal(1.0f, 0.12f);
		TestTrue(TEXT("Battle character stops at terrain steeper than the slope limit"), SteepSlopeCharacter->GetActorLocation().X < -30.0f);
		TestEqual(TEXT("Battle character does not climb the rejected steep slope"), static_cast<float>(SteepSlopeCharacter->GetActorLocation().Z), 55.0f);
	}

	AFRBattleCharacter* GapCharacter = World->SpawnActor<AFRBattleCharacter>(AFRBattleCharacter::StaticClass(), FVector(485.0f, 0.0f, 55.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Gap movement battle character is spawned"), GapCharacter);
	if (GapCharacter && GapTerrain)
	{
		GapCharacter->SetTerrain(GapTerrain);
		GapCharacter->GetTerrainMovement()->FootProbeHalfWidth = 0.0f;
		GapCharacter->SetActorLocation(FVector(485.0f, 0.0f, 55.0f));
		GapCharacter->BeginTurn();
		GapCharacter->MoveHorizontal(1.0f, 0.1f);
		TestTrue(TEXT("Battle character does not traverse an unsupported gap in one move"), GapCharacter->GetActorLocation().X > 485.0f && GapCharacter->GetActorLocation().X < 500.0f);
		TestTrue(TEXT("Battle character starts falling when horizontal movement reaches unsupported terrain"), GapCharacter->GetActorLocation().Z < 55.0f);
	}

	AFRBattleCharacter* SettlingCharacter = World->SpawnActor<AFRBattleCharacter>(AFRBattleCharacter::StaticClass(), FVector(5.0f, 0.0f, 105.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Settling battle character is spawned"), SettlingCharacter);
	if (SettlingCharacter)
	{
		SettlingCharacter->SetTerrain(Terrain);
		SettlingCharacter->GetTerrainMovement()->FootProbeHalfWidth = 0.0f;
		TestTrue(TEXT("Carve circle lowers nearby support under the character"), Terrain->CarveCircle(FVector(5.0f, 0.0f, 55.0f), 11.0f));
		SettlingCharacter->ReevaluateTerrainSupport();
		TestEqual(TEXT("Battle character settles onto a lower surface inside the step-down range"), static_cast<float>(SettlingCharacter->GetActorLocation().Z), 85.0f);
	}

	FVector ImpactLocation = FVector::ZeroVector;
	TestTrue(TEXT("Segment query hits solid terrain"), Terrain->FindFirstSolidAlongWorldSegment(FVector(-15.0f, 0.0f, 25.0f), FVector(-15.0f, 0.0f, 5.0f), ImpactLocation));
	TestTrue(TEXT("Segment impact lies inside the solid band"), ImpactLocation.Z >= 0.0f && ImpactLocation.Z < 10.0f);

	if (FastProjectileTerrain)
	{
		FVector FastImpactLocation = FVector::ZeroVector;
		TestTrue(TEXT("Fast segment query hits a one-cell vertical wall"), FastProjectileTerrain->FindFirstSolidAlongWorldSegment(FVector(203.0f, 0.0f, 35.0f), FVector(397.0f, 0.0f, 35.0f), FastImpactLocation));
		TestTrue(TEXT("Fast segment impact lies inside the one-cell wall"), FastImpactLocation.X >= 300.0f && FastImpactLocation.X < 310.0f && FastImpactLocation.Z >= 30.0f && FastImpactLocation.Z < 40.0f);

		AFRBattleCharacter* FastProjectileTarget = World->SpawnActor<AFRBattleCharacter>(AFRBattleCharacter::StaticClass(), FVector(260.0f, 500.0f, 35.0f), FRotator::ZeroRotator, SpawnParams);
		TestNotNull(TEXT("Fast projectile target is spawned before the wall"), FastProjectileTarget);
		if (FastProjectileTarget)
		{
			FastProjectileTarget->SetActorLocation(FVector(260.0f, 500.0f, 35.0f));
			const float TargetHealthBeforeHit = FastProjectileTarget->GetHealth();
			AFRProjectile* FastCharacterProjectile = World->SpawnActor<AFRProjectile>(AFRProjectile::StaticClass(), FVector(203.0f, 0.0f, 35.0f), FRotator::ZeroRotator, SpawnParams);
			TestNotNull(TEXT("Fast character projectile is spawned"), FastCharacterProjectile);
			if (FastCharacterProjectile)
			{
				UBoxComponent* TargetHurtbox = Cast<UBoxComponent>(FastProjectileTarget->GetDefaultSubobjectByName(TEXT("Hurtbox")));
				const float ExpectedImpactX = TargetHurtbox
					? TargetHurtbox->GetComponentLocation().X - TargetHurtbox->GetScaledBoxExtent().X
					: FastProjectileTarget->GetActorLocation().X;
				FastCharacterProjectile->InitializeProjectile(nullptr, FastProjectileTerrain, FVector(1940.0f, 0.0f, 0.0f), 20.0f, 30.0f, 50.0f, 20.0f, 0.0f);
				FastCharacterProjectile->Tick(0.1f);
				TestEqual(TEXT("Direct hit applies hit plus full explosion damage"), FastProjectileTarget->GetHealth(), TargetHealthBeforeHit - 50.0f);
				if (TargetHurtbox)
				{
					TestTrue(TEXT("Fast projectile resolves at the first hurtbox face"), FMath::IsNearlyEqual(static_cast<float>(FastCharacterProjectile->GetActorLocation().X), ExpectedImpactX, 0.1f));
				}
				TestTrue(TEXT("Later terrain remains solid when the earlier character is hit"), FastProjectileTerrain->IsSolidAtWorldLocation(FVector(305.0f, 0.0f, 35.0f)));
			}
			FastProjectileTarget->Destroy();
		}

		UFRTerrainMapDefinition* SurfaceImpactMap = NewObject<UFRTerrainMapDefinition>();
		SurfaceImpactMap->Resize(12, 3);
		SurfaceImpactMap->CellSize = 10.0f;
		SurfaceImpactMap->Clear(false);
		SurfaceImpactMap->FillRect(0, 0, 11, 0, true);
		AFRDestructibleTerrain* SurfaceImpactTerrain = World->SpawnActorDeferred<AFRDestructibleTerrain>(
			AFRDestructibleTerrain::StaticClass(),
			FTransform(FRotator::ZeroRotator, FVector(15000.0f, 0.0f, 0.0f)));
		TestNotNull(TEXT("Surface-impact terrain is spawned"), SurfaceImpactTerrain);
		if (SurfaceImpactTerrain)
		{
			SurfaceImpactTerrain->MapDefinition = SurfaceImpactMap;
			UGameplayStatics::FinishSpawningActor(SurfaceImpactTerrain, FTransform(FRotator::ZeroRotator, FVector(15000.0f, 0.0f, 0.0f)));
			const FVector RightFaceTerrainCell(15025.0f, 0.0f, 5.0f);
			TestTrue(TEXT("Terrain beside the target starts solid"), SurfaceImpactTerrain->IsSolidAtWorldLocation(RightFaceTerrainCell));

			AFRBattleCharacter* SurfaceImpactTarget = World->SpawnActor<AFRBattleCharacter>(AFRBattleCharacter::StaticClass(), FVector(15000.0f, 500.0f, 14.0f), FRotator::ZeroRotator, SpawnParams);
			TestNotNull(TEXT("Surface-impact target is spawned"), SurfaceImpactTarget);
			if (SurfaceImpactTarget)
			{
				SurfaceImpactTarget->SetActorLocation(FVector(15000.0f, 500.0f, 14.0f));
			}
			UBoxComponent* SurfaceImpactHurtbox = SurfaceImpactTarget
				? Cast<UBoxComponent>(SurfaceImpactTarget->GetDefaultSubobjectByName(TEXT("Hurtbox")))
				: nullptr;
			TestNotNull(TEXT("Surface-impact target has a hurtbox"), SurfaceImpactHurtbox);
			if (SurfaceImpactTarget && SurfaceImpactHurtbox)
			{
				const FVector BoxExtent = SurfaceImpactHurtbox->GetUnscaledBoxExtent();
				const FVector ExpectedRightFace = SurfaceImpactHurtbox->GetComponentTransform().TransformPosition(FVector(BoxExtent.X, 0.0f, 0.0f));
				const float TargetHealthBeforeSurfaceImpact = SurfaceImpactTarget->GetHealth();
				AFRProjectile* SurfaceImpactProjectile = World->SpawnActor<AFRProjectile>(
					AFRProjectile::StaticClass(),
					FVector(ExpectedRightFace.X + 50.0f, 0.0f, ExpectedRightFace.Z),
					FRotator::ZeroRotator,
					SpawnParams);
				TestNotNull(TEXT("Right-face projectile is spawned"), SurfaceImpactProjectile);
				if (SurfaceImpactProjectile)
				{
					SurfaceImpactProjectile->InitializeProjectile(nullptr, SurfaceImpactTerrain, FVector(-1000.0f, 0.0f, 0.0f), 20.0f, 0.0f, 0.0f, 0.0f, 0.0f, 12.0f);
					SurfaceImpactProjectile->Tick(0.1f);
					TestEqual(TEXT("Right-face direct hit applies point damage"), SurfaceImpactTarget->GetHealth(), TargetHealthBeforeSurfaceImpact - 20.0f);
					TestTrue(TEXT("Projectile explosion origin stays on the right hurtbox face"), FMath::IsNearlyEqual(static_cast<float>(SurfaceImpactProjectile->GetActorLocation().X), static_cast<float>(ExpectedRightFace.X), 0.1f));
					TestFalse(TEXT("Terrain damage is centered on the right hurtbox contact"), SurfaceImpactTerrain->IsSolidAtWorldLocation(RightFaceTerrainCell));
				}
			}
		}

		AFRBattleCharacter* RotatedHurtboxTarget = World->SpawnActor<AFRBattleCharacter>(AFRBattleCharacter::StaticClass(), FVector(17000.0f, 700.0f, 2000.0f), FRotator::ZeroRotator, SpawnParams);
		TestNotNull(TEXT("Rotated hurtbox target is spawned"), RotatedHurtboxTarget);
		if (RotatedHurtboxTarget)
		{
			RotatedHurtboxTarget->SetActorLocation(FVector(17000.0f, 700.0f, 2000.0f));
		}
		USceneComponent* RotatedBodyFrame = RotatedHurtboxTarget
			? Cast<USceneComponent>(RotatedHurtboxTarget->GetDefaultSubobjectByName(TEXT("BodyFrame")))
			: nullptr;
		UBoxComponent* RotatedHurtbox = RotatedHurtboxTarget
			? Cast<UBoxComponent>(RotatedHurtboxTarget->GetDefaultSubobjectByName(TEXT("Hurtbox")))
			: nullptr;
		TestNotNull(TEXT("Rotated target has a body frame"), RotatedBodyFrame);
		TestNotNull(TEXT("Rotated target has a hurtbox"), RotatedHurtbox);
		if (RotatedHurtboxTarget && RotatedBodyFrame && RotatedHurtbox)
		{
			RotatedBodyFrame->SetRelativeRotation(FRotator(35.0f, 0.0f, 0.0f));
			const FVector BoxExtent = RotatedHurtbox->GetUnscaledBoxExtent();
			const FTransform HurtboxTransform = RotatedHurtbox->GetComponentTransform();
			const FVector ExpectedImpact = HurtboxTransform.TransformPosition(FVector(BoxExtent.X, 0.0f, 0.0f));
			FVector SegmentStart = HurtboxTransform.TransformPosition(FVector(BoxExtent.X + 60.0f, 0.0f, 0.0f));
			FVector SegmentEnd = HurtboxTransform.TransformPosition(FVector(-BoxExtent.X - 60.0f, 0.0f, 0.0f));
			SegmentStart.Y = 0.0f;
			SegmentEnd.Y = 0.0f;
			FVector QueriedImpact = FVector::ZeroVector;
			TestTrue(TEXT("Rotated hurtbox intersects a high-speed XZ segment"), RotatedHurtboxTarget->FindHurtboxImpactAlongSegmentXZ(SegmentStart, SegmentEnd, QueriedImpact));
			TestTrue(TEXT("Rotated hurtbox reports its actual surface contact"), FVector2D(QueriedImpact.X, QueriedImpact.Z).Equals(FVector2D(ExpectedImpact.X, ExpectedImpact.Z), 0.1f));
			const FVector OutsidePoint = HurtboxTransform.TransformPosition(FVector(BoxExtent.X + 20.0f, 0.0f, 0.0f));
			TestTrue(TEXT("Hurtbox distance is measured from the rotated surface"), FMath::IsNearlyEqual(RotatedHurtboxTarget->GetDistanceToHurtboxXZ(OutsidePoint), 20.0f, 0.1f));

			const float TargetHealthBeforeRotatedHit = RotatedHurtboxTarget->GetHealth();
			AFRProjectile* RotatedHurtboxProjectile = World->SpawnActor<AFRProjectile>(AFRProjectile::StaticClass(), SegmentStart, FRotator::ZeroRotator, SpawnParams);
			TestNotNull(TEXT("Rotated hurtbox projectile is spawned"), RotatedHurtboxProjectile);
			if (RotatedHurtboxProjectile)
			{
				constexpr float RotatedProjectileDeltaSeconds = 0.1f;
				const float Wind = RuntimeGameMode ? RuntimeGameMode->GetWind() : 0.0f;
				const FVector InitialVelocity = (SegmentEnd - SegmentStart) / RotatedProjectileDeltaSeconds
					- FVector(Wind * RotatedProjectileDeltaSeconds, 0.0f, 0.0f);
				RotatedHurtboxProjectile->InitializeProjectile(nullptr, nullptr, InitialVelocity, 20.0f, 0.0f, 0.0f, 0.0f, 0.0f);
				RotatedHurtboxProjectile->Tick(RotatedProjectileDeltaSeconds);
				TestEqual(TEXT("High-speed projectile damages the rotated hurtbox target"), RotatedHurtboxTarget->GetHealth(), TargetHealthBeforeRotatedHit - 20.0f);
				TestTrue(TEXT("High-speed projectile resolves on the rotated hurtbox face"), FVector2D(RotatedHurtboxProjectile->GetActorLocation().X, RotatedHurtboxProjectile->GetActorLocation().Z).Equals(FVector2D(ExpectedImpact.X, ExpectedImpact.Z), 0.1f));
			}
		}

		AFRBattleCharacter* ProjectileOwner = World->SpawnActor<AFRBattleCharacter>(AFRBattleCharacter::StaticClass(), FVector(9000.0f, 0.0f, 2000.0f), FRotator::ZeroRotator, SpawnParams);
		AFRBattleCharacter* FriendlyTarget = World->SpawnActor<AFRBattleCharacter>(AFRBattleCharacter::StaticClass(), FVector(9060.0f, 0.0f, 2000.0f), FRotator::ZeroRotator, SpawnParams);
		AFRBattleCharacter* EnemyTarget = World->SpawnActor<AFRBattleCharacter>(AFRBattleCharacter::StaticClass(), FVector(9120.0f, 0.0f, 2000.0f), FRotator::ZeroRotator, SpawnParams);
		TestNotNull(TEXT("Projectile owner is spawned for team filtering"), ProjectileOwner);
		TestNotNull(TEXT("Friendly target is spawned for team filtering"), FriendlyTarget);
		TestNotNull(TEXT("Enemy target is spawned for team filtering"), EnemyTarget);
		if (ProjectileOwner && FriendlyTarget && EnemyTarget)
		{
			ProjectileOwner->ConfigureAsEnemy(false);
			FriendlyTarget->ConfigureAsEnemy(false);
			EnemyTarget->ConfigureAsEnemy(true);
			ProjectileOwner->SetActorLocation(FVector(9000.0f, 0.0f, 2000.0f));
			FriendlyTarget->SetActorLocation(FVector(9060.0f, 0.0f, 2000.0f));
			EnemyTarget->SetActorLocation(FVector(9120.0f, 0.0f, 2000.0f));
			const float OwnerHealthBeforeTeamShot = ProjectileOwner->GetHealth();
			const float FriendlyHealthBeforeTeamShot = FriendlyTarget->GetHealth();
			const float EnemyHealthBeforeTeamShot = EnemyTarget->GetHealth();
			AFRProjectile* TeamFilteredProjectile = World->SpawnActor<AFRProjectile>(AFRProjectile::StaticClass(), FVector(9000.0f, 0.0f, 2000.0f), FRotator::ZeroRotator, SpawnParams);
			TestNotNull(TEXT("Team-filtered projectile is spawned"), TeamFilteredProjectile);
			if (TeamFilteredProjectile)
			{
				TeamFilteredProjectile->InitializeProjectile(ProjectileOwner, nullptr, FVector(1200.0f, 0.0f, 0.0f), 20.0f, 30.0f, 70.0f, 70.0f, 0.0f);
				TeamFilteredProjectile->Tick(0.11f);
				TestEqual(TEXT("Projectile does not damage its owner"), ProjectileOwner->GetHealth(), OwnerHealthBeforeTeamShot);
				TestEqual(TEXT("Projectile ignores friendly characters in its path"), FriendlyTarget->GetHealth(), FriendlyHealthBeforeTeamShot);
				TestEqual(TEXT("Projectile damages enemy characters with hit plus blast damage"), EnemyTarget->GetHealth(), EnemyHealthBeforeTeamShot - 50.0f);
			}
		}

		AFRBattleCharacter* ExplosionFalloffTarget = World->SpawnActor<AFRBattleCharacter>(AFRBattleCharacter::StaticClass(), FVector(380.0f, 500.0f, 35.0f), FRotator::ZeroRotator, SpawnParams);
		TestNotNull(TEXT("Explosion falloff target is spawned behind the wall"), ExplosionFalloffTarget);
		if (ExplosionFalloffTarget)
		{
			ExplosionFalloffTarget->SetActorLocation(FVector(380.0f, 500.0f, 35.0f));
			const FVector ExplosionFalloffTargetLocation = ExplosionFalloffTarget->GetActorLocation();
			const float TargetHealthBeforeExplosion = ExplosionFalloffTarget->GetHealth();
			AFRProjectile* ExplosionFalloffProjectile = World->SpawnActor<AFRProjectile>(AFRProjectile::StaticClass(), FVector(280.0f, 0.0f, 35.0f), FRotator::ZeroRotator, SpawnParams);
			TestNotNull(TEXT("Explosion falloff projectile is spawned"), ExplosionFalloffProjectile);
			if (ExplosionFalloffProjectile)
			{
				FVector ExplosionFalloffImpactLocation = FVector::ZeroVector;
				TestTrue(TEXT("Explosion falloff projectile hits terrain before the target"), FastProjectileTerrain->FindFirstSolidAlongWorldSegment(FVector(280.0f, 0.0f, 35.0f), FVector(397.0f, 0.0f, 35.0f), ExplosionFalloffImpactLocation));
				const float ExplosionFalloffDistance = ExplosionFalloffTarget->GetDistanceToHurtboxXZ(ExplosionFalloffImpactLocation);
				const float CenterDistance = FVector2D(ExplosionFalloffTargetLocation.X - ExplosionFalloffImpactLocation.X, ExplosionFalloffTargetLocation.Z - ExplosionFalloffImpactLocation.Z).Size();
				ExplosionFalloffProjectile->InitializeProjectile(nullptr, FastProjectileTerrain, FVector(1170.0f, 0.0f, 0.0f), 0.0f, 80.0f, 100.0f, 25.0f, 0.0f);
				ExplosionFalloffProjectile->Tick(0.1f);
				const float ExpectedExplosionFalloffDamage = FMath::Lerp(80.0f, 0.0f, FMath::Clamp((ExplosionFalloffDistance - 25.0f) / 75.0f, 0.0f, 1.0f));
				TestTrue(TEXT("Explosion uses the closest hurtbox point instead of the actor center"), ExplosionFalloffDistance < CenterDistance);
				TestTrue(TEXT("Explosion damage falls off from the hurtbox surface to the blast edge"), FMath::IsNearlyEqual(ExplosionFalloffTarget->GetHealth(), TargetHealthBeforeExplosion - ExpectedExplosionFalloffDamage, 1.0f));
				TestTrue(TEXT("Zero terrain damage leaves impacted terrain intact"), FastProjectileTerrain->IsSolidAtWorldLocation(FVector(305.0f, 0.0f, 35.0f)));
			}
		}

		AFRBattleCharacter* ZeroRadiusTarget = World->SpawnActor<AFRBattleCharacter>(AFRBattleCharacter::StaticClass(), FVector(240.0f, 500.0f, 60.0f), FRotator::ZeroRotator, SpawnParams);
		TestNotNull(TEXT("Zero-radius direct-hit target is spawned before the wall"), ZeroRadiusTarget);
		if (ZeroRadiusTarget)
		{
			ZeroRadiusTarget->SetActorLocation(FVector(240.0f, 500.0f, 60.0f));
			const float TargetHealthBeforeZeroRadiusHit = ZeroRadiusTarget->GetHealth();
			AFRProjectile* ZeroRadiusProjectile = World->SpawnActor<AFRProjectile>(AFRProjectile::StaticClass(), FVector(203.0f, 0.0f, 35.0f), FRotator::ZeroRotator, SpawnParams);
			TestNotNull(TEXT("Zero-radius direct-hit projectile is spawned"), ZeroRadiusProjectile);
			if (ZeroRadiusProjectile)
			{
				ZeroRadiusProjectile->InitializeProjectile(nullptr, FastProjectileTerrain, FVector(740.0f, 0.0f, 0.0f), 20.0f, 0.0f, 0.0f, 0.0f, 0.0f);
				ZeroRadiusProjectile->Tick(0.1f);
				TestTrue(TEXT("Zero-radius direct hit applies finite point damage"), FMath::IsFinite(ZeroRadiusTarget->GetHealth()) && ZeroRadiusTarget->GetHealth() < TargetHealthBeforeZeroRadiusHit);
			}
		}

		AFRProjectile* FastProjectile = World->SpawnActor<AFRProjectile>(AFRProjectile::StaticClass(), FVector(203.0f, 0.0f, 35.0f), FRotator::ZeroRotator, SpawnParams);
		TestNotNull(TEXT("Fast projectile is spawned"), FastProjectile);
		if (FastProjectile)
		{
			FastProjectile->InitializeProjectile(nullptr, FastProjectileTerrain, FVector(1940.0f, 0.0f, 0.0f), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 6.0f);
			FastProjectile->Tick(0.1f);
			TestTrue(TEXT("Resolved projectile actor location stays at the terrain impact point"), FastProjectile->GetActorLocation().X >= 300.0f && FastProjectile->GetActorLocation().X < 310.0f);
			TestFalse(TEXT("Fast projectile carves the one-cell vertical wall instead of tunneling through it"), FastProjectileTerrain->IsSolidAtWorldLocation(FVector(305.0f, 0.0f, 35.0f)));
		}
	}

	AFRProjectile* AssignedTerrainProjectile = World->SpawnActor<AFRProjectile>(AFRProjectile::StaticClass(), FVector(-75.0f, 0.0f, 25.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Assigned-terrain projectile is spawned"), AssignedTerrainProjectile);
	if (AssignedTerrainProjectile && OverlappingTerrain)
	{
		if (UPrimitiveComponent* ProjectileCollision = Cast<UPrimitiveComponent>(AssignedTerrainProjectile->GetDefaultSubobjectByName(TEXT("Collision"))))
		{
			TestEqual(TEXT("Projectile root collision is disabled for custom terrain checks"), ProjectileCollision->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
		}
		if (UPrimitiveComponent* ProjectileVisual = Cast<UPrimitiveComponent>(AssignedTerrainProjectile->GetDefaultSubobjectByName(TEXT("Visual"))))
		{
			TestEqual(TEXT("Projectile visual collision is disabled"), ProjectileVisual->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
		}
		AssignedTerrainProjectile->InitializeProjectile(Character, Terrain, FVector(0.0f, 0.0f, -100.0f), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 24.0f);
		AssignedTerrainProjectile->Tick(0.25f);
		TestFalse(TEXT("Projectile carves its assigned terrain"), Terrain->IsSolidAtWorldLocation(FVector(-75.0f, 0.0f, 5.0f)));
		TestTrue(TEXT("Projectile does not carve overlapping unassigned terrain"), OverlappingTerrain->IsSolidAtWorldLocation(FVector(-75.0f, 0.0f, 5.0f)));
	}

	AFRBattleCharacter* FallingCharacter = World->SpawnActor<AFRBattleCharacter>(AFRBattleCharacter::StaticClass(), FVector(45.0f, 0.0f, 55.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Falling battle character is spawned"), FallingCharacter);
	if (FallingCharacter)
	{
		FallingCharacter->AddWeaponDefinition(CreateTestWeaponDefinition(FallingCharacter));
		FallingCharacter->SetTerrain(Terrain);
		FallingCharacter->BeginTurn();
		TestTrue(TEXT("Supported battle character reports shot charge can begin"), FallingCharacter->CanBeginShotCharge());
		FallingCharacter->BeginShotCharge();
		FallingCharacter->UpdateShotCharge(0.5f);
		TestTrue(TEXT("Supported battle character can start charging before support is destroyed"), FallingCharacter->IsChargingShot());
		const float BeforeReevaluateZ = FallingCharacter->GetActorLocation().Z;
		TestTrue(TEXT("Carve circle removes support under the falling character"), Terrain->CarveCircle(FVector(45.0f, 0.0f, 5.0f), 25.0f));
		FallingCharacter->ReevaluateTerrainSupport();
		TestTrue(TEXT("Battle character starts falling immediately after support terrain is destroyed"), FallingCharacter->GetActorLocation().Z < BeforeReevaluateZ);
		TestFalse(TEXT("Battle character cancels shot charge when terrain support is destroyed"), FallingCharacter->IsChargingShot());
		const float FallingCharacterX = FallingCharacter->GetActorLocation().X;
		FallingCharacter->BeginTurn();
		FallingCharacter->MoveHorizontal(1.0f, 0.25f);
		TestEqual(TEXT("Falling battle character cannot move horizontally while unsupported"), static_cast<float>(FallingCharacter->GetActorLocation().X), FallingCharacterX);
		TestFalse(TEXT("Falling battle character reports selected weapon not fireable while unsupported"), FallingCharacter->CanFireSelectedWeapon());
		TestFalse(TEXT("Falling battle character reports shot charge cannot begin while unsupported"), FallingCharacter->CanBeginShotCharge());
		const int32 AttackItemCharges = FallingCharacter->GetItemCharges(EFRItemType::AttackMultiplier);
		TestFalse(TEXT("Falling battle character reports combat items unusable while unsupported"), FallingCharacter->CanUseItemByType(EFRItemType::AttackMultiplier));
		TestFalse(TEXT("Falling battle character cannot use combat items while unsupported"), FallingCharacter->UseItemByType(EFRItemType::AttackMultiplier));
		TestEqual(TEXT("Falling battle character keeps item charges when unsupported item use is rejected"), FallingCharacter->GetItemCharges(EFRItemType::AttackMultiplier), AttackItemCharges);
		FallingCharacter->BeginShotCharge();
		FallingCharacter->UpdateShotCharge(1.0f);
		TestEqual(TEXT("Falling battle character cannot fire while unsupported"), FallingCharacter->ReleaseShotCharge(), 0);
		FallingCharacter->GetTerrainMovement()->FallDeathDepth = 1.0f;
		FallingCharacter->SetActorLocation(FVector(45.0f, 0.0f, -2.0f));
		FallingCharacter->Tick(0.01f);
		TestTrue(TEXT("Battle character is defeated after falling below the terrain death depth"), FallingCharacter->IsDefeated());
	}

	TestTrue(TEXT("Carve circle reports a terrain change"), Terrain->CarveCircle(FVector(-15.0f, 0.0f, 5.0f), 6.0f));
	TestFalse(TEXT("Carved bottom cell is no longer solid"), Terrain->IsSolidAtWorldLocation(FVector(-15.0f, 0.0f, 5.0f)));
	TestFalse(TEXT("No surface remains after carving that column"), Terrain->FindSurfaceZAtWorldX(-15.0f, 30.0f, 40.0f, SurfaceZ));
	if (TexturePlane && TexturePlane->IsVisible() && TextureTerrainInstances)
	{
		TestEqual(TEXT("Texture-rendered terrain does not rebuild fallback instances after carving"), TextureTerrainInstances->GetInstanceCount(), 0);
	}

	AFRDestructibleTerrain* RotatedTerrain = World->SpawnActor<AFRDestructibleTerrain>(AFRDestructibleTerrain::StaticClass(), FVector(200.0f, 0.0f, 0.0f), FRotator(0.0f, 90.0f, 90.0f), SpawnParams);
	TestNotNull(TEXT("Rotated terrain actor is spawned"), RotatedTerrain);
	if (RotatedTerrain)
	{
		TestTrue(TEXT("Terrain actor rotation is normalized for the gameplay X/Z plane"), RotatedTerrain->GetActorRotation().IsNearlyZero());
	}

	CleanupWorld();
	return true;
}

#endif
