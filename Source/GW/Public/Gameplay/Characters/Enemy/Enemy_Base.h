// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatAttacker.h"
#include "CombatDamageable.h"
#include "GWCharacter.h"
#include "Enemy_Base.generated.h"

class UHealthComponent;
class UWidgetComponent;
class UUserWidget;

/**
 *
 */
UCLASS()
class GW_API AEnemy_Base : public AGWCharacter, public ICombatDamageable, public ICombatAttacker
{
	GENERATED_BODY()

public:
	AEnemy_Base();

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UHealthComponent* HealthComponent;

	/** Health bar widget component displayed above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* HealthBarComponent;

	/** Widget class to use for the health bar */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> HealthWidgetClass;

	/** Reference to the health bar widget instance */
	UPROPERTY()
	UUserWidget* HealthWidget;

protected:

	virtual void BeginPlay() override;

	/** Updates the health bar widget to reflect current health */
	void UpdateHealthBar();

	/** Blueprint event to update health bar UI - Implement this in Blueprint */
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void OnHealthChanged(float HealthPercent);

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
