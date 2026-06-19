// Copyright Epic Games, Inc. All Rights Reserved.

#include "FortRogueCreateCombatAssetsCommandlet.h"

#include "Characters/FortRogueCharacterDefinition.h"
#include "Combat/FortRogueHomingProjectile.h"
#include "FortRogueGameplayTags.h"
#include "Run/FortRogueDefaultLoadoutDefinition.h"
#include "Run/FortRogueStageRunDefinition.h"
#include "Weapons/FortRogueWeaponDefinition.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/PackageName.h"
#include "PaperFlipbook.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

namespace
{
constexpr float ClassicDamage = 35.0f;
constexpr float ClassicBlastRadius = 150.0f;
constexpr float ClassicProjectileSpeed = 1200.0f;
constexpr float ClassicGravity = 980.0f;
constexpr float TinyBlastRadius = 35.0f;

template<typename AssetType>
AssetType* GetOrCreateAsset(const TCHAR* PackageName)
{
	UPackage* Package = CreatePackage(PackageName);
	if (!Package)
	{
		return nullptr;
	}

	Package->FullyLoad();
	const FString AssetName = FPackageName::GetLongPackageAssetName(PackageName);
	AssetType* Asset = FindObject<AssetType>(Package, *AssetName);
	if (!Asset)
	{
		Asset = NewObject<AssetType>(Package, AssetType::StaticClass(), *AssetName, RF_Public | RF_Standalone | RF_Transactional);
		FAssetRegistryModule::AssetCreated(Asset);
	}
	return Asset;
}

bool SaveAsset(UObject* Asset)
{
	if (!Asset)
	{
		return false;
	}

	UPackage* Package = Asset->GetOutermost();
	Package->MarkPackageDirty();
	const FString FileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());

	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	SaveArgs.SaveFlags = SAVE_NoError;
	return UPackage::SavePackage(Package, Asset, *FileName, SaveArgs);
}

FGameplayTagContainer MakeEffectTags(std::initializer_list<FGameplayTag> Tags)
{
	FGameplayTagContainer Container;
	for (const FGameplayTag& Tag : Tags)
	{
		Container.AddTag(Tag);
	}
	return Container;
}

void ConfigureWeapon(UFortRogueWeaponDefinition* Weapon, const FText& DisplayName, const FText& Description, const FGameplayTag& WeaponTag, const FGameplayTagContainer& EffectTags, float Damage, float BlastRadius, float ProjectileSpeed, float Gravity, int32 ProjectilesPerShot, int32 SalvoCount = 1, float SalvoInterval = 0.0f, TSubclassOf<AFortRogueProjectile> ProjectileClass = nullptr)
{
	if (!Weapon)
	{
		return;
	}

	Weapon->Modify();
	Weapon->Weapon.DisplayName = DisplayName;
	Weapon->Weapon.Description = Description;
	Weapon->Weapon.WeaponTag = WeaponTag;
	Weapon->Weapon.ShotEffectTags = EffectTags;
	Weapon->Weapon.Damage = Damage;
	Weapon->Weapon.BlastRadius = BlastRadius;
	Weapon->Weapon.ProjectileSpeed = ProjectileSpeed;
	Weapon->Weapon.Gravity = Gravity;
	Weapon->Weapon.ProjectilesPerShot = ProjectilesPerShot;
	Weapon->Weapon.SalvoCount = SalvoCount;
	Weapon->Weapon.SalvoInterval = SalvoInterval;
	Weapon->Weapon.ProjectileClass = ProjectileClass;
}

void ConfigureCharacter(UFortRogueCharacterDefinition* Character, const FText& DisplayName, const TCHAR* FlipbookPath, UFortRogueWeaponDefinition* FirstWeapon, UFortRogueWeaponDefinition* SecondWeapon)
{
	if (!Character)
	{
		return;
	}

	Character->Modify();
	Character->DisplayName = DisplayName;
	Character->BodyFlipbook = LoadObject<UPaperFlipbook>(nullptr, FlipbookPath);
	Character->MaxHealth = 100.0f;
	Character->BonusDamage = 0.0f;
	Character->MaxMoveBudget = 420.0f;
	Character->ShotPowerMultiplier = 1.0f;
	Character->WeaponLoadout.Reset();
	Character->WeaponLoadout.Add(FirstWeapon);
	Character->WeaponLoadout.Add(SecondWeapon);
}
}

UFortRogueCreateCombatAssetsCommandlet::UFortRogueCreateCombatAssetsCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

