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

void AFortRogueProjectile::InitializeProjectile(AFortRogueBattleCharacter* InOwnerCharacter, AFortRogueDestructibleTerrain* InTerrain, const FVector& InVelocity, float InDamage, float InBlastRadius, float InGravity, float InTerrainCarveRadius, float InTerrainFillRadius, FGameplayTag InWeaponTag, FGameplayTagContainer InEffectTags, TArray<FFortRogueImpactSpawnSpec> InImpactSpawns, TArray<FFRProjectileEffectSpec> InProjectileEffects)
{
	OwnerCharacter = InOwnerCharacter;
	AssignedTerrain = InTerrain;
	Velocity = InVelocity;
	WeaponTag = InWeaponTag;
	EffectTags = InEffectTags;
	ImpactSpawns = MoveTemp(InImpactSpawns);
	ProjectileEffects = MoveTemp(InProjectileEffects);
	Damage = FMath::Max(0.0f, InDamage);
	BlastRadius = FMath::Max(0.0f, InBlastRadius);
	TerrainCarveRadius = InTerrainCarveRadius >= 0.0f ? FMath::Max(0.0f, InTerrainCarveRadius) : BlastRadius;
	TerrainFillRadius = FMath::Max(0.0f, InTerrainFillRadius);
	Gravity = FMath::Max(0.0f, InGravity);
}

void AFortRogueProjectile::InitializeProjectileFromShotSpec(AFortRogueBattleCharacter* InOwnerCharacter, AFortRogueDestructibleTerrain* InTerrain, const FVector& InVelocity, const FFortRogueShotSpec& ShotSpec)
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
		ShotSpec.ImpactSpawns,
		ShotSpec.ProjectileEffects);
}

int32 AFortRogueProjectile::GetProjectileEffectCount() const
{
	return ProjectileEffects.Num();
}

bool AFortRogueProjectile::HasProjectileEffectClass(TSubclassOf<UFRProjectileEffectBase> EffectClass) const
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
		for (TActorIterator<AFortRogueDestructibleTerrain> It(GetWorld()); It; ++It)
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

void AFortRogueProjectile::ResolveImpact(const FVector& ImpactLocation)
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

	for (TActorIterator<AFortRogueBattleCharacter> It(GetWorld()); It; ++It)
	{
		AFortRogueBattleCharacter* Character = *It;
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

	SpawnImpactProjectiles(ImpactLocation);

	if (AFortRogueGameMode* GameMode = GetWorld()->GetAuthGameMode<AFortRogueGameMode>())
	{
		GameMode->NotifyProjectileResolved(this);
	}

	Destroy();
}

void AFortRogueProjectile::ApplyDefaultTerrainImpact(const FVector& ImpactLocation)
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

	for (TActorIterator<AFortRogueDestructibleTerrain> It(GetWorld()); It; ++It)
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

void AFortRogueProjectile::ApplyProjectileEffects(const FVector& ImpactLocation)
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

void AFortRogueProjectile::SpawnImpactProjectiles(const FVector& ImpactLocation)
{
	if (!GetWorld() || ImpactSpawns.Num() <= 0)
	{
		return;
	}

	const FVector FallbackDirection = OwnerCharacter && OwnerCharacter->IsEnemy()
		? FVector(-1.0f, 0.0f, 1.0f).GetSafeNormal()
		: FVector(1.0f, 0.0f, 1.0f).GetSafeNormal();
	const FVector BaseDirection = Velocity.GetSafeNormal(SMALL_NUMBER, FallbackDirection);
	const float BaseAngleDegrees = FMath::RadiansToDegrees(FMath::Atan2(BaseDirection.Z, BaseDirection.X));
	for (const FFortRogueImpactSpawnSpec& ImpactSpawn : ImpactSpawns)
	{
		const int32 ChildCount = FMath::Max(0, ImpactSpawn.ProjectileCount);
		for (int32 Index = 0; Index < ChildCount; ++Index)
		{
			const float SpreadAlpha = ChildCount > 1 ? static_cast<float>(Index) / static_cast<float>(ChildCount - 1) - 0.5f : 0.0f;
			const float ChildAngleDegrees = BaseAngleDegrees + SpreadAlpha * ImpactSpawn.SpreadDegrees;
			const float ChildAngleRadians = FMath::DegreesToRadians(ChildAngleDegrees);
			const FVector ChildDirection = FVector(FMath::Cos(ChildAngleRadians), 0.0f, FMath::Sin(ChildAngleRadians)).GetSafeNormal();
			const TSubclassOf<AFortRogueProjectile> ChildProjectileClass = ImpactSpawn.ProjectileClass ? ImpactSpawn.ProjectileClass : TSubclassOf<AFortRogueProjectile>(GetClass());

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = OwnerCharacter ? Cast<AActor>(OwnerCharacter) : GetOwner();
			SpawnParams.Instigator = OwnerCharacter;
			AFortRogueProjectile* ChildProjectile = GetWorld()->SpawnActor<AFortRogueProjectile>(ChildProjectileClass, ImpactLocation + ChildDirection * 18.0f, FRotator::ZeroRotator, SpawnParams);
			if (!ChildProjectile)
			{
				continue;
			}

			FGameplayTagContainer ChildEffectTags = EffectTags;
			ChildEffectTags.AppendTags(ImpactSpawn.ChildEffectTags);
			ChildProjectile->InitializeProjectile(
				OwnerCharacter,
				AssignedTerrain,
				ChildDirection * ImpactSpawn.LaunchSpeed,
				Damage * ImpactSpawn.DamageMultiplier,
				BlastRadius * ImpactSpawn.BlastRadiusMultiplier,
				Gravity * ImpactSpawn.GravityMultiplier,
				TerrainCarveRadius * ImpactSpawn.TerrainCarveRadiusMultiplier,
				ImpactSpawn.TerrainFillRadius,
				WeaponTag,
				ChildEffectTags);

			if (AFortRogueGameMode* GameMode = GetWorld()->GetAuthGameMode<AFortRogueGameMode>())
			{
				GameMode->NotifyProjectileSpawned(ChildProjectile);
			}
		}
	}
}

bool AFortRogueProjectile::UsesCustomTerrainImpact() const
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
