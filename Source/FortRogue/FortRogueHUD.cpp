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

	DrawText(FString::Printf(TEXT("FortRogue MVP - %s"), *GameMode->GetStatusText().ToString()), FColor::White, X, Y, GEngine->GetSmallFont(), Scale);
	Y += 28.0f;

	if (Player)
	{
		DrawText(FString::Printf(TEXT("%s HP %.0f/%.0f | Move %.0f | Aim %.0f deg | Power %.0f%%"),
			*Player->GetCharacterDisplayName().ToString(), Player->GetHealth(), Player->GetMaxHealth(), Player->GetMoveBudget(), Player->GetAimAngle(), Player->GetShotPower() * 100.0f),
			FColor::Green, X, Y, GEngine->GetSmallFont(), Scale);
		Y += 24.0f;
		DrawText(FString::Printf(TEXT("Weapon %d: %s | 1/2 Weapon | J Attack Amp x%d | H Repair x%d"),
			Player->GetSelectedWeaponIndex() + 1,
			*Player->GetCurrentWeapon().DisplayName.ToString(),
			Player->GetItemCharges(EFortRogueItemType::AttackMultiplier),
			Player->GetItemCharges(EFortRogueItemType::Heal)),
			FColor::Cyan, X, Y, GEngine->GetSmallFont(), Scale);
		Y += 24.0f;
		DrawText(TEXT("A/D Move | W/S Aim | Up/Down Power | Space Fire"), FColor::Cyan, X, Y, GEngine->GetSmallFont(), Scale);
		Y += 24.0f;
	}

	if (Enemy)
	{
		DrawText(FString::Printf(TEXT("Enemy HP %.0f/%.0f | Wind %.0f"), Enemy->GetHealth(), Enemy->GetMaxHealth(), GameMode->GetWind()),
			FColor::Red, X, Y, GEngine->GetSmallFont(), Scale);
		Y += 32.0f;
	}
}

void AFortRogueHUD::DrawRewardHUD(AFortRogueGameMode* GameMode, float X, float& Y)
{
	const TArray<FFortRogueRewardChoice> Choices = GameMode->GetRewardChoices();
	DrawText(TEXT("Choose reward: 1 / 2 / 3"), FColor::Yellow, X, Y, GEngine->GetSmallFont(), 1.2f);
	Y += 30.0f;

	for (int32 Index = 0; Index < Choices.Num(); ++Index)
	{
		const FFortRogueRewardChoice& Choice = Choices[Index];
		DrawText(FString::Printf(TEXT("%d. %s - %s"), Index + 1, *Choice.DisplayName.ToString(), *Choice.Description.ToString()),
			FColor::Yellow, X, Y, GEngine->GetSmallFont(), 1.1f);
		Y += 24.0f;
	}
}
