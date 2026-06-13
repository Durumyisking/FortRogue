// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/FortRogueBattleCharacter.h"

#include "AbilitySystem/FortRogueAbilitySet.h"
#include "AbilitySystem/FortRogueAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/FortRogueCombatSet.h"
#include "Combat/FortRogueProjectile.h"
#include "FortRogueGameplayTags.h"
#include "Items/FortRogueItemDefinition.h"
#include "Perks/FortRoguePerkDefinition.h"
#include "Weapons/FortRogueWeaponDefinition.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AFortRogueBattleCharacter::AFortRogueBattleCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Body->SetupAttachment(Root);
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Body->SetRelativeScale3D(FVector(0.65f, 0.08f, 0.9f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Body->SetStaticMesh(CubeMesh.Object);
	}

	AbilitySystemComponent = CreateDefaultSubobject<UFortRogueAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	CombatSet = CreateDefaultSubobject<UFortRogueCombatSet>(TEXT("CombatSet"));

	CharacterDisplayName = FText::FromString(TEXT("Rookie Tank"));
}

void AFortRogueBattleCharacter::BeginPlay()
{
	Super::BeginPlay();

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	AbilitySystemComponent->AddAttributeSetSubobject(CombatSet.Get());
	InitializeFromDefinition(CharacterDefinition);
	EnsureDefaultLoadout();
	GrantStartupAbilitySets();
}

UAbilitySystemComponent* AFortRogueBattleCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AFortRogueBattleCharacter::InitializeFromDefinition(UFortRogueCharacterDefinition* InCharacterDefinition)
{
	if (!InCharacterDefinition)
	{
		return;
	}

	CharacterDefinition = InCharacterDefinition;
	CharacterDisplayName = InCharacterDefinition->DisplayName;
	StartupAbilitySets = InCharacterDefinition->StartupAbilitySets;

	CombatSet->SetMaxHealth(InCharacterDefinition->MaxHealth);
	CombatSet->SetHealth(InCharacterDefinition->MaxHealth);
	CombatSet->SetDamage(InCharacterDefinition->BonusDamage);
	CombatSet->SetMaxMoveBudget(InCharacterDefinition->MaxMoveBudget);
	CombatSet->SetMoveBudget(InCharacterDefinition->MaxMoveBudget);

	WeaponLoadout.Reset();
	for (UFortRogueWeaponDefinition* WeaponDefinition : InCharacterDefinition->WeaponLoadout)
	{
		AddWeaponDefinition(WeaponDefinition);
	}

	ItemLoadout = InCharacterDefinition->ItemLoadout;
	for (FFortRogueItemStack& ItemStack : ItemLoadout)
	{
		if (ItemStack.ItemDefinition && ItemStack.Charges <= 0)
		{
			ItemStack.Charges = ItemStack.ItemDefinition->InitialCharges;
		}
	}
}

void AFortRogueBattleCharacter::ConfigureAsEnemy(bool bNewEnemy)
{
	bEnemy = bNewEnemy;
	bFacingRight = !bEnemy;
}

void AFortRogueBattleCharacter::BeginTurn()
{
	bActiveTurn = true;
	bFiredThisTurn = false;
	CombatSet->ResetTurnBudget();
}

void AFortRogueBattleCharacter::EndTurn()
{
	bActiveTurn = false;
}

void AFortRogueBattleCharacter::MoveHorizontal(float Axis, float DeltaSeconds)
{
	if (!bActiveTurn || bFiredThisTurn || IsDefeated() || FMath::IsNearlyZero(Axis))
	{
		return;
	}

	const float RequestedDelta = Axis * 260.0f * DeltaSeconds;
	const float Budget = CombatSet->GetMoveBudget();
	const float AppliedDelta = FMath::Clamp(RequestedDelta, -Budget, Budget);
	if (FMath::IsNearlyZero(AppliedDelta))
	{
		return;
	}

	AddActorWorldOffset(FVector(AppliedDelta, 0.0f, 0.0f));
	CombatSet->SetMoveBudget(Budget - FMath::Abs(AppliedDelta));
	bFacingRight = AppliedDelta >= 0.0f;
}