int32 UFortRogueCreateCombatAssetsCommandlet::Main(const FString& Params)
{
	using namespace FortRogueGameplayTags;

	UFortRogueWeaponDefinition* CannonClassic = GetOrCreateAsset<UFortRogueWeaponDefinition>(TEXT("/Game/FortRogue/Weapon/WP_Cannon_Classic"));
	UFortRogueWeaponDefinition* CannonSiege = GetOrCreateAsset<UFortRogueWeaponDefinition>(TEXT("/Game/FortRogue/Weapon/WP_Cannon_Siege"));
	UFortRogueWeaponDefinition* LaserClassic = GetOrCreateAsset<UFortRogueWeaponDefinition>(TEXT("/Game/FortRogue/Weapon/WP_Laser_Classic"));
	UFortRogueWeaponDefinition* LaserSiege = GetOrCreateAsset<UFortRogueWeaponDefinition>(TEXT("/Game/FortRogue/Weapon/WP_Laser_Siege"));
	UFortRogueWeaponDefinition* RepeaterSingleBurst = GetOrCreateAsset<UFortRogueWeaponDefinition>(TEXT("/Game/FortRogue/Weapon/WP_Repeater_SingleBurst"));
	UFortRogueWeaponDefinition* RepeaterTripleBurst = GetOrCreateAsset<UFortRogueWeaponDefinition>(TEXT("/Game/FortRogue/Weapon/WP_Repeater_TripleBurst"));
	UFortRogueWeaponDefinition* CrossbowHeavyBolt = GetOrCreateAsset<UFortRogueWeaponDefinition>(TEXT("/Game/FortRogue/Weapon/WP_Crossbow_HeavyBolt"));
	UFortRogueWeaponDefinition* CrossbowNeedleVolley = GetOrCreateAsset<UFortRogueWeaponDefinition>(TEXT("/Game/FortRogue/Weapon/WP_Crossbow_NeedleVolley"));
	UFortRogueWeaponDefinition* MissileRocket = GetOrCreateAsset<UFortRogueWeaponDefinition>(TEXT("/Game/FortRogue/Weapon/WP_Missile_Rocket"));
	UFortRogueWeaponDefinition* MissileGuided = GetOrCreateAsset<UFortRogueWeaponDefinition>(TEXT("/Game/FortRogue/Weapon/WP_Missile_Guided"));

	ConfigureWeapon(CannonClassic, FText::FromString(TEXT("Classic Cannon")), FText::FromString(TEXT("A reliable standard cannon shell.")), Weapon_Cannon_Classic, MakeEffectTags({}), ClassicDamage, ClassicBlastRadius, ClassicProjectileSpeed, ClassicGravity, 1);
	ConfigureWeapon(CannonSiege, FText::FromString(TEXT("Needle Bombard")), FText::FromString(TEXT("Tiny blast radius, triple classic damage.")), Weapon_Cannon_Siege, MakeEffectTags({ShotEffect_Damage, ShotEffect_BlastRadius}), ClassicDamage * 3.0f, TinyBlastRadius, ClassicProjectileSpeed, ClassicGravity, 1);
	ConfigureWeapon(LaserClassic, FText::FromString(TEXT("Prototype Laser Classic")), FText::FromString(TEXT("Placeholder laser loadout using the classic cannon profile.")), Weapon_Laser_Classic, MakeEffectTags({}), ClassicDamage, ClassicBlastRadius, ClassicProjectileSpeed, ClassicGravity, 1);
	ConfigureWeapon(LaserSiege, FText::FromString(TEXT("Prototype Laser Focus")), FText::FromString(TEXT("Placeholder laser loadout using the high damage tiny blast profile.")), Weapon_Laser_Siege, MakeEffectTags({ShotEffect_Damage, ShotEffect_BlastRadius}), ClassicDamage * 3.0f, TinyBlastRadius, ClassicProjectileSpeed, ClassicGravity, 1);
	ConfigureWeapon(RepeaterSingleBurst, FText::FromString(TEXT("Rhythm Cannon")), FText::FromString(TEXT("Fires one shell every 0.3 seconds, three times.")), Weapon_Repeater_SingleBurst, MakeEffectTags({ShotEffect_Salvo}), ClassicDamage, 120.0f, 1220.0f, ClassicGravity, 1, 3, 0.3f);
	ConfigureWeapon(RepeaterTripleBurst, FText::FromString(TEXT("Triple Rhythm Cannon")), FText::FromString(TEXT("Fires three shells every 0.3 seconds, three times.")), Weapon_Repeater_TripleBurst, MakeEffectTags({ShotEffect_Projectiles, ShotEffect_Salvo}), 18.0f, 95.0f, 1220.0f, ClassicGravity, 3, 3, 0.3f);
	ConfigureWeapon(CrossbowHeavyBolt, FText::FromString(TEXT("Heavy Bolt")), FText::FromString(TEXT("Tiny blast radius, triple classic damage.")), Weapon_Crossbow_HeavyBolt, MakeEffectTags({ShotEffect_Damage, ShotEffect_BlastRadius}), ClassicDamage * 3.0f, TinyBlastRadius, 1700.0f, 620.0f, 1);
	ConfigureWeapon(CrossbowNeedleVolley, FText::FromString(TEXT("Needle Volley")), FText::FromString(TEXT("Fifteen very low damage bolts with a tiny blast.")), Weapon_Crossbow_NeedleVolley, MakeEffectTags({ShotEffect_Projectiles, ShotEffect_BlastRadius}), 4.0f, 20.0f, 1700.0f, 620.0f, 15);
	ConfigureWeapon(MissileRocket, FText::FromString(TEXT("Classic Rocket")), FText::FromString(TEXT("Missile platform version of the classic cannon shell.")), Weapon_Missile_Rocket, MakeEffectTags({}), ClassicDamage, ClassicBlastRadius, ClassicProjectileSpeed, ClassicGravity, 1);
	ConfigureWeapon(MissileGuided, FText::FromString(TEXT("Seeker Missile")), FText::FromString(TEXT("Small blast missile with about 70 percent classic damage and homing guidance.")), Weapon_Missile_Guided, MakeEffectTags({ShotEffect_Homing, ShotEffect_BlastRadius}), ClassicDamage * 0.7f, TinyBlastRadius, 1050.0f, 620.0f, 1, 1, 0.0f, AFortRogueHomingProjectile::StaticClass());

	UFortRogueCharacterDefinition* CannonTank = GetOrCreateAsset<UFortRogueCharacterDefinition>(TEXT("/Game/FortRogue/Character/CannonTank"));
	UFortRogueCharacterDefinition* LaserTank = GetOrCreateAsset<UFortRogueCharacterDefinition>(TEXT("/Game/FortRogue/Character/LaserTank"));
	UFortRogueCharacterDefinition* RepeaterCannonTank = GetOrCreateAsset<UFortRogueCharacterDefinition>(TEXT("/Game/FortRogue/Character/RepeaterCannonTank"));
	UFortRogueCharacterDefinition* CrossbowTank = GetOrCreateAsset<UFortRogueCharacterDefinition>(TEXT("/Game/FortRogue/Character/CrossbowTank"));
	UFortRogueCharacterDefinition* MissileTank = GetOrCreateAsset<UFortRogueCharacterDefinition>(TEXT("/Game/FortRogue/Character/MissileTank"));

	ConfigureCharacter(CannonTank, FText::FromString(TEXT("Cannon")), TEXT("/Game/FortRogue/Character/GeneratedSprites/cannon_tank_flipbook.cannon_tank_flipbook"), CannonClassic, CannonSiege);
	ConfigureCharacter(LaserTank, FText::FromString(TEXT("Laser")), TEXT("/Game/FortRogue/Character/GeneratedSprites/laser_tank_flipbook.laser_tank_flipbook"), LaserClassic, LaserSiege);
	ConfigureCharacter(RepeaterCannonTank, FText::FromString(TEXT("Repeater Cannon")), TEXT("/Game/FortRogue/Character/GeneratedSprites/multi_missile_tank_flipbook.multi_missile_tank_flipbook"), RepeaterSingleBurst, RepeaterTripleBurst);
	ConfigureCharacter(CrossbowTank, FText::FromString(TEXT("Crossbow")), TEXT("/Game/FortRogue/Character/GeneratedSprites/crossbow_tank_flipbook.crossbow_tank_flipbook"), CrossbowHeavyBolt, CrossbowNeedleVolley);
	ConfigureCharacter(MissileTank, FText::FromString(TEXT("Missile")), TEXT("/Game/FortRogue/Character/GeneratedSprites/missile_tank_flipbook.missile_tank_flipbook"), MissileRocket, MissileGuided);

	UFortRogueDefaultLoadoutDefinition* DefaultLoadout = LoadObject<UFortRogueDefaultLoadoutDefinition>(nullptr, TEXT("/Game/FortRogue/Loadout/DA_DefaultLoadout.DA_DefaultLoadout"));
	if (DefaultLoadout)
	{
		DefaultLoadout->Modify();
		DefaultLoadout->WeaponDefinitions.Reset();
		DefaultLoadout->WeaponDefinitions.Add(CannonClassic);
		DefaultLoadout->WeaponDefinitions.Add(CannonSiege);
	}

	UFortRogueStageRunDefinition* DefaultStageRun = LoadObject<UFortRogueStageRunDefinition>(nullptr, TEXT("/Game/FortRogue/Run/DA_DefaultStageRun.DA_DefaultStageRun"));
	if (DefaultStageRun)
	{
		DefaultStageRun->Modify();
		DefaultStageRun->EnemyDefinitionPool.Reset();
		DefaultStageRun->EnemyDefinitionPool.Add(CannonTank);
		DefaultStageRun->EnemyDefinitionPool.Add(LaserTank);
		DefaultStageRun->EnemyDefinitionPool.Add(RepeaterCannonTank);
		DefaultStageRun->EnemyDefinitionPool.Add(CrossbowTank);
		DefaultStageRun->EnemyDefinitionPool.Add(MissileTank);
		DefaultStageRun->DefaultLoadoutDefinition = DefaultLoadout;
		DefaultStageRun->NormalizeStageData();
	}

	TArray<UObject*> AssetsToSave = {
		CannonClassic, CannonSiege, LaserClassic, LaserSiege, RepeaterSingleBurst, RepeaterTripleBurst, CrossbowHeavyBolt, CrossbowNeedleVolley, MissileRocket, MissileGuided,
		CannonTank, LaserTank, RepeaterCannonTank, CrossbowTank, MissileTank,
		DefaultLoadout, DefaultStageRun
	};

	bool bAllSaved = true;
	for (UObject* Asset : AssetsToSave)
	{
		if (Asset)
		{
			bAllSaved &= SaveAsset(Asset);
		}
	}
	return bAllSaved ? 0 : 1;
}
