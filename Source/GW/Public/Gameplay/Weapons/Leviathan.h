// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Leviathan.generated.h"

class AEnemy_Base;
class UProjectileMovementComponent;
class UTimelineComponent;
class APlayer_Base;

UENUM(BlueprintType)
enum class EAxeState : uint8
{
	Idle					UMETA(DisplayName = "Idle"),
	Launched				UMETA(DisplayName = "Launched"),
	LodgedInSomething		UMETA(DisplayName = "LodgedInSomething"),
	Returning				UMETA(DisplayName = "Returning")
};

UCLASS()
class GW_API ALeviathan : public AActor
{
	GENERATED_BODY()

private:
	UProjectileMovementComponent* ProjectileMovement;
	
public:	
	// Sets default values for this actor's properties
	ALeviathan();

protected:
	virtual void BeginPlay();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	USkeletalMeshComponent* SkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	USceneComponent* LodgePoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	USceneComponent* PivotPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	USceneComponent* DefaultSceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle")
	UParticleSystemComponent* AxeCatchParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle")
	UParticleSystemComponent* SwingParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle")
	UParticleSystemComponent* ThrowParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axe")
	FRotator CameraStartRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axe")
	FVector ThrowDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axe")
	FVector CameraLocationAtThrow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axe")
	FVector ImpactLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axe")
	FVector ImpactNormal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axe")
	float AxeThrowSpeed = 2500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axe")
	float AxeSpinAxisOffset = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axe|Recall")
	FVector InitialLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axe|Recall")
	FRotator InitialRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axe|Recall")
	float OptimalDistance = 1400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axe|Recall")
	float AxeReturnSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axe|Recall")
	float ReturnSpinRate = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axe|Recall")
	int32 NumOfSpins;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axe|Recall")
	FVector ReturnTargetLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axe|Recall")
	FVector AxeLocationLastTick;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axe|Recall")
	float AxeReturnRightScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axe|Recall")
	float AxeReturnTilt = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	FVector DefaultPivotPoint = FVector(4.332825f, 0.0f, 19.522278f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	FVector DefaultLodgePoint = FVector(12.813232f, 0.0f, 36.75943f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* ImpactSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* Whoosh1Sound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* Whoosh2Sound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* DullThudSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* BrownNoiseSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* ReturnNoBrownNoiseSound;
	
	UPROPERTY()
	UAudioComponent* ReturnWhoosh;
	
	FName HitBoneName;

	float ZAdjustment;

	EPhysicalSurface HitSurface;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axe|Damage")
	float ThrowingDamage = 10.5f;

	float AxeSpinRate = 2.5f;

	float AxeThrowTraceDistance= 60.f;

	float ImpulseStrength = 20000.f;

	float MaxCalculationDistance = 3000.f;

	float DistanceFromChar;

	FRotator LodgePointBaseRotation;

	EAxeState AxeState = EAxeState::Idle;

	// Change Enemy Actor
	AEnemy_Base* HitEnemyRef;

	APlayer_Base* PlayerRef;

protected:
	void StartAxeRotForward();

	void StopAxeRotation();

	void StopAxeMoving();

	void LodgeAxe();

	void AdjustAxeReturnLocation();

	void ReturnAxe();

protected:
	/********************
	 * Timeline
	 ********************/
	UPROPERTY()
	UTimelineComponent* AxeRotTimeline;

	UPROPERTY()
	UTimelineComponent* AxeThrowTraceTimeline;

	UPROPERTY()
	UTimelineComponent* WiggleTimeline;

	UPROPERTY()
	UTimelineComponent* AxeTraceReturnTimeline;
	
	UPROPERTY()
	UTimelineComponent* AxeReturnTimeline;

	UPROPERTY()
	UTimelineComponent* SpinTimeline;

	UPROPERTY(EditAnywhere, Category = "Axe")
	UCurveFloat* AxeRotCurve;

	UPROPERTY(EditAnywhere, Category = "Axe")
	UCurveFloat* AxeSoundCurve;

	UPROPERTY(EditAnywhere, Category = "Axe")
	UCurveFloat* AxeTraceCurve;

	UPROPERTY(EditAnywhere, Category = "Axe")
	UCurveFloat* WiggleCurve;

	UPROPERTY(EditAnywhere, Category = "Axe|Recall")
	UCurveFloat* SpinRotationCurve;

	UPROPERTY(EditAnywhere, Category = "Axe|Recall")
	UCurveFloat* SpinSoundCurve;

	UPROPERTY(EditAnywhere, Category = "Axe|Recall")
	UCurveFloat* AxeRotationCurve;

	UPROPERTY(EditAnywhere, Category = "Axe|Recall")
	UCurveFloat* AxeRightVectorCurve;

	UPROPERTY(EditAnywhere, Category = "Axe|Recall")
	UCurveFloat* AxeRotation2Curve;

	UPROPERTY(EditAnywhere, Category = "Axe|Recall")
	UCurveFloat* AxeReturnSpeedCurve;

	UPROPERTY(EditAnywhere, Category = "Axe|Recall")
	UCurveFloat* AxeReturnSoundCurve;

	UPROPERTY(EditAnywhere, Category = "Axe|Recall")
	UCurveFloat* AxeReturnTraceCurve;
	
protected:
	UFUNCTION()
	void UpdateAxeRotation(float Value);

	UFUNCTION()
	void UpdateAxeThrowTrace(float Value);

	UFUNCTION()
	void StopAxeThrowTrace();

	UFUNCTION()
	void OnAxeThrowFinished();

	UFUNCTION()
	void UpdateAxeWiggle(float Value);

	UFUNCTION()
	void OnAxeWiggleFinished();

	UFUNCTION()
	void UpdateAxeReturn(float Value);

	UFUNCTION()
	void OnAxeReturnFinished();

	UFUNCTION()
	void UpdateAxeReturnSpin(float Value);

	UFUNCTION()
	void OnSpinFinished();

	UFUNCTION()
	void UpdateAxeTraceReturn(float Value);

	UFUNCTION()
	void OnWhoosh1();

	UFUNCTION()
	void OnWhoosh2();

public:
	void Throw(FRotator CameraRotation, FVector ThrowDirectionVector, FVector CameraLocation);

	void Recall();

	void SnapAxeToStartPosition(FRotator StartRotation, FVector ThrowDirectionVector, FVector CameraLocation);

	void SetAxeState(int32 Index);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Axe")
	EAxeState GetAxeState() const { return AxeState; }
};
