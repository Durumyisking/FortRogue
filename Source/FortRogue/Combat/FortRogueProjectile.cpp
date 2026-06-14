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

namespace
{
float GetSegmentAlphaXZ(const FVector& StartLocation, const FVector& EndLocation, const FVector& TestLocation)
{
	const FVector2D Segment(EndLocation.X - StartLocation.X, EndLocation.Z - StartLocation.Z);
	const FVector2D ToTest(TestLocation.X - StartLocation.X, TestLocation.Z - StartLocation.Z);
	const float SegmentLengthSq = Segment.SizeSquared();
	if (SegmentLengthSq <= KINDA_SMALL_NUMBER)
	{
		return 0.0f;
	}

	return FMath::Clamp(FVector2D::DotProduct(ToTest, Segment) / SegmentLengthSq, 0.0f, 1.0f);
}

FVector GetClosestPointOnSegmentXZ(const FVector& StartLocation, const FVector& EndLocation, const FVector& TestLocation)
{
	return FMath::Lerp(StartLocation, EndLocation, GetSegmentAlphaXZ(StartLocation, EndLocation, TestLocation));
}

float GetDistanceXZ(const FVector& First, const FVector& Second)
{
	return FVector2D(First.X - Second.X, First.Z - Second.Z).Size();
}

float GetDistanceSquaredXZ(const FVector& First, const FVector& Second)
{
	return FVector2D(First.X - Second.X, First.Z - Second.Z).SizeSquared();
}
}

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

	bool bHasImpact = false;
	float BestImpactAlpha = TNumericLimits<float>::Max();
	FVector BestImpactLocation = NewLocation;

	auto ConsiderImpact = [&](const FVector& CandidateImpactLocation)
	{
		const float CandidateAlpha = GetSegmentAlphaXZ(OldLocation, NewLocation, CandidateImpactLocation);
		if (!bHasImpact || CandidateAlpha < BestImpactAlpha)
		{
			bHasImpact = true;
			BestImpactAlpha = CandidateAlpha;
			BestImpactLocation = CandidateImpactLocation;
		}
	};

	if (AssignedTerrain)
	{
		FVector ImpactLocation = FVector::ZeroVector;
		if (AssignedTerrain->FindFirstSolidAlongWorldSegment(OldLocation, NewLocation, ImpactLocation))
		{
			ConsiderImpact(ImpactLocation);
		}
	}
	else
	{
		for (TActorIterator<AFortRogueDestructibleTerrain> It(GetWorld()); It; ++It)
		{
			FVector ImpactLocation = FVector::ZeroVector;
			if (It->FindFirstSolidAlongWorldSegment(OldLocation, NewLocation, ImpactLocation))
			{
				ConsiderImpact(ImpactLocation);
			}
		}
	}

	for (TActorIterator<AFortRogueBattleCharacter> It(GetWorld()); It; ++It)
	{
		AFortRogueBattleCharacter* Character = *It;
		if (!Character || Character == OwnerCharacter || Character->IsDefeated())
		{
			continue;
		}

		const FVector ClosestPoint = GetClosestPointOnSegmentXZ(OldLocation, NewLocation, Character->GetActorLocation());
		if (GetDistanceSquaredXZ(Character->GetActorLocation(), ClosestPoint) <= FMath::Square(34.0f))
		{
			ConsiderImpact(ClosestPoint);
		}
	}

	if (bHasImpact)
	{
		ResolveImpact(BestImpactLocation);
		return;
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

		const float Distance = GetDistanceXZ(Character->GetActorLocation(), ImpactLocation);
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
