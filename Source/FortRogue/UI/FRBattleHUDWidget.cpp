// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FRBattleHUDWidget.h"

#include "Combat/FRBattleCharacter.h"
#include "FRGameMode.h"

AFRGameMode* UFRBattleHUDWidget::GetFortRogueGameMode() const
{
	return GetWorld() ? GetWorld()->GetAuthGameMode<AFRGameMode>() : nullptr;
}

AFRBattleCharacter* UFRBattleHUDWidget::GetPlayerCharacter() const
{
	if (AFRGameMode* GameMode = GetFortRogueGameMode())
	{
		return GameMode->GetPlayerCharacter();
	}
	return nullptr;
}

AFRBattleCharacter* UFRBattleHUDWidget::GetEnemyCharacter() const
{
	if (AFRGameMode* GameMode = GetFortRogueGameMode())
	{
		return GameMode->GetEnemyCharacter();
	}
	return nullptr;
}

FText UFRBattleHUDWidget::GetRunProgressSummary() const
{
	if (AFRGameMode* GameMode = GetFortRogueGameMode())
	{
		return GameMode->GetRunProgressSummary();
	}
	return FText::GetEmpty();
}

EFRBattleState UFRBattleHUDWidget::GetBattleState() const
{
	if (AFRGameMode* GameMode = GetFortRogueGameMode())
	{
		return GameMode->GetBattleState();
	}
	return EFRBattleState::PlayerTurn;
}

FText UFRBattleHUDWidget::GetStatusText() const
{
	if (AFRGameMode* GameMode = GetFortRogueGameMode())
	{
		return GameMode->GetStatusText();
	}
	return FText::GetEmpty();
}

FText UFRBattleHUDWidget::GetWindSummary() const
{
	if (AFRGameMode* GameMode = GetFortRogueGameMode())
	{
		return GameMode->GetWindSummary();
	}
	return FText::GetEmpty();
}

FText UFRBattleHUDWidget::GetPlayerCombatStatsSummary() const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetCombatStatsSummary();
	}
	return FText::GetEmpty();
}

bool UFRBattleHUDWidget::TryGetPlayerCombatAttributeValueByTag(FGameplayTag AttributeTag, float& OutValue) const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->TryGetCombatAttributeValueByTag(AttributeTag, OutValue);
	}
	OutValue = 0.0f;
	return false;
}

FText UFRBattleHUDWidget::GetPlayerShotSummary() const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetCurrentShotSummary();
	}
	return FText::GetEmpty();
}

FFRShotSpec UFRBattleHUDWidget::GetPlayerCurrentShotSpec() const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetCurrentShotSpec();
	}
	return FFRShotSpec();
}

bool UFRBattleHUDWidget::DoesPlayerShotModifierMeetCurrentShotConditions(const FFRShotModifierSpec& ShotModifier) const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->DoesShotModifierMeetCurrentShotConditions(ShotModifier);
	}
	return false;
}

FText UFRBattleHUDWidget::GetPlayerShotModifierCurrentConditionFailureSummary(const FFRShotModifierSpec& ShotModifier) const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetShotModifierCurrentConditionFailureSummary(ShotModifier);
	}
	return FText::GetEmpty();
}

TArray<FFRWeaponSpec> UFRBattleHUDWidget::GetPlayerWeaponLoadout() const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetWeaponLoadoutForBlueprint();
	}
	return TArray<FFRWeaponSpec>();
}

FFRWeaponSpec UFRBattleHUDWidget::GetPlayerCurrentWeaponSpec() const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetCurrentWeaponSpec();
	}
	return FFRWeaponSpec();
}

int32 UFRBattleHUDWidget::GetPlayerSelectedWeaponIndex() const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetSelectedWeaponIndex();
	}
	return INDEX_NONE;
}

bool UFRBattleHUDWidget::CanSelectPlayerWeapon(int32 WeaponIndex) const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->CanSelectWeapon(WeaponIndex);
	}
	return false;
}

bool UFRBattleHUDWidget::CanSelectPlayerWeaponByTag(FGameplayTag WeaponTag) const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->CanSelectWeaponByTag(WeaponTag);
	}
	return false;
}

int32 UFRBattleHUDWidget::GetPlayerWeaponIndexByTag(FGameplayTag WeaponTag) const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetWeaponIndexByTag(WeaponTag);
	}
	return INDEX_NONE;
}

TArray<FFRItemStack> UFRBattleHUDWidget::GetPlayerItemLoadout() const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetItemLoadoutForBlueprint();
	}
	return TArray<FFRItemStack>();
}

