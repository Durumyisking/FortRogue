// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FRBattleHUDWidget.h"

#include "AbilitySystem/FRAbilitySet.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Combat/FRBattleCharacter.h"
#include "CommonBorder.h"
#include "CommonTextBlock.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "FRGameMode.h"
#include "Items/FRItemDefinition.h"
#include "MVVMBlueprintLibrary.h"
#include "UI/FRBattleHUDModuleViewModels.h"
#include "UI/FRBattleHUDViewModel.h"

namespace
{
	const FLinearColor PanelColor(0.015f, 0.018f, 0.022f, 0.82f);
	const FLinearColor PanelAccentColor(0.16f, 0.18f, 0.20f, 0.9f);
	const FLinearColor TextColor(0.88f, 0.92f, 0.95f, 1.0f);
	const FLinearColor MutedTextColor(0.55f, 0.60f, 0.64f, 1.0f);
	const FLinearColor PlayerColor(0.12f, 0.62f, 0.42f, 1.0f);
	const FLinearColor EnemyColor(0.82f, 0.20f, 0.16f, 1.0f);
	const FLinearColor PowerColor(0.95f, 0.65f, 0.16f, 1.0f);
	const FLinearColor MoveColor(0.23f, 0.48f, 0.82f, 1.0f);
	const FLinearColor ReadyColor(0.18f, 0.60f, 0.38f, 0.95f);
	const FLinearColor DisabledColor(0.12f, 0.13f, 0.14f, 0.72f);
	const FLinearColor SelectedColor(0.95f, 0.72f, 0.26f, 0.95f);

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

	FLinearColor GetBattleStateColor(EFRBattleState BattleState)
	{
		switch (BattleState)
		{
		case EFRBattleState::PlayerTurn:
			return ReadyColor;
		case EFRBattleState::EnemyTurn:
			return EnemyColor;
		case EFRBattleState::ResolvingShot:
			return PowerColor;
		case EFRBattleState::Reward:
			return SelectedColor;
		case EFRBattleState::Won:
			return PlayerColor;
		case EFRBattleState::Lost:
			return EnemyColor;
		default:
			return PanelAccentColor;
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

	void SetText(UTextBlock* TextBlock, const FText& Text)
	{
		if (TextBlock)
		{
			TextBlock->SetText(Text);
		}
	}

	void SetBar(UProgressBar* Bar, float Value, const FLinearColor& FillColor)
	{
		if (Bar)
		{
			Bar->SetPercent(FMath::Clamp(Value, 0.0f, 1.0f));
			Bar->SetFillColorAndOpacity(FillColor);
		}
	}

	void SetSlotState(UBorder* Slot, bool bSelected, bool bEnabled)
	{
		if (!Slot)
		{
			return;
		}

		if (bSelected)
		{
			Slot->SetBrushColor(SelectedColor);
		}
		else if (bEnabled)
		{
			Slot->SetBrushColor(PanelAccentColor);
		}
		else
		{
			Slot->SetBrushColor(DisabledColor);
		}
	}
}

void UFRBattleHUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	CreateViewModels();
	ApplyBattleHUDViewModel(this);

	BuildDefaultHUD();
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
	RefreshDefaultHUD();
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

	TScriptInterface<INotifyFieldValueChanged> ViewModelInterface;
	ViewModelInterface.SetObject(ViewModel);
	ViewModelInterface.SetInterface(Cast<INotifyFieldValueChanged>(ViewModel));
	UMVVMBlueprintLibrary::SetViewModelByClass(Widget, ViewModelInterface);
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

	const float AimAngle = PlayerCharacter ? PlayerCharacter->GetAimAngle() : 0.0f;
	const FText AimTextValue = FText::FromString(FString::Printf(TEXT("Aim %.0f deg"), AimAngle));
	BattleHUDViewModel->SetAimText(AimTextValue);
	AimWindViewModel->SetAimText(AimTextValue);

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
	PlayerStatusViewModel->SetMoveBudgetPercent(MoveBudgetPercent);
	PlayerStatusViewModel->SetMoveBudgetText(MoveBudgetTextValue);

	const FFRWeaponSpec CurrentWeapon = PlayerCharacter ? PlayerCharacter->GetCurrentWeaponSpec() : FFRWeaponSpec();
	const FText CurrentWeaponTextValue = CurrentWeapon.DisplayName.IsEmpty()
		? FText::FromString(TEXT("No Shell"))
		: CurrentWeapon.DisplayName;
	BattleHUDViewModel->SetCurrentWeaponText(CurrentWeaponTextValue);
	LoadoutViewModel->SetCurrentWeaponText(CurrentWeaponTextValue);

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

void UFRBattleHUDWidget::BuildDefaultHUD()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("DefaultBattleHUDRoot"));
	WidgetTree->RootWidget = RootCanvas;

