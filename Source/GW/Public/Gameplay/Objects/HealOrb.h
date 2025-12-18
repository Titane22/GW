// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HealOrb.generated.h"

class USphereComponent;
class UNiagaraComponent;
class URotatingMovementComponent;

UCLASS()
class GW_API AHealOrb : public AActor
{
	GENERATED_BODY()

public:
	AHealOrb();

protected:
	/** Static mesh for physics simulation */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* OrbMesh;

	/** Niagara particle effect */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNiagaraComponent* OrbEffect;

	/** Amount of health to restore */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heal")
	float HealAmount = 20.0f;

	/** Detection radius for pulling towards player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float DetectionRadius = 500.0f;

	/** Speed at which orb moves towards player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MoveSpeed = 800.0f;

	/** Acceleration when moving towards player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float Acceleration = 1500.0f;

	/** Lifetime before auto-destroy if not collected */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lifetime")
	float Lifetime = 10.0f;

	/** Gravity force applied to orb */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float Gravity = 980.0f;

	/** Bounce damping factor (0-1, lower = more bounce loss) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float BounceDamping = 0.5f;

	/** Minimum velocity to keep bouncing (stops when below this) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float MinBounceVelocity = 50.0f;

	/** Initial launch impulse range (min, max) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	FVector2D LaunchImpulseRange = FVector2D(200.0f, 500.0f);

	/** Is orb being pulled towards player? */
	bool bIsBeingPulled;

	/** Has orb settled on ground? */
	bool bHasSettled;

	/** Target player reference */
	UPROPERTY()
	AActor* TargetPlayer;

	/** Lifetime timer */
	FTimerHandle LifetimeTimer;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	/** Called when overlapping with another actor */
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Destroy orb after lifetime expires */
	void DestroyOrb();

	/** Apply healing and destroy */
	void CollectOrb(AActor* Player);
};
