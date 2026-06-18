// Copyright Epic Games, Inc. All Rights Reserved.

#include "FortRogueGameMode.h"

#include "Characters/FortRogueCharacterDefinition.h"
#include "Combat/FortRogueBattleCharacter.h"
#include "Combat/FortRogueDestructibleTerrain.h"
#include "Combat/FortRogueProjectile.h"
#include "FortRogue.h"
#include "FortRogueGameplayTags.h"
#include "FortRogueHUD.h"
#include "FortRoguePlayerController.h"
#include "Items/FortRogueItemDefinition.h"
#include "Perks/FortRoguePerkDefinition.h"
#include "Run/FortRogueDefaultLoadoutDefinition.h"
#include "Run/FortRogueStageRunDefinition.h"
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

	CurrentStage = 1;
	if (StageRunDefinition)
	{
		StageRunDefinition->NormalizeStageData();
	}
	EncounteredEnemyDefinitions.Reset();
	SelectNextEnemyDefinition();
	SpawnMVPBattle();
	StartPlayerTurn();
}

void AFortRogueGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateBattleCamera(DeltaSeconds);
	CheckTurnDefeatState();
}

void AFortRogueGameMode::NotifyProjectileSpawned(AFortRogueProjectile* Projectile)
{
	if (Projectile)
	{
		ActiveProjectiles.Add(Projectile);
		if (BattleState == EFortRogueBattleState::ResolvingShot)
		{
			++PendingProjectiles;
		}
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
	ApplyRewardToPlayer(Reward);

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

int32 AFortRogueGameMode::GetCurrentStage() const
{
	return CurrentStage;
}

int32 AFortRogueGameMode::GetMaxStages() const
{
	return GetConfiguredStageCount();
}

void AFortRogueGameMode::SpawnMVPBattle()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	ClearBattleStage(PlayerCharacter != nullptr);

	const TSubclassOf<AFortRogueDestructibleTerrain> ActualTerrainClass = TerrainClass ? TerrainClass : TSubclassOf<AFortRogueDestructibleTerrain>(AFortRogueDestructibleTerrain::StaticClass());
	Terrain = World->SpawnActorDeferred<AFortRogueDestructibleTerrain>(ActualTerrainClass, FTransform(FRotator::ZeroRotator, TerrainLocation));
	if (Terrain)
	{
		Terrain->MapDefinition = GetStageTerrainMapDefinition();
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

	if (PlayerCharacter)
	{
		PlayerCharacter->SetActorLocation(PlayerLocation);
		PlayerCharacter->SetTerrain(Terrain);
	}
	else
	{
		const TSubclassOf<AFortRogueBattleCharacter> ActualPlayerClass = PlayerCharacterClass ? PlayerCharacterClass : TSubclassOf<AFortRogueBattleCharacter>(AFortRogueBattleCharacter::StaticClass());
		PlayerCharacter = World->SpawnActorDeferred<AFortRogueBattleCharacter>(ActualPlayerClass, FTransform(FRotator::ZeroRotator, PlayerLocation));
		if (PlayerCharacter)
		{
			PlayerCharacter->CharacterDefinition = PlayerDefinition;
			PlayerCharacter->DefaultLoadoutDefinition = GetDefaultLoadoutDefinition();
			PlayerCharacter->SetTerrain(Terrain);
			UGameplayStatics::FinishSpawningActor(PlayerCharacter, FTransform(FRotator::ZeroRotator, PlayerLocation));
		}
	}

	const TSubclassOf<AFortRogueBattleCharacter> ActualEnemyClass = EnemyCharacterClass ? EnemyCharacterClass : TSubclassOf<AFortRogueBattleCharacter>(AFortRogueBattleCharacter::StaticClass());
	EnemyCharacter = World->SpawnActorDeferred<AFortRogueBattleCharacter>(ActualEnemyClass, FTransform(FRotator::ZeroRotator, EnemyLocation));
	if (EnemyCharacter)
	{
		EnemyCharacter->CharacterDefinition = CurrentEnemyDefinition ? CurrentEnemyDefinition.Get() : EnemyDefinition.Get();
		EnemyCharacter->DefaultLoadoutDefinition = GetDefaultLoadoutDefinition();
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

void AFortRogueGameMode::ClearBattleStage(bool bKeepPlayerCharacter)
{
	for (const TWeakObjectPtr<AFortRogueProjectile>& ActiveProjectile : ActiveProjectiles)
	{
		if (ActiveProjectile.IsValid())
		{
			ActiveProjectile->Destroy();
		}
	}
	ActiveProjectiles.Reset();
	GetWorldTimerManager().ClearTimer(ShotResolutionTimerHandle);
	PendingProjectiles = 0;
	LastShooter.Reset();
	bHoldingImpactCamera = false;

	if (EnemyCharacter)
	{
		EnemyCharacter->Destroy();
		EnemyCharacter = nullptr;
	}

	if (Terrain)
	{
		Terrain->Destroy();
		Terrain = nullptr;
	}

	if (BattleCamera)
	{
		BattleCamera->Destroy();
		BattleCamera = nullptr;
	}

	if (!bKeepPlayerCharacter && PlayerCharacter)
	{
		PlayerCharacter->Destroy();
		PlayerCharacter = nullptr;
	}
}

void AFortRogueGameMode::SelectNextEnemyDefinition()
{
	TArray<UFortRogueCharacterDefinition*> Candidates;
	const TArray<TObjectPtr<UFortRogueCharacterDefinition>>* EnemyPool = StageRunDefinition ? &StageRunDefinition->EnemyDefinitionPool : nullptr;
	if (EnemyPool)
	{
		for (UFortRogueCharacterDefinition* Candidate : *EnemyPool)
		{
			if (Candidate && !EncounteredEnemyDefinitions.Contains(Candidate))
			{
				Candidates.Add(Candidate);
			}
		}
	}

	if (Candidates.Num() == 0 && EnemyPool && EnemyPool->Num() > 0)
	{
		EncounteredEnemyDefinitions.Reset();
		for (UFortRogueCharacterDefinition* Candidate : *EnemyPool)
		{
			if (Candidate)
			{
				Candidates.Add(Candidate);
			}
		}
	}

	if (Candidates.Num() > 0)
	{
		CurrentEnemyDefinition = Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
		EncounteredEnemyDefinitions.AddUnique(CurrentEnemyDefinition);
	}
	else
	{
		CurrentEnemyDefinition = EnemyDefinition;
	}
}

UFortRogueTerrainMapDefinition* AFortRogueGameMode::GetStageTerrainMapDefinition() const
{
	if (CurrentEnemyDefinition && CurrentEnemyDefinition->BattleMapDefinition)
	{
		return CurrentEnemyDefinition->BattleMapDefinition;
	}

	if (StageRunDefinition && StageRunDefinition->DefaultTerrainMapDefinition)
	{
		return StageRunDefinition->DefaultTerrainMapDefinition;
	}

	return TerrainMapDefinition;
}

UFortRogueDefaultLoadoutDefinition* AFortRogueGameMode::GetDefaultLoadoutDefinition() const
{
	return StageRunDefinition ? StageRunDefinition->DefaultLoadoutDefinition : nullptr;
}

const FFortRogueStageDifficultyData& AFortRogueGameMode::GetCurrentStageDifficulty() const
{
	if (StageRunDefinition)
	{
		return StageRunDefinition->GetStageDifficulty(CurrentStage);
	}

	static const FFortRogueStageDifficultyData DefaultDifficulty;
	return DefaultDifficulty;
}

int32 AFortRogueGameMode::GetConfiguredStageCount() const
{
	return StageRunDefinition ? FMath::Max(1, StageRunDefinition->StageCount) : 1;
}

void AFortRogueGameMode::HandleEnemyDefeated()
{
	ResetShotCameraState();
	if (CurrentStage >= GetConfiguredStageCount())
	{
		BattleState = EFortRogueBattleState::Won;
		SetStatus(TEXT("Run complete"));
		return;
	}

	ApplyRandomRewardAndLog();
	++CurrentStage;
	SelectNextEnemyDefinition();
	SpawnMVPBattle();
	StartPlayerTurn();
}

void AFortRogueGameMode::ApplyRandomRewardAndLog()
{
	if (!PlayerCharacter)
	{
		return;
	}

	const TArray<FFortRogueRewardChoice>* Rewards = StageRunDefinition ? &StageRunDefinition->RewardPool : nullptr;
	if (!Rewards || Rewards->Num() == 0)
	{
		UE_LOG(LogFortRogue, Log, TEXT("No reward pool configured for stage %d."), CurrentStage);
		return;
	}

	const int32 RewardIndex = FMath::RandRange(0, Rewards->Num() - 1);
	const FFortRogueRewardChoice& Reward = (*Rewards)[RewardIndex];
	ApplyRewardToPlayer(Reward);
	UE_LOG(LogFortRogue, Log, TEXT("Random reward selected after stage %d: %s"), CurrentStage, *Reward.DisplayName.ToString());
}

void AFortRogueGameMode::ApplyRewardToPlayer(const FFortRogueRewardChoice& Reward)
{
	if (!PlayerCharacter)
	{
		return;
	}

	if (Reward.PerkReward)
	{
		PlayerCharacter->ApplyPerkDefinition(Reward.PerkReward);
	}
	if (Reward.GrantedAbilitySet)
	{
		PlayerCharacter->GrantAbilitySet(Reward.GrantedAbilitySet);
	}
	PlayerCharacter->GrantShotModifiers(Reward.ShotModifiers);
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
	if (Reward.RepairCharges > 0 && Reward.ItemReward)
	{
		PlayerCharacter->AddItemDefinition(Reward.ItemReward, Reward.RepairCharges);
	}
	if (Reward.WeaponReward)
	{
		PlayerCharacter->AddWeaponDefinition(Reward.WeaponReward);
	}
	if (Reward.ItemReward && Reward.RepairCharges <= 0)
	{
		PlayerCharacter->AddItemDefinition(Reward.ItemReward);
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
	SetStatus(FString::Printf(TEXT("Stage %d/%d - Player turn"), CurrentStage, GetConfiguredStageCount()));
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
	SetStatus(FString::Printf(TEXT("Stage %d/%d - Enemy turn"), CurrentStage, GetConfiguredStageCount()));

	FTimerHandle EnemyTimerHandle;
	GetWorldTimerManager().SetTimer(EnemyTimerHandle, this, &AFortRogueGameMode::RunEnemyTurn, GetCurrentStageDifficulty().EnemyTurnDelaySeconds, false);
}

void AFortRogueGameMode::RunEnemyTurn()
{
	if (BattleState != EFortRogueBattleState::EnemyTurn || !EnemyCharacter || !PlayerCharacter || EnemyCharacter->IsDefeated())
	{
		return;
	}

	EnemyCharacter->FireAtTarget(PlayerCharacter, GetCurrentStageDifficulty());
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
		HandleEnemyDefeated();
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
	if (StageRunDefinition && StageRunDefinition->RewardPool.Num() > 0)
	{
		RewardChoices = StageRunDefinition->RewardPool;
	}
}

void AFortRogueGameMode::CheckTurnDefeatState()
{
	if (BattleState != EFortRogueBattleState::PlayerTurn && BattleState != EFortRogueBattleState::EnemyTurn)
	{
		return;
	}

	if (EnemyCharacter && EnemyCharacter->IsDefeated())
	{
		HandleEnemyDefeated();
		return;
	}

	if (PlayerCharacter && PlayerCharacter->IsDefeated())
	{
		BattleState = EFortRogueBattleState::Lost;
		SetStatus(TEXT("Defeat"));
	}
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
	if (!Terrain)
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