	UBorder* HeaderPanel = WidgetTree->ConstructWidget<UCommonBorder>(UCommonBorder::StaticClass(), TEXT("HeaderPanel"));
	HeaderPanel->SetBrushColor(PanelColor);
	UHorizontalBox* HeaderBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("HeaderBox"));
	HeaderPanel->SetContent(HeaderBox);
	if (UCanvasPanelSlot* HeaderSlot = RootCanvas->AddChildToCanvas(HeaderPanel))
	{
		HeaderSlot->SetAnchors(FAnchors(0.5f, 0.0f));
		HeaderSlot->SetAlignment(FVector2D(0.5f, 0.0f));
		HeaderSlot->SetPosition(FVector2D(0.0f, 18.0f));
		HeaderSlot->SetSize(FVector2D(900.0f, 72.0f));
	}

	TurnBadge = WidgetTree->ConstructWidget<UCommonBorder>(UCommonBorder::StaticClass(), TEXT("TurnBadge"));
	TurnBadge->SetBrushColor(ReadyColor);
	TurnText = AddText(TurnBadge, TEXT("TurnText"), FText::FromString(TEXT("PLAYER TURN")), 18.0f, FLinearColor::Black);
	if (UHorizontalBoxSlot* TurnSlot = HeaderBox->AddChildToHorizontalBox(TurnBadge))
	{
		TurnSlot->SetPadding(FMargin(12.0f, 12.0f, 10.0f, 12.0f));
		TurnSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
	}

	RunProgressText = AddText(HeaderBox, TEXT("RunProgressText"), FText::FromString(TEXT("Stage -/-")), 17.0f, TextColor);
	StatusText = AddText(HeaderBox, TEXT("StatusText"), FText::GetEmpty(), 16.0f, MutedTextColor);

	UBorder* LeftPanel = WidgetTree->ConstructWidget<UCommonBorder>(UCommonBorder::StaticClass(), TEXT("CombatPanel"));
	LeftPanel->SetBrushColor(PanelColor);
	UVerticalBox* LeftBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CombatBox"));
	LeftPanel->SetContent(LeftBox);
	if (UCanvasPanelSlot* LeftSlot = RootCanvas->AddChildToCanvas(LeftPanel))
	{
		LeftSlot->SetAnchors(FAnchors(0.0f, 0.0f));
		LeftSlot->SetPosition(FVector2D(24.0f, 112.0f));
		LeftSlot->SetSize(FVector2D(340.0f, 520.0f));
	}

	AddText(LeftBox, TEXT("PlayerLabel"), FText::FromString(TEXT("PLAYER ARMOR")), 13.0f, MutedTextColor);
	UTextBlock* PlayerHealthValueText = nullptr;
	PlayerHealthBar = AddLabeledBar(LeftBox, TEXT("PlayerHealthBar"), FText::FromString(TEXT("HP")), PlayerHealthValueText);
	PlayerHealthText = PlayerHealthValueText;
	AddText(LeftBox, TEXT("EnemyLabel"), FText::FromString(TEXT("ENEMY ARMOR")), 13.0f, MutedTextColor);
	UTextBlock* EnemyHealthValueText = nullptr;
	EnemyHealthBar = AddLabeledBar(LeftBox, TEXT("EnemyHealthBar"), FText::FromString(TEXT("HP")), EnemyHealthValueText);
	EnemyHealthText = EnemyHealthValueText;

	WindText = AddText(LeftBox, TEXT("WindText"), FText::FromString(TEXT("Wind: 0")), 17.0f, TextColor);
	AimText = AddText(LeftBox, TEXT("AimText"), FText::FromString(TEXT("Aim: 45 deg")), 17.0f, TextColor);

	USizeBox* AimBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("AimIndicatorBox"));
	AimBox->SetHeightOverride(86.0f);
	UCanvasPanel* AimCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("AimIndicatorCanvas"));
	AimBox->SetContent(AimCanvas);
	if (UVerticalBoxSlot* AimBoxSlot = LeftBox->AddChildToVerticalBox(AimBox))
	{
		AimBoxSlot->SetPadding(FMargin(12.0f, 8.0f, 12.0f, 10.0f));
	}

	UBorder* AimBase = WidgetTree->ConstructWidget<UCommonBorder>(UCommonBorder::StaticClass(), TEXT("AimBase"));
	AimBase->SetBrushColor(PanelAccentColor);
	if (UCanvasPanelSlot* BaseSlot = AimCanvas->AddChildToCanvas(AimBase))
	{
		BaseSlot->SetPosition(FVector2D(24.0f, 52.0f));
		BaseSlot->SetSize(FVector2D(44.0f, 20.0f));
	}

	AimBarrel = WidgetTree->ConstructWidget<UCommonBorder>(UCommonBorder::StaticClass(), TEXT("AimBarrel"));
	AimBarrel->SetBrushColor(PowerColor);
	AimBarrel->SetRenderTransformPivot(FVector2D(0.0f, 0.5f));
	if (UCanvasPanelSlot* BarrelSlot = AimCanvas->AddChildToCanvas(AimBarrel))
	{
		BarrelSlot->SetPosition(FVector2D(58.0f, 57.0f));
		BarrelSlot->SetSize(FVector2D(120.0f, 8.0f));
		BarrelSlot->SetAlignment(FVector2D(0.0f, 0.5f));
	}

	UTextBlock* ShotPowerValueText = nullptr;
	ShotPowerBar = AddLabeledBar(LeftBox, TEXT("ShotPowerBar"), FText::FromString(TEXT("POWER")), ShotPowerValueText);
	ShotPowerText = ShotPowerValueText;
	UTextBlock* MoveBudgetValueText = nullptr;
	MoveBudgetBar = AddLabeledBar(LeftBox, TEXT("MoveBudgetBar"), FText::FromString(TEXT("MOVE")), MoveBudgetValueText);
	MoveBudgetText = MoveBudgetValueText;

	UBorder* BottomPanel = WidgetTree->ConstructWidget<UCommonBorder>(UCommonBorder::StaticClass(), TEXT("LoadoutPanel"));
	BottomPanel->SetBrushColor(PanelColor);
	UVerticalBox* BottomBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LoadoutBox"));
	BottomPanel->SetContent(BottomBox);
	if (UCanvasPanelSlot* BottomSlot = RootCanvas->AddChildToCanvas(BottomPanel))
	{
		BottomSlot->SetAnchors(FAnchors(0.5f, 1.0f));
		BottomSlot->SetAlignment(FVector2D(0.5f, 1.0f));
		BottomSlot->SetPosition(FVector2D(0.0f, -24.0f));
		BottomSlot->SetSize(FVector2D(940.0f, 210.0f));
	}

	UHorizontalBox* CurrentWeaponBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("CurrentWeaponBox"));
	if (UVerticalBoxSlot* CurrentWeaponSlot = BottomBox->AddChildToVerticalBox(CurrentWeaponBox))
	{
		CurrentWeaponSlot->SetPadding(FMargin(12.0f, 10.0f, 12.0f, 4.0f));
	}
	CurrentWeaponText = AddText(CurrentWeaponBox, TEXT("CurrentWeaponText"), FText::FromString(TEXT("Shell")), 19.0f, TextColor);
	AttackTypeText = AddText(CurrentWeaponBox, TEXT("AttackTypeText"), FText::FromString(TEXT("BASIC")), 15.0f, SelectedColor);

	UHorizontalBox* WeaponBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("WeaponSlots"));
	if (UVerticalBoxSlot* WeaponBoxSlot = BottomBox->AddChildToVerticalBox(WeaponBox))
	{
		WeaponBoxSlot->SetPadding(FMargin(12.0f, 4.0f, 12.0f, 8.0f));
	}
	for (int32 Index = 0; Index < 5; ++Index)
	{
		UBorder* SlotBorder = AddSlot(WeaponBox, *FString::Printf(TEXT("WeaponSlot_%d"), Index), FVector2D(168.0f, 58.0f));
		UTextBlock* SlotText = AddText(SlotBorder, *FString::Printf(TEXT("WeaponSlotText_%d"), Index), FText::FromString(TEXT("-")), 14.0f, TextColor);
		WeaponSlots.Add(SlotBorder);
		WeaponSlotTexts.Add(SlotText);
	}

	UHorizontalBox* ItemBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ItemSlots"));
	if (UVerticalBoxSlot* ItemBoxSlot = BottomBox->AddChildToVerticalBox(ItemBox))
	{
		ItemBoxSlot->SetPadding(FMargin(12.0f, 0.0f, 12.0f, 10.0f));
	}
	for (int32 Index = 0; Index < 4; ++Index)
	{
		UBorder* SlotBorder = AddSlot(ItemBox, *FString::Printf(TEXT("ItemSlot_%d"), Index), FVector2D(210.0f, 48.0f));
		UTextBlock* SlotText = AddText(SlotBorder, *FString::Printf(TEXT("ItemSlotText_%d"), Index), FText::FromString(TEXT("-")), 13.0f, TextColor);
		ItemSlots.Add(SlotBorder);
		ItemSlotTexts.Add(SlotText);
	}

	UBorder* RightPanel = WidgetTree->ConstructWidget<UCommonBorder>(UCommonBorder::StaticClass(), TEXT("ShotPanel"));
	RightPanel->SetBrushColor(PanelColor);
	UVerticalBox* RightBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ShotBox"));
	RightPanel->SetContent(RightBox);
	if (UCanvasPanelSlot* RightSlot = RootCanvas->AddChildToCanvas(RightPanel))
	{
		RightSlot->SetAnchors(FAnchors(1.0f, 0.0f));
		RightSlot->SetAlignment(FVector2D(1.0f, 0.0f));
		RightSlot->SetPosition(FVector2D(-24.0f, 112.0f));
		RightSlot->SetSize(FVector2D(360.0f, 430.0f));
	}

	AddText(RightBox, TEXT("ShotLabel"), FText::FromString(TEXT("CURRENT SHOT")), 13.0f, MutedTextColor);
	ShotInfoText = AddText(RightBox, TEXT("ShotInfoText"), FText::GetEmpty(), 15.0f, TextColor);
	AddText(RightBox, TEXT("GrantedModifierLabel"), FText::FromString(TEXT("ACTIVE MODIFIERS")), 13.0f, MutedTextColor);
	GrantedModifierText = AddText(RightBox, TEXT("GrantedModifierText"), FText::FromString(TEXT("None")), 14.0f, TextColor);
	AddText(RightBox, TEXT("PendingModifierLabel"), FText::FromString(TEXT("NEXT SHOT")), 13.0f, MutedTextColor);
	PendingModifierText = AddText(RightBox, TEXT("PendingModifierText"), FText::FromString(TEXT("None")), 14.0f, TextColor);
	AddText(RightBox, TEXT("AbilitySetLabel"), FText::FromString(TEXT("TRAITS")), 13.0f, MutedTextColor);
	AbilitySetText = AddText(RightBox, TEXT("AbilitySetText"), FText::FromString(TEXT("None")), 14.0f, TextColor);
}

