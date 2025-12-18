// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GWCharacter.h"
#include "Variant_Combat/Interfaces/CombatAttacker.h"
#include "Variant_Combat/Interfaces/CombatDamageable.h"
#include "Player_Base.generated.h"

class UHealthComponent;
class ALeviathan;
class UTimelineComponent;
class UCurveFloat;
class UPlayerProgressionComponent;
class UCombatComponent;
/**
 * 
 */
UCLASS()
class GW_API APlayer_Base : public AGWCharacter, public ICombatDamageable, public ICombatAttacker
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	TSubclassOf<UUserWidget> AimHUD_Class;

	UPROPERTY()
	UUserWidget* AimHUD;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float ShoulderYOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float ShoulderZOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float TargetSpringAimZ;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	FVector TargetArmLength;

	UPROPERTY()
	UTimelineComponent* AimTimeline;

	UPROPERTY()
	UTimelineComponent* DesiredSocketTimeline;

	UPROPERTY()
	UTimelineComponent* RangedCameraTl;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Timeline")
	UCurveFloat* AimCurve;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Timeline")
	UCurveFloat* DesireSocketCurve;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Timeline")
	UCurveFloat* RangedCameraCurve;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons", meta = (AllowPrivateAccess = "true"))
	UChildActorComponent* LeviathanAxe;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons", meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* AxeCollision;

	// 스킬 진행도 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UPlayerProgressionComponent* ProgressionComponent;

	// 전투 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCombatComponent* CombatComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bUserControllerRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bRangedAttackMode = false;

public:
	APlayer_Base();
	
	UFUNCTION()
	void UpdateAim(float Value);

	UFUNCTION()
	void UpdateDesireSocket(float Value);

	void Catch();
	
	UFUNCTION()
	ALeviathan* GetLeviathanAxe() const { return LeviathanRef; }

	// ========== ICombatAttacker 인터페이스 구현 ==========

	virtual void DoAttackTrace(FName DamageSourceBone) override;
	virtual void CheckCombo() override;
	virtual void CheckChargedAttack() override;

	// ========== ICombatDamageable 인터페이스 구현 ==========

	virtual void ApplyDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation, const FVector& DamageImpulse) override;
	virtual void HandleDeath() override;
	virtual void ApplyHealing(float Healing, AActor* Healer) override;

protected:
	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void PerformAttack();
	
	UFUNCTION()
	void AimPressed();
	UFUNCTION()
	void AimReleased();

	void ThrowAxe();

	void ReturnAxe();

	UFUNCTION()
	void OnThrowNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);

	UFUNCTION()
	void OnThrowNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);

	UFUNCTION()
	void OnCatchNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);

	UFUNCTION()
	void OnCatchNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);

	virtual void BeginPlay();

	virtual void Tick(float DeltaTime);

	UFUNCTION()
	void LerpCameraPosition(float Value);
	
protected:
	ALeviathan* LeviathanRef = nullptr;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* AimAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* RecallAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float CameraTurnRate = 50.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axe Throw", meta = (AllowPrivateAccess = "true"))
	bool bIsAim;

	bool bAxeThrown;

	bool bAxeRecalling;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* ThrowEffortSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axe Throw", meta = (AllowPrivateAccess = "true"))
	float SpringArmLengthIdle = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axe Throw", meta = (AllowPrivateAccess = "true"))
	float SpringArmLengthAim = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axe Throw", meta = (AllowPrivateAccess = "true"))
	FVector CameraVector = FVector(20.0f, 50.0f, 60.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axe Throw", meta = (AllowPrivateAccess = "true"))
	FVector RangedCameraVector = FVector(30.0f, 40.0f, 65.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	FVector DesiredSocketOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UCameraShakeBase> ShakeClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axe Throw", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ThrowMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axe Throw", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* RecallMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axe Throw", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* CatchMontage;
};
