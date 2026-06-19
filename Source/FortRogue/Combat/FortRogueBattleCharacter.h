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

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	bool CanBeginShotCharge() const;

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Combat")
	void UpdateShotCharge(float DeltaSeconds);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Combat")
	int32 ReleaseShotCharge();

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Combat")
	void SelectWeapon(int32 WeaponIndex);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Weapons")
	bool SelectWeaponByTag(FGameplayTag WeaponTag);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Weapons")
	bool CanSelectWeapon(int32 WeaponIndex) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Weapons")
	bool CanSelectWeaponByTag(FGameplayTag WeaponTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Weapons")
	int32 GetWeaponIndexByTag(FGameplayTag WeaponTag) const;

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Combat")
	int32 FireSelectedWeapon();

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	bool CanFireSelectedWeapon() const;

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

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Items")
	bool UseItemByIndex(int32 ItemIndex);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Items")
	bool CanUseItemByType(EFortRogueItemType ItemType) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Items")
	bool CanUseItemByTag(FGameplayTag ItemTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Items")
	bool CanUseItemByIndex(int32 ItemIndex) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Items")
	int32 GetItemIndexByTag(FGameplayTag ItemTag) const;

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Rewards")
	void ApplyRewardDamage(float BonusDamage);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Rewards")
	void ApplyRewardHealth(float BonusHealth);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Rewards")
	void ApplyRewardMoveBudget(float BonusMoveBudget);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Rewards")
	void ApplyRewardProjectiles(int32 BonusProjectiles);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Rewards")
	void ApplyRewardShotPowerMultiplier(float BonusMultiplier);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Rewards")
	void ApplyPerkDefinition(UFortRoguePerkDefinition* PerkDefinition);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Abilities")
	void GrantAbilitySet(UFortRogueAbilitySet* AbilitySet);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Abilities")
	bool RemoveAbilitySet(UFortRogueAbilitySet* AbilitySet);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Abilities")
	int32 RemoveAbilitySetsByTag(FGameplayTag AbilitySetTag);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Abilities")
	int32 GetGrantedAbilitySetCount(UFortRogueAbilitySet* AbilitySet) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Abilities")
	int32 GetGrantedAbilitySetCountByTag(FGameplayTag AbilitySetTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Abilities")
	bool HasGrantedAbilitySetByTag(FGameplayTag AbilitySetTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Abilities")
	TArray<UFortRogueAbilitySet*> GetGrantedAbilitySetsForBlueprint() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Abilities")
	FText GetGrantedAbilitySetsSummary() const;

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Combat")
	void GrantShotModifiers(const TArray<FFortRogueShotModifierSpec>& ShotModifiers);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Combat")
	void GrantPendingShotModifiers(const TArray<FFortRogueShotModifierSpec>& ShotModifiers);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Combat")
	int32 RemoveGrantedShotModifiersByTag(FGameplayTag ModifierTag);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Combat")
	int32 RemovePendingShotModifiersByTag(FGameplayTag ModifierTag);

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

	UFUNCTION(BlueprintPure, Category = "FortRogue|Stats")
	float GetMaxMoveBudget() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Stats")
	float GetDamageBonus() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Stats")
	float GetShotPowerMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Stats")
	float GetProjectileCount() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Stats")
	bool TryGetCombatAttributeValueByTag(FGameplayTag AttributeTag, float& OutValue) const;

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Stats")
	bool TryApplyCombatAttributeDeltaByTag(FGameplayTag AttributeTag, float DeltaValue);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Stats")
	FText GetCombatStatsSummary() const;

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

	UFUNCTION(BlueprintPure, Category = "FortRogue|Items")
	int32 GetItemChargesByTag(FGameplayTag ItemTag) const;

	const TArray<FFortRogueItemStack>& GetItemLoadout() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Items")
	TArray<FFortRogueItemStack> GetItemLoadoutForBlueprint() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	TArray<FFortRogueShotModifierSpec> GetGrantedShotModifiersForBlueprint() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	TArray<FFortRogueShotModifierSpec> GetPendingShotModifiersForBlueprint() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	FText GetGrantedShotModifiersSummary() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	FText GetPendingShotModifiersSummary() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	int32 GetGrantedShotModifierCountByTag(FGameplayTag ModifierTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	int32 GetPendingShotModifierCountByTag(FGameplayTag ModifierTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	bool HasGrantedShotModifierByTag(FGameplayTag ModifierTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	bool HasPendingShotModifierByTag(FGameplayTag ModifierTag) const;

	const FFortRogueWeaponSpec& GetCurrentWeapon() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Weapons")
	FFortRogueWeaponSpec GetCurrentWeaponSpec() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	FFortRogueShotSpec GetCurrentShotSpec() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	FText GetCurrentShotSummary() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	bool DoesShotModifierMeetCurrentShotConditions(const FFortRogueShotModifierSpec& ShotModifier) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	FText GetShotModifierCurrentConditionFailureSummary(const FFortRogueShotModifierSpec& ShotModifier) const;

	const TArray<FFortRogueWeaponSpec>& GetWeaponLoadout() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Weapons")
	TArray<FFortRogueWeaponSpec> GetWeaponLoadoutForBlueprint() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Weapons")
	int32 GetSelectedWeaponIndex() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Character")
	FText GetCharacterDisplayName() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FortRogue|Abilities", meta = (ToolTip = "캐릭터가 BeginPlay 시 자동으로 부여받을 AbilitySet 목록입니다."))
	TArray<TObjectPtr<UFortRogueAbilitySet>> StartupAbilitySets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FortRogue|Character", meta = (ToolTip = "이 캐릭터에 적용할 기본 캐릭터 데이터입니다. 체력, 이동력, 로드아웃 같은 초기값을 설정합니다."))
	TObjectPtr<UFortRogueCharacterDefinition> CharacterDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FortRogue|Weapons", meta = (ToolTip = "캐릭터가 사용할 무기 스펙 목록입니다. 첫 번째 항목이 기본 선택 무기입니다."))
	TArray<FFortRogueWeaponSpec> WeaponLoadout;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FortRogue|Items", meta = (ToolTip = "캐릭터가 보유한 아이템과 사용 가능 횟수 목록입니다."))
	TArray<FFortRogueItemStack> ItemLoadout;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FortRogue|Perks", meta = (ToolTip = "캐릭터에게 지속 적용되는 샷 modifier 목록입니다. 퍽이나 보상으로 얻은 장기 효과를 담습니다."))
	TArray<FFortRogueShotModifierSpec> GrantedShotModifiers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FortRogue|Items", meta = (ToolTip = "다음 발사에만 적용되는 샷 modifier 목록입니다. 공격 아이템처럼 1회성 효과를 담습니다."))
	TArray<FFortRogueShotModifierSpec> PendingShotModifiers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FortRogue|Loadout", meta = (ToolTip = "CharacterDefinition에 로드아웃이 없을 때 사용할 기본 로드아웃 데이터입니다."))
	TObjectPtr<UFortRogueDefaultLoadoutDefinition> DefaultLoadoutDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Combat|Charge", meta = (ClampMin = "0.0", ClampMax = "1.0", ToolTip = "샷 차지를 시작했을 때의 최소 파워입니다. 0~1 범위를 사용하세요."))
	float MinShotPower = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Combat|Charge", meta = (ClampMin = "0.0", ClampMax = "1.0", ToolTip = "샷 차지가 끝까지 찼을 때의 최대 파워입니다. 0~1 범위를 사용하세요."))
	float MaxShotPower = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Combat|Charge", meta = (ClampMin = "0.01", ToolTip = "MinShotPower에서 MaxShotPower까지 차지되는 데 걸리는 시간입니다. 초 단위입니다."))
	float ShotChargeSeconds = 1.25f;

private:
	AFortRogueDestructibleTerrain* FindTerrain() const;
	bool CanUseAnyItem() const;
	bool CanUseItemStack(const FFortRogueItemStack& ItemStack) const;
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Combat|Charge", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0", ToolTip = "현재 발사 파워입니다. 차지 중 MinShotPower와 MaxShotPower 사이에서 갱신됩니다."))
	float ShotPower = 0.25f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ToolTip = "턴 중 좌우 이동 속도입니다. 월드 단위/초 기준입니다."))
	float MoveSpeed = 260.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ToolTip = "캐릭터 위치에서 발 접지점을 찾을 때 아래로 내리는 Z 오프셋입니다."))
	float FootOffsetZ = 45.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ToolTip = "발판 판정을 위해 캐릭터 중심 좌우로 검사할 반폭입니다."))
	float FootProbeHalfWidth = 22.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ToolTip = "한 번에 올라갈 수 있는 최대 지형 높이입니다. 이보다 높은 턱은 이동을 막습니다."))
	float MaxStepUp = 34.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ToolTip = "한 번에 내려갈 수 있는 최대 지형 높이입니다. 이보다 깊으면 낙하 상태로 처리됩니다."))
	float MaxStepDown = 56.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ToolTip = "이동 가능한 최대 지형 경사각입니다. 값이 클수록 더 가파른 경사를 오를 수 있습니다."))
	float MaxSlopeAngleDegrees = 52.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ToolTip = "접지 중 지형 표면에 붙이기 위해 허용하는 최대 보정 거리입니다."))
	float GroundSnapDistance = 12.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ToolTip = "몸체 기울기를 계산할 때 캐릭터 중심 좌우로 검사할 반폭입니다."))
	float BodySlopeProbeHalfWidth = 28.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ToolTip = "지형 경사에 맞춰 몸체를 시각적으로 기울일 최대 각도입니다."))
	float MaxBodySlopeVisualDegrees = 45.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ToolTip = "지형 지지가 없을 때 적용할 낙하 가속도입니다."))
	float GravityAcceleration = 980.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ToolTip = "낙하 중 도달할 수 있는 최대 하강 속도입니다."))
	float MaxFallSpeed = 1600.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Terrain Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ToolTip = "지형 아래로 이 깊이 이상 떨어지면 패배 처리할 기준 거리입니다."))
	float FallDeathDepth = 400.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Combat|Debug", meta = (AllowPrivateAccess = "true", ToolTip = "현재 조준과 무기 기준의 예상 탄도를 디버그 라인으로 그릴지 여부입니다."))
	bool bDrawProjectileTrajectory = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Combat|Debug", meta = (AllowPrivateAccess = "true", ClampMin = "1", ToolTip = "예상 탄도 디버그 라인을 몇 구간으로 나누어 계산할지 정합니다."))
	int32 TrajectoryDebugSteps = 50;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Combat|Debug", meta = (AllowPrivateAccess = "true", ClampMin = "0.01", ToolTip = "예상 탄도 디버그 계산에서 한 스텝이 의미하는 시간입니다. 초 단위입니다."))
	float TrajectoryDebugTimeStep = 0.08f;
	float ShotChargeElapsed = 0.0f;
	float VerticalVelocity = 0.0f;
	bool bChargingShot = false;
	float PendingAttackMultiplier = 1.0f;
	int32 SelectedWeaponIndex = 0;
};