void UFRBattleHUDWidget::RefreshDefaultHUD()
{
	if (!TurnText)
	{
		return;
	}

	AFRGameMode* GameMode = GetFortRogueGameMode();
	AFRBattleCharacter* PlayerCharacter = GetPlayerCharacter();
	AFRBattleCharacter* EnemyCharacter = GetEnemyCharacter();
	const EFRBattleState BattleState = GameMode ? GameMode->GetBattleState() : EFRBattleState::PlayerTurn;

	SetText(TurnText, GetBattleStateText(BattleState));
	if (TurnBadge)
	{
		TurnBadge->SetBrushColor(GetBattleStateColor(BattleState));
	}

	SetText(RunProgressText, GetRunProgressSummary());

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
	SetText(StatusText, FText::FromString(Status));

	RefreshCharacterBars(PlayerCharacter, EnemyCharacter);
	RefreshWeaponSlots(PlayerCharacter);
	RefreshItemSlots(PlayerCharacter);
	RefreshModifierSummaries(PlayerCharacter);

	const float Wind = GameMode ? GameMode->GetWind() : 0.0f;
	const FString WindArrow = Wind > 1.0f ? TEXT("->") : Wind < -1.0f ? TEXT("<-") : TEXT("--");
	SetText(WindText, FText::FromString(FString::Printf(TEXT("Wind %s %.0f"), *WindArrow, FMath::Abs(Wind))));

	const float AimAngle = PlayerCharacter ? PlayerCharacter->GetAimAngle() : 0.0f;
	SetText(AimText, FText::FromString(FString::Printf(TEXT("Aim %.0f deg"), AimAngle)));
	RefreshAimIndicator(AimAngle);

	const float ShotPower = PlayerCharacter ? PlayerCharacter->GetShotPower() : 0.0f;
	const float ShotCharge = PlayerCharacter ? PlayerCharacter->GetShotChargeAlpha() : 0.0f;
	SetBar(ShotPowerBar, PlayerCharacter && PlayerCharacter->IsChargingShot() ? ShotCharge : ShotPower, PowerColor);
	SetText(ShotPowerText, PercentText(PlayerCharacter && PlayerCharacter->IsChargingShot() ? ShotCharge : ShotPower));

	const float MoveBudget = PlayerCharacter ? PlayerCharacter->GetMoveBudget() : 0.0f;
	const float MaxMoveBudget = PlayerCharacter ? PlayerCharacter->GetMaxMoveBudget() : 0.0f;
	SetBar(MoveBudgetBar, SafePercent(MoveBudget, MaxMoveBudget), MoveColor);
	SetText(MoveBudgetText, FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), MoveBudget, MaxMoveBudget)));
}

