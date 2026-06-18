// Copyright Epic Games, Inc. All Rights Reserved.

#include "FortRogueHUD.h"

#include "Combat/FortRogueBattleCharacter.h"
#include "FortRogueGameMode.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"

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
}

void AFortRogueHUD::DrawBattleHUD(AFortRogueGameMode* GameMode, float X, float& Y)
{
	AFortRogueBattleCharacter* Player = GameMode->GetPlayerCharacter();
	AFortRogueBattleCharacter* Enemy = GameMode->GetEnemyCharacter();
	const float Scale = 1.15f;

	DrawText(FString::Printf(TEXT("FortRogue MVP - %s"), *GameMode->GetStatusText().ToString()), FColor::White, X, Y, GEngine->GetSmallFont(), Scale);
	Y += 28.0f;

	if (Player)
	{
		DrawText(FString::Printf(TEXT("%s HP %.0f/%.0f | Move %.0f | Aim %.0f deg"),
			*Player->GetCharacterDisplayName().ToString(), Player->GetHealth(), Player->GetMaxHealth(), Player->GetMoveBudget(), Player->GetAimAngle()),
			FColor::Green, X, Y, GEngine->GetSmallFont(), Scale);
		Y += 24.0f;
		DrawPowerGauge(Player, X, Y);
		DrawText(FString::Printf(TEXT("Weapon %d: %s | 1/2 Weapon | J Attack Amp x%d | H Repair x%d"),
			Player->GetSelectedWeaponIndex() + 1,
			*Player->GetCurrentWeapon().DisplayName.ToString(),
			Player->GetItemCharges(EFortRogueItemType::AttackMultiplier),
			Player->GetItemCharges(EFortRogueItemType::Heal)),
			FColor::Cyan, X, Y, GEngine->GetSmallFont(), Scale);
		Y += 24.0f;
		DrawText(TEXT("A/D Move | W/S Aim | Hold Space Power / Release Fire"), FColor::Cyan, X, Y, GEngine->GetSmallFont(), Scale);
		Y += 24.0f;
	}

	if (Enemy)
	{
		DrawText(FString::Printf(TEXT("Enemy HP %.0f/%.0f | Wind %.0f"), Enemy->GetHealth(), Enemy->GetMaxHealth(), GameMode->GetWind()),
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

