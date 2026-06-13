// Copyright Epic Games, Inc. All Rights Reserved.

#include "FortRogueGameMode.h"

#include "Combat/FortRogueBattleCharacter.h"
#include "Combat/FortRogueDestructibleTerrain.h"
#include "FortRogueGameplayTags.h"
#include "FortRogueHUD.h"
#include "FortRoguePlayerController.h"
#include "Items/FortRogueItemDefinition.h"
#include "Perks/FortRoguePerkDefinition.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

AFortRogueGameMode::AFortRogueGameMode()
{
	PlayerControllerClass = AFortRoguePlayerController::StaticClass();
	HUDClass = AFortRogueHUD::StaticClass();
	DefaultPawnClass = nullptr;
	StatusText = FText::FromString(TEXT("Ready"));
	PlayerCharacterClass = AFortRogueBattleCharacter::StaticClass();
	EnemyCharacterClass = AFortRogueBattleCharacter::StaticClass();
	TerrainClass = AFortRogueDestructibleTerrain::StaticClass();
	CameraClass = ACameraActor::StaticClass();
}

void AFortRogueGameMode::BeginPlay()
{
	Super::BeginPlay();

	SpawnMVPBattle();
	StartPlayerTurn();
}

void AFortRogueGameMode::NotifyShotFired(AFortRogueBattleCharacter* Shooter, int32 ProjectileCount)
{
	if (!Shooter || ProjectileCount <= 0)
	{
		return;
	}

	PendingProjectiles = ProjectileCount;
	LastShooter = Shooter;
	BattleState = EFortRogueBattleState::ResolvingShot;
	Shooter->EndTurn();
	SetStatus(Shooter->IsEnemy() ? TEXT("Enemy shell incoming") : TEXT("Shell fired"));
}

void AFortRogueGameMode::NotifyProjectileResolved(AFortRogueProjectile* Projectile)
{
	PendingProjectiles = FMath::Max(0, PendingProjectiles - 1);
	if (PendingProjectiles == 0)
	{
		FinishShotResolution();
	}
}

void AFortRogueGameMode::ApplyRewardChoice(int32 ChoiceIndex)
{
	if (BattleState != EFortRogueBattleState::Reward || !PlayerCharacter || !RewardChoices.IsValidIndex(ChoiceIndex))
	{
		return;
	}

	const FFortRogueRewardChoice& Reward = RewardChoices[ChoiceIndex];
	if (Reward.PerkReward)
	{
		PlayerCharacter->ApplyPerkDefinition(Reward.PerkReward);
	}
	if (Reward.DamageBonus > 0.0f)
	{
		PlayerCharacter->ApplyRewardDamage(Reward.DamageBonus);
	}
	if (Reward.MaxHealthBonus > 0.0f)
	{
		PlayerCharacter->ApplyRewardHealth(Reward.MaxHealthBonus);
	}
	if (Reward.ProjectileBonus > 0)
	{
		PlayerCharacter->ApplyRewardProjectiles(Reward.ProjectileBonus);
	}
	if (Reward.RepairCharges > 0)
	{
		if (Reward.ItemReward)
		{
			PlayerCharacter->AddItemDefinition(Reward.ItemReward, Reward.RepairCharges);
		}
	}
	if (Reward.WeaponReward)
	{
		PlayerCharacter->AddWeaponDefinition(Reward.WeaponReward);
	}
	if (Reward.ItemReward && Reward.RepairCharges <= 0)
	{
		PlayerCharacter->AddItemDefinition(Reward.ItemReward);
	}

	SetStatus(FString::Printf(TEXT("Reward chosen: %s"), *Reward.DisplayName.ToString()));
	RewardChoices.Reset();
}

float AFortRogueGameMode::GetWind() const
{
	return Wind;
}

EFortRogueBattleState AFortRogueGameMode::GetBattleState() const
{
	return BattleState;
}

AFortRogueBattleCharacter* AFortRogueGameMode::GetPlayerCharacter() const
{
	return PlayerCharacter;
}

