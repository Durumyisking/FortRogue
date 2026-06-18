// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystemInterface.h"
#include "AbilitySystem/FortRogueAbilitySet.h"
#include "Characters/FortRogueCharacterDefinition.h"
#include "Combat/FortRogueShotSpec.h"
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Run/FortRogueStageRunDefinition.h"
#include "Weapons/FortRogueWeaponDefinition.h"
#include "FortRogueBattleCharacter.generated.h"

class UFortRogueAbilitySet;
class UFortRogueAbilitySystemComponent;
class UFortRogueCombatSet;
class UFortRogueDefaultLoadoutDefinition;
class UPaperFlipbookComponent;
class UStaticMeshComponent;
class UFortRogueItemDefinition;
class UFortRoguePerkDefinition;
class AFortRogueDestructibleTerrain;

USTRUCT()
struct FFortRogueGrantedAbilitySetEntry
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UFortRogueAbilitySet> AbilitySet;

	UPROPERTY()
	FFortRogueAbilitySet_GrantedHandles Handles;
};

UCLASS()
class FORTROGUE_API AFortRogueBattleCharacter : public APawn, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AFortRogueBattleCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Character")
	void InitializeFromDefinition(UFortRogueCharacterDefinition* CharacterDefinition);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Character")
	void ConfigureAsEnemy(bool bNewEnemy);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Turn")
	void BeginTurn();

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Turn")
	void EndTurn();

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Turn")
	void MoveHorizontal(float Axis, float DeltaSeconds);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Combat")
	void AdjustAim(float Axis, float DeltaSeconds);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Combat")
	void AdjustPower(float Axis, float DeltaSeconds);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Combat")
	void BeginShotCharge();

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Combat")
	void UpdateShotCharge(float DeltaSeconds);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Combat")
	int32 ReleaseShotCharge();

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Combat")
	void SelectWeapon(int32 WeaponIndex);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Combat")
	int32 FireSelectedWeapon();

	UFUNCTION(BlueprintCallable, Category = "FortRogue|AI")
	void FireAtTarget(AFortRogueBattleCharacter* Target, const FFortRogueStageDifficultyData& DifficultyData);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Combat")
	void ApplyDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Terrain")
	void ReevaluateTerrainSupport();

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Terrain")
	void SetTerrain(AFortRogueDestructibleTerrain* InTerrain);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Items")
	bool UseItemByType(EFortRogueItemType ItemType);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Items")
	bool UseItemByTag(FGameplayTag ItemTag);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Rewards")
	void ApplyRewardDamage(float BonusDamage);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Rewards")
	void ApplyRewardHealth(float BonusHealth);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Rewards")
	void ApplyRewardProjectiles(int32 BonusProjectiles);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Rewards")
	void ApplyPerkDefinition(UFortRoguePerkDefinition* PerkDefinition);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Abilities")
	void GrantAbilitySet(UFortRogueAbilitySet* AbilitySet);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Abilities")
	bool RemoveAbilitySet(UFortRogueAbilitySet* AbilitySet);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Weapons")
	void AddWeaponDefinition(UFortRogueWeaponDefinition* WeaponDefinition);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Items")
	void AddItemDefinition(UFortRogueItemDefinition* ItemDefinition, int32 ChargesOverride = -1);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Character")
	bool IsEnemy() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Turn")
	bool IsActiveTurn() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Turn")
	bool HasFiredThisTurn() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Character")
	bool IsDefeated() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Stats")
	float GetHealth() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Stats")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Stats")
	float GetMoveBudget() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	float GetAimAngle() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	float GetShotPower() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	float GetShotChargeAlpha() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	bool IsChargingShot() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Items")
	int32 GetItemCharges(EFortRogueItemType ItemType) const;

	const FFortRogueWeaponSpec& GetCurrentWeapon() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Weapons")
	FFortRogueWeaponSpec GetCurrentWeaponSpec() const;

	const TArray<FFortRogueWeaponSpec>& GetWeaponLoadout() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Weapons")
	TArray<FFortRogueWeaponSpec> GetWeaponLoadoutForBlueprint() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Weapons")
	int32 GetSelectedWeaponIndex() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Character")
	FText GetCharacterDisplayName() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FortRogue|Abilities")
	TArray<TObjectPtr<UFortRogueAbilitySet>> StartupAbilitySets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FortRogue|Character")
	TObjectPtr<UFortRogueCharacterDefinition> CharacterDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FortRogue|Weapons")
	TArray<FFortRogueWeaponSpec> WeaponLoadout;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FortRogue|Items")
	TArray<FFortRogueItemStack> ItemLoadout;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FortRogue|Perks")
	TArray<FFortRogueShotModifierSpec> GrantedShotModifiers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FortRogue|Loadout")
	TObjectPtr<UFortRogueDefaultLoadoutDefinition> DefaultLoadoutDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Combat|Charge", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinShotPower = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Combat|Charge", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxShotPower = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Combat|Charge", meta = (ClampMin = "0.01"))
	float ShotChargeSeconds = 1.25f;

