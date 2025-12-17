// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Components/HealthComponent.h"
#include "GWCharacter.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	OwnerRef = Cast<AGWCharacter>(GetOwner());
	if (!OwnerRef)
		UE_LOG(LogTemp, Warning, TEXT("OwnerRef is Not Founded!"));
	CurrentHealth = MaxHealth;
}

float UHealthComponent::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	if (!OwnerRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("OwnerRef is Not Founded!"));
		return Damage;
	}
	// only process damage if the character is still alive
	if (CurrentHealth <= 0.0f)
	{
		return 0.0f;
	}

	// reduce the current HP
	CurrentHealth -= Damage;

	// have we run out of HP?
	if (CurrentHealth <= 0.0f)
	{
		// die
		Death();
	}
	else
	{
		// TODO: update the life bar

		// enable partial ragdoll physics, but keep the pelvis vertical
		OwnerRef->GetMesh()->SetPhysicsBlendWeight(0.5f);
		OwnerRef->GetMesh()->SetBodySimulatePhysics(PelvisBoneName, false);
	}

	// return the received damage amount
	return Damage;
}


// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UHealthComponent::ApplayDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation,
	const FVector& DamageImpulse)
{
	if (!OwnerRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("OwnerRef is Not Founded!"));
		return;
	}
	// pass the damage event to the actor
	FDamageEvent DamageEvent;
	const float ActualDamage = TakeDamage(Damage, DamageEvent, nullptr, DamageCauser);

	// only process knockback and effects if we received nonzero damage
	if (ActualDamage > 0.0f)
	{
		// apply the knockback impulse
		OwnerRef->GetCharacterMovement()->AddImpulse(DamageImpulse, true);

		// is the character ragdolling?
		if (OwnerRef->GetMesh()->IsSimulatingPhysics())
		{
			// apply an impulse to the ragdoll
			OwnerRef->GetMesh()->AddImpulseAtLocation(DamageImpulse * OwnerRef->GetMesh()->GetMass(), DamageLocation);
		}

		// pass control to BP to play effects, etc.
		// TODO: ReceivedDamage(ActualDamage, DamageLocation, DamageImpulse.GetSafeNormal());
	}
}

void UHealthComponent::Death()
{
	if (!OwnerRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("OwnerRef is Not Founded!"));
		return;
	}
	// disable movement while we're dead
	OwnerRef->GetCharacterMovement()->DisableMovement();

	// enable full ragdoll physics
	OwnerRef->GetMesh()->SetSimulatePhysics(true);
}

void UHealthComponent::ApplyHealing(float HealAmount, AActor* Healer)
{
	if (!OwnerRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("OwnerRef is Not Founded!"));
		return;
	}
}

