// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/FRProjectile.h"

#include "Combat/FRBattleCharacter.h"
#include "Combat/FRDestructibleTerrain.h"
#include "FRGameMode.h"
#include "Components/SphereComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
constexpr float CharacterHitRadius = 34.0f;

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

AFRProjectile::AFRProjectile()
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

void AFRProjectile::InitializeProjectile(AFRBattleCharacter* InOwnerCharacter, AFRDestructibleTerrain* InTerrain, const FVector& InVelocity, float InDamage, float InBlastRadius, float InGravity, float InTerrainCarveRadius, float InTerrainFillRadius, FGameplayTag InWeaponTag, FGameplayTagContainer InEffectTags, TArray<FFRProjectileEffectSpec> InProjectileEffects)
{
	OwnerCharacter = InOwnerCharacter;
	AssignedTerrain = InTerrain;
	Velocity = InVelocity;
	WeaponTag = InWeaponTag;
	EffectTags = InEffectTags;
	ProjectileEffects = MoveTemp(InProjectileEffects);
	Damage = FMath::Max(0.0f, InDamage);
	BlastRadius = FMath::Max(0.0f, InBlastRadius);
	TerrainCarveRadius = InTerrainCarveRadius >= 0.0f ? FMath::Max(0.0f, InTerrainCarveRadius) : BlastRadius;
	TerrainFillRadius = FMath::Max(0.0f, InTerrainFillRadius);
	Gravity = FMath::Max(0.0f, InGravity);
}

void AFRProjectile::InitializeProjectileFromShotSpec(AFRBattleCharacter* InOwnerCharacter, AFRDestructibleTerrain* InTerrain, const FVector& InVelocity, const FFRShotSpec& ShotSpec)
{
	InitializeProjectile(
		InOwnerCharacter,
		InTerrain,
		InVelocity,
		ShotSpec.Damage,
		ShotSpec.BlastRadius,
		ShotSpec.Gravity,
		ShotSpec.TerrainCarveRadius,
		ShotSpec.TerrainFillRadius,
		ShotSpec.WeaponTag,
		ShotSpec.EffectTags,
		ShotSpec.ProjectileEffects);
}

int32 AFRProjectile::GetProjectileEffectCount() const
{
	return ProjectileEffects.Num();
}

bool AFRProjectile::HasProjectileEffectClass(TSubclassOf<UFRProjectileEffectBase> EffectClass) const
{
	if (!EffectClass)
	{
		return false;
	}

	for (const FFRProjectileEffectSpec& ProjectileEffect : ProjectileEffects)
	{
		if (ProjectileEffect.EffectClass == EffectClass)
		{
			return true;
		}
	}
	return false;
}

void AFRProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bResolved)
	{
		return;
	}

	LifeSeconds += DeltaSeconds;

	AFRGameMode* GameMode = GetWorld()->GetAuthGameMode<AFRGameMode>();
	const float Wind = GameMode ? GameMode->GetWind() : 0.0f;
	ApplyHoming(DeltaSeconds);
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
		for (TActorIterator<AFRDestructibleTerrain> It(GetWorld()); It; ++It)
		{
			FVector ImpactLocation = FVector::ZeroVector;
			if (It->FindFirstSolidAlongWorldSegment(OldLocation, NewLocation, ImpactLocation))
			{
				ConsiderImpact(ImpactLocation);
			}
		}
	}

	for (TActorIterator<AFRBattleCharacter> It(GetWorld()); It; ++It)
	{
		AFRBattleCharacter* Character = *It;
		if (!Character || Character == OwnerCharacter || Character->IsDefeated())
		{
			continue;
		}

		const FVector ClosestPoint = GetClosestPointOnSegmentXZ(OldLocation, NewLocation, Character->GetActorLocation());
		if (GetDistanceSquaredXZ(Character->GetActorLocation(), ClosestPoint) <= FMath::Square(CharacterHitRadius))
		{
			ConsiderImpact(ClosestPoint);
		}
	}

	if (bHasImpact)
	{
		ResolveImpact(BestImpactLocation);
		return;
	}

	bool bOutOfBounds = false;
	if (AssignedTerrain)
	{
		bOutOfBounds = AssignedTerrain->IsProjectileOutOfBounds(NewLocation);
	}
	else
	{
		for (TActorIterator<AFRDestructibleTerrain> It(GetWorld()); It; ++It)
		{
			bOutOfBounds = It->IsProjectileOutOfBounds(NewLocation);
			break;
		}
	}

	if (LifeSeconds > MaxLifeSeconds || bOutOfBounds)
	{
		ResolveImpact(NewLocation);
	}
}

