// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FRBattleHUDWidget.h"

#include "AbilitySystem/FRAbilitySet.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Combat/FRBattleCharacter.h"
#include "FRGameMode.h"
#include "Items/FRItemDefinition.h"
#include "MVVMBlueprintLibrary.h"
#include "UI/FRBattleHUDModuleViewModels.h"
#include "UI/FRBattleHUDModuleWidgets.h"
#include "UI/FRBattleHUDViewModel.h"
#include "View/MVVMView.h"

namespace
{
	float SafePercent(float Value, float MaxValue)
	{
		return MaxValue > 0.0f ? FMath::Clamp(Value / MaxValue, 0.0f, 1.0f) : 0.0f;
	}

	FText PercentText(float Value)
	{
		return FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(Value * 100.0f)));
	}

	FString GetTagLeafName(const FGameplayTag& Tag)
	{
		if (!Tag.IsValid())
		{
			return TEXT("");
		}

		FString TagString = Tag.GetTagName().ToString();
		FString Left;
		FString Right;
		while (TagString.Split(TEXT("."), &Left, &Right))
		{
			TagString = Right;
		}
		return TagString;
	}

	FText GetBattleStateText(EFRBattleState BattleState)
	{
		switch (BattleState)
		{
		case EFRBattleState::PlayerTurn:
			return FText::FromString(TEXT("PLAYER TURN"));
		case EFRBattleState::EnemyTurn:
			return FText::FromString(TEXT("ENEMY TURN"));
		case EFRBattleState::ResolvingShot:
			return FText::FromString(TEXT("SHOT"));
		case EFRBattleState::Reward:
			return FText::FromString(TEXT("REWARD"));
		case EFRBattleState::Won:
			return FText::FromString(TEXT("VICTORY"));
		case EFRBattleState::Lost:
			return FText::FromString(TEXT("DEFEAT"));
		default:
			return FText::FromString(TEXT("BATTLE"));
		}
	}

	FText GetShotEffectSummary(const FGameplayTagContainer& EffectTags)
	{
		TArray<FString> Names;
		for (const FGameplayTag& Tag : EffectTags)
		{
			const FString LeafName = GetTagLeafName(Tag);
			if (!LeafName.IsEmpty())
			{
				Names.Add(LeafName);
			}
		}

		return Names.Num() > 0
			? FText::FromString(FString::Join(Names, TEXT(" / ")))
			: FText::FromString(TEXT("Basic"));
	}

	FText GetModifierSummary(const TArray<FFRShotModifierSpec>& Modifiers)
	{
		TArray<FString> Names;
		for (const FFRShotModifierSpec& Modifier : Modifiers)
		{
			const FString ModifierName = GetTagLeafName(Modifier.ModifierTag);
			if (!ModifierName.IsEmpty())
			{
				Names.Add(ModifierName);
				continue;
			}

			for (const FGameplayTag& Tag : Modifier.EffectTags)
			{
				const FString EffectName = GetTagLeafName(Tag);
				if (!EffectName.IsEmpty())
				{
					Names.Add(EffectName);
				}
			}
		}

		return Names.Num() > 0
			? FText::FromString(FString::Join(Names, TEXT(" / ")))
			: FText::FromString(TEXT("None"));
	}

	FText GetAbilitySetSummary(const TArray<UFRAbilitySet*>& AbilitySets)
	{
		TArray<FString> Names;
		for (const UFRAbilitySet* AbilitySet : AbilitySets)
		{
			if (AbilitySet)
			{
				Names.Add(AbilitySet->DisplayName.ToString());
			}
		}

		return Names.Num() > 0
			? FText::FromString(FString::Join(Names, TEXT(" / ")))
			: FText::FromString(TEXT("None"));
	}

	FText GetWeaponSlotLabel(int32 Index)
	{
		if (Index == 0)
		{
			return FText::FromString(TEXT("Basic"));
		}
		if (Index == 1)
		{
			return FText::FromString(TEXT("Special"));
		}
		return FText::FromString(FString::Printf(TEXT("%d"), Index + 1));
	}

	FText GetItemSlotLabel(int32 Index)
	{
		return FText::FromString(FString::Printf(TEXT("Item %d"), Index + 1));
	}
}

void UFRBattleHUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	CreateViewModels();
	ApplyBattleHUDViewModel(this);

	ApplyViewModelToChild(TEXT("TurnBanner"), BattleStateViewModel);
	ApplyViewModelToChild(TEXT("PlayerStatusPanel"), PlayerStatusViewModel);
	ApplyViewModelToChild(TEXT("AimWindIndicator"), AimWindViewModel);
	ApplyViewModelToChild(TEXT("ShotPowerMeter"), ShotPowerViewModel);
	ApplyViewModelToChild(TEXT("LoadoutBar"), LoadoutViewModel);
	ApplyViewModelToChild(TEXT("ShotInfoPanel"), ShotPreviewViewModel);
	ApplyViewModelToChild(TEXT("ModifierSummary"), ModifierSummaryViewModel);
}

void UFRBattleHUDWidget::RefreshBattleHUD_Implementation()
{
	RefreshViewModel();
	RefreshModuleWidgets();
}

void UFRBattleHUDWidget::CreateViewModels()
{
	if (!BattleHUDViewModel)
	{
		BattleHUDViewModel = NewObject<UFRBattleHUDViewModel>(this);
	}
	if (!BattleStateViewModel)
	{
		BattleStateViewModel = NewObject<UFRBattleStateViewModel>(this);
	}
	if (!PlayerStatusViewModel)
	{
		PlayerStatusViewModel = NewObject<UFRCombatantStatusViewModel>(this);
	}
	if (!AimWindViewModel)
	{
		AimWindViewModel = NewObject<UFRAimWindViewModel>(this);
	}
	if (!ShotPowerViewModel)
	{
		ShotPowerViewModel = NewObject<UFRShotPowerViewModel>(this);
	}
	if (!LoadoutViewModel)
	{
		LoadoutViewModel = NewObject<UFRLoadoutViewModel>(this);
	}
	if (!ShotPreviewViewModel)
	{
		ShotPreviewViewModel = NewObject<UFRShotPreviewViewModel>(this);
	}
	if (!ModifierSummaryViewModel)
	{
		ModifierSummaryViewModel = NewObject<UFRModifierSummaryViewModel>(this);
	}
}

void UFRBattleHUDWidget::ApplyViewModel(UUserWidget* Widget, UMVVMViewModelBase* ViewModel) const
{
	if (!Widget || !ViewModel)
	{
		return;
	}

	if (Widget->GetExtension<UMVVMView>())
	{
		TScriptInterface<INotifyFieldValueChanged> ViewModelInterface;
		ViewModelInterface.SetObject(ViewModel);
		ViewModelInterface.SetInterface(Cast<INotifyFieldValueChanged>(ViewModel));
		UMVVMBlueprintLibrary::SetViewModelByClass(Widget, ViewModelInterface);
	}

	if (UFRBattleHUDModuleWidgetBase* ModuleWidget = Cast<UFRBattleHUDModuleWidgetBase>(Widget))
	{
		ModuleWidget->SetRawViewModel(ViewModel);
	}
}

void UFRBattleHUDWidget::ApplyViewModelToChild(FName WidgetName, UMVVMViewModelBase* ViewModel) const
{
	if (!WidgetTree)
	{
		return;
	}

	ApplyViewModel(Cast<UUserWidget>(WidgetTree->FindWidget(WidgetName)), ViewModel);
}

void UFRBattleHUDWidget::ApplyBattleHUDViewModel(UUserWidget* Widget) const
{
	ApplyViewModel(Widget, BattleHUDViewModel);
}

