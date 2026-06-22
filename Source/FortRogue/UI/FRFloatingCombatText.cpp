// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FRFloatingCombatText.h"

#include "Components/WidgetComponent.h"
#include "UI/FRFloatingCombatTextWidget.h"

AFRFloatingCombatText::AFRFloatingCombatText()
{
	PrimaryActorTick.bCanEverTick = true;

	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));
	SetRootComponent(WidgetComponent);
	WidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WidgetComponent->SetWidgetClass(UFRFloatingCombatTextWidget::StaticClass());
	WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	WidgetComponent->SetDrawSize(FVector2D(96.0f, 36.0f));
	WidgetComponent->SetTwoSided(true);
}

void AFRFloatingCombatText::BeginPlay()
{
	Super::BeginPlay();
	UpdateDamageWidget();
}

void AFRFloatingCombatText::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ElapsedSeconds += DeltaSeconds;
	AddActorWorldOffset(FVector(0.0f, 0.0f, RiseSpeed * DeltaSeconds));

	const float Alpha = FMath::Clamp(ElapsedSeconds / FMath::Max(LifeSeconds, KINDA_SMALL_NUMBER), 0.0f, 1.0f);
	if (UUserWidget* UserWidget = WidgetComponent ? WidgetComponent->GetUserWidgetObject() : nullptr)
	{
		UserWidget->SetRenderOpacity(1.0f - Alpha);
	}

	if (ElapsedSeconds >= LifeSeconds)
	{
		Destroy();
	}
}

void AFRFloatingCombatText::InitializeDamageText(float DamageAmount)
{
	CachedDamageAmount = FMath::Max(0.0f, DamageAmount);
	UpdateDamageWidget();
}

void AFRFloatingCombatText::UpdateDamageWidget()
{
	if (!WidgetComponent)
	{
		return;
	}

	WidgetComponent->InitWidget();
	if (UFRFloatingCombatTextWidget* DamageWidget = Cast<UFRFloatingCombatTextWidget>(WidgetComponent->GetUserWidgetObject()))
	{
		DamageWidget->SetDamageAmount(CachedDamageAmount);
	}
}
