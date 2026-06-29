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

void AFRProjectile::InitializeProjectile(AFRBattleCharacter* InOwnerCharacter, AFRDestructibleTerrain* InTerrain, const FVector& InVelocity, float InHitDamage, float InExplosionDamage, float InBlastRadius, float InExplosionFullDamageRadius, float InGravity, float InTerrainDamage, float InTerrainFillRadius, FGameplayTag InWeaponTag, FGameplayTagContainer InEffectTags, TArray<FFRProjectileEffectSpec> InProjectileEffects)
{
	OwnerCharacter = InOwnerCharacter;
	AssignedTerrain = InTerrain;
	Velocity = InVelocity;
	WeaponTag = InWeaponTag;
	EffectTags = InEffectTags;
	ProjectileEffects = MoveTemp(InProjectileEffects);
	HitDamage = FMath::Max(0.0f, InHitDamage);
	Damage = FMath::Max(0.0f, InExplosionDamage);
	BlastRadius = FMath::Max(0.0f, InBlastRadius);
	ExplosionFullDamageRadius = FMath::Clamp(InExplosionFullDamageRadius, 0.0f, BlastRadius);
	TerrainDamage = FMath::Max(0.0f, InTerrainDamage);
	TerrainFillRadius = FMath::Max(0.0f, InTerrainFillRadius);
	Gravity = FMath::Max(0.0f, InGravity);
}