void UFRBattleHUDWidget::RefreshModuleWidgets() const
{
	if (!WidgetTree)
	{
		return;
	}

	const FName ModuleWidgetNames[] =
	{
		TEXT("TurnBanner"),
		TEXT("PlayerStatusPanel"),
		TEXT("AimWindIndicator"),
		TEXT("ShotPowerMeter"),
		TEXT("LoadoutBar"),
		TEXT("ShotInfoPanel"),
		TEXT("ModifierSummary")
	};

	for (const FName WidgetName : ModuleWidgetNames)
	{
		if (UFRBattleHUDModuleWidgetBase* ModuleWidget = Cast<UFRBattleHUDModuleWidgetBase>(WidgetTree->FindWidget(WidgetName)))
		{
			ModuleWidget->RefreshFromViewModel();
		}
	}
}

void UFRBattleHUDWidget::RefreshViewModel()
{
	CreateViewModels();
	if (!BattleHUDViewModel || !BattleStateViewModel || !PlayerStatusViewModel || !AimWindViewModel ||
		!ShotPowerViewModel || !LoadoutViewModel || !ShotPreviewViewModel || !ModifierSummaryViewModel)
	{
		return;
	}

	AFRGameMode* GameMode = GetFortRogueGameMode();
	AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter();
	AFRBattleCharacter* EnemyCharacter = GetEnemyCharacter();
	const EFRBattleState BattleState = GameMode ? GameMode->GetBattleState() : EFRBattleState::PlayerTurn;

	const FText TurnTextValue = GetBattleStateText(BattleState);
	const FText RunProgressTextValue = GetRunProgressSummary();
	BattleHUDViewModel->SetTurnText(TurnTextValue);
	BattleHUDViewModel->SetRunProgressText(RunProgressTextValue);
	BattleStateViewModel->SetTurnText(TurnTextValue);
	BattleStateViewModel->SetRunProgressText(RunProgressTextValue);

	FString Status = GetStatusText().ToString();
	if (BattleState == EFRBattleState::PlayerTurn && PlayerCharacter)
	{
		if (PlayerCharacter->IsChargingShot())
		{
			Status = TEXT("Release fire to launch");
		}
		else if (PlayerCharacter->CanBeginShotCharge())
		{
			Status = TEXT("Move, aim, select shell, then hold fire");
		}
		else
		{
			Status = TEXT("Cannot fire from current state");
		}
	}
	const FText StatusTextValue = FText::FromString(Status);
	BattleHUDViewModel->SetStatusText(StatusTextValue);
	BattleStateViewModel->SetStatusText(StatusTextValue);

	const float PlayerHealth = PlayerCharacter ? PlayerCharacter->GetHealth() : 0.0f;
	const float PlayerMaxHealth = PlayerCharacter ? PlayerCharacter->GetMaxHealth() : 0.0f;
	const float PlayerHealthPercent = SafePercent(PlayerHealth, PlayerMaxHealth);
	const FText PlayerHealthTextValue = FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), PlayerHealth, PlayerMaxHealth));
	BattleHUDViewModel->SetPlayerHealthPercent(PlayerHealthPercent);
	BattleHUDViewModel->SetPlayerHealthText(PlayerHealthTextValue);
	PlayerStatusViewModel->SetTitleText(FText::FromString(TEXT("PLAYER ARMOR")));
	PlayerStatusViewModel->SetCurrentHealthValue(PlayerHealth);
	PlayerStatusViewModel->SetMaxHealthValue(PlayerMaxHealth);
	PlayerStatusViewModel->SetHealthPercent(PlayerHealthPercent);
	PlayerStatusViewModel->SetHealthText(PlayerHealthTextValue);

	const float EnemyHealth = EnemyCharacter ? EnemyCharacter->GetHealth() : 0.0f;
	const float EnemyMaxHealth = EnemyCharacter ? EnemyCharacter->GetMaxHealth() : 0.0f;
	BattleHUDViewModel->SetEnemyHealthPercent(SafePercent(EnemyHealth, EnemyMaxHealth));
	BattleHUDViewModel->SetEnemyHealthText(EnemyCharacter
		? FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), EnemyHealth, EnemyMaxHealth))
		: FText::FromString(TEXT("-")));

	const float Wind = GameMode ? GameMode->GetWind() : 0.0f;
	const FString WindArrow = Wind > 1.0f ? TEXT("->") : Wind < -1.0f ? TEXT("<-") : TEXT("--");
	const FText WindTextValue = FText::FromString(FString::Printf(TEXT("Wind %s %.0f"), *WindArrow, FMath::Abs(Wind)));
	BattleHUDViewModel->SetWindText(WindTextValue);
	AimWindViewModel->SetWindText(WindTextValue);
	AimWindViewModel->SetWindValue(Wind);
	AimWindViewModel->SetAbsoluteWindValue(FMath::Abs(Wind));

	const float AimAngle = PlayerCharacter ? PlayerCharacter->GetAimAngle() : 0.0f;
	const FText AimTextValue = FText::FromString(FString::Printf(TEXT("Aim %.0f deg"), AimAngle));
	BattleHUDViewModel->SetAimText(AimTextValue);
	AimWindViewModel->SetAimText(AimTextValue);
	AimWindViewModel->SetAimAngleValue(AimAngle);

	const float ShotPower = PlayerCharacter ? PlayerCharacter->GetShotPower() : 0.0f;
	const float ShotCharge = PlayerCharacter ? PlayerCharacter->GetShotChargeAlpha() : 0.0f;
	const float DisplayedShotPower = PlayerCharacter && PlayerCharacter->IsChargingShot() ? ShotCharge : ShotPower;
	const FText ShotPowerTextValue = PercentText(DisplayedShotPower);
	BattleHUDViewModel->SetShotPowerPercent(DisplayedShotPower);
	BattleHUDViewModel->SetShotPowerText(ShotPowerTextValue);
	ShotPowerViewModel->SetShotPowerPercent(DisplayedShotPower);
	ShotPowerViewModel->SetShotPowerText(ShotPowerTextValue);

	const float MoveBudget = PlayerCharacter ? PlayerCharacter->GetMoveBudget() : 0.0f;
	const float MaxMoveBudget = PlayerCharacter ? PlayerCharacter->GetMaxMoveBudget() : 0.0f;
	const float MoveBudgetPercent = SafePercent(MoveBudget, MaxMoveBudget);
	const FText MoveBudgetTextValue = FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), MoveBudget, MaxMoveBudget));
	BattleHUDViewModel->SetMoveBudgetPercent(MoveBudgetPercent);
	BattleHUDViewModel->SetMoveBudgetText(MoveBudgetTextValue);
	PlayerStatusViewModel->SetMoveBudgetValue(MoveBudget);
	PlayerStatusViewModel->SetMaxMoveBudgetValue(MaxMoveBudget);
	PlayerStatusViewModel->SetMoveBudgetPercent(MoveBudgetPercent);
	PlayerStatusViewModel->SetMoveBudgetText(MoveBudgetTextValue);

	const FFRWeaponSpec CurrentWeapon = PlayerCharacter ? PlayerCharacter->GetCurrentWeaponSpec() : FFRWeaponSpec();
	const FText CurrentWeaponTextValue = CurrentWeapon.DisplayName.IsEmpty()
		? FText::FromString(TEXT("No Shell"))
		: CurrentWeapon.DisplayName;
	BattleHUDViewModel->SetCurrentWeaponText(CurrentWeaponTextValue);
	LoadoutViewModel->SetCurrentWeaponText(CurrentWeaponTextValue);

	const TArray<FFRWeaponSpec> WeaponLoadout = PlayerCharacter ? PlayerCharacter->GetWeaponLoadoutForBlueprint() : TArray<FFRWeaponSpec>();
	const int32 SelectedWeaponIndex = PlayerCharacter ? PlayerCharacter->GetSelectedWeaponIndex() : INDEX_NONE;
	const int32 WeaponSlotCount = FMath::Max(VisibleWeaponSlotCount, WeaponLoadout.Num());
	LoadoutViewModel->SetWeaponSlotCount(WeaponSlotCount);
	for (int32 Index = 0; Index < WeaponSlotCount; ++Index)
	{
		UFRLoadoutSlotViewModel* SlotViewModel = LoadoutViewModel->GetOrCreateWeaponSlotViewModel(Index);
		if (!SlotViewModel)
		{
			continue;
		}

		const bool bHasWeapon = WeaponLoadout.IsValidIndex(Index);
		const bool bCanSelectWeapon = bHasWeapon && PlayerCharacter && PlayerCharacter->CanSelectWeapon(Index);
		SlotViewModel->SetSlotLabelText(GetWeaponSlotLabel(Index));
		SlotViewModel->SetDisplayText(bHasWeapon ? WeaponLoadout[Index].DisplayName : FText::FromString(TEXT("-")));
		SlotViewModel->SetCountValue(0.0f);
		SlotViewModel->SetShowCount(false);
		SlotViewModel->SetOccupied(bHasWeapon);
		SlotViewModel->SetSelected(Index == SelectedWeaponIndex);
		SlotViewModel->SetEnabled(bCanSelectWeapon);
		SlotViewModel->SetStatusText(!bHasWeapon
			? FText::FromString(TEXT("EMPTY"))
			: bCanSelectWeapon ? FText::GetEmpty() : FText::FromString(TEXT("LOCKED")));
	}

	const TArray<FFRItemStack> ItemLoadout = PlayerCharacter ? PlayerCharacter->GetItemLoadoutForBlueprint() : TArray<FFRItemStack>();
	const int32 ItemSlotCount = FMath::Max(VisibleItemSlotCount, ItemLoadout.Num());
	LoadoutViewModel->SetItemSlotCount(ItemSlotCount);
	for (int32 Index = 0; Index < ItemSlotCount; ++Index)
	{
		UFRLoadoutSlotViewModel* SlotViewModel = LoadoutViewModel->GetOrCreateItemSlotViewModel(Index);
		if (!SlotViewModel)
		{
			continue;
		}

		const bool bHasItem = ItemLoadout.IsValidIndex(Index) && ItemLoadout[Index].ItemDefinition;
		const bool bCanUseItem = bHasItem && PlayerCharacter && PlayerCharacter->CanUseItemByIndex(Index);
		SlotViewModel->SetSlotLabelText(GetItemSlotLabel(Index));
		SlotViewModel->SetDisplayText(bHasItem ? ItemLoadout[Index].ItemDefinition->DisplayName : FText::FromString(TEXT("-")));
		SlotViewModel->SetCountValue(bHasItem ? static_cast<float>(ItemLoadout[Index].Charges) : 0.0f);
		SlotViewModel->SetShowCount(bHasItem);
		SlotViewModel->SetOccupied(bHasItem);
		SlotViewModel->SetSelected(false);
		SlotViewModel->SetEnabled(bCanUseItem);
		SlotViewModel->SetStatusText(!bHasItem
			? FText::FromString(TEXT("EMPTY"))
			: bCanUseItem ? FText::GetEmpty() : FText::FromString(TEXT("LOCKED")));
	}

	const FFRShotSpec ShotSpec = PlayerCharacter ? PlayerCharacter->GetCurrentShotSpec() : FFRShotSpec();
	const FText ShotEffectSummary = GetShotEffectSummary(ShotSpec.EffectTags);
	const FText ShotInfoTextValue = FText::FromString(FString::Printf(
		TEXT("Damage %.0f  Blast %.0f\nProjectiles x%d\nEffects %s\nSpeed %.0f  Gravity %.0f\nTerrain %.0f  Fill %.0f"),
		ShotSpec.Damage,
		ShotSpec.BlastRadius,
		ShotSpec.ProjectileCount,
		*ShotEffectSummary.ToString(),
		ShotSpec.LaunchSpeed,
		ShotSpec.Gravity,
		ShotSpec.TerrainDamage,
		ShotSpec.TerrainFillRadius));
	const FText ShotPrimaryTextValue = FText::FromString(FString::Printf(
		TEXT("Damage %.0f  Blast %.0f\nProjectiles x%d\nEffects %s"),
		ShotSpec.Damage,
		ShotSpec.BlastRadius,
		ShotSpec.ProjectileCount,
		*ShotEffectSummary.ToString()));
	const FText ShotSecondaryTextValue = FText::FromString(FString::Printf(
		TEXT("Speed %.0f  Gravity %.0f\nTerrain %.0f  Fill %.0f"),
		ShotSpec.LaunchSpeed,
		ShotSpec.Gravity,
		ShotSpec.TerrainDamage,
		ShotSpec.TerrainFillRadius));
	BattleHUDViewModel->SetShotInfoText(ShotInfoTextValue);
	BattleHUDViewModel->SetShotPrimaryText(ShotPrimaryTextValue);
	BattleHUDViewModel->SetShotSecondaryText(ShotSecondaryTextValue);
	ShotPreviewViewModel->SetPrimaryText(ShotPrimaryTextValue);
	ShotPreviewViewModel->SetSecondaryText(ShotSecondaryTextValue);
	ShotPreviewViewModel->SetDamageValue(ShotSpec.Damage);
	ShotPreviewViewModel->SetBlastRadiusValue(ShotSpec.BlastRadius);
	ShotPreviewViewModel->SetProjectileCountValue(ShotSpec.ProjectileCount);
	ShotPreviewViewModel->SetLaunchSpeedValue(ShotSpec.LaunchSpeed);
	ShotPreviewViewModel->SetGravityValue(ShotSpec.Gravity);
	ShotPreviewViewModel->SetTerrainDamageValue(ShotSpec.TerrainDamage);
	ShotPreviewViewModel->SetTerrainFillRadiusValue(ShotSpec.TerrainFillRadius);

	const FText GrantedModifierSummary = PlayerCharacter
		? GetModifierSummary(PlayerCharacter->GetGrantedShotModifiersForBlueprint())
		: FText::FromString(TEXT("None"));
	const FText PendingModifierSummary = PlayerCharacter
		? GetModifierSummary(PlayerCharacter->GetPendingShotModifiersForBlueprint())
		: FText::FromString(TEXT("None"));
	const FText AbilitySetSummary = PlayerCharacter
		? GetAbilitySetSummary(PlayerCharacter->GetGrantedAbilitySetsForBlueprint())
		: FText::FromString(TEXT("None"));
	BattleHUDViewModel->SetGrantedModifierText(GrantedModifierSummary);
	BattleHUDViewModel->SetPendingModifierText(PendingModifierSummary);
	BattleHUDViewModel->SetAbilitySetText(AbilitySetSummary);
	const FText ModifierSummaryTextValue = FText::FromString(FString::Printf(
		TEXT("Active: %s\nNext: %s\nTraits: %s"),
		*GrantedModifierSummary.ToString(),
		*PendingModifierSummary.ToString(),
		*AbilitySetSummary.ToString()));
	BattleHUDViewModel->SetModifierSummaryText(ModifierSummaryTextValue);
	ModifierSummaryViewModel->SetGrantedModifierText(GrantedModifierSummary);
	ModifierSummaryViewModel->SetPendingModifierText(PendingModifierSummary);
	ModifierSummaryViewModel->SetAbilitySetText(AbilitySetSummary);
	ModifierSummaryViewModel->SetSummaryText(ModifierSummaryTextValue);
}

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
