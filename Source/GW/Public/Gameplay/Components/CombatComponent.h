// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/Data/SkillData.h"
#include "CombatComponent.generated.h"

class APlayer_Base;
class UPlayerProgressionComponent;
class ALeviathan;
class UAnimInstance;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComboExecuted, FName, SkillID, int32, ComboIndex);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GW_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY()
	APlayer_Base* OwnerChar;
	
	// 컴포넌트 참조
	UPROPERTY()
	UPlayerProgressionComponent* ProgressionComp;

	// ========== 기본 콤보 시스템 ==========

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Combo")
	UAnimMontage* ArmedComboMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Combo")
	UAnimMontage* UnarmedComboMontage;
	
	UPROPERTY(EditAnywhere, Category="Combat|Combo")
	TArray<FName> ComboSectionNames;
	
	bool bIsAttacking;
	
	bool bComboInputPressed;

	int32 ComboCount;

	int32 MaxComboCount;

	// ========== 공격 판정 설정 ==========

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack Trace")
	float AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack Trace")
	float AttackRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack Trace")
	bool bDebugAttackTrace;

	// ========== 차징 시스템 ==========

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Charging")
	bool bIsCharging;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Charging")
	float ChargeTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Charging")
	float MaxChargeTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Charging")
	float ChargeMultiplier;

	// TODO: Skill System

	int32 CurrentComboIndex;
	FTimerHandle ComboResetTimer;
	float CurrentAttackDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float ComboResetTime;

	UPROPERTY(EditAnywhere, Category="Combat|Damage", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm/s"))
	float MeleeKnockbackImpulse = 250.0f;
	
	UPROPERTY(EditAnywhere, Category="Combat|Damage", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm/s"))
	float MeleeLaunchImpulse = 300.0f;
	
public:
	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnComboExecuted OnComboExecuted;

public:
	// ========== 기본 전투 함수 ==========

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PerformAttack();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void CheckComboInput();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat")
	EWeaponState GetCurrentWeaponState() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat")
	float GetCurrentAttackDamage() const { return CurrentAttackDamage; }

	UFUNCTION(BlueprintCallable, Category="Combat")
	void ResetCombo();

	// ========== 공격 로직 (Player_Base에서 호출) ==========

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PerformAttackTrace(FName DamageSourceBone);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void HandleChargedAttack();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartCharging();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StopCharging();
private:
	UFUNCTION()
	void OnComboMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void ExcuteCombo(FName SkillID);
};
