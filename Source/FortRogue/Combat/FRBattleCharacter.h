// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystemInterface.h"
#include "AbilitySystem/FRAbilitySet.h"
#include "Characters/FRCharacterDefinition.h"
#include "Combat/FRShotSpec.h"
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Run/FRStageRunDefinition.h"
#include "Weapons/FRWeaponDefinition.h"
#include "FRBattleCharacter.generated.h"

class UFRAbilitySet;
class UFRAbilitySystemComponent;
class UFRCombatSet;
class UFRDefaultLoadoutDefinition;
class UFRCharacterHealthBarWidget;
class UFRWorldStatusMarkerWidget;
class UFRTrajectoryPreviewPointWidget;
class UPaperFlipbookComponent;
class UStaticMeshComponent;
class UFRItemDefinition;
class UFRPerkDefinition;
class UWidgetComponent;
class AFRFloatingCombatText;
class AFRDestructibleTerrain;

USTRUCT()
struct FFRGrantedAbilitySetEntry
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UFRAbilitySet> AbilitySet;

	UPROPERTY()
	FFRAbilitySet_GrantedHandles Handles;
};

UCLASS()
class FORTROGUE_API AFRBattleCharacter : public APawn, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AFRBattleCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Character")
	void InitializeFromDefinition(UFRCharacterDefinition* CharacterDefinition);

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
	bool SelectWeaponByTag(UPARAM(meta = (Categories = "Weapon")) FGameplayTag WeaponTag);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Weapons")
	void SelectBasicAttack();

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Weapons")
	bool SelectSpecialAttack();

	UFUNCTION(BlueprintPure, Category = "FortRogue|Weapons")
	bool CanSelectWeapon(int32 WeaponIndex) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Weapons")
	bool CanSelectWeaponByTag(UPARAM(meta = (Categories = "Weapon")) FGameplayTag WeaponTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Weapons")
	bool CanSelectSpecialAttack() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Weapons")
	int32 GetWeaponIndexByTag(UPARAM(meta = (Categories = "Weapon")) FGameplayTag WeaponTag) const;

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Combat")
	int32 FireSelectedWeapon();

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	bool CanFireSelectedWeapon() const;

	UFUNCTION(BlueprintCallable, Category = "FortRogue|AI")
	void FireAtTarget(AFRBattleCharacter* Target, const FFRStageDifficultyData& DifficultyData);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Combat")
	void ApplyDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Terrain")
	void ReevaluateTerrainSupport();

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Terrain")
	void SetTerrain(AFRDestructibleTerrain* InTerrain);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Items")
	bool UseItemByType(EFRItemType ItemType);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Items")
	bool UseItemByTag(UPARAM(meta = (Categories = "Item")) FGameplayTag ItemTag);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Items")
	bool UseItemByIndex(int32 ItemIndex);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Items")
	bool CanUseItemByType(EFRItemType ItemType) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Items")
	bool CanUseItemByTag(UPARAM(meta = (Categories = "Item")) FGameplayTag ItemTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Items")
	bool CanUseItemByIndex(int32 ItemIndex) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Items")
	int32 GetItemIndexByTag(UPARAM(meta = (Categories = "Item")) FGameplayTag ItemTag) const;

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
	void ApplyPerkDefinition(UFRPerkDefinition* PerkDefinition);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Abilities")
	void GrantAbilitySet(UFRAbilitySet* AbilitySet);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Abilities")
	bool RemoveAbilitySet(UFRAbilitySet* AbilitySet);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Abilities")
	int32 RemoveAbilitySetsByTag(UPARAM(meta = (Categories = "Trait")) FGameplayTag AbilitySetTag);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Abilities")
	int32 GetGrantedAbilitySetCount(UFRAbilitySet* AbilitySet) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Abilities")
	int32 GetGrantedAbilitySetCountByTag(UPARAM(meta = (Categories = "Trait")) FGameplayTag AbilitySetTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Abilities")
	bool HasGrantedAbilitySetByTag(UPARAM(meta = (Categories = "Trait")) FGameplayTag AbilitySetTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Abilities")
	TArray<UFRAbilitySet*> GetGrantedAbilitySetsForBlueprint() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Abilities")
	FText GetGrantedAbilitySetsSummary() const;

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Combat")
	void GrantShotModifiers(const TArray<FFRShotModifierSpec>& ShotModifiers);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Combat")
	void GrantPendingShotModifiers(const TArray<FFRShotModifierSpec>& ShotModifiers);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Combat")
	int32 RemoveGrantedShotModifiersByTag(UPARAM(meta = (Categories = "ShotEffect")) FGameplayTag ModifierTag);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Combat")
	int32 RemovePendingShotModifiersByTag(UPARAM(meta = (Categories = "ShotEffect")) FGameplayTag ModifierTag);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Weapons")
	void AddWeaponDefinition(UFRWeaponDefinition* WeaponDefinition);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Weapons")
	void SetSpecialAttackEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Items")
	void AddItemDefinition(UFRItemDefinition* ItemDefinition, int32 ChargesOverride = -1);

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
	bool TryGetCombatAttributeValueByTag(UPARAM(meta = (Categories = "Attribute")) FGameplayTag AttributeTag, float& OutValue) const;

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Stats")
	bool TryApplyCombatAttributeDeltaByTag(UPARAM(meta = (Categories = "Attribute")) FGameplayTag AttributeTag, float DeltaValue);

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
	int32 GetItemCharges(EFRItemType ItemType) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Items")
	int32 GetItemChargesByTag(UPARAM(meta = (Categories = "Item")) FGameplayTag ItemTag) const;

	const TArray<FFRItemStack>& GetItemLoadout() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Items")
	TArray<FFRItemStack> GetItemLoadoutForBlueprint() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	TArray<FFRShotModifierSpec> GetGrantedShotModifiersForBlueprint() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	TArray<FFRShotModifierSpec> GetPendingShotModifiersForBlueprint() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	FText GetGrantedShotModifiersSummary() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	FText GetPendingShotModifiersSummary() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	int32 GetGrantedShotModifierCountByTag(UPARAM(meta = (Categories = "ShotEffect")) FGameplayTag ModifierTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	int32 GetPendingShotModifierCountByTag(UPARAM(meta = (Categories = "ShotEffect")) FGameplayTag ModifierTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	bool HasGrantedShotModifierByTag(UPARAM(meta = (Categories = "ShotEffect")) FGameplayTag ModifierTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	bool HasPendingShotModifierByTag(UPARAM(meta = (Categories = "ShotEffect")) FGameplayTag ModifierTag) const;

	const FFRWeaponSpec& GetCurrentWeapon() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Weapons")
	FFRWeaponSpec GetCurrentWeaponSpec() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	FFRShotSpec GetCurrentShotSpec() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	FText GetCurrentShotSummary() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	bool DoesShotModifierMeetCurrentShotConditions(const FFRShotModifierSpec& ShotModifier) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	FText GetShotModifierCurrentConditionFailureSummary(const FFRShotModifierSpec& ShotModifier) const;

	const TArray<FFRWeaponSpec>& GetWeaponLoadout() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Weapons")
	TArray<FFRWeaponSpec> GetWeaponLoadoutForBlueprint() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Weapons")
	int32 GetSelectedWeaponIndex() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Weapons")
	bool IsSpecialAttackEnabled() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Character")
	FText GetCharacterDisplayName() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FortRogue|Abilities", meta = (ToolTip = "캐릭터가 BeginPlay 시 자동으로 부여받을 AbilitySet 목록입니다."))
	TArray<TObjectPtr<UFRAbilitySet>> StartupAbilitySets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FortRogue|Character", meta = (ToolTip = "이 캐릭터에 적용할 기본 캐릭터 데이터입니다. 체력, 이동력, 로드아웃 같은 초기값을 설정합니다."))
	TObjectPtr<UFRCharacterDefinition> CharacterDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FortRogue|Weapons", meta = (ToolTip = "캐릭터가 사용할 무기 스펙 목록입니다. 첫 번째 항목이 기본 선택 무기입니다."))
	TArray<FFRWeaponSpec> WeaponLoadout;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FortRogue|Items", meta = (ToolTip = "캐릭터가 보유한 아이템과 사용 가능 횟수 목록입니다."))
	TArray<FFRItemStack> ItemLoadout;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FortRogue|Perks", meta = (ToolTip = "캐릭터에게 지속 적용되는 샷 modifier 목록입니다. 퍽이나 보상으로 얻은 장기 효과를 담습니다."))
	TArray<FFRShotModifierSpec> GrantedShotModifiers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FortRogue|Items", meta = (ToolTip = "다음 발사에만 적용되는 샷 modifier 목록입니다. 공격 아이템처럼 1회성 효과를 담습니다."))
	TArray<FFRShotModifierSpec> PendingShotModifiers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FortRogue|Loadout", meta = (ToolTip = "CharacterDefinition에 로드아웃이 없을 때 사용할 기본 로드아웃 데이터입니다."))
	TObjectPtr<UFRDefaultLoadoutDefinition> DefaultLoadoutDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Combat|Charge", meta = (ClampMin = "0.0", ClampMax = "1.0", ToolTip = "샷 차지를 시작했을 때의 최소 파워입니다. 0~1 범위를 사용하세요."))
	float MinShotPower = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Combat|Charge", meta = (ClampMin = "0.0", ClampMax = "1.0", ToolTip = "샷 차지가 끝까지 찼을 때의 최대 파워입니다. 0~1 범위를 사용하세요."))
	float MaxShotPower = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Combat|Charge", meta = (ClampMin = "0.01", ToolTip = "MinShotPower에서 MaxShotPower까지 차지되는 데 걸리는 시간입니다. 초 단위입니다."))
	float ShotChargeSeconds = 1.25f;

private:
	AFRDestructibleTerrain* FindTerrain() const;
	bool CanUseAnyItem() const;
	bool CanUseItemStack(const FFRItemStack& ItemStack) const;
	bool UseItemStack(FFRItemStack& ItemStack);
	bool IsSupportedByTerrain() const;
	bool FindFootprintSurfaceZ(const AFRDestructibleTerrain& Terrain, float CenterWorldX, float StartWorldZ, float SearchDistance, float& OutSurfaceZ) const;
	bool IsFootprintBlocked(const AFRDestructibleTerrain& Terrain, const FVector& CenterLocation, float FootWorldZ) const;
	bool TryResolveFootprintBlock(const AFRDestructibleTerrain& Terrain, FVector& InOutCenterLocation, float& InOutFootWorldZ) const;
	bool IsSlopeTraversable(float CurrentFootWorldZ, float NextSurfaceWorldZ, float HorizontalDistance, float TerrainCellSize) const;
	float ClampWorldXToTerrainBounds(const AFRDestructibleTerrain& Terrain, float WorldX) const;
	void UpdateBodyTerrainAlignment(const AFRDestructibleTerrain* Terrain);
	void ApplyTerrainGravity(float DeltaSeconds);
	bool CheckFallDeath();
	void SnapToTerrain();
	void SetFacingFromAxis(float Axis);
	void UpdateCharacterRotation(float PitchDegrees);
	void UpdateBodySpriteLocalTransform();
	float GetActorPitchDegrees() const;
	FVector GetProjectileLaunchDirection(float SpreadDegrees) const;
	FVector GetProjectileSpawnLocation(const FVector& LaunchDirection) const;
	float GetEffectiveShotPower() const;
	FFRShotSpec BuildShotSpec(const FFRWeaponSpec& Weapon) const;
	int32 SpawnShotSpecProjectiles(const FFRShotSpec& ShotSpec, bool bIncreasePendingProjectileCount);
	void DrawProjectileTrajectory();
	UWidgetComponent* GetOrCreateTrajectoryPreviewComponent(int32 PreviewIndex);
	void HideTrajectoryPreviewComponents(int32 StartIndex = 0);
	void UpdateCharacterWorldIndicators();
	void SpawnFloatingDamageText(float DamageAmount);
	void GrantStartupAbilitySets();
	void EnsureDefaultLoadout();
	void ResolveDefaultWorldUIWidgetClasses();

	UPROPERTY(VisibleAnywhere, Category = "FortRogue")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "FortRogue")
	TObjectPtr<UStaticMeshComponent> Body;

	UPROPERTY(VisibleAnywhere, Category = "FortRogue")
	TObjectPtr<UPaperFlipbookComponent> BodySprite;

	UPROPERTY(VisibleAnywhere, Category = "FortRogue|UI")
	TObjectPtr<UWidgetComponent> HealthBarComponent;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI", meta = (ToolTip = "캐릭터 위에 표시할 월드 체력바 위젯 클래스입니다."))
	TSubclassOf<UFRCharacterHealthBarWidget> HealthBarWidgetClass;

	UPROPERTY(VisibleAnywhere, Category = "FortRogue|UI")
	TObjectPtr<UWidgetComponent> StatusMarkerComponent;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI", meta = (ToolTip = "캐릭터 위에 표시할 선택/타겟 마커 위젯 클래스입니다."))
	TSubclassOf<UFRWorldStatusMarkerWidget> StatusMarkerWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI", meta = (ToolTip = "활성 턴 캐릭터에 표시할 월드 마커 텍스트입니다."))
	FText ActiveTurnMarkerText = FText::FromString(TEXT("ACTIVE"));

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI", meta = (ToolTip = "현재 공격 대상 캐릭터에 표시할 월드 마커 텍스트입니다."))
	FText TargetMarkerText = FText::FromString(TEXT("TARGET"));

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Trajectory", meta = (ToolTip = "조준 궤적 포인트를 표시할 UMG 위젯 클래스입니다."))
	TSubclassOf<UFRTrajectoryPreviewPointWidget> TrajectoryPreviewPointWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Trajectory", meta = (ClampMin = "1", ClampMax = "64", ToolTip = "화면에 표시할 조준 궤적 포인트 수입니다."))
	int32 TrajectoryPreviewPointCount = 16;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI|Trajectory", meta = (ToolTip = "조준 궤적 포인트 위젯의 draw size입니다."))
	FVector2D TrajectoryPreviewPointDrawSize = FVector2D(34.0f, 24.0f);

	UPROPERTY()
	TArray<TObjectPtr<UWidgetComponent>> TrajectoryPreviewComponents;

	UPROPERTY(VisibleAnywhere, Category = "FortRogue|Abilities")
	TObjectPtr<UFRAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, Category = "FortRogue|Abilities")
	TObjectPtr<UFRCombatSet> CombatSet;

	UPROPERTY()
	TObjectPtr<AFRDestructibleTerrain> AssignedTerrain;

	UPROPERTY()
	TArray<FFRGrantedAbilitySetEntry> GrantedAbilitySetEntries;

	FText CharacterDisplayName;
	bool bEnemy = false;
	bool bActiveTurn = false;
	bool bFiredThisTurn = false;
	bool bHasSpecialAttackSlot = false;
	bool bSpecialAttackEnabled = true;
	bool bFacingRight = true;
	float AimAngle = 45.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Combat|Charge", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0", ToolTip = "UI에 표시되는 현재 발사 게이지입니다. 0이어도 실제 발사는 MinShotPower를 사용합니다."))
	float ShotPower = 0.0f;
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Combat|Debug", meta = (AllowPrivateAccess = "true", ToolTip = "현재 조준과 무기 기준의 예상 탄도를 UMG 포인트로 표시할지 여부입니다."))
	bool bDrawProjectileTrajectory = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Combat|Debug", meta = (AllowPrivateAccess = "true", ClampMin = "1", ToolTip = "예상 탄도 디버그 라인을 몇 구간으로 나누어 계산할지 정합니다."))
	int32 TrajectoryDebugSteps = 50;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Combat|Debug", meta = (AllowPrivateAccess = "true", ClampMin = "0.01", ToolTip = "예상 탄도 디버그 계산에서 한 스텝이 의미하는 시간입니다. 초 단위입니다."))
	float TrajectoryDebugTimeStep = 0.08f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Combat|Debug", meta = (AllowPrivateAccess = "true", ToolTip = "직교 카메라에서 탄도 프리뷰가 캐릭터/지형에 파묻혀 보이지 않도록 카메라 쪽으로 당기는 거리입니다."))
	float TrajectoryPreviewCameraYOffset = 80.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|UI", meta = (AllowPrivateAccess = "true", ToolTip = "데미지를 입었을 때 표시할 플로팅 텍스트 액터 클래스입니다."))
	TSubclassOf<AFRFloatingCombatText> FloatingCombatTextClass;
	float ShotChargeElapsed = 0.0f;
	float VerticalVelocity = 0.0f;
	bool bChargingShot = false;
	float PendingAttackMultiplier = 1.0f;
	int32 SelectedWeaponIndex = 0;
};