int32 UFRBattleHUDWidget::GetPlayerItemCharges(EFRItemType ItemType) const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetItemCharges(ItemType);
	}
	return 0;
}

int32 UFRBattleHUDWidget::GetPlayerItemChargesByTag(FGameplayTag ItemTag) const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetItemChargesByTag(ItemTag);
	}
	return 0;
}

bool UFRBattleHUDWidget::CanUsePlayerItemByType(EFRItemType ItemType) const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->CanUseItemByType(ItemType);
	}
	return false;
}

bool UFRBattleHUDWidget::CanUsePlayerItemByTag(FGameplayTag ItemTag) const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->CanUseItemByTag(ItemTag);
	}
	return false;
}

bool UFRBattleHUDWidget::CanUsePlayerItemByIndex(int32 ItemIndex) const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->CanUseItemByIndex(ItemIndex);
	}
	return false;
}

int32 UFRBattleHUDWidget::GetPlayerItemIndexByTag(FGameplayTag ItemTag) const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetItemIndexByTag(ItemTag);
	}
	return INDEX_NONE;
}

TArray<FFRShotModifierSpec> UFRBattleHUDWidget::GetPlayerGrantedShotModifiers() const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetGrantedShotModifiersForBlueprint();
	}
	return TArray<FFRShotModifierSpec>();
}

TArray<FFRShotModifierSpec> UFRBattleHUDWidget::GetPlayerPendingShotModifiers() const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetPendingShotModifiersForBlueprint();
	}
	return TArray<FFRShotModifierSpec>();
}

FText UFRBattleHUDWidget::GetPlayerGrantedShotModifiersSummary() const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetGrantedShotModifiersSummary();
	}
	return FText::GetEmpty();
}

FText UFRBattleHUDWidget::GetPlayerPendingShotModifiersSummary() const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetPendingShotModifiersSummary();
	}
	return FText::GetEmpty();
}

int32 UFRBattleHUDWidget::GetPlayerGrantedShotModifierCountByTag(FGameplayTag ModifierTag) const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetGrantedShotModifierCountByTag(ModifierTag);
	}
	return 0;
}

int32 UFRBattleHUDWidget::GetPlayerPendingShotModifierCountByTag(FGameplayTag ModifierTag) const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetPendingShotModifierCountByTag(ModifierTag);
	}
	return 0;
}

bool UFRBattleHUDWidget::HasPlayerGrantedShotModifierByTag(FGameplayTag ModifierTag) const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->HasGrantedShotModifierByTag(ModifierTag);
	}
	return false;
}

bool UFRBattleHUDWidget::HasPlayerPendingShotModifierByTag(FGameplayTag ModifierTag) const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->HasPendingShotModifierByTag(ModifierTag);
	}
	return false;
}

TArray<UFRAbilitySet*> UFRBattleHUDWidget::GetPlayerGrantedAbilitySets() const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetGrantedAbilitySetsForBlueprint();
	}
	return TArray<UFRAbilitySet*>();
}

FText UFRBattleHUDWidget::GetPlayerGrantedAbilitySetsSummary() const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetGrantedAbilitySetsSummary();
	}
	return FText::GetEmpty();
}

int32 UFRBattleHUDWidget::GetPlayerGrantedAbilitySetCount(UFRAbilitySet* AbilitySet) const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetGrantedAbilitySetCount(AbilitySet);
	}
	return 0;
}

int32 UFRBattleHUDWidget::GetPlayerGrantedAbilitySetCountByTag(FGameplayTag AbilitySetTag) const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetGrantedAbilitySetCountByTag(AbilitySetTag);
	}
	return 0;
}

bool UFRBattleHUDWidget::HasPlayerGrantedAbilitySetByTag(FGameplayTag AbilitySetTag) const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->HasGrantedAbilitySetByTag(AbilitySetTag);
	}
	return false;
}

float UFRBattleHUDWidget::GetPlayerAimAngle() const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetAimAngle();
	}
	return 0.0f;
}

float UFRBattleHUDWidget::GetPlayerShotPower() const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetShotPower();
	}
	return 0.0f;
}

float UFRBattleHUDWidget::GetPlayerShotChargeAlpha() const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->GetShotChargeAlpha();
	}
	return 0.0f;
}

bool UFRBattleHUDWidget::IsPlayerChargingShot() const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->IsChargingShot();
	}
	return false;
}

bool UFRBattleHUDWidget::CanFirePlayerWeapon() const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->CanFireSelectedWeapon();
	}
	return false;
}

bool UFRBattleHUDWidget::CanBeginPlayerShotCharge() const
{
	if (AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		return PlayerCharacter->CanBeginShotCharge();
	}
	return false;
}
