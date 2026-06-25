// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FRFloatingDamageTextActor.h"

#include "Components/WidgetComponent.h"
#include "UI/FRFloatingDamageTextWidget.h"

AFRFloatingDamageTextActor::AFRFloatingDamageTextActor()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	DamageWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("DamageWidget"));
	DamageWidgetComponent->SetupAttachment(Root);
	DamageWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DamageWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	DamageWidgetComponent->SetDrawAtDesiredSize(false);
	DamageWidgetComponent->SetDrawSize(FVector2D(160.0f, 52.0f));
	DamageWidgetComponent->SetPivot(FVector2D(0.5f, 0.5f));

}

void AFRFloatingDamageTextActor::BeginPlay()
{
	Super::BeginPlay();

	if (!DamageWidgetClass)
	{
		DamageWidgetClass = LoadClass<UFRFloatingDamageTextWidget>(nullptr, TEXT("/Game/FortRogue/Widget/Character/WBP_FloatingDamageText.WBP_FloatingDamageText_C"));
	}
	if (DamageWidgetClass && DamageWidgetComponent)
	{
		DamageWidgetComponent->SetWidgetClass(DamageWidgetClass);
	}
	ApplyDamageTextToWidget();
}

void AFRFloatingDamageTextActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ElapsedSeconds += DeltaSeconds;
	AddActorWorldOffset(FVector(0.0f, 0.0f, RiseSpeed * DeltaSeconds));

	const float Alpha = FMath::Clamp(ElapsedSeconds / FMath::Max(LifetimeSeconds, KINDA_SMALL_NUMBER), 0.0f, 1.0f);
	if (UUserWidget* DamageWidget = DamageWidgetComponent ? DamageWidgetComponent->GetUserWidgetObject() : nullptr)
	{
		DamageWidget->SetRenderOpacity(1.0f - Alpha);
	}

	if (ElapsedSeconds >= LifetimeSeconds)
	{
		Destroy();
	}
}

void AFRFloatingDamageTextActor::InitializeDamageText(float DamageAmount)
{
	PendingDamageAmount = DamageAmount;
	ApplyDamageTextToWidget();
}

void AFRFloatingDamageTextActor::ApplyDamageTextToWidget()
{
	if (!DamageWidgetComponent)
	{
		return;
	}

	DamageWidgetComponent->InitWidget();
	if (UFRFloatingDamageTextWidget* DamageWidget = Cast<UFRFloatingDamageTextWidget>(DamageWidgetComponent->GetUserWidgetObject()))
	{
		DamageWidget->SetDamageAmount(PendingDamageAmount);
	}
}
