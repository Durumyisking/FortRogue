// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/FortRogueProjectile.h"

#include "Combat/FortRogueBattleCharacter.h"
#include "Combat/FortRogueDestructibleTerrain.h"
#include "FortRogueGameMode.h"
#include "Components/SphereComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AFortRogueProjectile::AFortRogueProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(14.0f);
	Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetRootComponent(Collision);

	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
	Visual->SetupAttachment(Collision);
	Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Visual->SetRelativeScale3D(FVector(0.28f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		Visual->SetStaticMesh(SphereMesh.Object);
	}
}

void AFortRogueProjectile::InitializeProjectile(AFortRogueBattleCharacter* InOwnerCharacter, AFortRogueDestructibleTerrain* InTerrain, const FVector& InVelocity, float InDamage, float InBlastRadius, float InGravity)
{
	OwnerCharacter = InOwnerCharacter;
	AssignedTerrain = InTerrain;
	Velocity = InVelocity;
	Damage = InDamage;
	BlastRadius = InBlastRadius;
	Gravity = InGravity;
}

void AFortRogueProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bResolved)
	{
		return;
	}

	LifeSeconds += DeltaSeconds;

	AFortRogueGameMode* GameMode = GetWorld()->GetAuthGameMode<AFortRogueGameMode>();
	const float Wind = GameMode ? GameMode->GetWind() : 0.0f;
	Velocity += FVector(Wind, 0.0f, -Gravity) * DeltaSeconds;

	const FVector OldLocation = GetActorLocation();
	const FVector NewLocation = OldLocation + Velocity * DeltaSeconds;
	SetActorLocation(NewLocation);

	if (AssignedTerrain)
	{
		FVector ImpactLocation = FVector::ZeroVector;
		if (AssignedTerrain->FindFirstSolidAlongWorldSegment(OldLocation, NewLocation, ImpactLocation))
		{
			ResolveImpact(ImpactLocation);
			return;
		}
	}
	else
	{
		for (TActorIterator<AFortRogueDestructibleTerrain> It(GetWorld()); It; ++It)
		{
			FVector ImpactLocation = FVector::ZeroVector;
			if (It->FindFirstSolidAlongWorldSegment(OldLocation, NewLocation, ImpactLocation))
			{
				ResolveImpact(ImpactLocation);
				return;
			}
		}
	}

	for (TActorIterator<AFortRogueBattleCharacter> It(GetWorld()); It; ++It)
	{
		AFortRogueBattleCharacter* Character = *It;
		if (Character && Character != OwnerCharacter && !Character->IsDefeated() && FVector::DistSquared(Character->GetActorLocation(), NewLocation) <= FMath::Square(34.0f))
		{
			ResolveImpact(NewLocation);
			return;
		}
	}

	if (LifeSeconds > 8.0f || FMath::Abs(NewLocation.X) > 4000.0f || NewLocation.Z < -400.0f || NewLocation.Z > 3000.0f)
	{
		ResolveImpact(NewLocation);
	}
}

void AFortRogueProjectile::ResolveImpact(const FVector& ImpactLocation)
{
	if (bResolved)
	{
		return;
	}

	bResolved = true;

	if (AssignedTerrain)
	{
		AssignedTerrain->CarveCircle(ImpactLocation, BlastRadius);
	}
	else
	{
		for (TActorIterator<AFortRogueDestructibleTerrain> It(GetWorld()); It; ++It)
		{
			It->CarveCircle(ImpactLocation, BlastRadius);
		}
	}

	for (TActorIterator<AFortRogueBattleCharacter> It(GetWorld()); It; ++It)
	{
		AFortRogueBattleCharacter* Character = *It;
		if (!Character || Character->IsDefeated())
		{
			continue;
		}

		const float Distance = FVector::Dist(Character->GetActorLocation(), ImpactLocation);
		if (Distance <= BlastRadius)
		{
			const float Falloff = 1.0f - FMath::Clamp(Distance / BlastRadius, 0.0f, 1.0f);
			Character->ApplyDamage(Damage * FMath::Max(0.25f, Falloff));
		}

		Character->ReevaluateTerrainSupport();
	}

	if (AFortRogueGameMode* GameMode = GetWorld()->GetAuthGameMode<AFortRogueGameMode>())
	{
		GameMode->NotifyProjectileResolved(this);
	}

	Destroy();
}
