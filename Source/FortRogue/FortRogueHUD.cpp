// Copyright Epic Games, Inc. All Rights Reserved.

#include "FortRogueHUD.h"

#include "Combat/FortRogueBattleCharacter.h"
#include "FortRogueGameMode.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Items/FortRogueItemDefinition.h"

namespace
{
FString GetItemInputPrefix(EFortRogueItemType ItemType)
{
	if (ItemType == EFortRogueItemType::AttackMultiplier)
	{
		return TEXT("J ");
	}
	if (ItemType == EFortRogueItemType::Heal)
	{
		return TEXT("H ");
	}
	return FString();
}

FString BuildItemLoadoutText(const AFortRogueBattleCharacter& Player)
{
	TArray<FString> ItemParts;
	for (const FFortRogueItemStack& ItemStack : Player.GetItemLoadout())
	{
		const UFortRogueItemDefinition* ItemDefinition = ItemStack.ItemDefinition;
		if (!ItemDefinition)
		{
			continue;
		}

		const FString InputPrefix = GetItemInputPrefix(ItemDefinition->ItemType);
		ItemParts.Add(FString::Printf(TEXT("%s%s x%d"),
			*InputPrefix,
			*ItemDefinition->DisplayName.ToString(),
			FMath::Max(0, ItemStack.Charges)));
	}

	return ItemParts.Num() > 0
		? FString::Printf(TEXT("Items: %s"), *FString::Join(ItemParts, TEXT(" | ")))
		: TEXT("Items: none");
}
}

void AFortRogueHUD::DrawHUD()
{
	Super::DrawHUD();

	AFortRogueGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFortRogueGameMode>() : nullptr;
	if (!GameMode)
	{
		return;
	}

	float Y = 32.0f;
	DrawBattleHUD(GameMode, 32.0f, Y);
	if (GameMode->GetBattleState() == EFortRogueBattleState::Reward)
	{
		DrawRewardHUD(GameMode, 32.0f, Y);
	}
}

void AFortRogueHUD::DrawBattleHUD(AFortRogueGameMode* GameMode, float X, float& Y)
{
	AFortRogueBattleCharacter* Player = GameMode->GetPlayerCharacter();
	AFortRogueBattleCharacter* Enemy = GameMode->GetEnemyCharacter();
	const float Scale = 1.15f;

	DrawText(FString::Printf(TEXT("FortRogue MVP - %s"), *GameMode->GetRunProgressSummary().ToString()), FColor::White, X, Y, GEngine->GetSmallFont(), Scale);
	Y += 28.0f;

	if (Player)
	{
		DrawText(FString::Printf(TEXT("%s HP %.0f/%.0f | Move %.0f | Aim %.0f deg"),
			*Player->GetCharacterDisplayName().ToString(), Player->GetHealth(), Player->GetMaxHealth(), Player->GetMoveBudget(), Player->GetAimAngle()),
			FColor::Green, X, Y, GEngine->GetSmallFont(), Scale);
		Y += 24.0f;
		DrawPowerGauge(Player, X, Y);
		const int32 MaxWeaponKey = FMath::Max(1, FMath::Min(Player->GetWeaponLoadout().Num(), 5));
		DrawText(FString::Printf(TEXT("Weapon %d: %s | 1-%d Weapon"),
			Player->GetSelectedWeaponIndex() + 1,
			*Player->GetCurrentWeapon().DisplayName.ToString(),
			MaxWeaponKey),
			FColor::Cyan, X, Y, GEngine->GetSmallFont(), Scale);
		Y += 24.0f;
		DrawText(BuildItemLoadoutText(*Player),
			FColor::Cyan, X, Y, GEngine->GetSmallFont(), Scale);
		Y += 24.0f;
		DrawText(Player->GetCurrentShotSummary().ToString(), FColor::Cyan, X, Y, GEngine->GetSmallFont(), Scale);
		Y += 24.0f;
		DrawText(TEXT("A/D Move | W/S Aim | Hold Space Power / Release Fire"), FColor::Cyan, X, Y, GEngine->GetSmallFont(), Scale);
		Y += 24.0f;
	}

	if (Enemy)
	{
		DrawText(FString::Printf(TEXT("Enemy HP %.0f/%.0f | %s"), Enemy->GetHealth(), Enemy->GetMaxHealth(), *GameMode->GetWindSummary().ToString()),
			FColor::Red, X, Y, GEngine->GetSmallFont(), Scale);
		Y += 32.0f;
	}
}

void AFortRogueHUD::DrawPowerGauge(AFortRogueBattleCharacter* Player, float X, float& Y)
{
	if (!Player)
	{
		return;
	}

	const float Scale = 1.15f;
	const float BarX = X + 94.0f;
	const float BarY = Y + 4.0f;
	const float BarWidth = 220.0f;
	const float BarHeight = 14.0f;
	const float FillWidth = BarWidth * Player->GetShotChargeAlpha();
	const FLinearColor FillColor = Player->IsChargingShot() ? FLinearColor(1.0f, 0.78f, 0.12f, 1.0f) : FLinearColor(0.2f, 0.8f, 0.25f, 1.0f);

	DrawText(FString::Printf(TEXT("Power %.0f%%"), Player->GetShotPower() * 100.0f), FColor::Green, X, Y, GEngine->GetSmallFont(), Scale);
	DrawRect(FLinearColor(0.02f, 0.02f, 0.02f, 0.85f), BarX, BarY, BarWidth, BarHeight);
	DrawRect(FillColor, BarX, BarY, FillWidth, BarHeight);
	DrawRect(FLinearColor::White, BarX - 1.0f, BarY - 1.0f, BarWidth + 2.0f, 1.0f);
	DrawRect(FLinearColor::White, BarX - 1.0f, BarY + BarHeight, BarWidth + 2.0f, 1.0f);
	DrawRect(FLinearColor::White, BarX - 1.0f, BarY - 1.0f, 1.0f, BarHeight + 2.0f);
	DrawRect(FLinearColor::White, BarX + BarWidth, BarY - 1.0f, 1.0f, BarHeight + 2.0f);
	Y += 24.0f;
}

void AFortRogueHUD::DrawRewardHUD(AFortRogueGameMode* GameMode, float X, float& Y)
{
	const TArray<FFortRogueRewardChoice> Choices = GameMode->GetRewardChoices();
	DrawText(FString::Printf(TEXT("Choose reward: 1-%d"), FMath::Max(1, Choices.Num())),
		FColor::Yellow, X, Y, GEngine->GetSmallFont(), 1.2f);
	Y += 30.0f;

	for (int32 Index = 0; Index < Choices.Num(); ++Index)
	{
		const FFortRogueRewardChoice& Choice = Choices[Index];
		const FText EffectSummary = Choice.GetEffectSummary();
		const FText RewardDetails = EffectSummary.IsEmpty() ? Choice.Description : EffectSummary;
		DrawText(FString::Printf(TEXT("%d. %s - %s"), Index + 1, *Choice.DisplayName.ToString(), *RewardDetails.ToString()),
			FColor::Yellow, X, Y, GEngine->GetSmallFont(), 1.1f);
		Y += 24.0f;
	}
}
