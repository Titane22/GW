// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatAttacker.h"
#include "CombatDamageable.h"
#include "GWCharacter.h"
#include "Enemy_Base.generated.h"

class UWidgetComponent;
class UUserWidget;
class AHealOrb;

/**
 * Enemy base class with 3D widget health bar
 */
UCLASS()
class GW_API AEnemy_Base : public AGWCharacter, public ICombatDamageable, public ICombatAttacker
{
	GENERATED_BODY()

public:
	AEnemy_Base();

protected:

	/** Health bar widget component displayed above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* HealthBarComponent;

	/** Widget class to use for the health bar */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> HealthWidgetClass;

	/** Reference to the health bar widget instance */
	UPROPERTY()
	UUserWidget* HealthWidget;

	/** Heal orb class to spawn on death */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	TSubclassOf<AHealOrb> HealOrbClass;

	/** Number of heal orbs to spawn on death */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	int32 HealOrbCount = 3;

	/** Time before destroying actor after death */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death")
	float DeathRemovalTime = 5.0f;

	/** Death timer handle */
	FTimerHandle DeathTimerHandle;

protected:

	virtual void BeginPlay() override;

	/** Destroys the actor after death timer */
	void RemoveFromWorld();

public:

	// ========== ICombatAttacker 인터페이스 구현 ==========

	virtual void DoAttackTrace(FName DamageSourceBone) override;
	virtual void CheckCombo() override;
	virtual void CheckChargedAttack() override;

	// ========== ICombatDamageable 인터페이스 구현 ==========

	virtual void ApplyDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation, const FVector& DamageImpulse) override;
	virtual void HandleDeath() override;
	virtual void ApplyHealing(float Healing, AActor* Healer) override;
};
