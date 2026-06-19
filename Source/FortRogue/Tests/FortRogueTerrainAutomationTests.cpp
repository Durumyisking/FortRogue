// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "AbilitySystem/Attributes/FortRogueCombatSet.h"
#include "AbilitySystem/FortRogueAbilitySet.h"
#include "Characters/FortRogueCharacterDefinition.h"
#include "Combat/FortRogueBattleCharacter.h"
#include "Combat/FortRogueDestructibleTerrain.h"
#include "Combat/FortRogueImpactSpawnSpec.h"
#include "Combat/FortRogueProjectile.h"
#include "Combat/FortRogueTerrainMapDefinition.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FortRogueGameMode.h"
#include "FortRogueGameplayTags.h"
#include "FortRoguePlayerController.h"
#include "Items/FortRogueItemDefinition.h"
#include "Kismet/GameplayStatics.h"
#include "PaperFlipbookComponent.h"
#include "Perks/FortRoguePerkDefinition.h"
#include "ProjectileEffects/FRProjectileEffect.h"
#include "ProjectileEffects/FRProjectileSplitEffect.h"
#include "Rewards/FortRogueRewardBlueprintLibrary.h"
#include "Rewards/FortRogueRewardTypes.h"
#include "Run/FortRogueDefaultLoadoutDefinition.h"
#include "Run/FortRogueStageRunDefinition.h"
#include "UI/FortRogueBattleHUDWidget.h"
#include "Weapons/FortRogueWeaponDefinition.h"
#include "UObject/UnrealType.h"