AFortRogueBattleCharacter* AFortRogueGameMode::GetEnemyCharacter() const
{
	return EnemyCharacter;
}

TArray<FFortRogueRewardChoice> AFortRogueGameMode::GetRewardChoices() const
{
	return RewardChoices;
}

FText AFortRogueGameMode::GetStatusText() const
{
	return StatusText;
}

void AFortRogueGameMode::SpawnMVPBattle()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const TSubclassOf<AFortRogueDestructibleTerrain> ActualTerrainClass = TerrainClass ? TerrainClass : TSubclassOf<AFortRogueDestructibleTerrain>(AFortRogueDestructibleTerrain::StaticClass());
	Terrain = World->SpawnActor<AFortRogueDestructibleTerrain>(ActualTerrainClass, TerrainLocation, FRotator::ZeroRotator);

	const float SurfaceZ = Terrain ? Terrain->GetSurfaceZ() : TerrainLocation.Z + 520.0f;
	FVector PlayerLocation = TerrainLocation + PlayerSpawnOffset;
	FVector EnemyLocation = TerrainLocation + EnemySpawnOffset;
	PlayerLocation.Z = SurfaceZ + PlayerSpawnOffset.Z;
	EnemyLocation.Z = SurfaceZ + EnemySpawnOffset.Z;

	const TSubclassOf<AFortRogueBattleCharacter> ActualPlayerClass = PlayerCharacterClass ? PlayerCharacterClass : TSubclassOf<AFortRogueBattleCharacter>(AFortRogueBattleCharacter::StaticClass());
	PlayerCharacter = World->SpawnActorDeferred<AFortRogueBattleCharacter>(ActualPlayerClass, FTransform(FRotator::ZeroRotator, PlayerLocation));
	if (PlayerCharacter)
	{
		PlayerCharacter->CharacterDefinition = PlayerDefinition;
		UGameplayStatics::FinishSpawningActor(PlayerCharacter, FTransform(FRotator::ZeroRotator, PlayerLocation));
	}

	const TSubclassOf<AFortRogueBattleCharacter> ActualEnemyClass = EnemyCharacterClass ? EnemyCharacterClass : TSubclassOf<AFortRogueBattleCharacter>(AFortRogueBattleCharacter::StaticClass());
	EnemyCharacter = World->SpawnActorDeferred<AFortRogueBattleCharacter>(ActualEnemyClass, FTransform(FRotator::ZeroRotator, EnemyLocation));
	if (EnemyCharacter)
	{
		EnemyCharacter->CharacterDefinition = EnemyDefinition;
		UGameplayStatics::FinishSpawningActor(EnemyCharacter, FTransform(FRotator::ZeroRotator, EnemyLocation));
	}

	if (PlayerCharacter)
	{
		PlayerCharacter->ConfigureAsEnemy(false);
	}
	if (EnemyCharacter)
	{
		EnemyCharacter->ConfigureAsEnemy(true);
	}

	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0))
	{
		PlayerController->Possess(PlayerCharacter);

		const TSubclassOf<ACameraActor> ActualCameraClass = CameraClass ? CameraClass : TSubclassOf<ACameraActor>(ACameraActor::StaticClass());
		ACameraActor* Camera = World->SpawnActor<ACameraActor>(ActualCameraClass, CameraLocation, CameraRotation);
		if (Camera && Camera->GetCameraComponent())
		{
			Camera->GetCameraComponent()->ProjectionMode = ECameraProjectionMode::Orthographic;
			Camera->GetCameraComponent()->OrthoWidth = CameraOrthoWidth;
			PlayerController->SetViewTarget(Camera);
		}
	}
}

void AFortRogueGameMode::StartPlayerTurn()
{
	BattleState = EFortRogueBattleState::PlayerTurn;
	Wind = FMath::RandRange(MinWind, MaxWind);
	if (PlayerCharacter)
	{
		PlayerCharacter->BeginTurn();
	}
	SetStatus(TEXT("Player turn"));
}