void UFRBattleHUDWidget::RefreshCharacterBars(AFRBattleCharacter* PlayerCharacter, AFRBattleCharacter* EnemyCharacter)
{
	const float PlayerHealth = PlayerCharacter ? PlayerCharacter->GetHealth() : 0.0f;
	const float PlayerMaxHealth = PlayerCharacter ? PlayerCharacter->GetMaxHealth() : 0.0f;
	SetBar(PlayerHealthBar, SafePercent(PlayerHealth, PlayerMaxHealth), PlayerColor);
	SetText(PlayerHealthText, FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), PlayerHealth, PlayerMaxHealth)));

	const float EnemyHealth = EnemyCharacter ? EnemyCharacter->GetHealth() : 0.0f;
	const float EnemyMaxHealth = EnemyCharacter ? EnemyCharacter->GetMaxHealth() : 0.0f;
	SetBar(EnemyHealthBar, SafePercent(EnemyHealth, EnemyMaxHealth), EnemyColor);
	SetText(EnemyHealthText, EnemyCharacter
		? FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), EnemyHealth, EnemyMaxHealth))
		: FText::FromString(TEXT("-")));
}

void UFRBattleHUDWidget::RefreshWeaponSlots(AFRBattleCharacter* PlayerCharacter)
{
	const TArray<FFRWeaponSpec> Weapons = PlayerCharacter ? PlayerCharacter->GetWeaponLoadoutForBlueprint() : TArray<FFRWeaponSpec>();
	const int32 SelectedIndex = PlayerCharacter ? PlayerCharacter->GetSelectedWeaponIndex() : INDEX_NONE;
	const FFRWeaponSpec CurrentWeapon = PlayerCharacter ? PlayerCharacter->GetCurrentWeaponSpec() : FFRWeaponSpec();

	SetText(CurrentWeaponText, CurrentWeapon.DisplayName.IsEmpty() ? FText::FromString(TEXT("No Shell")) : CurrentWeapon.DisplayName);

	const FString AttackType = SelectedIndex == 0 ? TEXT("BASIC") : SelectedIndex == 1 ? TEXT("SPECIAL") : TEXT("SHELL");
	SetText(AttackTypeText, FText::FromString(AttackType));

	for (int32 Index = 0; Index < WeaponSlots.Num(); ++Index)
	{
		const bool bHasWeapon = Weapons.IsValidIndex(Index);
		const bool bCanSelect = PlayerCharacter && PlayerCharacter->CanSelectWeapon(Index);
		SetSlotState(WeaponSlots[Index], Index == SelectedIndex, bHasWeapon && bCanSelect);

		if (WeaponSlotTexts.IsValidIndex(Index))
		{
			if (bHasWeapon)
			{
				const FString Prefix = Index == 0 ? TEXT("Basic") : Index == 1 ? TEXT("Special") : FString::Printf(TEXT("%d"), Index + 1);
				SetText(WeaponSlotTexts[Index], FText::FromString(FString::Printf(TEXT("%s\n%s"), *Prefix, *Weapons[Index].DisplayName.ToString())));
			}
			else
			{
				SetText(WeaponSlotTexts[Index], FText::FromString(TEXT("-")));
			}
		}
	}

	const FFRShotSpec ShotSpec = PlayerCharacter ? PlayerCharacter->GetCurrentShotSpec() : FFRShotSpec();
	SetText(ShotInfoText, FText::FromString(FString::Printf(
		TEXT("Damage %.0f  Blast %.0f\nProjectiles x%d\nEffects %s\nSpeed %.0f  Gravity %.0f\nTerrain %.0f  Fill %.0f"),
		ShotSpec.Damage,
		ShotSpec.BlastRadius,
		ShotSpec.ProjectileCount,
		*GetShotEffectSummary(ShotSpec.EffectTags).ToString(),
		ShotSpec.LaunchSpeed,
		ShotSpec.Gravity,
		ShotSpec.TerrainDamage,
		ShotSpec.TerrainFillRadius)));
}