private:
	AFortRogueDestructibleTerrain* FindTerrain() const;
	bool UseItemStack(FFortRogueItemStack& ItemStack);
	bool IsSupportedByTerrain() const;
	bool FindFootprintSurfaceZ(const AFortRogueDestructibleTerrain& Terrain, float CenterWorldX, float StartWorldZ, float SearchDistance, float& OutSurfaceZ) const;
	bool IsFootprintBlocked(const AFortRogueDestructibleTerrain& Terrain, const FVector& CenterLocation, float FootWorldZ) const;
	bool IsSlopeTraversable(float CurrentFootWorldZ, float NextSurfaceWorldZ, float HorizontalDistance, float TerrainCellSize) const;
	float ClampWorldXToTerrainBounds(const AFortRogueDestructibleTerrain& Terrain, float WorldX) const;
	void UpdateBodyTerrainAlignment(const AFortRogueDestructibleTerrain* Terrain);
	void ApplyTerrainGravity(float DeltaSeconds);
	bool CheckFallDeath();
	void SnapToTerrain();
	void SetFacingFromAxis(float Axis);
	float GetBodyPitchDegrees() const;
	FVector GetProjectileLaunchDirection(float SpreadDegrees) const;
	FVector GetProjectileSpawnLocation(const FVector& LaunchDirection) const;
	FFortRogueShotSpec BuildShotSpec(const FFortRogueWeaponSpec& Weapon) const;
	void DrawProjectileTrajectory() const;
	void GrantStartupAbilitySets();
	void EnsureDefaultLoadout();

	UPROPERTY(VisibleAnywhere, Category = "FortRogue")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "FortRogue")
	TObjectPtr<UStaticMeshComponent> Body;

	UPROPERTY(VisibleAnywhere, Category = "FortRogue")
	TObjectPtr<UPaperFlipbookComponent> BodySprite;

	UPROPERTY(VisibleAnywhere, Category = "FortRogue|Abilities")
	TObjectPtr<UFortRogueAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, Category = "FortRogue|Abilities")
	TObjectPtr<UFortRogueCombatSet> CombatSet;

	UPROPERTY()
	TObjectPtr<AFortRogueDestructibleTerrain> AssignedTerrain;

	UPROPERTY()
	TArray<FFortRogueGrantedAbilitySetEntry> GrantedAbilitySetEntries;

	FText CharacterDisplayName;
	bool bEnemy = false;
	bool bActiveTurn = false;
	bool bFiredThisTurn = false;
	bool bFacingRight = true;
	float AimAngle = 45.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Combat|Charge", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float ShotPower = 0.25f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float MoveSpeed = 260.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float FootOffsetZ = 45.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float FootProbeHalfWidth = 22.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float MaxStepUp = 34.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float MaxStepDown = 56.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float MaxSlopeAngleDegrees = 52.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float GroundSnapDistance = 12.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float BodySlopeProbeHalfWidth = 28.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float MaxBodySlopeVisualDegrees = 45.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float GravityAcceleration = 980.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float MaxFallSpeed = 1600.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float FallDeathDepth = 400.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Combat|Debug", meta = (AllowPrivateAccess = "true"))
	bool bDrawProjectileTrajectory = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Combat|Debug", meta = (AllowPrivateAccess = "true", ClampMin = "1"))
	int32 TrajectoryDebugSteps = 50;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Combat|Debug", meta = (AllowPrivateAccess = "true", ClampMin = "0.01"))
	float TrajectoryDebugTimeStep = 0.08f;
	float ShotChargeElapsed = 0.0f;
	float VerticalVelocity = 0.0f;
	bool bChargingShot = false;
	float PendingAttackMultiplier = 1.0f;
	int32 SelectedWeaponIndex = 0;
};
