// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FRFloatingCombatText.generated.h"

class UWidgetComponent;
class UFRFloatingCombatTextWidget;

UCLASS()
class FORTROGUE_API AFRFloatingCombatText : public AActor
{
	GENERATED_BODY()

public:
	AFRFloatingCombatText();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void InitializeDamageText(float DamageAmount);

private:
	void UpdateDamageWidget();

	UPROPERTY(VisibleAnywhere, Category = "FortRogue|UI")
	TObjectPtr<UWidgetComponent> WidgetComponent;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI", meta = (ToolTip = "데미지 숫자를 표시할 위젯 클래스입니다."))
	TSubclassOf<UFRFloatingCombatTextWidget> FloatingCombatTextWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI", meta = (ClampMin = "0.1"))
	float LifeSeconds = 0.85f;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|UI")
	float RiseSpeed = 82.0f;

	float ElapsedSeconds = 0.0f;
	float CachedDamageAmount = 0.0f;
};
