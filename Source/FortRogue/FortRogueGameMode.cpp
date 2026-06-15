// Copyright Epic Games, Inc. All Rights Reserved.

#include "FortRogueGameMode.h"

#include "Combat/FortRogueBattleCharacter.h"
#include "Combat/FortRogueDestructibleTerrain.h"
#include "Combat/FortRogueProjectile.h"
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

namespace
{
constexpr float TerrainCameraPadding = 240.0f;
constexpr float MinimumTerrainCameraOrthoWidth = 1200.0f;
constexpr float ExpectedWideViewportAspectRatio = 16.0f / 9.0f;
const FRotator BattleCameraRotation(0.0f, 90.0f, 0.0f);
}

AFortRogueGameMode::AFortRogueGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
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

void AFortRogueGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateBattleCamera(DeltaSeconds);
}

void AFortRogueGameMode::NotifyProjectileSpawned(AFortRogueProjectile* Projectile)
{
	if (Projectile)
	{
		ActiveProjectiles.Add(Projectile);
	}
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
	if (Projectile)
	{
		LastImpactCameraLocation = Projectile->GetActorLocation();
		ActiveProjectiles.RemoveAll([Projectile](const TWeakObjectPtr<AFortRogueProjectile>& ActiveProjectile)
		{
			return !ActiveProjectile.IsValid() || ActiveProjectile.Get() == Projectile;
		});
	}

	if (BattleState != EFortRogueBattleState::ResolvingShot)
	{
		return;
	}

	PendingProjectiles = FMath::Max(0, PendingProjectiles - 1);
	if (PendingProjectiles == 0)
	{
		bHoldingImpactCamera = true;
		GetWorldTimerManager().SetTimer(ShotResolutionTimerHandle, this, &AFortRogueGameMode::FinishShotResolution, ShotImpactCameraHoldSeconds, false);
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
	Terrain = World->SpawnActorDeferred<AFortRogueDestructibleTerrain>(ActualTerrainClass, FTransform(FRotator::ZeroRotator, TerrainLocation));
	if (Terrain)
	{
		Terrain->MapDefinition = TerrainMapDefinition;
		UGameplayStatics::FinishSpawningActor(Terrain, FTransform(FRotator::ZeroRotator, TerrainLocation));
	}

	FVector PlayerLocation = TerrainLocation + PlayerSpawnOffset;
	FVector EnemyLocation = TerrainLocation + EnemySpawnOffset;
	if (Terrain)
	{
		PlayerLocation = Terrain->GetPlayerSpawnWorldLocation();
		EnemyLocation = Terrain->GetEnemySpawnWorldLocation();
	}
	else
	{
		PlayerLocation.Z = TerrainLocation.Z + 520.0f + PlayerSpawnOffset.Z;
		EnemyLocation.Z = TerrainLocation.Z + 520.0f + EnemySpawnOffset.Z;
	}

	const TSubclassOf<AFortRogueBattleCharacter> ActualPlayerClass = PlayerCharacterClass ? PlayerCharacterClass : TSubclassOf<AFortRogueBattleCharacter>(AFortRogueBattleCharacter::StaticClass());
	PlayerCharacter = World->SpawnActorDeferred<AFortRogueBattleCharacter>(ActualPlayerClass, FTransform(FRotator::ZeroRotator, PlayerLocation));
	if (PlayerCharacter)
	{
		PlayerCharacter->CharacterDefinition = PlayerDefinition;
		PlayerCharacter->SetTerrain(Terrain);
		UGameplayStatics::FinishSpawningActor(PlayerCharacter, FTransform(FRotator::ZeroRotator, PlayerLocation));
	}

	const TSubclassOf<AFortRogueBattleCharacter> ActualEnemyClass = EnemyCharacterClass ? EnemyCharacterClass : TSubclassOf<AFortRogueBattleCharacter>(AFortRogueBattleCharacter::StaticClass());
	EnemyCharacter = World->SpawnActorDeferred<AFortRogueBattleCharacter>(ActualEnemyClass, FTransform(FRotator::ZeroRotator, EnemyLocation));
	if (EnemyCharacter)
	{
		EnemyCharacter->CharacterDefinition = EnemyDefinition;
		EnemyCharacter->SetTerrain(Terrain);
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

	const TSubclassOf<ACameraActor> ActualCameraClass = CameraClass ? CameraClass : TSubclassOf<ACameraActor>(ACameraActor::StaticClass());
	BattleCamera = World->SpawnActor<ACameraActor>(ActualCameraClass, GetDesiredCameraLocation(), GetBattleCameraRotation());
	if (BattleCamera && BattleCamera->GetCameraComponent())
	{
		BattleCamera->SetActorRotation(GetBattleCameraRotation());
		BattleCamera->GetCameraComponent()->ProjectionMode = ECameraProjectionMode::Orthographic;
		BattleCamera->GetCameraComponent()->OrthoWidth = GetInitialCameraOrthoWidth();
	}

	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0))
	{
		PlayerController->Possess(PlayerCharacter);
		PlayerController->SetViewTarget(BattleCamera);
	}
}