void UFRBattleHUDWidget::RefreshItemSlots(AFRBattleCharacter* PlayerCharacter)
{
	const TArray<FFRItemStack> Items = PlayerCharacter ? PlayerCharacter->GetItemLoadoutForBlueprint() : TArray<FFRItemStack>();

	for (int32 Index = 0; Index < ItemSlots.Num(); ++Index)
	{
		const bool bHasItem = Items.IsValidIndex(Index) && Items[Index].ItemDefinition;
		const bool bCanUse = PlayerCharacter && PlayerCharacter->CanUseItemByIndex(Index);
		SetSlotState(ItemSlots[Index], false, bHasItem && bCanUse);

		if (!ItemSlotTexts.IsValidIndex(Index))
		{
			continue;
		}

		if (bHasItem)
		{
			SetText(ItemSlotTexts[Index], FText::FromString(FString::Printf(TEXT("%s\nx%d"), *Items[Index].ItemDefinition->DisplayName.ToString(), Items[Index].Charges)));
		}
		else
		{
			SetText(ItemSlotTexts[Index], FText::FromString(TEXT("-")));
		}
	}
}

void UFRBattleHUDWidget::RefreshModifierSummaries(AFRBattleCharacter* PlayerCharacter)
{
	SetText(GrantedModifierText, PlayerCharacter
		? GetModifierSummary(PlayerCharacter->GetGrantedShotModifiersForBlueprint())
		: FText::FromString(TEXT("None")));
	SetText(PendingModifierText, PlayerCharacter
		? GetModifierSummary(PlayerCharacter->GetPendingShotModifiersForBlueprint())
		: FText::FromString(TEXT("None")));
	SetText(AbilitySetText, PlayerCharacter
		? GetAbilitySetSummary(PlayerCharacter->GetGrantedAbilitySetsForBlueprint())
		: FText::FromString(TEXT("None")));
}