void AFortRogueGameMode::StartEnemyTurn()
{
	BattleState = EFortRogueBattleState::EnemyTurn;
	Wind = FMath::RandRange(MinWind, MaxWind);
	if (EnemyCharacter)
	{
		EnemyCharacter->BeginTurn();
	}
	SetStatus(TEXT("Enemy turn"));

	FTimerHandle EnemyTimerHandle;
	GetWorldTimerManager().SetTimer(EnemyTimerHandle, this, &AFortRogueGameMode::RunEnemyTurn, 0.85f, false);
}

void AFortRogueGameMode::RunEnemyTurn()
{
	if (BattleState != EFortRogueBattleState::EnemyTurn || !EnemyCharacter || !PlayerCharacter || EnemyCharacter->IsDefeated())
	{
		return;
	}

	EnemyCharacter->FireAtTarget(PlayerCharacter);
	const int32 SpawnedProjectiles = EnemyCharacter->FireSelectedWeapon();
	NotifyShotFired(EnemyCharacter, SpawnedProjectiles);
}

void AFortRogueGameMode::FinishShotResolution()
{
	if (EnemyCharacter && EnemyCharacter->IsDefeated())
	{
		EnterRewardState();
		return;
	}

	if (PlayerCharacter && PlayerCharacter->IsDefeated())
	{
		BattleState = EFortRogueBattleState::Lost;
		SetStatus(TEXT("Defeat"));
		return;
	}

	if (LastShooter.IsValid() && LastShooter->IsEnemy())
	{
		StartPlayerTurn();
	}
	else
	{
		StartEnemyTurn();
	}
}

void AFortRogueGameMode::EnterRewardState()
{
	BattleState = EFortRogueBattleState::Reward;
	BuildRewardChoices();
	SetStatus(TEXT("Victory - choose a reward"));
}

void AFortRogueGameMode::BuildRewardChoices()
{
	RewardChoices.Reset();
	if (RewardPool.Num() > 0)
	{
		RewardChoices = RewardPool;
		return;
	}

	FFortRogueRewardChoice Warhead;
	Warhead.Type = EFortRogueRewardType::Trait;
	Warhead.DisplayName = FText::FromString(TEXT("Heavy Warhead"));
	Warhead.Description = FText::FromString(TEXT("+15 weapon damage"));
	Warhead.RewardTag = FortRogueGameplayTags::Trait_Damage;
	Warhead.DamageBonus = 15.0f;
	RewardChoices.Add(Warhead);

	FFortRogueRewardChoice Cluster;
	Cluster.Type = EFortRogueRewardType::Weapon;
	Cluster.DisplayName = FText::FromString(TEXT("Cluster Shell"));
	Cluster.Description = FText::FromString(TEXT("+1 projectile per shot"));
	Cluster.RewardTag = FortRogueGameplayTags::Trait_Projectiles;
	Cluster.ProjectileBonus = 1;
	RewardChoices.Add(Cluster);

	FFortRogueRewardChoice Repair;
	Repair.Type = EFortRogueRewardType::Consumable;
	Repair.DisplayName = FText::FromString(TEXT("Repair Kit"));
	Repair.Description = FText::FromString(TEXT("+2 repair item charges"));
	Repair.RewardTag = FortRogueGameplayTags::Item_Repair;
	UFortRogueItemDefinition* RepairItem = NewObject<UFortRogueItemDefinition>(this, TEXT("RewardRepairKit"));
	RepairItem->DisplayName = Repair.DisplayName;
	RepairItem->ItemTag = FortRogueGameplayTags::Item_Repair;
	RepairItem->ItemType = EFortRogueItemType::Heal;
	RepairItem->InitialCharges = 1;
	RepairItem->HealAmount = 35.0f;
	Repair.ItemReward = RepairItem;
	Repair.RepairCharges = 2;
	RewardChoices.Add(Repair);
}

void AFortRogueGameMode::SetStatus(const FString& NewStatus)
{
	StatusText = FText::FromString(NewStatus);
}