void AFortRogueBattleCharacter::AdjustAim(float Axis, float DeltaSeconds)
{
	if (!bActiveTurn || bFiredThisTurn || FMath::IsNearlyZero(Axis))
	{
		return;
	}

	AimAngle = FMath::Clamp(AimAngle + Axis * 70.0f * DeltaSeconds, 5.0f, 85.0f);
}

void AFortRogueBattleCharacter::AdjustPower(float Axis, float DeltaSeconds)
{
	if (!bActiveTurn || bFiredThisTurn || FMath::IsNearlyZero(Axis))
	{
		return;
	}

	ShotPower = FMath::Clamp(ShotPower + Axis * 0.75f * DeltaSeconds, 0.25f, 1.0f);
}

void AFortRogueBattleCharacter::SelectWeapon(int32 WeaponIndex)
{
	if (WeaponLoadout.IsValidIndex(WeaponIndex))
	{
		SelectedWeaponIndex = WeaponIndex;
	}
}

int32 AFortRogueBattleCharacter::FireSelectedWeapon()
{
	if (!bActiveTurn || bFiredThisTurn || IsDefeated())
	{
		return 0;
	}

	bFiredThisTurn = true;

	const FFortRogueWeaponSpec& Weapon = GetCurrentWeapon();
	const int32 ProjectileCount = FMath::Max(1, Weapon.ProjectilesPerShot + FMath::RoundToInt(CombatSet->GetProjectileCount()) - 1);
	const float FacingSign = bFacingRight ? 1.0f : -1.0f;
	const float Damage = (Weapon.Damage + CombatSet->GetDamage()) * PendingAttackMultiplier;
	const float Speed = Weapon.ProjectileSpeed * ShotPower * CombatSet->GetShotPowerMultiplier();
	int32 SpawnedProjectiles = 0;
	PendingAttackMultiplier = 1.0f;

	for (int32 Index = 0; Index < ProjectileCount; ++Index)
	{
		const float Spread = (ProjectileCount > 1) ? (Index - (ProjectileCount - 1) * 0.5f) * 5.0f : 0.0f;
		const float AngleRadians = FMath::DegreesToRadians(FMath::Clamp(AimAngle + Spread, 5.0f, 85.0f));
		const FVector Direction(FMath::Cos(AngleRadians) * FacingSign, 0.0f, FMath::Sin(AngleRadians));
		const FVector SpawnLocation = GetActorLocation() + FVector(FacingSign * 70.0f, 0.0f, 70.0f);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		const TSubclassOf<AFortRogueProjectile> ProjectileClass = Weapon.ProjectileClass ? Weapon.ProjectileClass : TSubclassOf<AFortRogueProjectile>(AFortRogueProjectile::StaticClass());
		AFortRogueProjectile* Projectile = GetWorld()->SpawnActor<AFortRogueProjectile>(ProjectileClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
		if (Projectile)
		{
			Projectile->InitializeProjectile(this, Direction * Speed, Damage, Weapon.BlastRadius, Weapon.Gravity);
			++SpawnedProjectiles;
		}
	}

	return SpawnedProjectiles;
}

void AFortRogueBattleCharacter::FireAtTarget(AFortRogueBattleCharacter* Target)
{
	if (!Target)
	{
		return;
	}

	const FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
	bFacingRight = ToTarget.X >= 0.0f;
	AimAngle = FMath::Clamp(FMath::RadiansToDegrees(FMath::Atan2(FMath::Max(220.0f, FMath::Abs(ToTarget.Z) + 300.0f), FMath::Max(1.0f, FMath::Abs(ToTarget.X)))), 35.0f, 72.0f);
	ShotPower = FMath::Clamp(FMath::Abs(ToTarget.X) / 1450.0f, 0.45f, 0.92f);
}

void AFortRogueBattleCharacter::ApplyDamage(float DamageAmount)
{
	CombatSet->ApplyDamage(DamageAmount);
}

bool AFortRogueBattleCharacter::UseItemByType(EFortRogueItemType ItemType)
{
	if (!bActiveTurn || IsDefeated())
	{
		return false;
	}

	for (FFortRogueItemStack& ItemStack : ItemLoadout)
	{
		const UFortRogueItemDefinition* ItemDefinition = ItemStack.ItemDefinition;
		if (!ItemDefinition || ItemDefinition->ItemType != ItemType || ItemStack.Charges <= 0)
		{
			continue;
		}

		--ItemStack.Charges;
		if (ItemType == EFortRogueItemType::Heal)
		{
			CombatSet->Heal(ItemDefinition->HealAmount);
		}
		else if (ItemType == EFortRogueItemType::AttackMultiplier)
		{
			PendingAttackMultiplier = FMath::Max(PendingAttackMultiplier, ItemDefinition->AttackMultiplier);
		}
		return true;
	}

	return false;
}

void AFortRogueBattleCharacter::ApplyRewardDamage(float BonusDamage)
{
	CombatSet->AddDamage(BonusDamage);
}

void AFortRogueBattleCharacter::ApplyRewardHealth(float BonusHealth)
{
	CombatSet->AddMaxHealth(BonusHealth);
}

void AFortRogueBattleCharacter::ApplyRewardProjectiles(int32 BonusProjectiles)
{
	CombatSet->AddProjectileCount(BonusProjectiles);
}

void AFortRogueBattleCharacter::ApplyPerkDefinition(UFortRoguePerkDefinition* PerkDefinition)
{
	if (!PerkDefinition)
	{
		return;
	}

	ApplyRewardDamage(PerkDefinition->DamageBonus);
	ApplyRewardHealth(PerkDefinition->MaxHealthBonus);
	ApplyRewardProjectiles(PerkDefinition->ProjectileBonus);

	if (PerkDefinition->GrantedAbilitySet)
	{
		FFortRogueAbilitySet_GrantedHandles Handles;
		PerkDefinition->GrantedAbilitySet->GiveToAbilitySystem(AbilitySystemComponent, &Handles, this);
		StartupAbilitySetHandles.Add(Handles);
	}
}

void AFortRogueBattleCharacter::AddWeaponDefinition(UFortRogueWeaponDefinition* WeaponDefinition)
{
	if (WeaponDefinition)
	{
		WeaponLoadout.Add(WeaponDefinition->Weapon);
	}
}

void AFortRogueBattleCharacter::AddItemDefinition(UFortRogueItemDefinition* ItemDefinition, int32 ChargesOverride)
{
	if (!ItemDefinition)
	{
		return;
	}

	for (FFortRogueItemStack& ItemStack : ItemLoadout)
	{
		if (ItemStack.ItemDefinition == ItemDefinition)
		{
			ItemStack.Charges += ChargesOverride == INDEX_NONE ? ItemDefinition->InitialCharges : FMath::Max(0, ChargesOverride);
			return;
		}
	}

	FFortRogueItemStack NewStack;
	NewStack.ItemDefinition = ItemDefinition;
	NewStack.Charges = ChargesOverride == INDEX_NONE ? ItemDefinition->InitialCharges : FMath::Max(0, ChargesOverride);
	ItemLoadout.Add(NewStack);
}

bool AFortRogueBattleCharacter::IsEnemy() const
{
	return bEnemy;
}

bool AFortRogueBattleCharacter::IsActiveTurn() const
{
	return bActiveTurn;
}

bool AFortRogueBattleCharacter::HasFiredThisTurn() const
{
	return bFiredThisTurn;
}

bool AFortRogueBattleCharacter::IsDefeated() const
{
	return CombatSet->GetHealth() <= 0.0f;
}

float AFortRogueBattleCharacter::GetHealth() const
{
	return CombatSet->GetHealth();
}

float AFortRogueBattleCharacter::GetMaxHealth() const
{
	return CombatSet->GetMaxHealth();
}

float AFortRogueBattleCharacter::GetMoveBudget() const
{
	return CombatSet->GetMoveBudget();
}

float AFortRogueBattleCharacter::GetAimAngle() const
{
	return AimAngle;
}

float AFortRogueBattleCharacter::GetShotPower() const
{
	return ShotPower;
}

int32 AFortRogueBattleCharacter::GetItemCharges(EFortRogueItemType ItemType) const
{
	int32 TotalCharges = 0;
	for (const FFortRogueItemStack& ItemStack : ItemLoadout)
	{
		if (ItemStack.ItemDefinition && ItemStack.ItemDefinition->ItemType == ItemType)
		{
			TotalCharges += ItemStack.Charges;
		}
	}
	return TotalCharges;
}

const FFortRogueWeaponSpec& AFortRogueBattleCharacter::GetCurrentWeapon() const
{
	if (WeaponLoadout.IsValidIndex(SelectedWeaponIndex))
	{
		return WeaponLoadout[SelectedWeaponIndex];
	}

	static const FFortRogueWeaponSpec FallbackWeapon;
	return FallbackWeapon;
}

FFortRogueWeaponSpec AFortRogueBattleCharacter::GetCurrentWeaponSpec() const
{
	return GetCurrentWeapon();
}

const TArray<FFortRogueWeaponSpec>& AFortRogueBattleCharacter::GetWeaponLoadout() const
{
	return WeaponLoadout;
}

TArray<FFortRogueWeaponSpec> AFortRogueBattleCharacter::GetWeaponLoadoutForBlueprint() const
{
	return WeaponLoadout;
}

int32 AFortRogueBattleCharacter::GetSelectedWeaponIndex() const
{
	return SelectedWeaponIndex;
}

FText AFortRogueBattleCharacter::GetCharacterDisplayName() const
{
	return CharacterDisplayName;
}

void AFortRogueBattleCharacter::GrantStartupAbilitySets()
{
	for (const UFortRogueAbilitySet* AbilitySet : StartupAbilitySets)
	{
		if (!AbilitySet)
		{
			continue;
		}

		FFortRogueAbilitySet_GrantedHandles Handles;
		AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, &Handles, this);
		StartupAbilitySetHandles.Add(Handles);
	}
}