AFRBattleCharacter* AFRProjectile::FindHomingTarget() const
{
	if (!GetWorld() || !OwnerCharacter)
	{
		return nullptr;
	}

	AFRBattleCharacter* BestTarget = nullptr;
	float BestDistanceSq = FMath::Square(HomingRange);
	for (TActorIterator<AFRBattleCharacter> It(GetWorld()); It; ++It)
	{
		AFRBattleCharacter* Candidate = *It;
		if (!Candidate || Candidate == OwnerCharacter || Candidate->IsDefeated() || Candidate->IsEnemy() == OwnerCharacter->IsEnemy())
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared2D(Candidate->GetActorLocation(), GetActorLocation())
			+ FMath::Square(Candidate->GetActorLocation().Z - GetActorLocation().Z);
		if (DistanceSq < BestDistanceSq)
		{
			BestDistanceSq = DistanceSq;
			BestTarget = Candidate;
		}
	}
	return BestTarget;
}

void AFRProjectile::ApplyHoming(float DeltaSeconds)
{
	if (!bHoming || Velocity.IsNearlyZero())
	{
		return;
	}

	AFRBattleCharacter* Target = FindHomingTarget();
	if (!Target)
	{
		return;
	}

	const float Speed = Velocity.Size();
	const FVector DesiredDirection = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	if (DesiredDirection.IsNearlyZero())
	{
		return;
	}

	const FVector CurrentDirection = Velocity.GetSafeNormal();
	const float Alpha = FMath::Clamp(HomingTurnRate * DeltaSeconds, 0.0f, 1.0f);
	Velocity = FMath::Lerp(CurrentDirection, DesiredDirection, Alpha).GetSafeNormal() * Speed;
}

void AFRProjectile::ResolveImpact(const FVector& ImpactLocation)
{
	if (bResolved)
	{
		return;
	}

	bResolved = true;
	SetActorLocation(ImpactLocation);

	if (!UsesCustomTerrainImpact())
	{
		ApplyDefaultTerrainImpact(ImpactLocation);
	}
	ApplyProjectileEffects(ImpactLocation);

	for (TActorIterator<AFRBattleCharacter> It(GetWorld()); It; ++It)
	{
		AFRBattleCharacter* Character = *It;
		if (!Character || Character->IsDefeated())
		{
			continue;
		}

		const float Distance = GetDistanceXZ(Character->GetActorLocation(), ImpactLocation);
		if (BlastRadius <= KINDA_SMALL_NUMBER)
		{
			if (Distance <= CharacterHitRadius)
			{
				Character->ApplyDamage(Damage);
			}
		}
		else if (Distance <= BlastRadius)
		{
			const float Falloff = 1.0f - FMath::Clamp(Distance / BlastRadius, 0.0f, 1.0f);
			Character->ApplyDamage(Damage * FMath::Max(0.25f, Falloff));
		}

		Character->ReevaluateTerrainSupport();
	}

	if (AFRGameMode* GameMode = GetWorld()->GetAuthGameMode<AFRGameMode>())
	{
		GameMode->NotifyProjectileResolved(this);
	}

	Destroy();
}

void AFRProjectile::ApplyDefaultTerrainImpact(const FVector& ImpactLocation)
{
	if (AssignedTerrain)
	{
		if (TerrainFillRadius > 0.0f)
		{
			AssignedTerrain->FillCircle(ImpactLocation, TerrainFillRadius);
		}
		else
		{
			AssignedTerrain->CarveCircle(ImpactLocation, TerrainCarveRadius);
		}
		return;
	}

	for (TActorIterator<AFRDestructibleTerrain> It(GetWorld()); It; ++It)
	{
		if (TerrainFillRadius > 0.0f)
		{
			It->FillCircle(ImpactLocation, TerrainFillRadius);
		}
		else
		{
			It->CarveCircle(ImpactLocation, TerrainCarveRadius);
		}
	}
}

void AFRProjectile::ApplyProjectileEffects(const FVector& ImpactLocation)
{
	if (ProjectileEffects.Num() <= 0)
	{
		return;
	}

	FFRProjectileEffectImpactContext Context;
	Context.World = GetWorld();
	Context.Projectile = this;
	Context.OwnerCharacter = OwnerCharacter;
	Context.AssignedTerrain = AssignedTerrain;
	Context.ImpactLocation = ImpactLocation;
	Context.Velocity = Velocity;
	Context.WeaponTag = WeaponTag;
	Context.EffectTags = EffectTags;
	Context.Damage = Damage;
	Context.BlastRadius = BlastRadius;
	Context.TerrainCarveRadius = TerrainCarveRadius;
	Context.TerrainFillRadius = TerrainFillRadius;
	Context.Gravity = Gravity;

	for (const FFRProjectileEffectSpec& ProjectileEffect : ProjectileEffects)
	{
		ProjectileEffect.HandleImpact(Context);
	}
}

bool AFRProjectile::UsesCustomTerrainImpact() const
{
	for (const FFRProjectileEffectSpec& ProjectileEffect : ProjectileEffects)
	{
		if (ProjectileEffect.UsesCustomTerrainImpact())
		{
			return true;
		}
	}
	return false;
}
