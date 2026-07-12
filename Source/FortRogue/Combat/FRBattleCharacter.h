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
class UFRBattleCharacterStatusWidget;
class UFRBattleCharacterAimIndicatorWidget;
class UFRTerrainMovementComponent;
class UFRCharacterSpriteAnimator;
class UWidgetComponent;
class UBoxComponent;
class UPaperFlipbookComponent;
class UStaticMeshComponent;
class UFRItemDefinition;
class UFRPerkDefinition;
class AFRDestructibleTerrain;
class AFRFloatingDamageTextActor;

USTRUCT()
struct FFRGrantedAbilitySetEntry
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UFRAbilitySet> AbilitySet;

	UPROPERTY()
	FFRAbilitySet_GrantedHandles Handles;
};

/** 런타임에 부여된 샷 modifier와 그 출처(퍽)를 함께 기록합니다. 퍽 제거 시 출처로 되돌립니다. */
USTRUCT()
struct FFRGrantedShotModifierEntry
{
	GENERATED_BODY()

	UPROPERTY()
	FFRShotModifierSpec Spec;

	UPROPERTY()
	TObjectPtr<UFRPerkDefinition> SourcePerk;
};

/** 획득한 퍽 한 개의 기록입니다. 빌드 UI 표시, 스택 카운트, 퍽 제거에 사용합니다. */
USTRUCT(BlueprintType)
struct FFRAcquiredPerkEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Perk")
	TObjectPtr<UFRPerkDefinition> PerkDefinition;
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

	UFUNCTION(BlueprintPure, Category = "FortRogue|Weapons")
	int32 GetBasicAttackIndex() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Weapons")
	int32 GetSpecialAttackIndex() const;

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Combat")
	int32 FireSelectedWeapon();

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	bool CanFireSelectedWeapon() const;

	UFUNCTION(BlueprintCallable, Category = "FortRogue|AI")
	void FireAtTarget(AFRBattleCharacter* Target, const FFRStageDifficultyData& DifficultyData);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Combat")
	void ApplyDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Combat")
	void ApplyHeal(float HealAmount);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Combat", meta = (ToolTip = "다음 발사에 적용할 공격 배율을 등록합니다. 이미 더 큰 배율이 대기 중이면 큰 값이 유지됩니다."))
	void ApplyPendingAttackMultiplier(float Multiplier);

	bool FindHurtboxImpactAlongSegmentXZ(const FVector& StartLocation, const FVector& EndLocation, FVector& OutImpactLocation) const;
	float GetDistanceToHurtboxXZ(const FVector& WorldLocation) const;

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Terrain")
	void ReevaluateTerrainSupport();

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Combat")
	void ApplyImpactKnockback(float HorizontalDistance, FVector ImpactLocation, FVector ImpactVelocity);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Terrain")
	void SetTerrain(AFRDestructibleTerrain* InTerrain);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Terrain")
	UFRTerrainMovementComponent* GetTerrainMovement() const;

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

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Perks")
	void ApplyPerkDefinition(UFRPerkDefinition* PerkDefinition);

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Perks", meta = (ToolTip = "획득한 퍽 한 스택을 제거하고 스탯/능력/샷 modifier를 되돌립니다."))
	bool RemovePerkDefinition(UFRPerkDefinition* PerkDefinition);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Perks")
	TArray<UFRPerkDefinition*> GetAcquiredPerksForBlueprint() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Perks")
	int32 GetPerkStackCount(const UFRPerkDefinition* PerkDefinition) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Perks")
	bool HasPerkByTag(UPARAM(meta = (Categories = "Trait")) FGameplayTag PerkTag) const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Perks")
	FText GetAcquiredPerksSummary() const;

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
	float GetMinAimAngle() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Stats")
	float GetMaxAimAngle() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Stats")
	bool TryGetCombatAttributeValueByTag(UPARAM(meta = (Categories = "Attribute")) FGameplayTag AttributeTag, float& OutValue) const;

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Stats")
	bool TryApplyCombatAttributeDeltaByTag(UPARAM(meta = (Categories = "Attribute")) FGameplayTag AttributeTag, float DeltaValue);

	UFUNCTION(BlueprintPure, Category = "FortRogue|Stats")
	FText GetCombatStatsSummary() const;

	UFUNCTION(BlueprintPure, Category = "FortRogue|Combat")
	float GetAimAngle() const;

	bool IsFacingRight() const;

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
	void HandleSurfacePitchChanged(float PitchDegrees);
	void HandleSupportLost();
	void HandleFellToDeath();
	void SetFacingFromAxis(float Axis);
	void UpdateCharacterRotation(float PitchDegrees);
	void UpdateBodyFrameChildrenLocalTransform();

	/** 피격 판정 박스를 현재 스프라이트의 렌더 바운드(투명 여백 제외)에 맞춥니다. 스프라이트가 없으면 기본 크기를 유지합니다. */
	void RefreshHurtboxToSpriteBounds(float InSpriteScale);

	void UpdateAimDrivenComponents();
	void InitializeCharacterWidgets();
	void RefreshCharacterWidgets();
	void SpawnFloatingDamageText(float DamageAmount);
	float GetActorPitchDegrees() const;
	FVector GetProjectileLaunchDirection(float SpreadDegrees) const;
	FVector GetLaunchDirectionForAim(float AimDegrees) const;
	FVector GetAimPivotWorldLocation() const;
	FVector GetProjectileSpawnLocation(const FVector& LaunchDirection) const;
	FVector GetMuzzleSpawnLocation() const;
	float GetEffectiveShotPower() const;
	float GetEffectiveShotPowerFor(float InShotPower) const;
	FFRShotSpec BuildShotSpec(const FFRWeaponSpec& Weapon) const;
	FFRShotSpec BuildShotSpecForAim(const FFRWeaponSpec& Weapon, float InAimAngle, float InShotPower) const;
	int32 SpawnShotSpecProjectiles(const FFRShotSpec& ShotSpec, bool bIncreasePendingProjectileCount);
	void GrantStartupAbilitySets();
	void EnsureDefaultLoadout();
	void AppendGrantedShotModifiers(const TArray<FFRShotModifierSpec>& ShotModifiers, UFRPerkDefinition* SourcePerk);
	float GetWorldWind() const;

	UPROPERTY(VisibleAnywhere, Category = "FortRogue")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "FortRogue")
	TObjectPtr<USceneComponent> BodyFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FortRogue|Combat", meta = (AllowPrivateAccess = "true", ToolTip = "투사체 직접 충돌과 폭발 거리 계산에 사용하는 피격 박스입니다. BodyFrame과 함께 발바닥 피벗을 기준으로 회전합니다."))
	TObjectPtr<UBoxComponent> Hurtbox;

	UPROPERTY(VisibleAnywhere, Category = "FortRogue")
	TObjectPtr<UStaticMeshComponent> Body;

	UPROPERTY(VisibleAnywhere, Category = "FortRogue")
	TObjectPtr<UPaperFlipbookComponent> BodySprite;

	UPROPERTY(VisibleAnywhere, Category = "FortRogue", meta = (ToolTip = "전투 상태(대기/이동/발사/피격)에 맞춰 BodySprite 플립북을 전환하는 연출 컴포넌트입니다."))
	TObjectPtr<UFRCharacterSpriteAnimator> SpriteAnimator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FortRogue|Combat", meta = (AllowPrivateAccess = "true", ToolTip = "투사체 생성 위치와 발사 방향의 기준이 되는 총구입니다. BodyFrame 스케일을 따라 위치 오프셋도 함께 커집니다."))
	TObjectPtr<USceneComponent> Muzzle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FortRogue|Terrain", meta = (AllowPrivateAccess = "true", ToolTip = "지형 이동/중력/낙사 판정을 담당하는 컴포넌트입니다."))
	TObjectPtr<UFRTerrainMovementComponent> TerrainMovement;

	UPROPERTY(VisibleAnywhere, Category = "FortRogue|UI")
	TObjectPtr<UWidgetComponent> AngleIndicatorWidget;

	UPROPERTY(VisibleAnywhere, Category = "FortRogue|UI")
	TObjectPtr<UWidgetComponent> HpWidget;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI", meta = (ToolTip = "각도 인디케이터 위젯 클래스입니다. 비워두면 프로젝트 설정(FortRogue Game Flow)의 기본 클래스를 사용합니다."))
	TSubclassOf<UFRBattleCharacterAimIndicatorWidget> AngleIndicatorWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI", meta = (ToolTip = "체력 표시 위젯 클래스입니다. 비워두면 프로젝트 설정(FortRogue Game Flow)의 기본 클래스를 사용합니다."))
	TSubclassOf<UFRBattleCharacterStatusWidget> HpWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Combat|Aim", meta = (AllowPrivateAccess = "true", ToolTip = "BodyFrame 로컬 공간에서 조준 각도기의 중심이자 총구 오프셋의 시작점입니다."))
	FVector AimPivotLocalLocation = FVector(0.0f, 0.0f, 80.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Combat|Aim", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ToolTip = "AimPivotLocalLocation에서 조준 방향으로 떨어진 총구 끝 거리입니다. BodyFrame 스케일을 적용받습니다."))
	float MuzzleForwardOffset = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Combat|Aim", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ToolTip = "각도 인디케이터가 캐릭터 스프라이트에 가려지지 않도록 전투 카메라 방향으로 빼는 거리입니다."))
	float AimIndicatorCameraOffsetY = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI")
	TSubclassOf<AFRFloatingDamageTextActor> FloatingDamageTextActorClass;

	UPROPERTY(VisibleAnywhere, Category = "FortRogue|Abilities")
	TObjectPtr<UFRAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, Category = "FortRogue|Abilities")
	TObjectPtr<UFRCombatSet> CombatSet;

	UPROPERTY()
	TArray<FFRGrantedAbilitySetEntry> GrantedAbilitySetEntries;

	UPROPERTY()
	TArray<FFRGrantedShotModifierEntry> GrantedShotModifierEntries;

	UPROPERTY()
	TArray<FFRShotModifierSpec> PendingShotModifiers;

	UPROPERTY()
	TArray<FFRAcquiredPerkEntry> AcquiredPerkEntries;

	FText CharacterDisplayName;
	bool bEnemy = false;
	bool bActiveTurn = false;
	bool bFiredThisTurn = false;
	bool bFacingRight = true;
	float AimAngle = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FortRogue|Combat|Charge", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0", ToolTip = "UI에 표시되는 현재 발사 게이지입니다. 0이어도 실제 발사는 MinShotPower를 사용합니다."))
	float ShotPower = 0.0f;

	float ShotChargeElapsed = 0.0f;
	bool bChargingShot = false;
	bool bSpecialAttackEnabled = true;
	int32 BasicAttackIndex = INDEX_NONE;
	int32 SpecialAttackIndex = INDEX_NONE;
	float PendingAttackMultiplier = 1.0f;
	int32 SelectedWeaponIndex = 0;
};