void AFortRogueGameMode::StartPlayerTurn()
{
	ResetShotCameraState();
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
	ResetShotCameraState();
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
	if (SpawnedProjectiles <= 0)
	{
		EnemyCharacter->EndTurn();
		StartPlayerTurn();
		return;
	}

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
	ResetShotCameraState();
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

void AFortRogueGameMode::UpdateBattleCamera(float DeltaSeconds)
{
	if (!BattleCamera)
	{
		return;
	}

	const FVector CurrentLocation = BattleCamera->GetActorLocation();
	const FVector DesiredLocation = GetDesiredCameraLocation();
	const FVector NewLocation = FMath::VInterpTo(CurrentLocation, DesiredLocation, DeltaSeconds, CameraFollowInterpSpeed);
	BattleCamera->SetActorLocation(NewLocation);
	BattleCamera->SetActorRotation(GetBattleCameraRotation());
}

void AFortRogueGameMode::ResetShotCameraState()
{
	ActiveProjectiles.Reset();
	bHoldingImpactCamera = false;
	GetWorldTimerManager().ClearTimer(ShotResolutionTimerHandle);
}

float AFortRogueGameMode::GetInitialCameraOrthoWidth() const
{
	if (!Terrain || !TerrainMapDefinition)
	{
		return CameraOrthoWidth;
	}

	const float WidthFit = Terrain->Width + TerrainCameraPadding;
	const float HeightFit = Terrain->Height * ExpectedWideViewportAspectRatio + TerrainCameraPadding;
	return FMath::Max(MinimumTerrainCameraOrthoWidth, FMath::Max(WidthFit, HeightFit));
}

FRotator AFortRogueGameMode::GetBattleCameraRotation() const
{
	return BattleCameraRotation;
}

FVector AFortRogueGameMode::GetDesiredCameraLocation() const
{
	FVector DesiredLocation = CameraLocation;
	if (BattleCamera)
	{
		DesiredLocation = BattleCamera->GetActorLocation();
		DesiredLocation.Y = CameraLocation.Y;
	}

	const FVector FocusLocation = GetCameraFocusLocation();
	DesiredLocation.X = FocusLocation.X;
	DesiredLocation.Z = FocusLocation.Z;
	return ClampCameraLocationToTerrainBounds(DesiredLocation);
}

FVector AFortRogueGameMode::GetCameraFocusLocation() const
{
	if (BattleState == EFortRogueBattleState::ResolvingShot)
	{
		for (const TWeakObjectPtr<AFortRogueProjectile>& Projectile : ActiveProjectiles)
		{
			if (Projectile.IsValid())
			{
				const FVector ProjectileLocation = Projectile->GetActorLocation();
				return FVector(ProjectileLocation.X, CameraLocation.Y, ProjectileLocation.Z + CameraProjectileZOffset);
			}
		}

		if (bHoldingImpactCamera)
		{
			return FVector(LastImpactCameraLocation.X, CameraLocation.Y, LastImpactCameraLocation.Z + CameraProjectileZOffset);
		}
	}

	const AFortRogueBattleCharacter* FocusCharacter = nullptr;
	if (BattleState == EFortRogueBattleState::EnemyTurn)
	{
		FocusCharacter = EnemyCharacter;
	}
	else
	{
		FocusCharacter = PlayerCharacter;
	}

	if (FocusCharacter)
	{
		const FVector CharacterLocation = FocusCharacter->GetActorLocation();
		return FVector(CharacterLocation.X, CameraLocation.Y, CharacterLocation.Z + CameraTurnZOffset);
	}

	return CameraLocation;
}

FVector AFortRogueGameMode::ClampCameraLocationToTerrainBounds(const FVector& DesiredLocation) const
{
	if (!Terrain)
	{
		return DesiredLocation;
	}

	float OrthoWidth = CameraOrthoWidth;
	if (BattleCamera && BattleCamera->GetCameraComponent())
	{
		OrthoWidth = BattleCamera->GetCameraComponent()->OrthoWidth;
	}
	else
	{
		OrthoWidth = GetInitialCameraOrthoWidth();
	}

	const float ViewHalfWidth = OrthoWidth * 0.5f;
	const float TerrainHalfWidth = Terrain->Width * 0.5f;
	const float PaddingHalf = TerrainCameraPadding * 0.5f;
	const float TerrainCenterX = Terrain->GetActorLocation().X;

	FVector ClampedLocation = DesiredLocation;
	if (ViewHalfWidth >= TerrainHalfWidth + PaddingHalf)
	{
		ClampedLocation.X = TerrainCenterX;
	}
	else
	{
		const float MinCameraX = TerrainCenterX - TerrainHalfWidth - PaddingHalf + ViewHalfWidth;
		const float MaxCameraX = TerrainCenterX + TerrainHalfWidth + PaddingHalf - ViewHalfWidth;
		ClampedLocation.X = FMath::Clamp(static_cast<float>(DesiredLocation.X), MinCameraX, MaxCameraX);
	}

	return ClampedLocation;
}