void UFRBattleHUDWidget::RefreshAimIndicator(float AimAngle)
{
	if (!AimBarrel)
	{
		return;
	}

	FWidgetTransform Transform = AimBarrel->GetRenderTransform();
	Transform.Angle = -FMath::Clamp(AimAngle, 0.0f, 90.0f);
	AimBarrel->SetRenderTransform(Transform);
}

UTextBlock* UFRBattleHUDWidget::AddText(UWidget* Parent, FName WidgetName, const FText& InitialText, float FontSize, const FLinearColor& Color)
{
	UTextBlock* Text = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), WidgetName);
	Text->SetText(InitialText);
	Text->SetColorAndOpacity(FSlateColor(Color));
	Text->SetAutoWrapText(true);

	FSlateFontInfo FontInfo = Text->GetFont();
	FontInfo.Size = FMath::RoundToInt(FontSize);
	Text->SetFont(FontInfo);

	if (UBorder* Border = Cast<UBorder>(Parent))
	{
		Border->SetContent(Text);
	}
	else if (UVerticalBox* VerticalBox = Cast<UVerticalBox>(Parent))
	{
		if (UVerticalBoxSlot* TextSlot = VerticalBox->AddChildToVerticalBox(Text))
		{
			TextSlot->SetPadding(FMargin(12.0f, 6.0f, 12.0f, 2.0f));
		}
	}
	else if (UHorizontalBox* HorizontalBox = Cast<UHorizontalBox>(Parent))
	{
		if (UHorizontalBoxSlot* TextSlot = HorizontalBox->AddChildToHorizontalBox(Text))
		{
			TextSlot->SetPadding(FMargin(10.0f, 10.0f, 10.0f, 10.0f));
			TextSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			TextSlot->SetVerticalAlignment(VAlign_Center);
		}
	}
	else if (UOverlay* Overlay = Cast<UOverlay>(Parent))
	{
		Overlay->AddChildToOverlay(Text);
	}

	return Text;
}