void AFRProjectile::InitializeProjectileFromShotSpec(AFRBattleCharacter* InOwnerCharacter, AFRDestructibleTerrain* InTerrain, const FVector& InVelocity, const FFRShotSpec& ShotSpec)
{
	InitializeProjectile(
		InOwnerCharacter,
		InTerrain,
		InVelocity,
		ShotSpec.HitDamage,
		ShotSpec.Damage,
		ShotSpec.BlastRadius,
		ShotSpec.ExplosionFullDamageRadius,
		ShotSpec.Gravity,
		ShotSpec.TerrainDamage,
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
	AFRBattleCharacter* BestDirectHitCharacter = nullptr;

	auto ConsiderImpact = [&](const FVector& CandidateImpactLocation, AFRBattleCharacter* DirectHitCharacter)
	{
		const float CandidateAlpha = GetSegmentAlphaXZ(OldLocation, NewLocation, CandidateImpactLocation);
		if (!bHasImpact || CandidateAlpha < BestImpactAlpha)
		{
			bHasImpact = true;
			BestImpactAlpha = CandidateAlpha;
			BestImpactLocation = CandidateImpactLocation;
			BestDirectHitCharacter = DirectHitCharacter;
		}
	};

	if (AssignedTerrain)
	{
		FVector ImpactLocation = FVector::ZeroVector;
		if (AssignedTerrain->FindFirstSolidAlongWorldSegment(OldLocation, NewLocation, ImpactLocation))
		{
			ConsiderImpact(ImpactLocation, nullptr);
		}
	}
	else
	{
		for (TActorIterator<AFRDestructibleTerrain> It(GetWorld()); It; ++It)
		{
			FVector ImpactLocation = FVector::ZeroVector;
			if (It->FindFirstSolidAlongWorldSegment(OldLocation, NewLocation, ImpactLocation))
			{
				ConsiderImpact(ImpactLocation, nullptr);
			}
		}
	}

	for (TActorIterator<AFRBattleCharacter> It(GetWorld()); It; ++It)
	{
		AFRBattleCharacter* Character = *It;
		if (!CanAffectCharacter(Character))
		{
			continue;
		}

		FVector CharacterImpactLocation = FVector::ZeroVector;
		if (Character->FindHurtboxImpactAlongSegmentXZ(OldLocation, NewLocation, CharacterImpactLocation))
		{
			ConsiderImpact(CharacterImpactLocation, Character);
		}
	}

	if (bHasImpact)
	{
		ResolveImpact(BestImpactLocation, BestDirectHitCharacter);
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

void AFRProjectile::ResolveImpact(const FVector& ImpactLocation, AFRBattleCharacter* DirectHitCharacter)
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
	const FFRProjectileEffectImpactContext EffectContext = BuildProjectileEffectContext(ImpactLocation, DirectHitCharacter);
	ApplyProjectileEffects(EffectContext);

	for (TActorIterator<AFRBattleCharacter> It(GetWorld()); It; ++It)
	{
		AFRBattleCharacter* Character = *It;
		if (!CanAffectCharacter(Character))
		{
			continue;
		}

		const float Distance = Character->GetDistanceToHurtboxXZ(ImpactLocation);
		float TotalDamage = 0.0f;
		if (Character == DirectHitCharacter)
		{
			TotalDamage += HitDamage;
		}

		const float ExplosionDamage = CalculateExplosionDamage(Distance);
		if (ExplosionDamage > 0.0f)
		{
			TotalDamage += ExplosionDamage;
		}

		if (TotalDamage > 0.0f)
		{
			Character->ApplyDamage(TotalDamage);
		}

		Character->ReevaluateTerrainSupport();
	}

	ApplyProjectilePostImpactEffects(EffectContext);

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
		else if (TerrainDamage > 0.0f)
		{
			AssignedTerrain->CarveCircle(ImpactLocation, TerrainDamage);
		}
		return;
	}

	for (TActorIterator<AFRDestructibleTerrain> It(GetWorld()); It; ++It)
	{
		if (TerrainFillRadius > 0.0f)
		{
			It->FillCircle(ImpactLocation, TerrainFillRadius);
		}
		else if (TerrainDamage > 0.0f)
		{
			It->CarveCircle(ImpactLocation, TerrainDamage);
		}
	}
}

float AFRProjectile::CalculateExplosionDamage(float Distance) const
{
	if (Damage <= 0.0f || BlastRadius <= 0.0f || Distance > BlastRadius)
	{
		return 0.0f;
	}

	if (Distance <= ExplosionFullDamageRadius || ExplosionFullDamageRadius >= BlastRadius)
	{
		return Damage;
	}

	const float FalloffRange = FMath::Max(KINDA_SMALL_NUMBER, BlastRadius - ExplosionFullDamageRadius);
	const float FalloffAlpha = FMath::Clamp((Distance - ExplosionFullDamageRadius) / FalloffRange, 0.0f, 1.0f);
	return FMath::Lerp(Damage, 0.0f, FalloffAlpha);
}

bool AFRProjectile::CanAffectCharacter(const AFRBattleCharacter* Character) const
{
	if (!Character || Character->IsDefeated())
	{
		return false;
	}

	if (!OwnerCharacter)
	{
		return true;
	}

	return Character != OwnerCharacter && Character->IsEnemy() != OwnerCharacter->IsEnemy();
}

FFRProjectileEffectImpactContext AFRProjectile::BuildProjectileEffectContext(const FVector& ImpactLocation, AFRBattleCharacter* DirectHitCharacter) const
{
	FFRProjectileEffectImpactContext Context;
	Context.World = GetWorld();
	Context.Projectile = const_cast<AFRProjectile*>(this);
	Context.OwnerCharacter = OwnerCharacter;
	Context.DirectHitCharacter = DirectHitCharacter;
	Context.AssignedTerrain = AssignedTerrain;
	Context.ImpactLocation = ImpactLocation;
	Context.Velocity = Velocity;
	Context.WeaponTag = WeaponTag;
	Context.EffectTags = EffectTags;
	Context.HitDamage = HitDamage;
	Context.Damage = Damage;
	Context.BlastRadius = BlastRadius;
	Context.ExplosionFullDamageRadius = ExplosionFullDamageRadius;
	Context.TerrainDamage = TerrainDamage;
	Context.TerrainFillRadius = TerrainFillRadius;
	Context.Gravity = Gravity;
	Context.RuntimeEffects = &ProjectileEffects;
	return Context;
}

void AFRProjectile::ApplyProjectileEffects(const FFRProjectileEffectImpactContext& Context)
{
	for (const FFRProjectileEffectSpec& ProjectileEffect : ProjectileEffects)
	{
		ProjectileEffect.HandleImpact(Context);
	}
}

void AFRProjectile::ApplyProjectilePostImpactEffects(const FFRProjectileEffectImpactContext& Context)
{
	for (const FFRProjectileEffectSpec& ProjectileEffect : ProjectileEffects)
	{
		ProjectileEffect.HandlePostImpact(Context);
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
