// Copyright Epic Games, Inc. All Rights Reserved.

#include "FortRogueGameplayTags.h"

namespace FortRogueGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_MoveLeft, "InputTag.Move.Left", "Move the active character left.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_MoveRight, "InputTag.Move.Right", "Move the active character right.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_AimUp, "InputTag.Aim.Up", "Raise the shot angle.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_AimDown, "InputTag.Aim.Down", "Lower the shot angle.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Fire, "InputTag.Fire", "Fire the selected weapon.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_UseItem, "InputTag.Item.Use", "Use the selected consumable item.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Turn_Player, "State.Turn.Player", "The player can act.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Turn_Enemy, "State.Turn.Enemy", "The enemy can act.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Turn_Resolving, "State.Turn.Resolving", "A shot is resolving.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Battle_Won, "State.Battle.Won", "The MVP battle has been won.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_Shell, "Weapon.Shell", "Basic arcing shell.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_Cluster, "Weapon.Cluster", "Multiple projectile shell reward.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ShotEffect_Damage, "ShotEffect.Damage", "Shot modifier that changes damage.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ShotEffect_BlastRadius, "ShotEffect.BlastRadius", "Shot modifier that changes explosion radius.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ShotEffect_Projectiles, "ShotEffect.Projectiles", "Shot modifier that changes projectile count.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ShotEffect_LowGravity, "ShotEffect.LowGravity", "Shot modifier that lowers projectile gravity.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ShotEffect_Drill, "ShotEffect.Drill", "Shot effect for terrain digging projectiles.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ShotEffect_SplitOnImpact, "ShotEffect.SplitOnImpact", "Shot effect for projectiles that split after impact.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ShotEffect_TerrainCreate, "ShotEffect.TerrainCreate", "Shot effect for projectiles that create terrain.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_AttackAmp, "Item.AttackAmp", "Consumable item that boosts a future shot.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Repair, "Item.Repair", "Consumable repair item.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_NextShot, "Item.NextShot", "Consumable item that grants a next-shot modifier.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Trait_Damage, "Trait.Damage", "Damage increasing trait.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Trait_Projectiles, "Trait.Projectiles", "Projectile count increasing trait.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Trait_Health, "Trait.Health", "Maximum health increasing trait.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Trait_ShotModifier, "Trait.ShotModifier", "Trait that modifies future shots.");
}
