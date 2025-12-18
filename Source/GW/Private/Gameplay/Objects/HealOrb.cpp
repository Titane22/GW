// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Objects/HealOrb.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "GWCharacter.h"
#include "Gameplay/Components/HealthComponent.h"

AHealOrb::AHealOrb()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create static mesh (sphere for physics)
	OrbMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OrbMesh"));
	RootComponent = OrbMesh;

	// Enable physics simulation
	OrbMesh->SetSimulatePhysics(true);
	OrbMesh->SetEnableGravity(true);
	OrbMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	OrbMesh->SetCollisionObjectType(ECC_WorldDynamic);
	OrbMesh->SetCollisionResponseToAllChannels(ECR_Block);
	OrbMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Physics properties
	OrbMesh->SetMassOverrideInKg(NAME_None, 1.0f, true);
	OrbMesh->SetLinearDamping(0.5f);
	OrbMesh->SetAngularDamping(0.5f);
	OrbMesh->BodyInstance.bOverrideMass = true;

	// Bounce settings
	OrbMesh->SetPhysMaterialOverride(nullptr); // Will set bounce in blueprint
	OrbMesh->BodyInstance.bUseCCD = true; // Continuous collision detection

	// Create Niagara effect
	OrbEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("OrbEffect"));
	OrbEffect->SetupAttachment(OrbMesh);
	OrbEffect->SetRelativeLocation(FVector::ZeroVector);
	OrbEffect->SetAutoActivate(true);

	// Initialize variables
	bIsBeingPulled = false;
	bHasSettled = false;
	TargetPlayer = nullptr;
}

void AHealOrb::BeginPlay()
{
	Super::BeginPlay();

	// Bind overlap event
	OrbMesh->OnComponentBeginOverlap.AddDynamic(this, &AHealOrb::OnOverlapBegin);

	// Set lifetime timer
	GetWorldTimerManager().SetTimer(LifetimeTimer, this, &AHealOrb::DestroyOrb, Lifetime, false);

	// Apply initial random launch impulse
	float LaunchImpulse = FMath::RandRange(LaunchImpulseRange.X, LaunchImpulseRange.Y);
	FVector RandomDirection = FVector(
		FMath::RandRange(-1.0f, 1.0f),
		FMath::RandRange(-1.0f, 1.0f),
		FMath::RandRange(0.7f, 1.0f) // Bias upward
	).GetSafeNormal();

	OrbMesh->AddImpulse(RandomDirection * LaunchImpulse * OrbMesh->GetMass(), NAME_None, true);
	UE_LOG(LogTemp, Log, TEXT("HealOrb spawned with impulse: %s"), *(RandomDirection * LaunchImpulse).ToString());
}

void AHealOrb::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Check if orb has settled (velocity is low)
	if (!bHasSettled && OrbMesh->IsSimulatingPhysics())
	{
		FVector Velocity = OrbMesh->GetPhysicsLinearVelocity();
		if (Velocity.Size() < MinBounceVelocity)
		{
			bHasSettled = true;
			UE_LOG(LogTemp, Log, TEXT("HealOrb settled"));
		}
	}

	// Find nearest player if settled and not already targeting
	if (!TargetPlayer && bHasSettled)
	{
		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		if (PlayerPawn)
		{
			float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
			if (Distance <= DetectionRadius)
			{
				TargetPlayer = PlayerPawn;
				bIsBeingPulled = true;
				// Disable physics when being pulled
				OrbMesh->SetSimulatePhysics(false);
				OrbMesh->SetCollisionResponseToAllChannels(ECR_Overlap);
				UE_LOG(LogTemp, Log, TEXT("HealOrb started pulling to player"));
			}
		}
	}

	// Move towards player if being pulled
	if (bIsBeingPulled && TargetPlayer)
	{
		FVector DirectionToPlayer = (TargetPlayer->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		FVector NewLocation = GetActorLocation() + DirectionToPlayer * MoveSpeed * DeltaTime;
		SetActorLocation(NewLocation);
	}
}

void AHealOrb::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check if overlapping with player
	if (AGWCharacter* Player = Cast<AGWCharacter>(OtherActor))
	{
		CollectOrb(Player);
	}
}

void AHealOrb::CollectOrb(AActor* Player)
{
	if (!Player)
		return;

	// Get health component and apply healing
	if (AGWCharacter* Character = Cast<AGWCharacter>(Player))
	{
		if (UHealthComponent* HealthComp = Character->GetHealthComponent())
		{
			HealthComp->ApplyHealing(HealAmount, this);
			UE_LOG(LogTemp, Log, TEXT("HealOrb: Restored %.1f health"), HealAmount);
		}
	}

	// TODO: Play collection sound/VFX

	// Destroy orb
	Destroy();
}

void AHealOrb::DestroyOrb()
{
	if (bIsBeingPulled && TargetPlayer)
		return;
	// Lifetime expired, destroy without healing
	Destroy();
}

