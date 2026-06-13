// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FortRogueProjectile.generated.h"

class AFortRogueBattleCharacter;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class FORTROGUE_API AFortRogueProjectile : public AActor
{
	GENERATED_BODY()

public:
	AFortRogueProjectile();

	virtual void Tick(float DeltaSeconds) override;

	void InitializeProjectile(AFortRogueBattleCharacter* InOwnerCharacter, const FVector& InVelocity, float InDamage, float InBlastRadius, float InGravity);

private:
	void ResolveImpact(const FVector& ImpactLocation);

	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<UStaticMeshComponent> Visual;

	UPROPERTY()
	TObjectPtr<AFortRogueBattleCharacter> OwnerCharacter;

	FVector Velocity = FVector::ZeroVector;
	float Damage = 35.0f;
	float BlastRadius = 150.0f;
	float Gravity = 980.0f;
	float LifeSeconds = 0.0f;
	bool bResolved = false;
};