namespace
{
UFortRogueWeaponDefinition* CreateTestWeaponDefinition(UObject* Outer)
{
	UFortRogueWeaponDefinition* WeaponDefinition = NewObject<UFortRogueWeaponDefinition>(Outer);
	WeaponDefinition->Weapon.DisplayName = FText::FromString(TEXT("Test Shell"));
	WeaponDefinition->Weapon.Damage = 35.0f;
	WeaponDefinition->Weapon.BlastRadius = 145.0f;
	WeaponDefinition->Weapon.ProjectileSpeed = 1180.0f;
	WeaponDefinition->Weapon.ProjectilesPerShot = 1;
	return WeaponDefinition;
}

UFortRogueDefaultLoadoutDefinition* CreateTestDefaultLoadout(UObject* Outer)
{
	UFortRogueDefaultLoadoutDefinition* LoadoutDefinition = NewObject<UFortRogueDefaultLoadoutDefinition>(Outer);
	LoadoutDefinition->WeaponDefinitions.Add(CreateTestWeaponDefinition(LoadoutDefinition));
	return LoadoutDefinition;
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFortRogueTerrainMapDefinitionEditTest, "FortRogue.Terrain.MapDefinition.Edits", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFortRogueTerrainMapDefinitionEditTest::RunTest(const FString& Parameters)
{
	UFortRogueTerrainMapDefinition* Map = NewObject<UFortRogueTerrainMapDefinition>();
	TestNotNull(TEXT("Map asset object is created"), Map);
	TestEqual(TEXT("Default map width uses 4x Fortress screen pixels"), Map->CellsX, 1280);
	TestEqual(TEXT("Default map height uses 4x Fortress screen pixels"), Map->CellsZ, 960);
	TestEqual(TEXT("Default map uses one-unit world cells"), Map->CellSize, 1.0f);
	const float HalfMapWidth = Map->CellsX * Map->CellSize * 0.5f;
	TestTrue(TEXT("Default player spawn is inside the map width"), FMath::Abs(Map->PlayerSpawnLocal.X) < HalfMapWidth);
	TestTrue(TEXT("Default enemy spawn is inside the map width"), FMath::Abs(Map->EnemySpawnLocal.X) < HalfMapWidth);
	TestTrue(TEXT("Default spawns start above the map"), Map->PlayerSpawnLocal.Z > Map->CellsZ * Map->CellSize && Map->EnemySpawnLocal.Z > Map->CellsZ * Map->CellSize);

	UFortRogueStageRunDefinition* StageRun = NewObject<UFortRogueStageRunDefinition>();
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

	UFortRogueStageRunDefinition* InvalidStageRunData = NewObject<UFortRogueStageRunDefinition>();
	InvalidStageRunData->StageCount = 0;
	InvalidStageRunData->RewardChoiceCount = 0;
	InvalidStageRunData->StageDifficultyData.Reset();
	InvalidStageRunData->StageDifficultyData.AddDefaulted();
	FFortRogueRewardChoice InvalidStageReward;
	InvalidStageRunData->RewardPool.Add(InvalidStageReward);
	const FString InvalidStageRunDataSummary = InvalidStageRunData->GetDataValidationSummary().ToString();
	TestTrue(TEXT("Stage run data validation reports invalid stage counts"), InvalidStageRunDataSummary.Contains(TEXT("stage count")));
	TestTrue(TEXT("Stage run data validation reports invalid reward choice counts"), InvalidStageRunDataSummary.Contains(TEXT("reward choice count")));
	TestTrue(TEXT("Stage run data validation reports difficulty row mismatches"), InvalidStageRunDataSummary.Contains(TEXT("stage difficulty rows")));
	TestTrue(TEXT("Stage run data validation reports nested reward warnings"), InvalidStageRunDataSummary.Contains(TEXT("reward pool data")));
	UFortRogueStageRunDefinition* LockedStageRunData = NewObject<UFortRogueStageRunDefinition>();
	FFortRogueRewardChoice LockedReward;
	LockedReward.DisplayName = FText::FromString(TEXT("Locked Reward"));
	LockedReward.DamageBonus = 1.0f;
	LockedReward.RequiredRewardTags.AddTag(FortRogueGameplayTags::Trait_Damage);
	LockedStageRunData->RewardPool.Add(LockedReward);
	LockedStageRunData->RewardChoiceCount = 1;
	TestTrue(TEXT("Stage run data validation reports missing starting rewards"), LockedStageRunData->GetDataValidationSummary().ToString().Contains(TEXT("run start")));
	TestTrue(TEXT("Blueprint helper reports stage run data validation"), UFortRogueRewardBlueprintLibrary::GetStageRunDataValidationSummary(InvalidStageRunData).ToString().Contains(TEXT("stage count")));
	TestTrue(TEXT("Blueprint helper reports missing stage run assets"), UFortRogueRewardBlueprintLibrary::GetStageRunDataValidationSummary(nullptr).ToString().Contains(TEXT("missing stage run")));
	UFortRogueStageRunDefinition* ValidStageRunData = NewObject<UFortRogueStageRunDefinition>();
	FFortRogueRewardChoice ValidStageReward;
	ValidStageReward.DisplayName = FText::FromString(TEXT("Valid Stage Reward"));
	ValidStageReward.DamageBonus = 1.0f;
	ValidStageRunData->RewardPool.Add(ValidStageReward);
	ValidStageRunData->RewardChoiceCount = 1;
	TestTrue(TEXT("Stage run data validation is empty for valid run data"), ValidStageRunData->GetDataValidationSummary().ToString().IsEmpty());

	UFortRogueDefaultLoadoutDefinition* InvalidLoadoutData = NewObject<UFortRogueDefaultLoadoutDefinition>();
	InvalidLoadoutData->WeaponDefinitions.Add(nullptr);
	FFortRogueDefaultItemStack InvalidItemStack;
	InvalidItemStack.Charges = 0;
	InvalidLoadoutData->ItemDefinitions.Add(InvalidItemStack);
	const FString InvalidLoadoutDataSummary = InvalidLoadoutData->GetDataValidationSummary().ToString();
	TestTrue(TEXT("Default loadout data validation reports missing weapon entries"), InvalidLoadoutDataSummary.Contains(TEXT("missing weapon entry")));
	TestTrue(TEXT("Default loadout data validation reports missing item entries"), InvalidLoadoutDataSummary.Contains(TEXT("missing item entry")));
	TestTrue(TEXT("Default loadout data validation reports invalid item charges"), InvalidLoadoutDataSummary.Contains(TEXT("item charges")));
	UFortRogueDefaultLoadoutDefinition* EmptyLoadoutData = NewObject<UFortRogueDefaultLoadoutDefinition>();
	TestTrue(TEXT("Default loadout data validation reports empty weapon lists"), EmptyLoadoutData->GetDataValidationSummary().ToString().Contains(TEXT("weapon definitions are empty")));
	UFortRogueDefaultLoadoutDefinition* NestedInvalidLoadoutData = NewObject<UFortRogueDefaultLoadoutDefinition>();
	NestedInvalidLoadoutData->WeaponDefinitions.Add(NewObject<UFortRogueWeaponDefinition>(NestedInvalidLoadoutData));
	FFortRogueDefaultItemStack NestedInvalidItemStack;
	NestedInvalidItemStack.ItemDefinition = NewObject<UFortRogueItemDefinition>(NestedInvalidLoadoutData);
	NestedInvalidLoadoutData->ItemDefinitions.Add(NestedInvalidItemStack);
	const FString NestedInvalidLoadoutDataSummary = NestedInvalidLoadoutData->GetDataValidationSummary().ToString();
	TestTrue(TEXT("Default loadout data validation reports nested weapon warnings"), NestedInvalidLoadoutDataSummary.Contains(TEXT("weapon data")));
	TestTrue(TEXT("Default loadout data validation reports nested item warnings"), NestedInvalidLoadoutDataSummary.Contains(TEXT("item data")));
	TestTrue(TEXT("Blueprint helper reports default loadout data validation"), UFortRogueRewardBlueprintLibrary::GetDefaultLoadoutDataValidationSummary(InvalidLoadoutData).ToString().Contains(TEXT("missing weapon entry")));
	TestTrue(TEXT("Blueprint helper reports missing default loadout assets"), UFortRogueRewardBlueprintLibrary::GetDefaultLoadoutDataValidationSummary(nullptr).ToString().Contains(TEXT("missing loadout")));
	UFortRogueDefaultLoadoutDefinition* ValidLoadoutData = NewObject<UFortRogueDefaultLoadoutDefinition>();
	UFortRogueWeaponDefinition* ValidLoadoutWeapon = NewObject<UFortRogueWeaponDefinition>(ValidLoadoutData);
	ValidLoadoutWeapon->Weapon.DisplayName = FText::FromString(TEXT("Valid Loadout Shell"));
	ValidLoadoutWeapon->Weapon.WeaponTag = FortRogueGameplayTags::Weapon_Shell;
	ValidLoadoutWeapon->Weapon.Damage = 10.0f;
	ValidLoadoutWeapon->Weapon.BlastRadius = 100.0f;
	ValidLoadoutWeapon->Weapon.ProjectileSpeed = 1000.0f;
	ValidLoadoutWeapon->Weapon.ProjectilesPerShot = 1;
	ValidLoadoutData->WeaponDefinitions.Add(ValidLoadoutWeapon);
	TestTrue(TEXT("Default loadout data validation is empty for valid loadout data"), ValidLoadoutData->GetDataValidationSummary().ToString().IsEmpty());

	TestGameplayTagCategories(*this, FFortRogueShotModifierSpec::StaticStruct(), GET_MEMBER_NAME_CHECKED(FFortRogueShotModifierSpec, EffectTags), TEXT("ShotEffect"));
	TestGameplayTagCategories(*this, FFortRogueShotModifierSpec::StaticStruct(), GET_MEMBER_NAME_CHECKED(FFortRogueShotModifierSpec, ModifierTag), TEXT("ShotEffect"));
	TestGameplayTagCategories(*this, FFortRogueShotModifierSpec::StaticStruct(), GET_MEMBER_NAME_CHECKED(FFortRogueShotModifierSpec, RequiredShotTags), TEXT("Weapon,ShotEffect"));
	TestGameplayTagCategories(*this, FFortRogueShotModifierSpec::StaticStruct(), GET_MEMBER_NAME_CHECKED(FFortRogueShotModifierSpec, BlockedShotTags), TEXT("Weapon,ShotEffect"));
	TestGameplayTagCategories(*this, FFortRogueWeaponSpec::StaticStruct(), GET_MEMBER_NAME_CHECKED(FFortRogueWeaponSpec, WeaponTag), TEXT("Weapon"));
	TestGameplayTagCategories(*this, FFortRogueWeaponSpec::StaticStruct(), GET_MEMBER_NAME_CHECKED(FFortRogueWeaponSpec, ShotEffectTags), TEXT("ShotEffect"));
	TestGameplayTagCategories(*this, UFortRogueItemDefinition::StaticClass(), GET_MEMBER_NAME_CHECKED(UFortRogueItemDefinition, ItemTag), TEXT("Item"));
	TestGameplayTagCategories(*this, FFortRogueRewardChoice::StaticStruct(), GET_MEMBER_NAME_CHECKED(FFortRogueRewardChoice, RewardTag), TEXT("Weapon,Item,Trait"));
	TestGameplayTagCategories(*this, FFortRogueRewardChoice::StaticStruct(), GET_MEMBER_NAME_CHECKED(FFortRogueRewardChoice, RequiredRewardTags), TEXT("Weapon,Item,Trait"));
	TestGameplayTagCategories(*this, FFortRogueRewardChoice::StaticStruct(), GET_MEMBER_NAME_CHECKED(FFortRogueRewardChoice, BlockedRewardTags), TEXT("Weapon,Item,Trait"));
	TestGameplayTagCategories(*this, FFortRogueImpactSpawnSpec::StaticStruct(), GET_MEMBER_NAME_CHECKED(FFortRogueImpactSpawnSpec, ChildEffectTags), TEXT("ShotEffect"));
	TestGameplayTagCategories(*this, FFortRogueShotSpec::StaticStruct(), GET_MEMBER_NAME_CHECKED(FFortRogueShotSpec, WeaponTag), TEXT("Weapon"));
	TestGameplayTagCategories(*this, FFortRogueShotSpec::StaticStruct(), GET_MEMBER_NAME_CHECKED(FFortRogueShotSpec, EffectTags), TEXT("Weapon,ShotEffect"));
	TestGameplayTagCategories(*this, UFortRoguePerkDefinition::StaticClass(), GET_MEMBER_NAME_CHECKED(UFortRoguePerkDefinition, PerkTag), TEXT("Trait"));
	TestGameplayTagCategories(*this, FFRProjectileEffectSplitParams::StaticStruct(), GET_MEMBER_NAME_CHECKED(FFRProjectileEffectSplitParams, ChildEffectTags), TEXT("ShotEffect"));
	TestGameplayTagCategories(*this, FFortRogueAbilitySet_GameplayAbility::StaticStruct(), GET_MEMBER_NAME_CHECKED(FFortRogueAbilitySet_GameplayAbility, InputTag), TEXT("InputTag"));
	TestGameplayTagCategories(*this, UFortRogueAbilitySet::StaticClass(), GET_MEMBER_NAME_CHECKED(UFortRogueAbilitySet, AbilitySetTag), TEXT("Trait"));
	TestPropertyMetaData(*this, FFRProjectileEffectSpec::StaticStruct(), GET_MEMBER_NAME_CHECKED(FFRProjectileEffectSpec, EffectClass), TEXT("AllowAbstract"), TEXT("false"));
	TestPropertyAdvancedDisplay(*this, FFortRogueShotModifierSpec::StaticStruct(), GET_MEMBER_NAME_CHECKED(FFortRogueShotModifierSpec, TerrainCarveRadiusBonus));
	TestPropertyAdvancedDisplay(*this, FFortRogueShotModifierSpec::StaticStruct(), GET_MEMBER_NAME_CHECKED(FFortRogueShotModifierSpec, TerrainCarveRadiusMultiplier));
	TestPropertyAdvancedDisplay(*this, FFortRogueShotModifierSpec::StaticStruct(), GET_MEMBER_NAME_CHECKED(FFortRogueShotModifierSpec, TerrainFillRadiusBonus));
	TestPropertyAdvancedDisplay(*this, FFortRogueShotModifierSpec::StaticStruct(), GET_MEMBER_NAME_CHECKED(FFortRogueShotModifierSpec, TerrainFillRadiusMultiplier));
	TestFunctionParamGameplayTagCategories(*this, AFortRogueBattleCharacter::StaticClass(), GET_FUNCTION_NAME_CHECKED(AFortRogueBattleCharacter, TryGetCombatAttributeValueByTag), TEXT("AttributeTag"), TEXT("Attribute"));
	TestFunctionParamGameplayTagCategories(*this, AFortRogueBattleCharacter::StaticClass(), GET_FUNCTION_NAME_CHECKED(AFortRogueBattleCharacter, SelectWeaponByTag), TEXT("WeaponTag"), TEXT("Weapon"));
	TestFunctionParamGameplayTagCategories(*this, AFortRoguePlayerController::StaticClass(), GET_FUNCTION_NAME_CHECKED(AFortRoguePlayerController, RemovePlayerGrantedShotModifiersByTag), TEXT("ModifierTag"), TEXT("ShotEffect"));
	TestFunctionParamGameplayTagCategories(*this, UFortRogueBattleHUDWidget::StaticClass(), GET_FUNCTION_NAME_CHECKED(UFortRogueBattleHUDWidget, GetPlayerItemIndexByTag), TEXT("ItemTag"), TEXT("Item"));

	UFortRogueAbilitySet* NamedAbilitySet = NewObject<UFortRogueAbilitySet>();
	NamedAbilitySet->DisplayName = FText::FromString(TEXT("Wind Split"));
	NamedAbilitySet->Description = FText::FromString(TEXT("Adds wind-aware split behavior."));
	TestTrue(TEXT("Ability set summary includes display name"), NamedAbilitySet->GetEffectSummary().ToString().Contains(TEXT("Wind Split")));
	TestTrue(TEXT("Ability set summary includes descriptions"), NamedAbilitySet->GetEffectSummary().ToString().Contains(TEXT("Adds wind-aware split behavior.")));
	TestTrue(TEXT("Blueprint helper summarizes ability set assets"), UFortRogueRewardBlueprintLibrary::GetAbilitySetEffectSummary(NamedAbilitySet).ToString().Contains(TEXT("Wind Split")));
	UFortRogueAbilitySet* InvalidAbilitySetData = NewObject<UFortRogueAbilitySet>();
	InvalidAbilitySetData->DisplayName = FText::GetEmpty();
	const FString InvalidAbilitySetDataSummary = InvalidAbilitySetData->GetDataValidationSummary().ToString();
	TestTrue(TEXT("Ability set data validation reports missing display names"), InvalidAbilitySetDataSummary.Contains(TEXT("missing display name")));
	TestTrue(TEXT("Ability set data validation reports missing grants"), InvalidAbilitySetDataSummary.Contains(TEXT("missing granted ability")));
	TestTrue(TEXT("Blueprint helper reports ability set data validation"), UFortRogueRewardBlueprintLibrary::GetAbilitySetDataValidationSummary(InvalidAbilitySetData).ToString().Contains(TEXT("missing display name")));
	TestTrue(TEXT("Blueprint helper reports missing ability set assets"), UFortRogueRewardBlueprintLibrary::GetAbilitySetDataValidationSummary(nullptr).ToString().Contains(TEXT("missing ability set")));
	FFortRogueShotModifierSpec SummaryModifier;
	SummaryModifier.DisplayName = FText::FromString(TEXT("Split Boost"));
	SummaryModifier.Description = FText::FromString(TEXT("Adds child shots after impact."));
	SummaryModifier.ModifierTag = FortRogueGameplayTags::ShotEffect_Projectiles;
	SummaryModifier.EffectTags.AddTag(FortRogueGameplayTags::ShotEffect_SplitOnImpact);
	SummaryModifier.ProjectileCountBonus = 2;
	TArray<FFortRogueShotModifierSpec> SummaryModifiers = { SummaryModifier };
	TestTrue(TEXT("Blueprint helper summarizes standalone shot modifiers"), UFortRogueRewardBlueprintLibrary::GetShotModifierEffectSummary(SummaryModifiers).ToString().Contains(TEXT("projectiles +2")));
	TestTrue(TEXT("Blueprint helper summarizes shot modifier display names"), UFortRogueRewardBlueprintLibrary::GetShotModifierEffectSummary(SummaryModifiers).ToString().Contains(TEXT("modifier Split Boost")));
	TestTrue(TEXT("Blueprint helper summarizes shot modifier descriptions"), UFortRogueRewardBlueprintLibrary::GetShotModifierEffectSummary(SummaryModifiers).ToString().Contains(TEXT("Adds child shots after impact.")));
	TestTrue(TEXT("Blueprint helper summarizes shot modifier tags"), UFortRogueRewardBlueprintLibrary::GetShotModifierEffectSummary(SummaryModifiers).ToString().Contains(TEXT("modifier tag ShotEffect.Projectiles")));
	TestTrue(TEXT("Blueprint helper summarizes shot effect tags"), UFortRogueRewardBlueprintLibrary::GetShotModifierEffectSummary(SummaryModifiers).ToString().Contains(TEXT("ShotEffect.SplitOnImpact")));
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
	FFortRogueShotSpec EffectShotSpec;
	EffectShotSpec.TerrainCarveRadius = 100.0f;
	DrillEffect.ApplyToShotSpec(EffectShotSpec);
	TerrainCreateEffect.ApplyToShotSpec(EffectShotSpec);
	TestTrue(TEXT("Projectile effect CDO adds drill tags"), EffectShotSpec.EffectTags.HasTagExact(FortRogueGameplayTags::ShotEffect_Drill));
	TestTrue(TEXT("Projectile effect CDO adds terrain create tags"), EffectShotSpec.EffectTags.HasTagExact(FortRogueGameplayTags::ShotEffect_TerrainCreate));
	TestEqual(TEXT("Projectile drill effect updates terrain carve radius"), EffectShotSpec.TerrainCarveRadius, 125.0f);
	TestEqual(TEXT("Projectile terrain create effect updates terrain fill radius"), EffectShotSpec.TerrainFillRadius, 60.0f);
	FFortRogueShotModifierSpec EffectModifierData;
	EffectModifierData.DisplayName = FText::FromString(TEXT("Valid Projectile Effects"));
	EffectModifierData.ProjectileEffects.Add(DrillEffect);
	EffectModifierData.ProjectileEffects.Add(TerrainCreateEffect);
	FFRProjectileEffectSplitParams SplitParams;
	SplitParams.ProjectileCount = 2;
	SplitParams.ChildShotModifiers.Add(EffectModifierData);
	FFRProjectileEffectSpec SplitEffect;
	SplitEffect.EffectClass = UFRProjectileEffectSplit::StaticClass();
	SplitEffect.Parameters = FInstancedStruct::Make(SplitParams);
	SplitEffect.ApplyToShotSpec(EffectShotSpec);
	TestTrue(TEXT("Projectile split effect CDO adds split tags"), EffectShotSpec.EffectTags.HasTagExact(FortRogueGameplayTags::ShotEffect_SplitOnImpact));
	EffectModifierData.ProjectileEffects.Add(SplitEffect);
	TestTrue(TEXT("Shot modifier data validation accepts projectile effects"), EffectModifierData.GetDataValidationSummary().ToString().IsEmpty());
	TArray<FFortRogueShotModifierSpec> EffectModifierSummaryData = { EffectModifierData };
	const FString EffectModifierSummary = UFortRogueRewardBlueprintLibrary::GetShotModifierEffectSummary(EffectModifierSummaryData).ToString();
	TestTrue(TEXT("Shot modifier summary counts projectile effects"), EffectModifierSummary.Contains(TEXT("projectile effects 3")));
	TestTrue(TEXT("Shot modifier summary names drill projectile effects"), EffectModifierSummary.Contains(UFRProjectileEffectDrill::StaticClass()->GetDisplayNameText().ToString()));
	TestTrue(TEXT("Shot modifier summary names terrain create projectile effects"), EffectModifierSummary.Contains(UFRProjectileEffectTerrainCreate::StaticClass()->GetDisplayNameText().ToString()));
	TestTrue(TEXT("Shot modifier summary names split projectile effects"), EffectModifierSummary.Contains(UFRProjectileEffectSplit::StaticClass()->GetDisplayNameText().ToString()));
	TestTrue(TEXT("Shot modifier gameplay effect helper accepts projectile effects"), EffectModifierData.HasGameplayEffect());
	FFortRogueShotModifierSpec EmptyProjectileEffectSummaryModifier;
	EmptyProjectileEffectSummaryModifier.ProjectileEffects.AddDefaulted();
	TArray<FFortRogueShotModifierSpec> EmptyProjectileEffectSummaryData = { EmptyProjectileEffectSummaryModifier };
	TestFalse(TEXT("Shot modifier summary ignores empty projectile effect placeholders"), UFortRogueRewardBlueprintLibrary::GetShotModifierEffectSummary(EmptyProjectileEffectSummaryData).ToString().Contains(TEXT("projectile effects")));
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
	FFortRogueShotModifierSpec InvalidSplitChildCountModifier;
	InvalidSplitChildCountModifier.DisplayName = FText::FromString(TEXT("Invalid Split Child Count"));
	InvalidSplitChildCountModifier.ProjectileCountBonus = 1;
	InvalidSplitParams.ChildShotModifiers.Add(InvalidSplitChildCountModifier);
	FFortRogueShotModifierSpec LegacySplitChildModifier;
	LegacySplitChildModifier.DisplayName = FText::FromString(TEXT("Legacy Split Child"));
	FFortRogueImpactSpawnSpec LegacySplitImpactSpawn;
	LegacySplitImpactSpawn.ProjectileCount = 1;
	LegacySplitChildModifier.ImpactSpawns.Add(LegacySplitImpactSpawn);
	InvalidSplitParams.ChildShotModifiers.Add(LegacySplitChildModifier);
	FFRProjectileEffectSpec InvalidSplitEffect;
	InvalidSplitEffect.EffectClass = UFRProjectileEffectSplit::StaticClass();
	InvalidSplitEffect.Parameters = FInstancedStruct::Make(InvalidSplitParams);
	const FString InvalidSplitEffectSummary = InvalidSplitEffect.GetDataValidationSummary().ToString();
	TestTrue(TEXT("Projectile effect data validation reports class-owned split warnings"), InvalidSplitEffectSummary.Contains(TEXT("split projectile count")));
	TestTrue(TEXT("Projectile effect data validation reports invalid split child modifiers"), InvalidSplitEffectSummary.Contains(TEXT("split child shot modifier")));
	TestTrue(TEXT("Projectile effect data validation reports ignored split child projectile bonuses"), InvalidSplitEffectSummary.Contains(TEXT("projectile count bonus")));
	TestTrue(TEXT("Projectile effect data validation reports legacy split child impact spawns"), InvalidSplitEffectSummary.Contains(TEXT("legacy impact spawns")));
	FFortRogueShotModifierSpec InvalidShotModifierData;
	InvalidShotModifierData.bUseAimAngleRange = true;
	InvalidShotModifierData.MinAimAngle = 80.0f;
	InvalidShotModifierData.MaxAimAngle = 20.0f;
	InvalidShotModifierData.RequiredShotTags.AddTag(FortRogueGameplayTags::Weapon_Shell);
	InvalidShotModifierData.BlockedShotTags.AddTag(FortRogueGameplayTags::Weapon_Shell);
	InvalidShotModifierData.ProjectileEffects.AddDefaulted();
	InvalidShotModifierData.ImpactSpawns.AddDefaulted();
	const FString InvalidShotModifierDataSummary = InvalidShotModifierData.GetDataValidationSummary().ToString();
	TestTrue(TEXT("Shot modifier data validation reports missing display names"), InvalidShotModifierDataSummary.Contains(TEXT("missing display name")));
	TestTrue(TEXT("Shot modifier data validation reports missing effects"), InvalidShotModifierDataSummary.Contains(TEXT("missing shot effect")));
	TestTrue(TEXT("Shot modifier data validation reports inverted aim ranges"), InvalidShotModifierDataSummary.Contains(TEXT("aim range")));
	TestTrue(TEXT("Shot modifier data validation reports overlapping shot tags"), InvalidShotModifierDataSummary.Contains(TEXT("overlap")));
	TestTrue(TEXT("Shot modifier data validation reports invalid projectile effects"), InvalidShotModifierDataSummary.Contains(TEXT("projectile effect")));
	TestTrue(TEXT("Shot modifier data validation reports empty impact spawns"), InvalidShotModifierDataSummary.Contains(TEXT("projectile count")));
	TestTrue(TEXT("Blueprint helper reports shot modifier data validation"), UFortRogueRewardBlueprintLibrary::GetShotModifierDataValidationSummary(InvalidShotModifierData).ToString().Contains(TEXT("missing shot effect")));
	TestFalse(TEXT("Shot modifier gameplay effect helper rejects empty placeholders"), InvalidShotModifierData.HasGameplayEffect());
	FFortRogueShotModifierSpec ValidShotModifierData;
	ValidShotModifierData.DisplayName = FText::FromString(TEXT("Valid Shot Modifier"));
	ValidShotModifierData.DamageBonus = 5.0f;
	TestTrue(TEXT("Shot modifier data validation is empty for valid modifier data"), ValidShotModifierData.GetDataValidationSummary().ToString().IsEmpty());
	FFortRogueShotSpec ConditionShotSpec;
	ConditionShotSpec.WeaponTag = FortRogueGameplayTags::Weapon_Shell;
	FFortRogueShotModifierSpec ConditionModifier;
	ConditionModifier.RequiredShotTags.AddTag(FortRogueGameplayTags::Weapon_Shell);
	TestTrue(TEXT("Shot modifier condition helper accepts required weapon tags"), ConditionModifier.MeetsShotConditions(ConditionShotSpec, 45.0f, 0.0f, true));
	TestTrue(TEXT("Blueprint shot modifier condition helper accepts required shot tags"), UFortRogueRewardBlueprintLibrary::DoesShotModifierMeetShotConditions(ConditionModifier, ConditionShotSpec, 45.0f, 0.0f, true));
	ConditionModifier.RequiredShotTags.Reset();
	ConditionShotSpec.EffectTags.AddTag(FortRogueGameplayTags::ShotEffect_Drill);
	ConditionModifier.RequiredShotTags.AddTag(FortRogueGameplayTags::ShotEffect_Drill);
	TestTrue(TEXT("Shot modifier condition helper accepts required effect tags"), ConditionModifier.MeetsShotConditions(ConditionShotSpec, 45.0f, 0.0f, true));
	ConditionModifier.RequiredShotTags.Reset();
	ConditionModifier.RequiredShotTags.AddTag(FortRogueGameplayTags::Weapon_Shell);
	ConditionModifier.RequiredShotTags.AddTag(FortRogueGameplayTags::ShotEffect_TerrainCreate);
	TestFalse(TEXT("Shot modifier condition helper requires all required shot tags"), ConditionModifier.MeetsShotConditions(ConditionShotSpec, 45.0f, 0.0f, true));
	ConditionShotSpec.EffectTags.AddTag(FortRogueGameplayTags::ShotEffect_TerrainCreate);
	TestTrue(TEXT("Shot modifier condition helper accepts all required shot tags"), ConditionModifier.MeetsShotConditions(ConditionShotSpec, 45.0f, 0.0f, true));
	ConditionModifier.RequiredShotTags.Reset();
	ConditionModifier.RequiredShotTags.AddTag(FortRogueGameplayTags::Weapon_Cluster);
	TestFalse(TEXT("Shot modifier condition helper rejects missing required shot tags"), ConditionModifier.MeetsShotConditions(ConditionShotSpec, 45.0f, 0.0f, true));
	TestTrue(TEXT("Shot modifier condition failure summary names required shot tags"), ConditionModifier.GetShotConditionFailureSummary(ConditionShotSpec, 45.0f, 0.0f, true).ToString().Contains(TEXT("requires shot tag")));
	ConditionModifier.RequiredShotTags.Reset();
	ConditionModifier.BlockedShotTags.AddTag(FortRogueGameplayTags::Weapon_Shell);
	TestFalse(TEXT("Shot modifier condition helper rejects blocked weapon tags"), ConditionModifier.MeetsShotConditions(ConditionShotSpec, 45.0f, 0.0f, true));
	TestTrue(TEXT("Blueprint shot modifier condition failure summary names blocked shot tags"), UFortRogueRewardBlueprintLibrary::GetShotModifierConditionFailureSummary(ConditionModifier, ConditionShotSpec, 45.0f, 0.0f, true).ToString().Contains(TEXT("blocked by shot tag")));
	ConditionModifier.BlockedShotTags.Reset();
	ConditionModifier.bUseAimAngleRange = true;
	ConditionModifier.MinAimAngle = 30.0f;
	ConditionModifier.MaxAimAngle = 60.0f;
	TestFalse(TEXT("Shot modifier condition helper rejects out-of-range aim"), ConditionModifier.MeetsShotConditions(ConditionShotSpec, 20.0f, 0.0f, true));
	TestTrue(TEXT("Shot modifier condition failure summary names aim range"), ConditionModifier.GetShotConditionFailureSummary(ConditionShotSpec, 20.0f, 0.0f, true).ToString().Contains(TEXT("requires aim")));
	ConditionModifier.bUseAimAngleRange = false;
	ConditionModifier.bRequireWindAligned = true;
	ConditionModifier.MinWindMagnitude = 10.0f;
	TestTrue(TEXT("Shot modifier condition helper accepts aligned wind"), ConditionModifier.MeetsShotConditions(ConditionShotSpec, 45.0f, 15.0f, true));
	TestFalse(TEXT("Shot modifier condition helper rejects opposite wind"), ConditionModifier.MeetsShotConditions(ConditionShotSpec, 45.0f, -15.0f, true));
	TestTrue(TEXT("Shot modifier condition failure summary names aligned wind"), ConditionModifier.GetShotConditionFailureSummary(ConditionShotSpec, 45.0f, -15.0f, true).ToString().Contains(TEXT("requires aligned wind")));
	FFortRogueRewardChoice AbilitySetReward;
	AbilitySetReward.GrantedAbilitySet = NamedAbilitySet;
	TestTrue(TEXT("Reward summary names directly granted ability set"), AbilitySetReward.GetEffectSummary().ToString().Contains(TEXT("ability set Wind Split")));
	FFortRogueRewardChoice InvalidRewardData;
	InvalidRewardData.bOfferOncePerRun = true;
	InvalidRewardData.RewardWeight = 0.0f;
	InvalidRewardData.RequiredRewardTags.AddTag(FortRogueGameplayTags::Trait_Damage);
	InvalidRewardData.BlockedRewardTags.AddTag(FortRogueGameplayTags::Trait_Damage);
	const FString InvalidRewardDataSummary = InvalidRewardData.GetDataValidationSummary().ToString();
	TestTrue(TEXT("Reward data validation reports missing display names"), InvalidRewardDataSummary.Contains(TEXT("missing display name")));
	TestTrue(TEXT("Reward data validation reports invalid weights"), InvalidRewardDataSummary.Contains(TEXT("greater than 0")));
	TestTrue(TEXT("Reward data validation reports missing once-per-run tags"), InvalidRewardDataSummary.Contains(TEXT("RewardTag")));
	TestTrue(TEXT("Reward data validation reports missing effects"), InvalidRewardDataSummary.Contains(TEXT("missing gameplay effect")));
	TestTrue(TEXT("Reward data validation reports overlapping condition tags"), InvalidRewardDataSummary.Contains(TEXT("overlap")));
	TestTrue(TEXT("Blueprint helper reports reward data validation"), UFortRogueRewardBlueprintLibrary::GetRewardDataValidationSummary(InvalidRewardData).ToString().Contains(TEXT("missing display name")));
	FFortRogueRewardChoice EmptyModifierRewardData;
	EmptyModifierRewardData.DisplayName = FText::FromString(TEXT("Empty Modifier Reward"));
	EmptyModifierRewardData.ShotModifiers.AddDefaulted();
	const FString EmptyModifierRewardDataSummary = EmptyModifierRewardData.GetDataValidationSummary().ToString();
	TestTrue(TEXT("Reward data validation ignores empty shot modifiers as gameplay effects"), EmptyModifierRewardDataSummary.Contains(TEXT("missing gameplay effect")));
	TestTrue(TEXT("Reward data validation reports nested empty shot modifiers"), EmptyModifierRewardDataSummary.Contains(TEXT("shot modifier data")));
	FFortRogueRewardChoice ValidRewardData;
	ValidRewardData.DisplayName = FText::FromString(TEXT("Valid Reward"));
	ValidRewardData.RewardTag = FortRogueGameplayTags::Trait_Damage;
	ValidRewardData.bOfferOncePerRun = true;
	ValidRewardData.DamageBonus = 3.0f;
	TestTrue(TEXT("Reward data validation is empty for valid reward data"), ValidRewardData.GetDataValidationSummary().ToString().IsEmpty());

	UFortRogueWeaponDefinition* SummaryWeapon = NewObject<UFortRogueWeaponDefinition>();
	SummaryWeapon->Weapon.DisplayName = FText::FromString(TEXT("Fork Shell"));
	SummaryWeapon->Weapon.Description = FText::FromString(TEXT("Splits the battlefield."));
	SummaryWeapon->Weapon.WeaponTag = FortRogueGameplayTags::Weapon_Shell;
	TestTrue(TEXT("Blueprint helper summarizes weapon assets"), UFortRogueRewardBlueprintLibrary::GetWeaponEffectSummary(SummaryWeapon).ToString().Contains(TEXT("weapon Fork Shell")));
	TestTrue(TEXT("Blueprint helper summarizes weapon descriptions"), UFortRogueRewardBlueprintLibrary::GetWeaponEffectSummary(SummaryWeapon).ToString().Contains(TEXT("Splits the battlefield.")));
	TestTrue(TEXT("Blueprint helper summarizes weapon tags"), UFortRogueRewardBlueprintLibrary::GetWeaponEffectSummary(SummaryWeapon).ToString().Contains(TEXT("tag Weapon.Shell")));
	TestTrue(TEXT("Blueprint helper summarizes weapon base damage"), UFortRogueRewardBlueprintLibrary::GetWeaponEffectSummary(SummaryWeapon).ToString().Contains(TEXT("damage 35")));
	TestTrue(TEXT("Blueprint helper summarizes weapon blast radius"), UFortRogueRewardBlueprintLibrary::GetWeaponEffectSummary(SummaryWeapon).ToString().Contains(TEXT("blast 150")));
	UFortRogueWeaponDefinition* InvalidWeaponData = NewObject<UFortRogueWeaponDefinition>();
	InvalidWeaponData->Weapon.DisplayName = FText::GetEmpty();
	InvalidWeaponData->Weapon.Damage = 0.0f;
	InvalidWeaponData->Weapon.BlastRadius = 0.0f;
	InvalidWeaponData->Weapon.ProjectileSpeed = 0.0f;
	InvalidWeaponData->Weapon.ProjectilesPerShot = 0;
	InvalidWeaponData->Weapon.ImpactSpawns.AddDefaulted();
	InvalidWeaponData->Weapon.ShotModifiers.Add(InvalidShotModifierData);
	const FString InvalidWeaponDataSummary = InvalidWeaponData->GetDataValidationSummary().ToString();
	TestTrue(TEXT("Weapon data validation reports missing display names"), InvalidWeaponDataSummary.Contains(TEXT("missing display name")));
	TestTrue(TEXT("Weapon data validation reports missing tags"), InvalidWeaponDataSummary.Contains(TEXT("WeaponTag")));
	TestTrue(TEXT("Weapon data validation reports missing effects"), InvalidWeaponDataSummary.Contains(TEXT("missing weapon effect")));
	TestTrue(TEXT("Weapon data validation reports invalid projectile speed"), InvalidWeaponDataSummary.Contains(TEXT("projectile speed")));
	TestTrue(TEXT("Weapon data validation reports invalid projectile counts"), InvalidWeaponDataSummary.Contains(TEXT("projectiles per shot")));
	TestTrue(TEXT("Weapon data validation reports empty impact spawns"), InvalidWeaponDataSummary.Contains(TEXT("impact spawn projectile count")));
	TestTrue(TEXT("Weapon data validation reports nested shot modifier warnings"), InvalidWeaponDataSummary.Contains(TEXT("shot modifier data")));
	TestTrue(TEXT("Blueprint helper reports weapon data validation"), UFortRogueRewardBlueprintLibrary::GetWeaponDataValidationSummary(InvalidWeaponData).ToString().Contains(TEXT("missing display name")));
	TestTrue(TEXT("Blueprint helper reports missing weapon assets"), UFortRogueRewardBlueprintLibrary::GetWeaponDataValidationSummary(nullptr).ToString().Contains(TEXT("missing weapon")));
	TestTrue(TEXT("Weapon data validation is empty for valid weapon data"), SummaryWeapon->GetDataValidationSummary().ToString().IsEmpty());

	UFortRogueItemDefinition* AbilityItem = NewObject<UFortRogueItemDefinition>();
	AbilityItem->DisplayName = FText::FromString(TEXT("Storm Capsule"));
	AbilityItem->Description = FText::FromString(TEXT("Empowers the next shot."));
	AbilityItem->ItemTag = FortRogueGameplayTags::Item_NextShot;
	AbilityItem->InitialCharges = 2;
	AbilityItem->UseAbilitySet = NamedAbilitySet;
	FFortRogueRewardChoice ItemAbilityReward;
	ItemAbilityReward.ItemReward = AbilityItem;
	TestTrue(TEXT("Reward summary names item ability set"), ItemAbilityReward.GetEffectSummary().ToString().Contains(TEXT("ability set Wind Split")));
	TestTrue(TEXT("Reward summary includes item descriptions"), ItemAbilityReward.GetEffectSummary().ToString().Contains(TEXT("Empowers the next shot.")));
	TestTrue(TEXT("Reward summary names item tags"), ItemAbilityReward.GetEffectSummary().ToString().Contains(TEXT("tag Item.NextShot")));
	TestTrue(TEXT("Reward summary names item initial charges"), ItemAbilityReward.GetEffectSummary().ToString().Contains(TEXT("charges 2")));
	ItemAbilityReward.RepairCharges = 3;
	TestFalse(TEXT("Reward summary does not duplicate default charges when override charges exist"), ItemAbilityReward.GetEffectSummary().ToString().Contains(TEXT("charges 2")));
	TestTrue(TEXT("Reward summary keeps override charges when present"), ItemAbilityReward.GetEffectSummary().ToString().Contains(TEXT("charges +3")));
	ItemAbilityReward.RepairCharges = 0;
	TestTrue(TEXT("Blueprint helper summarizes item assets"), UFortRogueRewardBlueprintLibrary::GetItemEffectSummary(AbilityItem).ToString().Contains(TEXT("ability set Wind Split")));
	UFortRogueItemDefinition* HealSummaryItem = NewObject<UFortRogueItemDefinition>();
	HealSummaryItem->ItemType = EFortRogueItemType::Heal;
	HealSummaryItem->HealAmount = 42.0f;
	TestTrue(TEXT("Reward summary names heal item amount"), UFortRogueRewardBlueprintLibrary::GetItemEffectSummary(HealSummaryItem).ToString().Contains(TEXT("heal +42")));
	UFortRogueItemDefinition* AttackSummaryItem = NewObject<UFortRogueItemDefinition>();
	AttackSummaryItem->ItemType = EFortRogueItemType::AttackMultiplier;
	AttackSummaryItem->AttackMultiplier = 2.0f;
	TestTrue(TEXT("Reward summary names attack multiplier item amount"), UFortRogueRewardBlueprintLibrary::GetItemEffectSummary(AttackSummaryItem).ToString().Contains(TEXT("next shot attack x2")));
	UFortRogueItemDefinition* InvalidItemData = NewObject<UFortRogueItemDefinition>();
	InvalidItemData->DisplayName = FText::GetEmpty();
	InvalidItemData->ItemType = EFortRogueItemType::AttackMultiplier;
	InvalidItemData->InitialCharges = 0;
	InvalidItemData->AttackMultiplier = 1.0f;
	const FString InvalidItemDataSummary = InvalidItemData->GetDataValidationSummary().ToString();
	TestTrue(TEXT("Item data validation reports missing display names"), InvalidItemDataSummary.Contains(TEXT("missing display name")));
	TestTrue(TEXT("Item data validation reports missing tags"), InvalidItemDataSummary.Contains(TEXT("ItemTag")));
	TestTrue(TEXT("Item data validation reports invalid charges"), InvalidItemDataSummary.Contains(TEXT("initial charges")));
	TestTrue(TEXT("Item data validation reports missing effects"), InvalidItemDataSummary.Contains(TEXT("missing item effect")));
	TestTrue(TEXT("Item data validation reports invalid attack multipliers"), InvalidItemDataSummary.Contains(TEXT("attack multiplier")));
	UFortRogueItemDefinition* EmptyModifierItemData = NewObject<UFortRogueItemDefinition>();
	EmptyModifierItemData->DisplayName = FText::FromString(TEXT("Empty Modifier Item"));
	EmptyModifierItemData->ItemTag = FortRogueGameplayTags::Item_NextShot;
	EmptyModifierItemData->ItemType = EFortRogueItemType::AbilitySet;
	EmptyModifierItemData->UseShotModifiers.AddDefaulted();
	const FString EmptyModifierItemDataSummary = EmptyModifierItemData->GetDataValidationSummary().ToString();
	TestTrue(TEXT("Item data validation ignores empty shot modifiers as gameplay effects"), EmptyModifierItemDataSummary.Contains(TEXT("missing item effect")));
	TestTrue(TEXT("Item data validation reports nested empty shot modifiers"), EmptyModifierItemDataSummary.Contains(TEXT("shot modifier data")));
	UFortRogueItemDefinition* NestedInvalidItemData = NewObject<UFortRogueItemDefinition>();
	NestedInvalidItemData->DisplayName = FText::FromString(TEXT("Nested Invalid Item"));
	NestedInvalidItemData->ItemTag = FortRogueGameplayTags::Item_NextShot;
	NestedInvalidItemData->ItemType = EFortRogueItemType::AbilitySet;
	NestedInvalidItemData->UseAbilitySet = InvalidAbilitySetData;
	NestedInvalidItemData->UseShotModifiers.Add(InvalidShotModifierData);
	const FString NestedInvalidItemDataSummary = NestedInvalidItemData->GetDataValidationSummary().ToString();
	TestTrue(TEXT("Item data validation reports nested ability set warnings"), NestedInvalidItemDataSummary.Contains(TEXT("ability set data")));
	TestTrue(TEXT("Item data validation reports nested shot modifier warnings"), NestedInvalidItemDataSummary.Contains(TEXT("shot modifier data")));
	TestTrue(TEXT("Blueprint helper reports item data validation"), UFortRogueRewardBlueprintLibrary::GetItemDataValidationSummary(InvalidItemData).ToString().Contains(TEXT("missing display name")));
	TestTrue(TEXT("Blueprint helper reports missing item assets"), UFortRogueRewardBlueprintLibrary::GetItemDataValidationSummary(nullptr).ToString().Contains(TEXT("missing item")));
	UFortRogueItemDefinition* ValidItemData = NewObject<UFortRogueItemDefinition>();
	ValidItemData->DisplayName = FText::FromString(TEXT("Valid Heal"));
	ValidItemData->ItemTag = FortRogueGameplayTags::Item_Repair;
	ValidItemData->ItemType = EFortRogueItemType::Heal;
	ValidItemData->InitialCharges = 1;
	ValidItemData->HealAmount = 10.0f;
	TestTrue(TEXT("Item data validation is empty for valid item data"), ValidItemData->GetDataValidationSummary().ToString().IsEmpty());

	UFortRoguePerkDefinition* AbilityPerk = NewObject<UFortRoguePerkDefinition>();
	AbilityPerk->DisplayName = FText::FromString(TEXT("Storm Training"));
	AbilityPerk->Description = FText::FromString(TEXT("Every run shot bends harder."));
	AbilityPerk->PerkTag = FortRogueGameplayTags::Trait_ShotModifier;
	AbilityPerk->Rarity = EFortRoguePerkRarity::Epic;
	AbilityPerk->GrantedAbilitySet = NamedAbilitySet;
	AbilityPerk->DamageBonus = -2.0f;
	AbilityPerk->MaxHealthBonus = -8.0f;
	AbilityPerk->MaxMoveBudgetBonus = -1.0f;
	AbilityPerk->ProjectileBonus = -1;
	AbilityPerk->ShotPowerMultiplierBonus = -0.2f;
	FFortRogueRewardChoice PerkAbilityReward;
	PerkAbilityReward.PerkReward = AbilityPerk;
	TestTrue(TEXT("Reward summary names perk ability set"), PerkAbilityReward.GetEffectSummary().ToString().Contains(TEXT("ability set Wind Split")));
	TestTrue(TEXT("Reward summary includes perk descriptions"), PerkAbilityReward.GetEffectSummary().ToString().Contains(TEXT("Every run shot bends harder.")));
	TestTrue(TEXT("Reward summary names perk tags"), PerkAbilityReward.GetEffectSummary().ToString().Contains(TEXT("tag Trait.ShotModifier")));
	TestTrue(TEXT("Reward summary names perk rarity"), PerkAbilityReward.GetEffectSummary().ToString().Contains(TEXT("rarity Epic")));
	TestTrue(TEXT("Reward summary names negative perk damage bonuses"), PerkAbilityReward.GetEffectSummary().ToString().Contains(TEXT("damage -2")));
	TestTrue(TEXT("Reward summary names negative perk max health bonuses"), PerkAbilityReward.GetEffectSummary().ToString().Contains(TEXT("max HP -8")));
	TestTrue(TEXT("Reward summary names negative perk move bonuses"), PerkAbilityReward.GetEffectSummary().ToString().Contains(TEXT("move -1")));
	TestTrue(TEXT("Reward summary names negative perk projectile bonuses"), PerkAbilityReward.GetEffectSummary().ToString().Contains(TEXT("projectiles -1")));
	TestTrue(TEXT("Reward summary names negative perk shot power bonuses"), PerkAbilityReward.GetEffectSummary().ToString().Contains(TEXT("shot power -0.2")));
	TestTrue(TEXT("Blueprint helper summarizes perk assets"), UFortRogueRewardBlueprintLibrary::GetPerkEffectSummary(AbilityPerk).ToString().Contains(TEXT("ability set Wind Split")));
	TestTrue(TEXT("Blueprint helper summarizes perk rarity"), UFortRogueRewardBlueprintLibrary::GetPerkEffectSummary(AbilityPerk).ToString().Contains(TEXT("rarity Epic")));
	UFortRoguePerkDefinition* InvalidPerkData = NewObject<UFortRoguePerkDefinition>();
	InvalidPerkData->DisplayName = FText::GetEmpty();
	const FString InvalidPerkDataSummary = InvalidPerkData->GetDataValidationSummary().ToString();
	TestTrue(TEXT("Perk data validation reports missing display names"), InvalidPerkDataSummary.Contains(TEXT("missing display name")));
	TestTrue(TEXT("Perk data validation reports missing tags"), InvalidPerkDataSummary.Contains(TEXT("PerkTag")));
	TestTrue(TEXT("Perk data validation reports missing effects"), InvalidPerkDataSummary.Contains(TEXT("missing perk effect")));
	UFortRoguePerkDefinition* EmptyModifierPerkData = NewObject<UFortRoguePerkDefinition>();
	EmptyModifierPerkData->DisplayName = FText::FromString(TEXT("Empty Modifier Perk"));
	EmptyModifierPerkData->PerkTag = FortRogueGameplayTags::Trait_ShotModifier;
	EmptyModifierPerkData->ShotModifiers.AddDefaulted();
	const FString EmptyModifierPerkDataSummary = EmptyModifierPerkData->GetDataValidationSummary().ToString();
	TestTrue(TEXT("Perk data validation ignores empty shot modifiers as gameplay effects"), EmptyModifierPerkDataSummary.Contains(TEXT("missing perk effect")));
	TestTrue(TEXT("Perk data validation reports nested empty shot modifiers"), EmptyModifierPerkDataSummary.Contains(TEXT("shot modifier data")));
	UFortRoguePerkDefinition* NestedInvalidPerkData = NewObject<UFortRoguePerkDefinition>();
	NestedInvalidPerkData->DisplayName = FText::FromString(TEXT("Nested Invalid Perk"));
	NestedInvalidPerkData->PerkTag = FortRogueGameplayTags::Trait_ShotModifier;
	NestedInvalidPerkData->GrantedAbilitySet = InvalidAbilitySetData;
	NestedInvalidPerkData->ShotModifiers.Add(InvalidShotModifierData);
	const FString NestedInvalidPerkDataSummary = NestedInvalidPerkData->GetDataValidationSummary().ToString();
	TestTrue(TEXT("Perk data validation reports nested ability set warnings"), NestedInvalidPerkDataSummary.Contains(TEXT("ability set data")));
	TestTrue(TEXT("Perk data validation reports nested shot modifier warnings"), NestedInvalidPerkDataSummary.Contains(TEXT("shot modifier data")));
	TestTrue(TEXT("Blueprint helper reports perk data validation"), UFortRogueRewardBlueprintLibrary::GetPerkDataValidationSummary(InvalidPerkData).ToString().Contains(TEXT("missing display name")));
	TestTrue(TEXT("Blueprint helper reports missing perk assets"), UFortRogueRewardBlueprintLibrary::GetPerkDataValidationSummary(nullptr).ToString().Contains(TEXT("missing perk")));
	UFortRoguePerkDefinition* ValidPerkData = NewObject<UFortRoguePerkDefinition>();
	ValidPerkData->DisplayName = FText::FromString(TEXT("Valid Perk"));
	ValidPerkData->PerkTag = FortRogueGameplayTags::Trait_Damage;
	ValidPerkData->DamageBonus = 1.0f;
	TestTrue(TEXT("Perk data validation is empty for valid perk data"), ValidPerkData->GetDataValidationSummary().ToString().IsEmpty());
	FFortRogueRewardChoice RiskReward;
	RiskReward.DisplayName = FText::FromString(TEXT("Glass Cannon"));
	RiskReward.Description = FText::FromString(TEXT("Trades survivability for tempo."));
	RiskReward.DamageBonus = -5.0f;
	RiskReward.MaxHealthBonus = -20.0f;
	RiskReward.MaxMoveBudgetBonus = -3.0f;
	RiskReward.ProjectileBonus = -1;
	RiskReward.ShotPowerMultiplierBonus = -0.25f;
	const FString RiskRewardSummary = RiskReward.GetEffectSummary().ToString();
	TestTrue(TEXT("Reward summary includes reward display names"), RiskRewardSummary.Contains(TEXT("reward Glass Cannon")));
	TestTrue(TEXT("Reward summary includes reward descriptions"), RiskRewardSummary.Contains(TEXT("Trades survivability for tempo.")));
	TestTrue(TEXT("Reward summary names negative damage bonuses"), RiskRewardSummary.Contains(TEXT("damage -5")));
	TestTrue(TEXT("Reward summary names negative max health bonuses"), RiskRewardSummary.Contains(TEXT("max HP -20")));
	TestTrue(TEXT("Reward summary names negative move bonuses"), RiskRewardSummary.Contains(TEXT("move -3")));
	TestTrue(TEXT("Reward summary names negative projectile bonuses"), RiskRewardSummary.Contains(TEXT("projectiles -1")));
	TestTrue(TEXT("Reward summary names negative shot power bonuses"), RiskRewardSummary.Contains(TEXT("shot power -0.25")));
	FFortRogueRewardChoice TaggedReward;
	TaggedReward.RewardTag = FortRogueGameplayTags::Trait_Damage;
	TestTrue(TEXT("Reward summary names reward tags"), TaggedReward.GetEffectSummary().ToString().Contains(TEXT("reward tag Trait.Damage")));

	UFortRogueTerrainMapDefinition* CorruptMap = NewObject<UFortRogueTerrainMapDefinition>();
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

	UFortRogueTerrainMapDefinition* StrokeMap = NewObject<UFortRogueTerrainMapDefinition>();
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

	UFortRogueTerrainMapDefinition* ResizedMap = NewObject<UFortRogueTerrainMapDefinition>();
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFortRogueTerrainMapDefinitionImportTest, "FortRogue.Terrain.MapDefinition.ImportTextureMask", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFortRogueTerrainMapDefinitionImportTest::RunTest(const FString& Parameters)
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

	UFortRogueTerrainMapDefinition* Map = NewObject<UFortRogueTerrainMapDefinition>();
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

	UFortRogueTerrainMapDefinition* ColorMap = NewObject<UFortRogueTerrainMapDefinition>();
	TestNotNull(TEXT("Color import map asset object is created"), ColorMap);
	const bool bColorImported = ColorMap->ImportSolidMaskFromTextureByColor(Texture, FLinearColor::White, 0.01f, 2);
	TestTrue(TEXT("Texture color mask imports"), bColorImported);
	TestEqual(TEXT("Color import updates map width"), ColorMap->CellsX, 4);
	TestEqual(TEXT("Color import updates map height"), ColorMap->CellsZ, 3);
	TestEqual(TEXT("Color-matched source pixel maps to solid terrain"), ColorMap->SolidMask[ColorMap->ToIndex(0, 2)], static_cast<uint8>(1));
	TestEqual(TEXT("Color-matched solid cells receive requested texture layer"), ColorMap->TextureLayerMask[ColorMap->ToIndex(1, 1)], static_cast<uint8>(2));
	TestEqual(TEXT("Nonmatching color cells remain empty"), ColorMap->SolidMask[ColorMap->ToIndex(2, 2)], static_cast<uint8>(0));

	UFortRogueTerrainMapDefinition* ResampledMap = NewObject<UFortRogueTerrainMapDefinition>();
	TestNotNull(TEXT("Resampled import map asset object is created"), ResampledMap);
	ResampledMap->Resize(2, 2);
	const bool bResampledImported = ResampledMap->ImportSolidMaskFromTextureByColor(Texture, FLinearColor::White, 0.01f, 4, false);
	TestTrue(TEXT("Texture color mask imports while keeping current map size"), bResampledImported);
	TestEqual(TEXT("Keep-size import preserves map width"), ResampledMap->CellsX, 2);
	TestEqual(TEXT("Keep-size import preserves map height"), ResampledMap->CellsZ, 2);
	TestEqual(TEXT("Keep-size import samples matching source color"), ResampledMap->SolidMask[ResampledMap->ToIndex(1, 0)], static_cast<uint8>(1));
	TestEqual(TEXT("Keep-size import assigns requested texture layer"), ResampledMap->TextureLayerMask[ResampledMap->ToIndex(1, 0)], static_cast<uint8>(4));

	UFortRogueTerrainMapDefinition* HighResolutionMap = NewObject<UFortRogueTerrainMapDefinition>();
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

	UFortRogueTerrainMapDefinition* UpscaledEdgeMap = NewObject<UFortRogueTerrainMapDefinition>();
	TestNotNull(TEXT("Upscaled edge import map asset object is created"), UpscaledEdgeMap);
	UpscaledEdgeMap->Resize(4, 1);
	const bool bUpscaledEdgeImported = UpscaledEdgeMap->ImportSolidMaskFromTexture(EdgeTexture, true, 0.2f, 3, false);
	TestTrue(TEXT("Low-resolution edge mask imports into a larger map"), bUpscaledEdgeImported);
	TestEqual(TEXT("Upscaled import keeps the first edge cell empty"), UpscaledEdgeMap->SolidMask[UpscaledEdgeMap->ToIndex(0, 0)], static_cast<uint8>(0));
	TestEqual(TEXT("Upscaled import interpolates the edge instead of nearest-only sampling"), UpscaledEdgeMap->SolidMask[UpscaledEdgeMap->ToIndex(1, 0)], static_cast<uint8>(1));
	TestEqual(TEXT("Upscaled import keeps the solid side filled"), UpscaledEdgeMap->SolidMask[UpscaledEdgeMap->ToIndex(2, 0)], static_cast<uint8>(1));
	TestEqual(TEXT("Upscaled import assigns the requested layer on interpolated cells"), UpscaledEdgeMap->TextureLayerMask[UpscaledEdgeMap->ToIndex(1, 0)], static_cast<uint8>(3));

	UFortRogueTerrainMapDefinition* RegionMap = NewObject<UFortRogueTerrainMapDefinition>();
	TestNotNull(TEXT("Region import map asset object is created"), RegionMap);
	const bool bRegionImported = RegionMap->ImportSolidMaskFromTextureRegion(Texture, 1, 1, 2, 2, true, 0.5f, 5);
	TestTrue(TEXT("Texture region alpha mask imports"), bRegionImported);
	TestEqual(TEXT("Region import updates map width to source region width"), RegionMap->CellsX, 2);
	TestEqual(TEXT("Region import updates map height to source region height"), RegionMap->CellsZ, 2);
	TestEqual(TEXT("Region top-left source pixel maps to top map cell"), RegionMap->SolidMask[RegionMap->ToIndex(0, 1)], static_cast<uint8>(1));
	TestEqual(TEXT("Region nonmatching source pixel remains empty"), RegionMap->SolidMask[RegionMap->ToIndex(1, 1)], static_cast<uint8>(0));
	TestEqual(TEXT("Region imported solid cell receives requested texture layer"), RegionMap->TextureLayerMask[RegionMap->ToIndex(0, 1)], static_cast<uint8>(5));

	UFortRogueTerrainMapDefinition* RegionColorMap = NewObject<UFortRogueTerrainMapDefinition>();
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFortRogueTerrainGameModeMapDefinitionTest, "FortRogue.Terrain.GameMode.UsesMapDefinition", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFortRogueTerrainGameModeMapDefinitionTest::RunTest(const FString& Parameters)
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

	UFortRogueTerrainMapDefinition* Map = NewObject<UFortRogueTerrainMapDefinition>();
	Map->Resize(20, 6);
	Map->CellSize = 10.0f;
	Map->Clear(false);
	Map->FillRect(0, 0, 19, 0, true);
	Map->PlayerSpawnLocal = FVector(-40.0f, 0.0f, 140.0f);
	Map->EnemySpawnLocal = FVector(40.0f, 0.0f, 140.0f);

	FURL URL;
	World->SetGameMode(URL);
	AFortRogueGameMode* GameMode = World->GetAuthGameMode<AFortRogueGameMode>();
	TestNotNull(TEXT("FortRogue game mode is created"), GameMode);
	if (!GameMode)
	{
		CleanupWorld();
		return false;
	}

	FObjectProperty* TerrainMapProperty = FindFProperty<FObjectProperty>(AFortRogueGameMode::StaticClass(), TEXT("TerrainMapDefinition"));
	TestNotNull(TEXT("Game mode exposes a terrain map definition property"), TerrainMapProperty);
	if (!TerrainMapProperty)
	{
		CleanupWorld();
		return false;
	}
	TerrainMapProperty->SetObjectPropertyValue_InContainer(GameMode, Map);

	UFortRogueStageRunDefinition* TestStageRunDefinition = NewObject<UFortRogueStageRunDefinition>(GameMode);
	TestStageRunDefinition->StageCount = 2;
	TestStageRunDefinition->NormalizeStageData();
	UFortRogueCharacterDefinition* TestEnemyDefinition = NewObject<UFortRogueCharacterDefinition>(TestStageRunDefinition);
	TestEnemyDefinition->BattleMapDefinition = Map;
	TestStageRunDefinition->EnemyDefinitionPool = { TestEnemyDefinition };
	GameMode->StageRunDefinition = TestStageRunDefinition;

	if (FFloatProperty* MinWindProperty = FindFProperty<FFloatProperty>(AFortRogueGameMode::StaticClass(), TEXT("MinWind")))
	{
		MinWindProperty->SetPropertyValue_InContainer(GameMode, 120.0f);
	}
	if (FFloatProperty* MaxWindProperty = FindFProperty<FFloatProperty>(AFortRogueGameMode::StaticClass(), TEXT("MaxWind")))
	{
		MaxWindProperty->SetPropertyValue_InContainer(GameMode, 120.0f);
	}

	World->InitializeActorsForPlay(URL);
	World->BeginPlay();

	AFortRogueDestructibleTerrain* SpawnedTerrain = nullptr;
	for (TActorIterator<AFortRogueDestructibleTerrain> It(World); It; ++It)
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
	TestEqual(TEXT("Game mode turn wind can be fixed for deterministic projectile tests"), GameMode->GetWind(), 120.0f);
	TestTrue(TEXT("Game mode wind summary includes signed wind"), GameMode->GetWindSummary().ToString().Contains(TEXT("Wind +120")));
	TestTrue(TEXT("Game mode run progress summary includes stage progress"), GameMode->GetRunProgressSummary().ToString().Contains(TEXT("Stage 1/2")));
	TestTrue(TEXT("Game mode run progress summary includes status text"), GameMode->GetRunProgressSummary().ToString().Contains(GameMode->GetStatusText().ToString()));
	AFortRoguePlayerController* TestPlayerController = World->SpawnActor<AFortRoguePlayerController>(AFortRoguePlayerController::StaticClass());
	TestNotNull(TEXT("FortRogue player controller is spawnable for UI helper tests"), TestPlayerController);
	if (TestPlayerController && GameMode->GetPlayerCharacter())
	{
		float PlayerAttributeValue = -1.0f;
		TestEqual(TEXT("Player controller exposes current battle state"), TestPlayerController->GetCurrentBattleState(), GameMode->GetBattleState());
		TestEqual(TEXT("Player controller exposes current status text"), TestPlayerController->GetCurrentStatusText().ToString(), GameMode->GetStatusText().ToString());
		TestTrue(TEXT("Player controller reads player attributes by tag"), TestPlayerController->TryGetPlayerCombatAttributeValueByTag(FortRogueGameplayTags::Attribute_Health, PlayerAttributeValue));
		TestEqual(TEXT("Player controller attribute value matches player character"), PlayerAttributeValue, GameMode->GetPlayerCharacter()->GetHealth());
		TestEqual(TEXT("Player controller exposes player aim angle"), TestPlayerController->GetPlayerAimAngle(), GameMode->GetPlayerCharacter()->GetAimAngle());
		TestEqual(TEXT("Player controller exposes player shot power"), TestPlayerController->GetPlayerShotPower(), GameMode->GetPlayerCharacter()->GetShotPower());
		TestEqual(TEXT("Player controller exposes player shot charge alpha"), TestPlayerController->GetPlayerShotChargeAlpha(), GameMode->GetPlayerCharacter()->GetShotChargeAlpha());
		TestEqual(TEXT("Player controller exposes player shot charge state"), TestPlayerController->IsPlayerChargingShot(), GameMode->GetPlayerCharacter()->IsChargingShot());
		TestEqual(TEXT("Player controller exposes player shot charge begin state"), TestPlayerController->CanBeginPlayerShotCharge(), GameMode->GetBattleState() == EFortRogueBattleState::PlayerTurn && GameMode->GetPlayerCharacter()->CanBeginShotCharge());
		const float PlayerBaseDamage = GameMode->GetPlayerCharacter()->GetDamageBonus();
		TestTrue(TEXT("Player controller applies player attribute deltas by tag"), TestPlayerController->TryApplyPlayerCombatAttributeDeltaByTag(FortRogueGameplayTags::Attribute_Damage, 3.0f));
		TestEqual(TEXT("Player controller attribute delta updates player character"), GameMode->GetPlayerCharacter()->GetDamageBonus(), PlayerBaseDamage + 3.0f);
		TestFalse(TEXT("Player controller rejects invalid attribute value tags"), TestPlayerController->TryGetPlayerCombatAttributeValueByTag(FGameplayTag(), PlayerAttributeValue));
		TestEqual(TEXT("Rejected player controller attribute value resets output"), PlayerAttributeValue, 0.0f);
		TestFalse(TEXT("Player controller rejects invalid attribute delta tags"), TestPlayerController->TryApplyPlayerCombatAttributeDeltaByTag(FGameplayTag(), 1.0f));
		UFortRogueWeaponDefinition* ControllerWeapon = CreateTestWeaponDefinition(TestPlayerController);
		ControllerWeapon->Weapon.WeaponTag = FortRogueGameplayTags::Weapon_Cluster;
		GameMode->GetPlayerCharacter()->AddWeaponDefinition(ControllerWeapon);
		const int32 ControllerWeaponIndex = GameMode->GetPlayerCharacter()->GetWeaponLoadout().Num() - 1;
		TestTrue(TEXT("Player controller can select weapons by loadout index"), TestPlayerController->CanSelectPlayerWeapon(ControllerWeaponIndex));
		TestTrue(TEXT("Player controller selects weapons by loadout index"), TestPlayerController->SelectPlayerWeaponByIndex(ControllerWeaponIndex));
		TestTrue(TEXT("Player controller weapon index selection updates current weapon"), GameMode->GetPlayerCharacter()->GetCurrentWeapon().WeaponTag.MatchesTagExact(FortRogueGameplayTags::Weapon_Cluster));
		TestFalse(TEXT("Player controller rejects invalid weapon loadout indexes"), TestPlayerController->SelectPlayerWeaponByIndex(INDEX_NONE));
		TestEqual(TEXT("Player controller exposes player weapon loadout count"), TestPlayerController->GetPlayerWeaponLoadout().Num(), GameMode->GetPlayerCharacter()->GetWeaponLoadout().Num());
		TestTrue(TEXT("Player controller exposes current weapon spec"), TestPlayerController->GetPlayerCurrentWeaponSpec().WeaponTag.MatchesTagExact(FortRogueGameplayTags::Weapon_Cluster));
		TestEqual(TEXT("Player controller exposes selected weapon index"), TestPlayerController->GetPlayerSelectedWeaponIndex(), ControllerWeaponIndex);
		const FFortRogueShotSpec ControllerShotSpec = TestPlayerController->GetPlayerCurrentShotSpec();
		TestTrue(TEXT("Player controller exposes current shot weapon tag"), ControllerShotSpec.WeaponTag.MatchesTagExact(FortRogueGameplayTags::Weapon_Cluster));
		TestTrue(TEXT("Player controller exposes current shot effect tags"), ControllerShotSpec.EffectTags.HasTagExact(FortRogueGameplayTags::Weapon_Cluster));
		TestFalse(TEXT("Player controller exposes current shot summary"), TestPlayerController->GetPlayerCurrentShotSummary().ToString().IsEmpty());
		FFortRogueShotModifierSpec CurrentShotConditionModifier;
		CurrentShotConditionModifier.RequiredShotTags.AddTag(FortRogueGameplayTags::Weapon_Cluster);
		TestTrue(TEXT("Player controller checks current shot modifier conditions"), TestPlayerController->DoesPlayerShotModifierMeetCurrentShotConditions(CurrentShotConditionModifier));
		TestTrue(TEXT("Battle character checks current shot modifier conditions"), GameMode->GetPlayerCharacter()->DoesShotModifierMeetCurrentShotConditions(CurrentShotConditionModifier));
		CurrentShotConditionModifier.RequiredShotTags.Reset();
		CurrentShotConditionModifier.RequiredShotTags.AddTag(FortRogueGameplayTags::Weapon_Shell);
		TestFalse(TEXT("Player controller rejects current shot modifier conditions"), TestPlayerController->DoesPlayerShotModifierMeetCurrentShotConditions(CurrentShotConditionModifier));
		TestTrue(TEXT("Player controller reports current shot modifier condition failures"), TestPlayerController->GetPlayerShotModifierCurrentConditionFailureSummary(CurrentShotConditionModifier).ToString().Contains(TEXT("requires shot tag")));
		UFortRogueAbilitySet* ControllerAbilitySet = NewObject<UFortRogueAbilitySet>(TestPlayerController);
		ControllerAbilitySet->DisplayName = FText::FromString(TEXT("Controller Ability Set"));
		ControllerAbilitySet->AbilitySetTag = FortRogueGameplayTags::Trait_ShotModifier;
		TestEqual(TEXT("Player controller ability set count starts empty"), TestPlayerController->GetPlayerGrantedAbilitySetCount(ControllerAbilitySet), 0);
		TestPlayerController->GrantPlayerAbilitySet(ControllerAbilitySet);
		TestPlayerController->GrantPlayerAbilitySet(ControllerAbilitySet);
		TestEqual(TEXT("Player controller counts granted ability sets"), TestPlayerController->GetPlayerGrantedAbilitySetCount(ControllerAbilitySet), 2);
		TestEqual(TEXT("Player controller counts granted ability sets by tag"), TestPlayerController->GetPlayerGrantedAbilitySetCountByTag(FortRogueGameplayTags::Trait_ShotModifier), 2);
		TestTrue(TEXT("Player controller reports granted ability sets by tag"), TestPlayerController->HasPlayerGrantedAbilitySetByTag(FortRogueGameplayTags::Trait_ShotModifier));
		TestEqual(TEXT("Player controller exposes granted ability set list"), TestPlayerController->GetPlayerGrantedAbilitySets().Num(), 2);
		TestTrue(TEXT("Player controller summarizes granted ability sets"), TestPlayerController->GetPlayerGrantedAbilitySetsSummary().ToString().Contains(TEXT("Controller Ability Set")));
		TestTrue(TEXT("Player controller removes one granted ability set"), TestPlayerController->RemovePlayerAbilitySet(ControllerAbilitySet));
		TestEqual(TEXT("Player controller ability set count updates after object removal"), TestPlayerController->GetPlayerGrantedAbilitySetCount(ControllerAbilitySet), 1);
		TestEqual(TEXT("Player controller removes granted ability sets by tag"), TestPlayerController->RemovePlayerAbilitySetsByTag(FortRogueGameplayTags::Trait_ShotModifier), 1);
		TestFalse(TEXT("Player controller reports missing ability sets by tag"), TestPlayerController->HasPlayerGrantedAbilitySetByTag(FortRogueGameplayTags::Trait_ShotModifier));
		FFortRogueShotModifierSpec ControllerModifier;
		ControllerModifier.ModifierTag = FortRogueGameplayTags::ShotEffect_Damage;
		TArray<FFortRogueShotModifierSpec> ControllerModifiers = { ControllerModifier };
		TestPlayerController->GrantPlayerShotModifiers(ControllerModifiers);
		TestEqual(TEXT("Player controller counts granted shot modifiers by tag"), TestPlayerController->GetPlayerGrantedShotModifierCountByTag(FortRogueGameplayTags::ShotEffect_Damage), 1);
		TestTrue(TEXT("Player controller reports granted shot modifiers by tag"), TestPlayerController->HasPlayerGrantedShotModifierByTag(FortRogueGameplayTags::ShotEffect_Damage));
		TestEqual(TEXT("Player controller exposes granted shot modifier list"), TestPlayerController->GetPlayerGrantedShotModifiers().Num(), 1);
		TestTrue(TEXT("Player controller summarizes granted shot modifiers"), TestPlayerController->GetPlayerGrantedShotModifiersSummary().ToString().Contains(TEXT("modifier tag")));
		TestEqual(TEXT("Player controller removes granted shot modifiers by tag"), TestPlayerController->RemovePlayerGrantedShotModifiersByTag(FortRogueGameplayTags::ShotEffect_Damage), 1);
		TestFalse(TEXT("Player controller reports missing granted shot modifiers by tag"), TestPlayerController->HasPlayerGrantedShotModifierByTag(FortRogueGameplayTags::ShotEffect_Damage));
		TestPlayerController->GrantPlayerPendingShotModifiers(ControllerModifiers);
		TestEqual(TEXT("Player controller directly grants pending shot modifiers by tag"), TestPlayerController->GetPlayerPendingShotModifierCountByTag(FortRogueGameplayTags::ShotEffect_Damage), 1);
		TestEqual(TEXT("Player controller exposes pending shot modifier list"), TestPlayerController->GetPlayerPendingShotModifiers().Num(), 1);
		TestTrue(TEXT("Player controller summarizes pending shot modifiers"), TestPlayerController->GetPlayerPendingShotModifiersSummary().ToString().Contains(TEXT("modifier tag")));
		TestEqual(TEXT("Player controller removes directly granted pending shot modifiers"), TestPlayerController->RemovePlayerPendingShotModifiersByTag(FortRogueGameplayTags::ShotEffect_Damage), 1);
		UFortRogueItemDefinition* ControllerPendingItem = NewObject<UFortRogueItemDefinition>(TestPlayerController);
		ControllerPendingItem->ItemType = EFortRogueItemType::AbilitySet;
		ControllerPendingItem->ItemTag = FortRogueGameplayTags::Item_NextShot;
		ControllerPendingItem->UseShotModifiers.Add(ControllerModifier);
		GameMode->GetPlayerCharacter()->AddItemDefinition(ControllerPendingItem, 1);
		const int32 ControllerPendingItemIndex = GameMode->GetPlayerCharacter()->GetItemLoadout().Num() - 1;
		TestEqual(TEXT("Player controller exposes player item loadout count"), TestPlayerController->GetPlayerItemLoadout().Num(), GameMode->GetPlayerCharacter()->GetItemLoadout().Num());
		TestTrue(TEXT("Player controller counts player item charges by type"), TestPlayerController->GetPlayerItemCharges(EFortRogueItemType::AbilitySet) >= 1);
		TestEqual(TEXT("Player controller counts player item charges by tag"), TestPlayerController->GetPlayerItemChargesByTag(FortRogueGameplayTags::Item_NextShot), 1);
		TestPlayerController->UsePlayerItemByIndex(ControllerPendingItemIndex);
		TestEqual(TEXT("Player controller item charge tags update after use"), TestPlayerController->GetPlayerItemChargesByTag(FortRogueGameplayTags::Item_NextShot), 0);
		TestEqual(TEXT("Player controller counts pending shot modifiers by tag"), TestPlayerController->GetPlayerPendingShotModifierCountByTag(FortRogueGameplayTags::ShotEffect_Damage), 1);
		TestTrue(TEXT("Player controller reports pending shot modifiers by tag"), TestPlayerController->HasPlayerPendingShotModifierByTag(FortRogueGameplayTags::ShotEffect_Damage));
		TestEqual(TEXT("Player controller exposes item-granted pending shot modifiers"), TestPlayerController->GetPlayerPendingShotModifiers().Num(), 1);
		TestEqual(TEXT("Player controller removes pending shot modifiers by tag"), TestPlayerController->RemovePlayerPendingShotModifiersByTag(FortRogueGameplayTags::ShotEffect_Damage), 1);
		TestFalse(TEXT("Player controller reports missing pending shot modifiers by tag"), TestPlayerController->HasPlayerPendingShotModifierByTag(FortRogueGameplayTags::ShotEffect_Damage));
	}
	if (AFortRogueBattleCharacter* PlayerCharacter = GameMode->GetPlayerCharacter())
	{
		const float BaseDamageBonus = PlayerCharacter->GetDamageBonus();
		PlayerCharacter->ApplyRewardDamage(10.0f);
		FFortRogueRewardChoice NegativeDamageReward;
		NegativeDamageReward.DamageBonus = -4.0f;
		GameMode->ApplyRewardToPlayer(NegativeDamageReward);
		TestEqual(TEXT("Game mode applies negative direct reward deltas"), PlayerCharacter->GetDamageBonus(), BaseDamageBonus + 6.0f);
	}

	auto HasRewardChoiceTag = [](const TArray<FFortRogueRewardChoice>& RewardChoices, FGameplayTag RewardTag)
	{
		for (const FFortRogueRewardChoice& RewardChoice : RewardChoices)
		{
			if (RewardChoice.RewardTag.MatchesTagExact(RewardTag))
			{
				return true;
			}
		}
		return false;
	};

	FFortRogueRewardChoice BaseReward;
	BaseReward.DisplayName = FText::FromString(TEXT("Damage Seed"));
	BaseReward.RewardTag = FortRogueGameplayTags::Trait_Damage;
	FFortRogueRewardChoice RequiredReward;
	RequiredReward.DisplayName = FText::FromString(TEXT("Projectile Branch"));
	RequiredReward.RewardTag = FortRogueGameplayTags::Trait_Projectiles;
	RequiredReward.RequiredRewardTags.AddTag(FortRogueGameplayTags::Trait_Damage);
	FFortRogueRewardChoice BlockedReward;
	BlockedReward.DisplayName = FText::FromString(TEXT("Health Branch"));
	BlockedReward.RewardTag = FortRogueGameplayTags::Trait_Health;
	BlockedReward.BlockedRewardTags.AddTag(FortRogueGameplayTags::Trait_Damage);
	FFortRogueRewardChoice OpenReward;
	OpenReward.DisplayName = FText::FromString(TEXT("Open Branch"));
	OpenReward.RewardTag = FortRogueGameplayTags::Trait_ShotModifier;
	TestStageRunDefinition->RewardChoiceCount = 5;
	TestStageRunDefinition->RewardPool = { BaseReward, RequiredReward, BlockedReward, OpenReward };

	GameMode->ChosenRewardTags.Reset();
	GameMode->BuildRewardChoices();
	FGameplayTagContainer EmptyChosenRewardTags;
	TestFalse(TEXT("Reward condition helper rejects missing required tags"), RequiredReward.MeetsRewardTagConditions(EmptyChosenRewardTags));
	TestFalse(TEXT("Blueprint reward condition helper rejects missing required tags"), UFortRogueRewardBlueprintLibrary::DoesRewardMeetTagConditions(RequiredReward, EmptyChosenRewardTags));
	TestTrue(TEXT("Reward condition failure summary names missing required tags"), RequiredReward.GetRewardTagConditionFailureSummary(EmptyChosenRewardTags).ToString().Contains(TEXT("requires reward")));
	TestFalse(TEXT("Reward choices hide rewards before required reward tags are chosen"), HasRewardChoiceTag(GameMode->GetRewardChoices(), FortRogueGameplayTags::Trait_Projectiles));
	TestTrue(TEXT("Reward choices keep rewards whose blocked tags are absent"), HasRewardChoiceTag(GameMode->GetRewardChoices(), FortRogueGameplayTags::Trait_Health));
	GameMode->RewardChoices = { RequiredReward, BlockedReward };
	TestTrue(TEXT("Game mode reward choice failure summary names missing required tags"), GameMode->GetRewardChoiceConditionFailureSummary(0).ToString().Contains(TEXT("requires reward")));
	TestTrue(TEXT("Game mode reward choice failure summary is empty for invalid choices"), GameMode->GetRewardChoiceConditionFailureSummary(GameMode->GetRewardChoices().Num()).ToString().IsEmpty());
	GameMode->BattleState = EFortRogueBattleState::Reward;
	TestFalse(TEXT("Game mode rejects reward choices with unmet required tags"), GameMode->CanApplyRewardChoice(0));
	if (TestPlayerController)
	{
		TestEqual(TEXT("Player controller exposes current reward choice count"), TestPlayerController->GetCurrentRewardChoiceCount(), GameMode->GetRewardChoiceCount());
		TestEqual(TEXT("Player controller exposes current reward choices"), TestPlayerController->GetCurrentRewardChoices().Num(), GameMode->GetRewardChoices().Num());
		TestTrue(TEXT("Player controller exposes reward condition failure summaries"), TestPlayerController->GetCurrentRewardChoiceConditionFailureSummary(0).ToString().Contains(TEXT("requires reward")));
		TestFalse(TEXT("Player controller rejects reward choices with unmet required tags"), TestPlayerController->CanChooseReward(0));
	}
	GameMode->BattleState = EFortRogueBattleState::PlayerTurn;
	if (TestPlayerController)
	{
		TestFalse(TEXT("Player controller rejects rewards outside reward state"), TestPlayerController->CanChooseReward(0));
		TestFalse(TEXT("Player controller does not choose rewards outside reward state"), TestPlayerController->ChooseRewardByIndex(0));
	}

	GameMode->ChosenRewardTags = { FortRogueGameplayTags::Trait_Damage };
	TestTrue(TEXT("Game mode exposes chosen reward tags for UI"), GameMode->GetChosenRewardTags().HasTagExact(FortRogueGameplayTags::Trait_Damage));
	TestTrue(TEXT("Reward condition helper accepts satisfied required tags"), RequiredReward.MeetsRewardTagConditions(GameMode->GetChosenRewardTags()));
	TestFalse(TEXT("Reward condition helper rejects blocked chosen tags"), BlockedReward.MeetsRewardTagConditions(GameMode->GetChosenRewardTags()));
	TestTrue(TEXT("Blueprint reward condition failure summary names blocked tags"), UFortRogueRewardBlueprintLibrary::GetRewardTagConditionFailureSummary(BlockedReward, GameMode->GetChosenRewardTags()).ToString().Contains(TEXT("blocked by reward")));
	TestTrue(TEXT("Reward condition failure summary is empty when conditions pass"), RequiredReward.GetRewardTagConditionFailureSummary(GameMode->GetChosenRewardTags()).ToString().IsEmpty());
	TestTrue(TEXT("Game mode reward choice failure summary names blocked tags"), GameMode->GetRewardChoiceConditionFailureSummary(1).ToString().Contains(TEXT("blocked by reward")));
	TestTrue(TEXT("Game mode reward choice failure summary is empty when conditions pass"), GameMode->GetRewardChoiceConditionFailureSummary(0).ToString().IsEmpty());
	GameMode->BattleState = EFortRogueBattleState::Reward;
	TestTrue(TEXT("Game mode accepts reward choices with satisfied required tags"), GameMode->CanApplyRewardChoice(0));
	TestFalse(TEXT("Game mode rejects reward choices with blocked tags"), GameMode->CanApplyRewardChoice(1));
	GameMode->BattleState = EFortRogueBattleState::PlayerTurn;
	GameMode->BuildRewardChoices();
	TestTrue(TEXT("Reward choices include rewards after required reward tags are chosen"), HasRewardChoiceTag(GameMode->GetRewardChoices(), FortRogueGameplayTags::Trait_Projectiles));
	TestFalse(TEXT("Reward choices hide rewards blocked by chosen reward tags"), HasRewardChoiceTag(GameMode->GetRewardChoices(), FortRogueGameplayTags::Trait_Health));
	TestEqual(TEXT("Game mode reward choice count matches current choices"), GameMode->GetRewardChoiceCount(), GameMode->GetRewardChoices().Num());
	TestTrue(TEXT("Game mode returns reward choices by index"), GameMode->GetRewardChoice(0).RewardTag.IsValid());
	TestFalse(TEXT("Game mode returns default reward choices for invalid indexes"), GameMode->GetRewardChoice(GameMode->GetRewardChoiceCount()).RewardTag.IsValid());
	TestFalse(TEXT("Game mode rejects reward choice outside reward state"), GameMode->CanApplyRewardChoice(0));
	TestFalse(TEXT("Game mode reward choice summary is available for valid choices"), GameMode->GetRewardChoiceSummary(0).ToString().IsEmpty());
	TestTrue(TEXT("Game mode reward choice summary is empty for invalid choices"), GameMode->GetRewardChoiceSummary(GameMode->GetRewardChoices().Num()).ToString().IsEmpty());
	if (TestPlayerController)
	{
		TestTrue(TEXT("Player controller exposes chosen reward tags"), TestPlayerController->GetChosenRewardTags().HasTagExact(FortRogueGameplayTags::Trait_Damage));
		TestTrue(TEXT("Player controller returns reward choices by index"), TestPlayerController->GetCurrentRewardChoice(0).RewardTag.IsValid());
		TestFalse(TEXT("Player controller reward choice summary is available for valid choices"), TestPlayerController->GetCurrentRewardChoiceSummary(0).ToString().IsEmpty());
		TestTrue(TEXT("Player controller reward choice failure summary is empty for valid choices"), TestPlayerController->GetCurrentRewardChoiceConditionFailureSummary(0).ToString().IsEmpty());
	}
	GameMode->BattleState = EFortRogueBattleState::Reward;
	TestTrue(TEXT("Game mode reports valid reward choices selectable"), GameMode->CanApplyRewardChoice(0));
	TestFalse(TEXT("Game mode rejects negative reward choice indexes"), GameMode->CanApplyRewardChoice(-1));
	TestFalse(TEXT("Game mode rejects reward choice indexes past the end"), GameMode->CanApplyRewardChoice(GameMode->GetRewardChoices().Num()));
	if (TestPlayerController)
	{
		TestTrue(TEXT("Player controller reports valid reward choices selectable"), TestPlayerController->CanChooseReward(0));
		TestFalse(TEXT("Player controller rejects invalid reward choice indexes"), TestPlayerController->CanChooseReward(GameMode->GetRewardChoices().Num()));
	}
	const int32 StageBeforeRewardChoice = GameMode->GetCurrentStage();
	const FGameplayTag AppliedRewardTag = GameMode->GetRewardChoice(0).RewardTag;
	if (TestPlayerController)
	{
		TestTrue(TEXT("Player controller chooses rewards by index"), TestPlayerController->ChooseRewardByIndex(0));
	}
	else
	{
		GameMode->ApplyRewardChoice(0);
	}
	TestTrue(TEXT("Game mode records applied reward tags"), AppliedRewardTag.IsValid() && GameMode->GetChosenRewardTags().HasTagExact(AppliedRewardTag));
	TestEqual(TEXT("Game mode advances to the next stage after reward choice"), GameMode->GetCurrentStage(), StageBeforeRewardChoice + 1);
	TestEqual(TEXT("Game mode clears reward choices after applying one"), GameMode->GetRewardChoiceCount(), 0);
	TestEqual(TEXT("Game mode returns to player turn after reward choice"), GameMode->GetBattleState(), EFortRogueBattleState::PlayerTurn);
	GameMode->BattleState = EFortRogueBattleState::PlayerTurn;
	TestStageRunDefinition->RewardPool.Reset();
	GameMode->RewardChoices.Reset();
	GameMode->ChosenRewardTags.Reset();

	AFortRogueProjectile* WindProjectile = World->SpawnActor<AFortRogueProjectile>(AFortRogueProjectile::StaticClass(), FVector(0.0f, 0.0f, 500.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Wind test projectile is spawned"), WindProjectile);
	if (WindProjectile)
	{
		WindProjectile->InitializeProjectile(nullptr, nullptr, FVector::ZeroVector, 0.0f, 0.0f, 0.0f);
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
		GameMode->GetEnemyCharacter()->ReevaluateTerrainSupport();

		GameMode->StartEnemyTurn();
		TestEqual(TEXT("Game mode can enter the enemy turn for an unsupported enemy"), GameMode->GetBattleState(), EFortRogueBattleState::EnemyTurn);
		GameMode->RunEnemyTurn();
		TestEqual(TEXT("Unsupported enemy that cannot fire yields the turn back to the player"), GameMode->GetBattleState(), EFortRogueBattleState::PlayerTurn);
	}

	AFortRogueProjectile* StrayProjectile = World->SpawnActor<AFortRogueProjectile>(AFortRogueProjectile::StaticClass(), FVector(25.0f, 0.0f, 500.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Stray projectile is spawned"), StrayProjectile);
	if (StrayProjectile)
	{
		TestEqual(TEXT("Game mode starts on the player turn before stray projectile resolution"), GameMode->GetBattleState(), EFortRogueBattleState::PlayerTurn);
		GameMode->NotifyProjectileResolved(StrayProjectile);
		World->Tick(ELevelTick::LEVELTICK_All, 1.0f);
		TestEqual(TEXT("Stray projectile resolution outside a shot does not advance the turn"), GameMode->GetBattleState(), EFortRogueBattleState::PlayerTurn);
	}

	UFortRogueTerrainMapDefinition* NextStageMap = NewObject<UFortRogueTerrainMapDefinition>();
	NextStageMap->Resize(16, 8);
	NextStageMap->CellSize = 12.0f;
	NextStageMap->Clear(false);
	NextStageMap->FillRect(0, 0, 15, 1, true);
	NextStageMap->PlayerSpawnLocal = FVector(-36.0f, 0.0f, 140.0f);
	NextStageMap->EnemySpawnLocal = FVector(36.0f, 0.0f, 140.0f);

	UFortRogueCharacterDefinition* NextStageEnemyDefinition = NewObject<UFortRogueCharacterDefinition>();
	NextStageEnemyDefinition->DisplayName = FText::FromString(TEXT("Map Carrier"));
	NextStageEnemyDefinition->BattleMapDefinition = NextStageMap;

	TestStageRunDefinition->EnemyDefinitionPool = { NextStageEnemyDefinition };
	TestEqual(TEXT("Game mode begins on the first stage before enemy defeat transition"), GameMode->GetCurrentStage(), 1);
	TestNotNull(TEXT("Game mode has an enemy before stage transition"), GameMode->GetEnemyCharacter());
	if (GameMode->GetEnemyCharacter())
	{
		GameMode->GetEnemyCharacter()->ApplyDamage(100000.0f);
		GameMode->CheckTurnDefeatState();
		TestEqual(TEXT("Defeating an enemy advances to the next stage without entering rewards"), GameMode->GetCurrentStage(), 2);
		TestEqual(TEXT("Next stage starts on the player turn"), GameMode->GetBattleState(), EFortRogueBattleState::PlayerTurn);
		TestTrue(TEXT("Next stage uses the encountered enemy character definition"), GameMode->CurrentEnemyDefinition == NextStageEnemyDefinition);
		TestNotNull(TEXT("Next stage respawns terrain"), GameMode->Terrain.Get());
		if (GameMode->Terrain)
		{
			TestEqual(TEXT("Next stage terrain follows the selected enemy map definition"), GameMode->Terrain->MapDefinition.Get(), NextStageMap);
			TestEqual(TEXT("Next stage terrain width follows the selected enemy map definition"), GameMode->Terrain->Width, 192.0f);
		}
	}

	CleanupWorld();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFortRogueDestructibleTerrainRuntimeTest, "FortRogue.Terrain.DestructibleTerrain.RuntimeQueries", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFortRogueDestructibleTerrainRuntimeTest::RunTest(const FString& Parameters)
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

	UFortRogueTerrainMapDefinition* Map = NewObject<UFortRogueTerrainMapDefinition>();
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

	UFortRogueTerrainMapDefinition* OverlappingMap = NewObject<UFortRogueTerrainMapDefinition>();
	OverlappingMap->Resize(20, 6);
	OverlappingMap->CellSize = 10.0f;
	OverlappingMap->Clear(false);
	OverlappingMap->FillRect(0, 0, 19, 0, true);

	UFortRogueTerrainMapDefinition* FastProjectileMap = NewObject<UFortRogueTerrainMapDefinition>();
	FastProjectileMap->Resize(10, 6);
	FastProjectileMap->CellSize = 10.0f;
	FastProjectileMap->Clear(false);
	FastProjectileMap->FillRect(5, 3, 5, 3, true);

	UFortRogueTerrainMapDefinition* GapMap = NewObject<UFortRogueTerrainMapDefinition>();
	GapMap->Resize(12, 4);
	GapMap->CellSize = 10.0f;
	GapMap->Clear(false);
	GapMap->FillRect(0, 0, 4, 0, true);
	GapMap->FillRect(7, 0, 11, 0, true);

	FActorSpawnParameters SpawnParams;
	AFortRogueDestructibleTerrain* Terrain = World->SpawnActor<AFortRogueDestructibleTerrain>(AFortRogueDestructibleTerrain::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
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

	AFortRogueDestructibleTerrain* OverlappingTerrain = World->SpawnActor<AFortRogueDestructibleTerrain>(AFortRogueDestructibleTerrain::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Overlapping terrain actor is spawned"), OverlappingTerrain);
	if (OverlappingTerrain)
	{
		OverlappingTerrain->MapDefinition = OverlappingMap;
	}

	AFortRogueDestructibleTerrain* FastProjectileTerrain = World->SpawnActor<AFortRogueDestructibleTerrain>(AFortRogueDestructibleTerrain::StaticClass(), FVector(300.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Fast projectile test terrain actor is spawned"), FastProjectileTerrain);
	if (FastProjectileTerrain)
	{
		FastProjectileTerrain->MapDefinition = FastProjectileMap;
	}

	AFortRogueDestructibleTerrain* GapTerrain = World->SpawnActor<AFortRogueDestructibleTerrain>(AFortRogueDestructibleTerrain::StaticClass(), FVector(500.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Gap movement test terrain actor is spawned"), GapTerrain);
	if (GapTerrain)
	{
		GapTerrain->MapDefinition = GapMap;
	}

	UFortRogueTerrainMapDefinition* TransformMap = NewObject<UFortRogueTerrainMapDefinition>();
	TransformMap->Resize(4, 2);
	TransformMap->CellSize = 10.0f;
	TransformMap->Clear(true);

	AFortRogueDestructibleTerrain* TransformTerrain = World->SpawnActorDeferred<AFortRogueDestructibleTerrain>(
		AFortRogueDestructibleTerrain::StaticClass(),
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

	UFortRogueTerrainMapDefinition* RuntimeCorruptMap = NewObject<UFortRogueTerrainMapDefinition>();
	RuntimeCorruptMap->CellsX = 3;
	RuntimeCorruptMap->CellsZ = 2;
	RuntimeCorruptMap->CellSize = 0.0f;
	RuntimeCorruptMap->SolidMask = { 2, 0, 1 };
	RuntimeCorruptMap->TextureLayerMask = { 4, 7 };

	AFortRogueDestructibleTerrain* RuntimeCorruptTerrain = World->SpawnActorDeferred<AFortRogueDestructibleTerrain>(
		AFortRogueDestructibleTerrain::StaticClass(),
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

	UFortRogueTerrainMapDefinition* PreviewMap = NewObject<UFortRogueTerrainMapDefinition>();
	PreviewMap->Resize(6, 3);
	PreviewMap->CellSize = 10.0f;
	PreviewMap->Clear(false);
	PreviewMap->FillTexturedRect(2, 1, 2, 1, 4);

	AFortRogueDestructibleTerrain* PreviewTerrain = World->SpawnActorDeferred<AFortRogueDestructibleTerrain>(AFortRogueDestructibleTerrain::StaticClass(), FTransform(FRotator::ZeroRotator, FVector(700.0f, 0.0f, 0.0f)));
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

	UFortRogueTerrainMapDefinition* FillRuntimeMap = NewObject<UFortRogueTerrainMapDefinition>();
	FillRuntimeMap->Resize(4, 3);
	FillRuntimeMap->CellSize = 10.0f;
	FillRuntimeMap->Clear(false);
	AFortRogueDestructibleTerrain* FillRuntimeTerrain = World->SpawnActorDeferred<AFortRogueDestructibleTerrain>(AFortRogueDestructibleTerrain::StaticClass(), FTransform(FRotator::ZeroRotator, FVector(1200.0f, 0.0f, 0.0f)));
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

	AFortRogueBattleCharacter* Character = World->SpawnActor<AFortRogueBattleCharacter>(AFortRogueBattleCharacter::StaticClass(), FVector(-5.0f, 0.0f, 55.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Battle character is spawned"), Character);
	if (Character)
	{
		TestEqual(TEXT("Battle character actor starts facing right"), Character->GetActorRotation(), FRotator::ZeroRotator);
		if (UPrimitiveComponent* Body = Cast<UPrimitiveComponent>(Character->GetDefaultSubobjectByName(TEXT("Body"))))
		{
			TestEqual(TEXT("Battle character body does not use Unreal collision"), Body->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
		}
		UPaperFlipbookComponent* BodySprite = Cast<UPaperFlipbookComponent>(Character->GetDefaultSubobjectByName(TEXT("BodySprite")));
		TestNotNull(TEXT("Battle character sprite component exists"), BodySprite);
		if (BodySprite)
		{
			TestEqual(TEXT("Battle character sprite does not use Unreal collision"), BodySprite->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
			TestEqual(TEXT("Battle character sprite inherits actor facing"), BodySprite->GetRelativeRotation(), FRotator::ZeroRotator);
			TestEqual(TEXT("Battle character sprite bottom is aligned to the character foot offset"), static_cast<float>(BodySprite->GetRelativeLocation().Z), -45.0f);
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
		for (TActorIterator<AFortRogueProjectile> It(World); It; ++It)
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
	FFortRogueShotModifierSpec RuntimeSplitChildModifier;
	RuntimeSplitChildModifier.DisplayName = FText::FromString(TEXT("Runtime Split Child"));
	RuntimeSplitChildModifier.ProjectileEffects.Add(RuntimeSplitChildDrillEffect);
	RuntimeSplitChildModifier.ProjectileEffects.Add(RuntimeSplitChildTerrainEffect);
	FFRProjectileEffectSplitParams RuntimeBlockedNestedSplitParams;
	RuntimeBlockedNestedSplitParams.ProjectileCount = 1;
	RuntimeBlockedNestedSplitParams.LaunchSpeed = 180.0f;
	FFRProjectileEffectSpec RuntimeBlockedNestedSplitEffect;
	RuntimeBlockedNestedSplitEffect.EffectClass = UFRProjectileEffectSplit::StaticClass();
	RuntimeBlockedNestedSplitEffect.Parameters = FInstancedStruct::Make(RuntimeBlockedNestedSplitParams);
	FFortRogueShotModifierSpec RuntimeBlockedSplitChildModifier;
	RuntimeBlockedSplitChildModifier.DisplayName = FText::FromString(TEXT("Runtime Blocked Split Child"));
	RuntimeBlockedSplitChildModifier.BlockedShotTags.AddTag(FortRogueGameplayTags::ShotEffect_Drill);
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
	AFortRogueProjectile* RuntimeSplitChildProjectile = nullptr;
	for (TActorIterator<AFortRogueProjectile> It(World); It; ++It)
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

	auto GetCombatSet = [](AFortRogueBattleCharacter* BattleCharacter)
	{
		if (!BattleCharacter)
		{
			return static_cast<UFortRogueCombatSet*>(nullptr);
		}

		if (FObjectProperty* Property = FindFProperty<FObjectProperty>(BattleCharacter->GetClass(), TEXT("CombatSet")))
		{
			return Cast<UFortRogueCombatSet>(Property->GetObjectPropertyValue_InContainer(BattleCharacter));
		}

		return static_cast<UFortRogueCombatSet*>(nullptr);
	};

	AFortRogueBattleCharacter* StatCharacter = World->SpawnActor<AFortRogueBattleCharacter>(AFortRogueBattleCharacter::StaticClass(), FVector(-15.0f, 0.0f, 55.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Stat battle character is spawned"), StatCharacter);
	if (StatCharacter)
	{
		UFortRogueCharacterDefinition* CharacterDefinition = NewObject<UFortRogueCharacterDefinition>();
		CharacterDefinition->MaxHealth = 175.0f;
		CharacterDefinition->BonusDamage = 12.0f;
		CharacterDefinition->MaxMoveBudget = 25.0f;
		CharacterDefinition->ShotPowerMultiplier = 1.8f;
		StatCharacter->InitializeFromDefinition(CharacterDefinition);
		if (UFortRogueCombatSet* CombatSet = GetCombatSet(StatCharacter))
		{
			TestEqual(TEXT("Character definition controls max health"), CombatSet->GetMaxHealth(), 175.0f);
			TestEqual(TEXT("Character definition controls turn movement budget"), CombatSet->GetMaxMoveBudget(), 25.0f);
			TestEqual(TEXT("Character definition controls shot power multiplier"), CombatSet->GetShotPowerMultiplier(), 1.8f);
		}
		TestEqual(TEXT("Battle character getter exposes max move budget"), StatCharacter->GetMaxMoveBudget(), 25.0f);
		TestEqual(TEXT("Battle character getter exposes damage bonus"), StatCharacter->GetDamageBonus(), 12.0f);
		TestEqual(TEXT("Battle character getter exposes shot power multiplier"), StatCharacter->GetShotPowerMultiplier(), 1.8f);
		TestEqual(TEXT("Battle character getter exposes base projectile count"), StatCharacter->GetProjectileCount(), 1.0f);
		float TaggedAttributeValue = -1.0f;
		TestTrue(TEXT("Battle character reads health by attribute tag"), StatCharacter->TryGetCombatAttributeValueByTag(FortRogueGameplayTags::Attribute_Health, TaggedAttributeValue));
		TestEqual(TEXT("Tagged health value matches getter"), TaggedAttributeValue, 175.0f);
		TestTrue(TEXT("Battle character reads max health by attribute tag"), StatCharacter->TryGetCombatAttributeValueByTag(FortRogueGameplayTags::Attribute_MaxHealth, TaggedAttributeValue));
		TestEqual(TEXT("Tagged max health value matches getter"), TaggedAttributeValue, 175.0f);
		TestTrue(TEXT("Battle character reads move budget by attribute tag"), StatCharacter->TryGetCombatAttributeValueByTag(FortRogueGameplayTags::Attribute_MoveBudget, TaggedAttributeValue));
		TestEqual(TEXT("Tagged move budget value matches getter"), TaggedAttributeValue, 25.0f);
		TestTrue(TEXT("Battle character reads max move budget by attribute tag"), StatCharacter->TryGetCombatAttributeValueByTag(FortRogueGameplayTags::Attribute_MaxMoveBudget, TaggedAttributeValue));
		TestEqual(TEXT("Tagged max move budget value matches getter"), TaggedAttributeValue, 25.0f);
		TestTrue(TEXT("Battle character reads damage by attribute tag"), StatCharacter->TryGetCombatAttributeValueByTag(FortRogueGameplayTags::Attribute_Damage, TaggedAttributeValue));
		TestEqual(TEXT("Tagged damage value matches getter"), TaggedAttributeValue, 12.0f);
		TestTrue(TEXT("Battle character reads shot power multiplier by attribute tag"), StatCharacter->TryGetCombatAttributeValueByTag(FortRogueGameplayTags::Attribute_ShotPowerMultiplier, TaggedAttributeValue));
		TestEqual(TEXT("Tagged shot power multiplier value matches getter"), TaggedAttributeValue, 1.8f);
		TestTrue(TEXT("Battle character reads projectile count by attribute tag"), StatCharacter->TryGetCombatAttributeValueByTag(FortRogueGameplayTags::Attribute_ProjectileCount, TaggedAttributeValue));
		TestEqual(TEXT("Tagged projectile count value matches getter"), TaggedAttributeValue, 1.0f);
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
		TestTrue(TEXT("Battle character applies health deltas by attribute tag"), StatCharacter->TryApplyCombatAttributeDeltaByTag(FortRogueGameplayTags::Attribute_Health, -25.0f));
		TestEqual(TEXT("Tagged health delta damages current health"), StatCharacter->GetHealth(), 150.0f);
		TestTrue(TEXT("Battle character applies healing deltas by attribute tag"), StatCharacter->TryApplyCombatAttributeDeltaByTag(FortRogueGameplayTags::Attribute_Health, 1000.0f));
		TestEqual(TEXT("Tagged healing delta clamps to max health"), StatCharacter->GetHealth(), 175.0f);
		TestTrue(TEXT("Battle character applies max health deltas by attribute tag"), StatCharacter->TryApplyCombatAttributeDeltaByTag(FortRogueGameplayTags::Attribute_MaxHealth, -1000.0f));
		TestEqual(TEXT("Tagged max health delta clamps to its minimum"), StatCharacter->GetMaxHealth(), 1.0f);
		TestTrue(TEXT("Battle character restores max health by attribute tag"), StatCharacter->TryApplyCombatAttributeDeltaByTag(FortRogueGameplayTags::Attribute_MaxHealth, 174.0f));
		TestEqual(TEXT("Tagged max health restore updates max health"), StatCharacter->GetMaxHealth(), 175.0f);
		TestTrue(TEXT("Battle character applies move budget deltas by attribute tag"), StatCharacter->TryApplyCombatAttributeDeltaByTag(FortRogueGameplayTags::Attribute_MoveBudget, -500.0f));
		TestEqual(TEXT("Tagged move budget delta clamps to zero"), StatCharacter->GetMoveBudget(), 0.0f);
		TestTrue(TEXT("Battle character restores move budget by attribute tag"), StatCharacter->TryApplyCombatAttributeDeltaByTag(FortRogueGameplayTags::Attribute_MoveBudget, 500.0f));
		TestEqual(TEXT("Tagged move budget restore clamps to max move"), StatCharacter->GetMoveBudget(), 25.0f);
		TestTrue(TEXT("Battle character applies max move deltas by attribute tag"), StatCharacter->TryApplyCombatAttributeDeltaByTag(FortRogueGameplayTags::Attribute_MaxMoveBudget, -500.0f));
		TestEqual(TEXT("Tagged max move delta clamps to zero"), StatCharacter->GetMaxMoveBudget(), 0.0f);
		TestTrue(TEXT("Battle character restores max move by attribute tag"), StatCharacter->TryApplyCombatAttributeDeltaByTag(FortRogueGameplayTags::Attribute_MaxMoveBudget, 25.0f));
		TestEqual(TEXT("Tagged max move restore updates move budget"), StatCharacter->GetMoveBudget(), 25.0f);
		TestTrue(TEXT("Battle character applies damage deltas by attribute tag"), StatCharacter->TryApplyCombatAttributeDeltaByTag(FortRogueGameplayTags::Attribute_Damage, -1000.0f));
		TestEqual(TEXT("Tagged damage delta clamps to zero"), StatCharacter->GetDamageBonus(), 0.0f);
		TestTrue(TEXT("Battle character restores damage by attribute tag"), StatCharacter->TryApplyCombatAttributeDeltaByTag(FortRogueGameplayTags::Attribute_Damage, 12.0f));
		TestEqual(TEXT("Tagged damage restore updates damage"), StatCharacter->GetDamageBonus(), 12.0f);
		TestTrue(TEXT("Battle character applies shot power deltas by attribute tag"), StatCharacter->TryApplyCombatAttributeDeltaByTag(FortRogueGameplayTags::Attribute_ShotPowerMultiplier, -1000.0f));
		TestEqual(TEXT("Tagged shot power delta clamps to zero"), StatCharacter->GetShotPowerMultiplier(), 0.0f);
		TestTrue(TEXT("Battle character restores shot power by attribute tag"), StatCharacter->TryApplyCombatAttributeDeltaByTag(FortRogueGameplayTags::Attribute_ShotPowerMultiplier, 1.8f));
		TestEqual(TEXT("Tagged shot power restore updates multiplier"), StatCharacter->GetShotPowerMultiplier(), 1.8f);
		TestTrue(TEXT("Battle character applies projectile deltas by attribute tag"), StatCharacter->TryApplyCombatAttributeDeltaByTag(FortRogueGameplayTags::Attribute_ProjectileCount, -1000.0f));
		TestEqual(TEXT("Tagged projectile delta clamps to one"), StatCharacter->GetProjectileCount(), 1.0f);
		TestTrue(TEXT("Battle character restores projectile count by attribute tag"), StatCharacter->TryApplyCombatAttributeDeltaByTag(FortRogueGameplayTags::Attribute_ProjectileCount, 2.0f));
		TestEqual(TEXT("Tagged projectile restore updates count"), StatCharacter->GetProjectileCount(), 3.0f);
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
		UFortRogueAbilitySet* EmptyAbilitySet = NewObject<UFortRogueAbilitySet>(StatCharacter);
		EmptyAbilitySet->DisplayName = FText::FromString(TEXT("Empty Ability Set"));
		TestEqual(TEXT("Battle character ability set count starts empty"), StatCharacter->GetGrantedAbilitySetCount(EmptyAbilitySet), 0);
		StatCharacter->GrantAbilitySet(EmptyAbilitySet);
		StatCharacter->GrantAbilitySet(EmptyAbilitySet);
		TestEqual(TEXT("Battle character ability set count tracks repeated grants"), StatCharacter->GetGrantedAbilitySetCount(EmptyAbilitySet), 2);
		TestEqual(TEXT("Battle character exposes granted ability set list"), StatCharacter->GetGrantedAbilitySetsForBlueprint().Num(), 2);
		TestTrue(TEXT("Battle character summarizes granted ability sets"), StatCharacter->GetGrantedAbilitySetsSummary().ToString().Contains(TEXT("Empty Ability Set")));
		TestTrue(TEXT("Battle character removes one granted ability set entry"), StatCharacter->RemoveAbilitySet(EmptyAbilitySet));
		TestEqual(TEXT("Battle character ability set count updates after removal"), StatCharacter->GetGrantedAbilitySetCount(EmptyAbilitySet), 1);
		UFortRogueAbilitySet* TaggedAbilitySet = NewObject<UFortRogueAbilitySet>(StatCharacter);
		TaggedAbilitySet->AbilitySetTag = FortRogueGameplayTags::Trait_ShotModifier;
		StatCharacter->GrantAbilitySet(TaggedAbilitySet);
		StatCharacter->GrantAbilitySet(TaggedAbilitySet);
		TestEqual(TEXT("Battle character ability set tag count tracks repeated grants"), StatCharacter->GetGrantedAbilitySetCountByTag(FortRogueGameplayTags::Trait_ShotModifier), 2);
		TestTrue(TEXT("Battle character reports granted ability sets by tag"), StatCharacter->HasGrantedAbilitySetByTag(FortRogueGameplayTags::Trait_ShotModifier));
		TestEqual(TEXT("Battle character removes granted ability sets by tag"), StatCharacter->RemoveAbilitySetsByTag(FortRogueGameplayTags::Trait_ShotModifier), 2);
		TestEqual(TEXT("Battle character ability set tag count updates after removal"), StatCharacter->GetGrantedAbilitySetCountByTag(FortRogueGameplayTags::Trait_ShotModifier), 0);
		TestFalse(TEXT("Battle character reports missing ability sets by tag"), StatCharacter->HasGrantedAbilitySetByTag(FortRogueGameplayTags::Trait_ShotModifier));
		UFortRogueWeaponDefinition* ShellWeapon = CreateTestWeaponDefinition(StatCharacter);
		ShellWeapon->Weapon.WeaponTag = FortRogueGameplayTags::Weapon_Shell;
		UFortRogueWeaponDefinition* ClusterWeapon = CreateTestWeaponDefinition(StatCharacter);
		ClusterWeapon->Weapon.WeaponTag = FortRogueGameplayTags::Weapon_Cluster;
		StatCharacter->AddWeaponDefinition(ShellWeapon);
		StatCharacter->AddWeaponDefinition(ClusterWeapon);
		TestEqual(TEXT("Battle character finds weapon index by tag"), StatCharacter->GetWeaponIndexByTag(FortRogueGameplayTags::Weapon_Cluster), 1);
		TestEqual(TEXT("Battle character returns INDEX_NONE for invalid weapon tags"), StatCharacter->GetWeaponIndexByTag(FGameplayTag()), INDEX_NONE);
		TestTrue(TEXT("Battle character reports valid weapon index selectable"), StatCharacter->CanSelectWeapon(1));
		TestFalse(TEXT("Battle character rejects invalid weapon index selection queries"), StatCharacter->CanSelectWeapon(-1));
		TestTrue(TEXT("Battle character reports valid weapon tag selectable"), StatCharacter->CanSelectWeaponByTag(FortRogueGameplayTags::Weapon_Cluster));
		TestFalse(TEXT("Battle character rejects invalid weapon tag selection queries"), StatCharacter->CanSelectWeaponByTag(FGameplayTag()));
		TestFalse(TEXT("Battle character rejects invalid weapon selection tags"), StatCharacter->SelectWeaponByTag(FGameplayTag()));
		TestTrue(TEXT("Battle character selects a weapon by tag"), StatCharacter->SelectWeaponByTag(FortRogueGameplayTags::Weapon_Cluster));
		TestTrue(TEXT("Battle character selection by tag updates the current weapon"), StatCharacter->GetCurrentWeapon().WeaponTag.MatchesTagExact(FortRogueGameplayTags::Weapon_Cluster));
		UFortRogueWeaponDefinition* UnsafeWeapon = CreateTestWeaponDefinition(StatCharacter);
		UnsafeWeapon->Weapon.Damage = -100.0f;
		UnsafeWeapon->Weapon.BlastRadius = -20.0f;
		UnsafeWeapon->Weapon.ProjectileSpeed = -500.0f;
		UnsafeWeapon->Weapon.Gravity = -980.0f;
		FFortRogueShotModifierSpec UnsafeModifier;
		UnsafeModifier.DamageBonus = -1000.0f;
		UnsafeWeapon->Weapon.ShotModifiers.Add(UnsafeModifier);
		StatCharacter->AddWeaponDefinition(UnsafeWeapon);
		StatCharacter->SelectWeapon(StatCharacter->GetWeaponLoadout().Num() - 1);
		const FFortRogueShotSpec UnsafeShotSpec = StatCharacter->GetCurrentShotSpec();
		TestEqual(TEXT("Shot spec clamps negative damage after modifiers"), UnsafeShotSpec.Damage, 0.0f);
		TestEqual(TEXT("Shot spec clamps negative blast radius"), UnsafeShotSpec.BlastRadius, 0.0f);
		TestEqual(TEXT("Shot spec clamps negative terrain carve radius"), UnsafeShotSpec.TerrainCarveRadius, 0.0f);
		TestEqual(TEXT("Shot spec clamps negative launch speed"), UnsafeShotSpec.LaunchSpeed, 0.0f);
		TestEqual(TEXT("Shot spec clamps negative gravity"), UnsafeShotSpec.Gravity, 0.0f);
		const FString CurrentShotSummary = StatCharacter->GetCurrentShotSummary().ToString();
		TestTrue(TEXT("Battle character shot summary includes damage"), CurrentShotSummary.Contains(TEXT("Shot Dmg 0")));
		TestTrue(TEXT("Battle character shot summary includes blast radius"), CurrentShotSummary.Contains(TEXT("Blast 0")));
		TestTrue(TEXT("Battle character shot summary includes projectile count"), CurrentShotSummary.Contains(TEXT("Projectiles 2")));
		UFortRogueItemDefinition* PendingModifierItem = NewObject<UFortRogueItemDefinition>(StatCharacter);
		PendingModifierItem->ItemType = EFortRogueItemType::AbilitySet;
		FFortRogueShotModifierSpec PendingModifier;
		PendingModifier.ModifierTag = FortRogueGameplayTags::ShotEffect_Damage;
		PendingModifier.DamageBonus = 5.0f;
		TArray<FFortRogueShotModifierSpec> GrantedModifierTest = { PendingModifier };
		StatCharacter->GrantShotModifiers(GrantedModifierTest);
		TestTrue(TEXT("Battle character reports granted shot modifiers by tag"), StatCharacter->HasGrantedShotModifierByTag(FortRogueGameplayTags::ShotEffect_Damage));
		TestEqual(TEXT("Battle character exposes granted shot modifier list"), StatCharacter->GetGrantedShotModifiersForBlueprint().Num(), 1);
		TestTrue(TEXT("Battle character summarizes granted shot modifiers"), StatCharacter->GetGrantedShotModifiersSummary().ToString().Contains(TEXT("modifier tag")));
		TestEqual(TEXT("Battle character removes granted shot modifiers by tag"), StatCharacter->RemoveGrantedShotModifiersByTag(FortRogueGameplayTags::ShotEffect_Damage), 1);
		TestFalse(TEXT("Battle character reports missing granted shot modifiers by tag"), StatCharacter->HasGrantedShotModifierByTag(FortRogueGameplayTags::ShotEffect_Damage));
		PendingModifierItem->UseShotModifiers.Add(PendingModifier);
		StatCharacter->AddItemDefinition(PendingModifierItem, 1);
		const int32 PendingModifierItemIndex = StatCharacter->GetItemLoadout().Num() - 1;
		StatCharacter->BeginTurn();
		TestTrue(TEXT("Battle character uses item that grants pending shot modifier"), StatCharacter->UseItemByIndex(PendingModifierItemIndex));
		TestEqual(TEXT("Battle character counts pending shot modifiers by tag"), StatCharacter->GetPendingShotModifierCountByTag(FortRogueGameplayTags::ShotEffect_Damage), 1);
		TestTrue(TEXT("Battle character reports pending shot modifiers by tag"), StatCharacter->HasPendingShotModifierByTag(FortRogueGameplayTags::ShotEffect_Damage));
		TestEqual(TEXT("Battle character exposes pending shot modifier list"), StatCharacter->GetPendingShotModifiersForBlueprint().Num(), 1);
		TestTrue(TEXT("Battle character summarizes pending shot modifiers"), StatCharacter->GetPendingShotModifiersSummary().ToString().Contains(TEXT("modifier tag")));
		TestEqual(TEXT("Battle character removes pending shot modifiers by tag"), StatCharacter->RemovePendingShotModifiersByTag(FortRogueGameplayTags::ShotEffect_Damage), 1);
		TestEqual(TEXT("Battle character pending shot modifier count updates after removal"), StatCharacter->GetPendingShotModifierCountByTag(FortRogueGameplayTags::ShotEffect_Damage), 0);
		TestFalse(TEXT("Battle character reports missing pending shot modifiers by tag"), StatCharacter->HasPendingShotModifierByTag(FortRogueGameplayTags::ShotEffect_Damage));
	}

	AFortRogueBattleCharacter* ItemSlotCharacter = World->SpawnActor<AFortRogueBattleCharacter>(AFortRogueBattleCharacter::StaticClass(), FVector(-15.0f, 0.0f, 55.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Item slot battle character is spawned"), ItemSlotCharacter);
	if (ItemSlotCharacter)
	{
		UFortRogueItemDefinition* HealItem = NewObject<UFortRogueItemDefinition>(ItemSlotCharacter);
		HealItem->DisplayName = FText::FromString(TEXT("Test Repair"));
		HealItem->ItemType = EFortRogueItemType::Heal;
		HealItem->ItemTag = FortRogueGameplayTags::Item_Repair;
		HealItem->InitialCharges = 2;
		HealItem->HealAmount = 25.0f;
		ItemSlotCharacter->SetTerrain(Terrain);
		ItemSlotCharacter->AddItemDefinition(HealItem, 2);
		ItemSlotCharacter->BeginTurn();
		ItemSlotCharacter->ApplyDamage(40.0f);
		const float HealthBeforeItem = ItemSlotCharacter->GetHealth();
		TestTrue(TEXT("Battle character can report usable item by loadout index"), ItemSlotCharacter->CanUseItemByIndex(0));
		TestTrue(TEXT("Battle character can report usable item by type"), ItemSlotCharacter->CanUseItemByType(EFortRogueItemType::Heal));
		TestTrue(TEXT("Battle character can report usable item by tag"), ItemSlotCharacter->CanUseItemByTag(FortRogueGameplayTags::Item_Repair));
		TestEqual(TEXT("Battle character finds item index by tag"), ItemSlotCharacter->GetItemIndexByTag(FortRogueGameplayTags::Item_Repair), 0);
		TestEqual(TEXT("Battle character returns INDEX_NONE for invalid item tags"), ItemSlotCharacter->GetItemIndexByTag(FGameplayTag()), INDEX_NONE);
		TestFalse(TEXT("Battle character rejects negative item loadout index"), ItemSlotCharacter->UseItemByIndex(-1));
		TestFalse(TEXT("Battle character cannot use negative item loadout index"), ItemSlotCharacter->CanUseItemByIndex(-1));
		TestFalse(TEXT("Battle character rejects item loadout index past the end"), ItemSlotCharacter->UseItemByIndex(1));
		TestEqual(TEXT("Rejected item slot use keeps item charges"), ItemSlotCharacter->GetItemCharges(EFortRogueItemType::Heal), 2);
		TestTrue(TEXT("Battle character uses an item by loadout index"), ItemSlotCharacter->UseItemByIndex(0));
		TestEqual(TEXT("Item slot use consumes one charge"), ItemSlotCharacter->GetItemCharges(EFortRogueItemType::Heal), 1);
		TestTrue(TEXT("Item slot use applies the item effect"), ItemSlotCharacter->GetHealth() > HealthBeforeItem);
		TestTrue(TEXT("Battle character still reports usable item with remaining charges"), ItemSlotCharacter->CanUseItemByIndex(0));
		TestTrue(TEXT("Battle character consumes final item charge"), ItemSlotCharacter->UseItemByIndex(0));
		TestFalse(TEXT("Battle character reports unusable item after charges are spent"), ItemSlotCharacter->CanUseItemByIndex(0));
	}

	AFortRogueBattleCharacter* ExhaustedTurnCharacter = World->SpawnActor<AFortRogueBattleCharacter>(AFortRogueBattleCharacter::StaticClass(), FVector(45.0f, 0.0f, 55.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Exhausted turn battle character is spawned"), ExhaustedTurnCharacter);
	if (ExhaustedTurnCharacter)
	{
		ExhaustedTurnCharacter->AddWeaponDefinition(CreateTestWeaponDefinition(ExhaustedTurnCharacter));
		ExhaustedTurnCharacter->SetTerrain(Terrain);
		ExhaustedTurnCharacter->BeginTurn();
		if (UFortRogueCombatSet* CombatSet = GetCombatSet(ExhaustedTurnCharacter))
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
		TestEqual(TEXT("Exhausted movement budget still leaves the budget at zero"), ExhaustedTurnCharacter->GetMoveBudget(), 0.0f);
		TestTrue(TEXT("Battle character reports selected weapon fireable before firing"), ExhaustedTurnCharacter->CanFireSelectedWeapon());
		TestTrue(TEXT("Battle character reports shot charge can begin before firing"), ExhaustedTurnCharacter->CanBeginShotCharge());
		TestEqual(TEXT("Facing change with exhausted movement budget fires to the requested side"), ExhaustedTurnCharacter->FireSelectedWeapon(), 1);
		TestFalse(TEXT("Battle character reports selected weapon not fireable after firing"), ExhaustedTurnCharacter->CanFireSelectedWeapon());
		AFortRogueProjectile* ExhaustedTurnProjectile = nullptr;
		for (TActorIterator<AFortRogueProjectile> It(World); It; ++It)
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
			const FVector ExpectedLeftLaunchDirection(FMath::Cos(FMath::DegreesToRadians(135.0f)), 0.0f, FMath::Sin(FMath::DegreesToRadians(135.0f)));
			const FVector ExpectedProjectileSpawnLocation = ExhaustedTurnCharacter->GetActorLocation() + ExpectedLeftLaunchDirection * 70.0f + FVector(0.0f, 0.0f, 35.0f);
			TestTrue(TEXT("Projectile and trajectory start from the character launch offset"), ExhaustedTurnProjectile->GetActorLocation().Equals(ExpectedProjectileSpawnLocation, 0.1));
			ExhaustedTurnProjectile->Destroy();
		}
	}

	AFortRogueBattleCharacter* AimFacingCharacter = World->SpawnActor<AFortRogueBattleCharacter>(AFortRogueBattleCharacter::StaticClass(), FVector(15.0f, 0.0f, 55.0f), FRotator::ZeroRotator, SpawnParams);
	AFortRogueBattleCharacter* LeftAimTarget = World->SpawnActor<AFortRogueBattleCharacter>(AFortRogueBattleCharacter::StaticClass(), FVector(-45.0f, 0.0f, 55.0f), FRotator::ZeroRotator, SpawnParams);
	AFortRogueBattleCharacter* RightAimTarget = World->SpawnActor<AFortRogueBattleCharacter>(AFortRogueBattleCharacter::StaticClass(), FVector(45.0f, 0.0f, 55.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Aim-facing battle character is spawned"), AimFacingCharacter);
	TestNotNull(TEXT("Left aim target is spawned"), LeftAimTarget);
	TestNotNull(TEXT("Right aim target is spawned"), RightAimTarget);
	if (AimFacingCharacter && LeftAimTarget && RightAimTarget)
	{
		UPaperFlipbookComponent* AimFacingSprite = Cast<UPaperFlipbookComponent>(AimFacingCharacter->GetDefaultSubobjectByName(TEXT("BodySprite")));
		TestNotNull(TEXT("Aim-facing sprite component exists"), AimFacingSprite);
		FFortRogueStageDifficultyData AimFacingDifficulty;
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

	AFortRogueBattleCharacter* BoundaryCharacter = World->SpawnActor<AFortRogueBattleCharacter>(AFortRogueBattleCharacter::StaticClass(), FVector(500.0f, 0.0f, 200.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Boundary battle character is spawned"), BoundaryCharacter);
	if (BoundaryCharacter)
	{
		BoundaryCharacter->SetActorLocation(FVector(500.0f, 0.0f, 200.0f));
		BoundaryCharacter->SetTerrain(Terrain);
		const float FootProbeHalfWidth = GetFloatProperty(BoundaryCharacter, TEXT("FootProbeHalfWidth"), 22.0f);
		const float MaxCharacterX = Terrain->GetActorLocation().X + Terrain->Width * 0.5f - FootProbeHalfWidth;
		TestEqual(TEXT("Assigning terrain clamps character inside terrain width"), static_cast<float>(BoundaryCharacter->GetActorLocation().X), MaxCharacterX);
		BoundaryCharacter->BeginTurn();
		BoundaryCharacter->MoveHorizontal(1.0f, 1.0f);
		TestEqual(TEXT("Battle character movement stays inside terrain width"), static_cast<float>(BoundaryCharacter->GetActorLocation().X), MaxCharacterX);
	}

	AFortRogueBattleCharacter* LateBoundCharacter = World->SpawnActor<AFortRogueBattleCharacter>(AFortRogueBattleCharacter::StaticClass(), FVector(-15.0f, 0.0f, 200.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Late-bound terrain battle character is spawned"), LateBoundCharacter);
	if (LateBoundCharacter)
	{
		LateBoundCharacter->SetActorLocation(FVector(-15.0f, 0.0f, 200.0f));
		LateBoundCharacter->SetTerrain(Terrain);
		const float FootOffsetZ = GetFloatProperty(LateBoundCharacter, TEXT("FootOffsetZ"), 45.0f);
		TestEqual(TEXT("Assigning terrain after BeginPlay snaps character feet onto the highest footprint surface"), static_cast<float>(LateBoundCharacter->GetActorLocation().Z), 60.0f + FootOffsetZ);
	}

	AFortRogueBattleCharacter* RampCharacter = World->SpawnActor<AFortRogueBattleCharacter>(AFortRogueBattleCharacter::StaticClass(), FVector(-95.0f, 0.0f, 55.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Ramp battle character is spawned"), RampCharacter);
	if (RampCharacter)
	{
		RampCharacter->AddWeaponDefinition(CreateTestWeaponDefinition(RampCharacter));
		RampCharacter->SetTerrain(Terrain);
		SetFloatProperty(RampCharacter, TEXT("FootProbeHalfWidth"), 0.0f);
		SetFloatProperty(RampCharacter, TEXT("BodySlopeProbeHalfWidth"), 5.0f);
		RampCharacter->SetActorLocation(FVector(-95.0f, 0.0f, 55.0f));
		RampCharacter->BeginTurn();
		RampCharacter->MoveHorizontal(1.0f, 0.12f);
		TestTrue(TEXT("Battle character climbs a traversable gentle slope"), RampCharacter->GetActorLocation().X > -70.0f && RampCharacter->GetActorLocation().Z >= 65.0f);
		UStaticMeshComponent* RampBody = Cast<UStaticMeshComponent>(RampCharacter->GetDefaultSubobjectByName(TEXT("Body")));
		TestNotNull(TEXT("Ramp battle character body exists"), RampBody);
		TestTrue(TEXT("Battle character actor aligns to terrain slope"), RampCharacter->GetActorRotation().Pitch > 15.0f);
		if (RampBody)
		{
			TestEqual(TEXT("Battle character body inherits actor terrain rotation"), RampBody->GetRelativeRotation(), FRotator::ZeroRotator);
		}
		const float RampPitch = RampCharacter->GetActorRotation().Pitch;
		TestEqual(TEXT("Slope-aligned battle character fires from actor rotation"), RampCharacter->FireSelectedWeapon(), 1);
		AFortRogueProjectile* RampProjectile = nullptr;
		for (TActorIterator<AFortRogueProjectile> It(World); It; ++It)
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
			const float ExpectedLaunchAngle = RampPitch + 45.0f;
			const FVector ExpectedRampLaunchDirection(FMath::Cos(FMath::DegreesToRadians(ExpectedLaunchAngle)), 0.0f, FMath::Sin(FMath::DegreesToRadians(ExpectedLaunchAngle)));
			const FVector ExpectedRampProjectileSpawnLocation = RampCharacter->GetActorLocation() + ExpectedRampLaunchDirection * 70.0f + FVector(0.0f, 0.0f, 35.0f);
			TestTrue(TEXT("Slope-aligned projectile launch uses actor pitch"), RampProjectile->GetActorLocation().Equals(ExpectedRampProjectileSpawnLocation, 0.1));
			RampProjectile->Destroy();
		}
	}

	AFortRogueBattleCharacter* SteepSlopeCharacter = World->SpawnActor<AFortRogueBattleCharacter>(AFortRogueBattleCharacter::StaticClass(), FVector(-45.0f, 0.0f, 55.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Steep slope battle character is spawned"), SteepSlopeCharacter);
	if (SteepSlopeCharacter)
	{
		SteepSlopeCharacter->SetTerrain(Terrain);
		SetFloatProperty(SteepSlopeCharacter, TEXT("FootProbeHalfWidth"), 0.0f);
		SteepSlopeCharacter->SetActorLocation(FVector(-45.0f, 0.0f, 55.0f));
		SteepSlopeCharacter->BeginTurn();
		SteepSlopeCharacter->MoveHorizontal(1.0f, 0.12f);
		TestTrue(TEXT("Battle character stops at terrain steeper than the slope limit"), SteepSlopeCharacter->GetActorLocation().X < -30.0f);
		TestEqual(TEXT("Battle character does not climb the rejected steep slope"), static_cast<float>(SteepSlopeCharacter->GetActorLocation().Z), 55.0f);
	}

	AFortRogueBattleCharacter* GapCharacter = World->SpawnActor<AFortRogueBattleCharacter>(AFortRogueBattleCharacter::StaticClass(), FVector(485.0f, 0.0f, 55.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Gap movement battle character is spawned"), GapCharacter);
	if (GapCharacter && GapTerrain)
	{
		GapCharacter->SetTerrain(GapTerrain);
		SetFloatProperty(GapCharacter, TEXT("FootProbeHalfWidth"), 0.0f);
		GapCharacter->SetActorLocation(FVector(485.0f, 0.0f, 55.0f));
		GapCharacter->BeginTurn();
		GapCharacter->MoveHorizontal(1.0f, 0.1f);
		TestTrue(TEXT("Battle character does not traverse an unsupported gap in one move"), GapCharacter->GetActorLocation().X > 485.0f && GapCharacter->GetActorLocation().X < 500.0f);
		TestTrue(TEXT("Battle character starts falling when horizontal movement reaches unsupported terrain"), GapCharacter->GetActorLocation().Z < 55.0f);
	}

	AFortRogueBattleCharacter* SettlingCharacter = World->SpawnActor<AFortRogueBattleCharacter>(AFortRogueBattleCharacter::StaticClass(), FVector(5.0f, 0.0f, 105.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Settling battle character is spawned"), SettlingCharacter);
	if (SettlingCharacter)
	{
		SettlingCharacter->SetTerrain(Terrain);
		SetFloatProperty(SettlingCharacter, TEXT("FootProbeHalfWidth"), 0.0f);
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

		AFortRogueBattleCharacter* FastProjectileTarget = World->SpawnActor<AFortRogueBattleCharacter>(AFortRogueBattleCharacter::StaticClass(), FVector(260.0f, 500.0f, 35.0f), FRotator::ZeroRotator, SpawnParams);
		TestNotNull(TEXT("Fast projectile target is spawned before the wall"), FastProjectileTarget);
		if (FastProjectileTarget)
		{
			FastProjectileTarget->SetActorLocation(FVector(260.0f, 500.0f, 35.0f));
			const float TargetHealthBeforeHit = FastProjectileTarget->GetHealth();
			AFortRogueProjectile* FastCharacterProjectile = World->SpawnActor<AFortRogueProjectile>(AFortRogueProjectile::StaticClass(), FVector(203.0f, 0.0f, 35.0f), FRotator::ZeroRotator, SpawnParams);
			TestNotNull(TEXT("Fast character projectile is spawned"), FastCharacterProjectile);
			if (FastCharacterProjectile)
			{
				FastCharacterProjectile->InitializeProjectile(nullptr, FastProjectileTerrain, FVector(1940.0f, 0.0f, 0.0f), 20.0f, 12.0f, 0.0f);
				FastCharacterProjectile->Tick(0.1f);
				TestTrue(TEXT("Fast projectile hits the character in the X/Z gameplay plane before later terrain"), FastProjectileTarget->GetHealth() < TargetHealthBeforeHit);
				TestTrue(TEXT("Later terrain remains solid when the earlier character is hit"), FastProjectileTerrain->IsSolidAtWorldLocation(FVector(305.0f, 0.0f, 35.0f)));
			}
		}

		AFortRogueBattleCharacter* ZeroRadiusTarget = World->SpawnActor<AFortRogueBattleCharacter>(AFortRogueBattleCharacter::StaticClass(), FVector(240.0f, 500.0f, 60.0f), FRotator::ZeroRotator, SpawnParams);
		TestNotNull(TEXT("Zero-radius direct-hit target is spawned before the wall"), ZeroRadiusTarget);
		if (ZeroRadiusTarget)
		{
			ZeroRadiusTarget->SetActorLocation(FVector(240.0f, 500.0f, 60.0f));
			const float TargetHealthBeforeZeroRadiusHit = ZeroRadiusTarget->GetHealth();
			AFortRogueProjectile* ZeroRadiusProjectile = World->SpawnActor<AFortRogueProjectile>(AFortRogueProjectile::StaticClass(), FVector(203.0f, 0.0f, 35.0f), FRotator::ZeroRotator, SpawnParams);
			TestNotNull(TEXT("Zero-radius direct-hit projectile is spawned"), ZeroRadiusProjectile);
			if (ZeroRadiusProjectile)
			{
				ZeroRadiusProjectile->InitializeProjectile(nullptr, FastProjectileTerrain, FVector(740.0f, 0.0f, 0.0f), 20.0f, 0.0f, 0.0f);
				ZeroRadiusProjectile->Tick(0.1f);
				TestTrue(TEXT("Zero-radius direct hit applies finite point damage"), FMath::IsFinite(ZeroRadiusTarget->GetHealth()) && ZeroRadiusTarget->GetHealth() < TargetHealthBeforeZeroRadiusHit);
			}
		}

		AFortRogueProjectile* FastProjectile = World->SpawnActor<AFortRogueProjectile>(AFortRogueProjectile::StaticClass(), FVector(203.0f, 0.0f, 35.0f), FRotator::ZeroRotator, SpawnParams);
		TestNotNull(TEXT("Fast projectile is spawned"), FastProjectile);
		if (FastProjectile)
		{
			FastProjectile->InitializeProjectile(FastProjectileTarget, FastProjectileTerrain, FVector(1940.0f, 0.0f, 0.0f), 0.0f, 6.0f, 0.0f);
			FastProjectile->Tick(0.1f);
			TestTrue(TEXT("Resolved projectile actor location stays at the terrain impact point"), FastProjectile->GetActorLocation().X >= 300.0f && FastProjectile->GetActorLocation().X < 310.0f);
			TestFalse(TEXT("Fast projectile carves the one-cell vertical wall instead of tunneling through it"), FastProjectileTerrain->IsSolidAtWorldLocation(FVector(305.0f, 0.0f, 35.0f)));
		}
	}

	AFortRogueProjectile* AssignedTerrainProjectile = World->SpawnActor<AFortRogueProjectile>(AFortRogueProjectile::StaticClass(), FVector(-75.0f, 0.0f, 25.0f), FRotator::ZeroRotator, SpawnParams);
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
		AssignedTerrainProjectile->InitializeProjectile(Character, Terrain, FVector(0.0f, 0.0f, -100.0f), 0.0f, 24.0f, 0.0f);
		AssignedTerrainProjectile->Tick(0.25f);
		TestFalse(TEXT("Projectile carves its assigned terrain"), Terrain->IsSolidAtWorldLocation(FVector(-75.0f, 0.0f, 5.0f)));
		TestTrue(TEXT("Projectile does not carve overlapping unassigned terrain"), OverlappingTerrain->IsSolidAtWorldLocation(FVector(-75.0f, 0.0f, 5.0f)));
	}

	AFortRogueBattleCharacter* FallingCharacter = World->SpawnActor<AFortRogueBattleCharacter>(AFortRogueBattleCharacter::StaticClass(), FVector(45.0f, 0.0f, 55.0f), FRotator::ZeroRotator, SpawnParams);
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
		const int32 AttackItemCharges = FallingCharacter->GetItemCharges(EFortRogueItemType::AttackMultiplier);
		TestFalse(TEXT("Falling battle character reports combat items unusable while unsupported"), FallingCharacter->CanUseItemByType(EFortRogueItemType::AttackMultiplier));
		TestFalse(TEXT("Falling battle character cannot use combat items while unsupported"), FallingCharacter->UseItemByType(EFortRogueItemType::AttackMultiplier));
		TestEqual(TEXT("Falling battle character keeps item charges when unsupported item use is rejected"), FallingCharacter->GetItemCharges(EFortRogueItemType::AttackMultiplier), AttackItemCharges);
		FallingCharacter->BeginShotCharge();
		FallingCharacter->UpdateShotCharge(1.0f);
		TestEqual(TEXT("Falling battle character cannot fire while unsupported"), FallingCharacter->ReleaseShotCharge(), 0);
		SetFloatProperty(FallingCharacter, TEXT("FallDeathDepth"), 1.0f);
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

	AFortRogueDestructibleTerrain* RotatedTerrain = World->SpawnActor<AFortRogueDestructibleTerrain>(AFortRogueDestructibleTerrain::StaticClass(), FVector(200.0f, 0.0f, 0.0f), FRotator(0.0f, 90.0f, 90.0f), SpawnParams);
	TestNotNull(TEXT("Rotated terrain actor is spawned"), RotatedTerrain);
	if (RotatedTerrain)
	{
		TestTrue(TEXT("Terrain actor rotation is normalized for the gameplay X/Z plane"), RotatedTerrain->GetActorRotation().IsNearlyZero());
	}

	CleanupWorld();
	return true;
}

#endif
