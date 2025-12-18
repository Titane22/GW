// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Characters/Enemy/Enemy_Base.h"
#include "Gameplay/Components/HealthComponent.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"

AEnemy_Base::AEnemy_Base()
{
	HealthComponent = CreateDefaultSubobject<UHealthComponent>("HealthComponent");

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

void AEnemy_Base::UpdateHealthBar()
{
	if (!HealthComponent)
		return;

	// Calculate health percentage
	float HealthPercent = HealthComponent->GetCurrentHealth() / HealthComponent->GetMaxHealth();
	UE_LOG(LogTemp, Warning, TEXT("HealthPercent = %f"), HealthPercent);
	// Call Blueprint implementable event
	OnHealthChanged(HealthPercent);
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
	if (HealthComponent)
	{
		HealthComponent->ApplayDamage(
			Damage,
			DamageCauser,
			DamageLocation,
			DamageImpulse
		);

		// Update health bar after taking damage
		UpdateHealthBar();
	}
}

void AEnemy_Base::HandleDeath()
{
	if (HealthComponent)
	{
		HealthComponent->Death();
	}

	// Hide health bar on death
	if (HealthBarComponent)
	{
		HealthBarComponent->SetVisibility(false);
	}
}

void AEnemy_Base::ApplyHealing(float Healing, AActor* Healer)
{
	if (HealthComponent)
	{
		HealthComponent->ApplyHealing(Healing, Healer);

		// Update health bar after healing
		UpdateHealthBar();
	}
}
