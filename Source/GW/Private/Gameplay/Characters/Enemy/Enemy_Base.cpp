// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Characters/Enemy/Enemy_Base.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "Gameplay/Components/HealthComponent.h"
#include "Blueprint/UserWidget.h"
#include "Gameplay/Objects/HealOrb.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

AEnemy_Base::AEnemy_Base()
{
	// Create the health bar widget component
	HealthBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarComponent"));
	HealthBarComponent->SetupAttachment(RootComponent);
	HealthBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarComponent->SetDrawAtDesiredSize(true); // 고정 크기 유지
	HealthBarComponent->SetDrawSize(FVector2D(200.0f, 20.0f));
	HealthBarComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f)); // Above character
	HealthBarComponent->SetPivot(FVector2D(0.5f, 0.5f)); // 중앙 기준
}

void AEnemy_Base::BeginPlay()
{
	Super::BeginPlay();

	// Set the widget class if specified
	if (HealthBarComponent && HealthWidgetClass)
	{
		HealthBarComponent->SetWidgetClass(HealthWidgetClass);
		HealthBarComponent->InitWidget();

		HealthWidget = HealthBarComponent->GetUserWidgetObject();
		if (HealthWidget)
		{
			// Initialize health bar to full
			UpdateHealthBar();
		}
	}
}

void AEnemy_Base::DoAttackTrace(FName DamageSourceBone)
{
}

void AEnemy_Base::CheckCombo()
{
}

void AEnemy_Base::CheckChargedAttack()
{
}

void AEnemy_Base::ApplyDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation,
                              const FVector& DamageImpulse)
{
	if (GetHealthComponent())
	{
		GetHealthComponent()->ApplayDamage(
			Damage,
			DamageCauser,
			DamageLocation,
			DamageImpulse
		);

		// Update health bar after taking damage (부모 클래스의 함수 호출)
		UpdateHealthBar();
	}
}

void AEnemy_Base::HandleDeath()
{
	UE_LOG(LogTemp, Warning, TEXT("Enemy HandleDeath called!"));

	// Hide health bar on death
	if (HealthBarComponent)
	{
		HealthBarComponent->SetVisibility(false);
	}

	// Disable movement
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DisableMovement();
	}

	// Disable capsule collision to prevent falling through floor
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Enable ragdoll physics
	if (GetMesh())
	{
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	}

	// Spawn heal orbs
	if (HealOrbClass)
	{
		FVector SpawnLocation = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);

		for (int32 i = 0; i < HealOrbCount; i++)
		{
			// Random offset for spreading orbs
			FVector RandomOffset = FVector(
				FMath::RandRange(-100.0f, 100.0f),
				FMath::RandRange(-100.0f, 100.0f),
				FMath::RandRange(50.0f, 150.0f)
			);

			FVector FinalLocation = SpawnLocation + RandomOffset;

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			GetWorld()->SpawnActor<AHealOrb>(HealOrbClass, FinalLocation, FRotator::ZeroRotator, SpawnParams);
		}

		UE_LOG(LogTemp, Log, TEXT("Enemy spawned %d heal orbs"), HealOrbCount);
	}

	// Set timer to destroy actor after delay
	GetWorldTimerManager().SetTimer(DeathTimerHandle, this, &AEnemy_Base::RemoveFromWorld, DeathRemovalTime, false);
}

void AEnemy_Base::RemoveFromWorld()
{
	UE_LOG(LogTemp, Log, TEXT("Removing enemy from world"));
	Destroy();
}

void AEnemy_Base::ApplyHealing(float Healing, AActor* Healer)
{
	if (GetHealthComponent())
	{
		GetHealthComponent()->ApplyHealing(Healing, Healer);

		// Update health bar after healing (부모 클래스의 함수 호출)
		UpdateHealthBar();
	}
}
