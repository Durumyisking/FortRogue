// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FRFloatingDamageTextActor.generated.h"

class UFRFloatingDamageTextWidget;
class USceneComponent;
class UWidgetComponent;

UCLASS()
class FORTROGUE_API AFRFloatingDamageTextActor : public AActor
{
	GENERATED_BODY()

public:
	AFRFloatingDamageTextActor();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Damage Text")
	void InitializeDamageText(float DamageAmount);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FortRogue|Damage Text")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FortRogue|Damage Text")
	TObjectPtr<UWidgetComponent> DamageWidgetComponent;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Damage Text")
	TSubclassOf<UFRFloatingDamageTextWidget> DamageWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Damage Text", meta = (ClampMin = "0.01"))
	float LifetimeSeconds = 0.9f;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Damage Text")
	float RiseSpeed = 90.0f;

private:
	void ApplyDamageTextToWidget();

	float ElapsedSeconds = 0.0f;
	float PendingDamageAmount = 0.0f;
};