UProgressBar* UFRBattleHUDWidget::AddLabeledBar(UVerticalBox* Parent, FName WidgetName, const FText& LabelText, UTextBlock*& OutValueText)
{
	UVerticalBox* BarBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), *FString::Printf(TEXT("%s_Box"), *WidgetName.ToString()));
	if (UVerticalBoxSlot* BarBoxSlot = Parent->AddChildToVerticalBox(BarBox))
	{
		BarBoxSlot->SetPadding(FMargin(12.0f, 4.0f, 12.0f, 10.0f));
	}

	UHorizontalBox* LabelBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), *FString::Printf(TEXT("%s_LabelBox"), *WidgetName.ToString()));
	BarBox->AddChildToVerticalBox(LabelBox);
	AddText(LabelBox, *FString::Printf(TEXT("%s_Label"), *WidgetName.ToString()), LabelText, 13.0f, MutedTextColor);
	OutValueText = AddText(LabelBox, *FString::Printf(TEXT("%s_Value"), *WidgetName.ToString()), FText::FromString(TEXT("-")), 13.0f, TextColor);

	UProgressBar* Bar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), WidgetName);
	Bar->SetPercent(0.0f);
	if (UVerticalBoxSlot* BarSlot = BarBox->AddChildToVerticalBox(Bar))
	{
		BarSlot->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 0.0f));
	}
	return Bar;
}

UBorder* UFRBattleHUDWidget::AddSlot(UHorizontalBox* Parent, FName WidgetName, const FVector2D& Size)
{
	USizeBox* SlotSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), *FString::Printf(TEXT("%s_Size"), *WidgetName.ToString()));
	SlotSize->SetWidthOverride(Size.X);
	SlotSize->SetHeightOverride(Size.Y);

	UBorder* SlotBorder = WidgetTree->ConstructWidget<UCommonBorder>(UCommonBorder::StaticClass(), WidgetName);
	SlotBorder->SetBrushColor(DisabledColor);
	SlotSize->SetContent(SlotBorder);

	if (UHorizontalBoxSlot* HSlot = Parent->AddChildToHorizontalBox(SlotSize))
	{
		HSlot->SetPadding(FMargin(5.0f, 0.0f, 5.0f, 0.0f));
		HSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
	}
	return SlotBorder;
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