void AFortRogueBattleCharacter::EnsureDefaultLoadout()
{
	if (WeaponLoadout.Num() == 0)
	{
		FFortRogueWeaponSpec Shell1;
		Shell1.DisplayName = FText::FromString(TEXT("Cannon Shot 1"));
		Shell1.Description = FText::FromString(TEXT("Fortress-style basic weapon 1: stable direct shell."));
		Shell1.WeaponTag = FortRogueGameplayTags::Weapon_Shell;
		Shell1.Damage = 35.0f;
		Shell1.BlastRadius = 145.0f;
		Shell1.ProjectileSpeed = 1180.0f;
		Shell1.ProjectilesPerShot = 1;
		WeaponLoadout.Add(Shell1);

		FFortRogueWeaponSpec Shell2;
		Shell2.DisplayName = FText::FromString(TEXT("Cannon Shot 2"));
		Shell2.Description = FText::FromString(TEXT("Fortress-style basic weapon 2: heavier arc and wider crater."));
		Shell2.WeaponTag = FortRogueGameplayTags::Weapon_Cluster;
		Shell2.Damage = 25.0f;
		Shell2.BlastRadius = 185.0f;
		Shell2.ProjectileSpeed = 980.0f;
		Shell2.ProjectilesPerShot = 2;
		WeaponLoadout.Add(Shell2);
	}

	if (ItemLoadout.Num() == 0)
	{
		UFortRogueItemDefinition* AttackItem = NewObject<UFortRogueItemDefinition>(this, TEXT("DefaultAttackMultiplierItem"));
		AttackItem->DisplayName = FText::FromString(TEXT("Attack Amp"));
		AttackItem->ItemTag = FortRogueGameplayTags::Trait_Damage;
		AttackItem->ItemType = EFortRogueItemType::AttackMultiplier;
		AttackItem->InitialCharges = 1;
		AttackItem->AttackMultiplier = 1.5f;

		FFortRogueItemStack AttackStack;
		AttackStack.ItemDefinition = AttackItem;
		AttackStack.Charges = AttackItem->InitialCharges;
		ItemLoadout.Add(AttackStack);

		UFortRogueItemDefinition* HealItem = NewObject<UFortRogueItemDefinition>(this, TEXT("DefaultHealItem"));
		HealItem->DisplayName = FText::FromString(TEXT("Repair Kit"));
		HealItem->ItemTag = FortRogueGameplayTags::Item_Repair;
		HealItem->ItemType = EFortRogueItemType::Heal;
		HealItem->InitialCharges = 1;
		HealItem->HealAmount = 35.0f;

		FFortRogueItemStack HealStack;
		HealStack.ItemDefinition = HealItem;
		HealStack.Charges = HealItem->InitialCharges;
		ItemLoadout.Add(HealStack);
	}
}
