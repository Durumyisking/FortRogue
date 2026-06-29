// Copyright Epic Games, Inc. All Rights Reserved.

#include "FRGameplayTags.h"

namespace FRGameplayTags
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

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Health, "Attribute.Health", "Current character health.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_MaxHealth, "Attribute.MaxHealth", "Maximum character health.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_MoveBudget, "Attribute.MoveBudget", "Current turn movement budget.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_MaxMoveBudget, "Attribute.MaxMoveBudget", "Maximum turn movement budget.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Damage, "Attribute.Damage", "Flat shot damage bonus.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_ShotPowerMultiplier, "Attribute.ShotPowerMultiplier", "Shot power multiplier.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_ProjectileCount, "Attribute.ProjectileCount", "Current projectile count attribute.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_MinAimAngle, "Attribute.MinAimAngle", "Minimum character aim angle.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_MaxAimAngle, "Attribute.MaxAimAngle", "Maximum character aim angle.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_Shell, "Weapon.Shell", "Basic arcing shell.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_Cluster, "Weapon.Cluster", "Multiple projectile shell reward.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_Cannon_Classic, "Weapon.Cannon.Classic", "Classic cannon shell.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_Cannon_Siege, "Weapon.Cannon.Siege", "Small blast high damage cannon shell.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_Laser_Classic, "Weapon.Laser.Classic", "Laser placeholder classic shell.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_Laser_Siege, "Weapon.Laser.Siege", "Laser placeholder high damage shell.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_Repeater_SingleBurst, "Weapon.Repeater.SingleBurst", "Repeater salvo firing one projectile per burst.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_Repeater_TripleBurst, "Weapon.Repeater.TripleBurst", "Repeater salvo firing three projectiles per burst.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_Crossbow_HeavyBolt, "Weapon.Crossbow.HeavyBolt", "Small blast high damage crossbow bolt.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_Crossbow_NeedleVolley, "Weapon.Crossbow.NeedleVolley", "Low damage crossbow volley.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_Missile_Rocket, "Weapon.Missile.Rocket", "Classic missile rocket.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_Missile_Guided, "Weapon.Missile.Guided", "Small blast homing missile.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_Cannon_Special, "Weapon.Cannon.Special", "Cannon class special: high damage with a tiny blast.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_Bandit_Special, "Weapon.Bandit.Special", "Bandit class special: nine weak projectiles at once.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_Miner_Special, "Weapon.Miner.Special", "Miner class special: a drilling projectile.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_Engineer_Special, "Weapon.Engineer.Special", "Engineer class special: creates a vertical terrain column.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ShotEffect_Damage, "ShotEffect.Damage", "Shot modifier that changes damage.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ShotEffect_BlastRadius, "ShotEffect.BlastRadius", "Shot modifier that changes explosion radius.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ShotEffect_Projectiles, "ShotEffect.Projectiles", "Shot modifier that changes projectile count.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ShotEffect_LowGravity, "ShotEffect.LowGravity", "Shot modifier that lowers projectile gravity.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ShotEffect_Salvo, "ShotEffect.Salvo", "Weapon fires repeated salvos.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ShotEffect_Homing, "ShotEffect.Homing", "Projectile steers toward a target.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ShotEffect_Drill, "ShotEffect.Drill", "Shot effect for terrain digging projectiles.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ShotEffect_SplitOnImpact, "ShotEffect.SplitOnImpact", "Shot effect for projectiles that split after impact.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ShotEffect_TerrainCreate, "ShotEffect.TerrainCreate", "Shot effect for projectiles that create terrain.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ShotEffect_TerrainColumn, "ShotEffect.TerrainColumn", "Shot effect for projectiles that create a vertical terrain column.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ShotEffect_Knockback, "ShotEffect.Knockback", "Projectile pushes a directly hit character.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_AttackAmp, "Item.AttackAmp", "Consumable item that boosts a future shot.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Repair, "Item.Repair", "Consumable repair item.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_NextShot, "Item.NextShot", "Consumable item that grants a next-shot modifier.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Trait_Damage, "Trait.Damage", "Damage increasing trait.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Trait_Projectiles, "Trait.Projectiles", "Projectile count increasing trait.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Trait_Health, "Trait.Health", "Maximum health increasing trait.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Trait_ShotModifier, "Trait.ShotModifier", "Trait that modifies future shots.");
}
