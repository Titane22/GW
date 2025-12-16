// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Weapons/Leviathan.h"

#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Gameplay/Characters/Player_Base.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
ALeviathan::ALeviathan()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// 컴포넌트 계층 구조 생성
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	PivotPoint = CreateDefaultSubobject<USceneComponent>(TEXT("PivotPoint"));
	PivotPoint->SetupAttachment(RootComponent);
	PivotPoint->SetRelativeLocation(DefaultPivotPoint);

	LodgePoint = CreateDefaultSubobject<USceneComponent>(TEXT("LodgePoint"));
	LodgePoint->SetupAttachment(PivotPoint);
	LodgePoint->SetRelativeLocation(DefaultLodgePoint);

	// 스켈레탈 메시 설정
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(LodgePoint);
	SkeletalMesh->SetRelativeLocation(FVector(-17.146057f, 0.0f, -56.281708f));
	SkeletalMesh->SetRelativeRotation(FRotator(1080.0f, 0.0f, 0.000044f));
	SkeletalMesh->SetSimulatePhysics(false);
	SkeletalMesh->SetEnableGravity(false);
	SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SkeletalMesh->SetMobility(EComponentMobility::Movable);

	// 파티클 시스템 설정
	AxeCatchParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Axe Catch Particle"));
	AxeCatchParticle->SetupAttachment(SkeletalMesh);

	SwingParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Swing Particle"));
	SwingParticle->SetupAttachment(SkeletalMesh);

	ThrowParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Throw Particle"));
	ThrowParticle->SetupAttachment(SkeletalMesh);

	// 프로젝타일 무브먼트 컴포넌트 설정
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));
	ProjectileMovement->bSimulationEnabled = false;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->Velocity = FVector::ZeroVector;

	// 타임라인 컴포넌트 설정
	AxeRotTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("AxeRotTimeline"));
	AxeThrowTraceTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("AxeThrowTraceTimeline"));
	AxeReturnTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("AxeReturnTimeline"));
	SpinTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("SpinTimeline"));
	AxeTraceReturnTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("AxeTraceReturnTimeline"));
	WiggleTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("WiggleTimeline"));
}

void ALeviathan::BeginPlay()
{
	Super::BeginPlay();

	if (APlayer_Base* Player = Cast<APlayer_Base>(GetWorld()->GetFirstPlayerController()->GetPawn()))
		PlayerRef = Player;

	if (AxeCatchParticle)
		AxeCatchParticle->SetActorParameter(FName("VertSurfaceActor"), this);

	if (AxeRotTimeline && AxeRotCurve && AxeSoundCurve)
	{
		AxeRotTimeline->SetTimelineLength(1.f);
		AxeRotTimeline->SetPlayRate(AxeSpinRate);
		AxeRotTimeline->SetLooping(true);

		FOnTimelineFloat RotCallback;
		RotCallback.BindUFunction(this, "UpdateAxeRotation");
		AxeRotTimeline->AddInterpFloat(AxeRotCurve, RotCallback, FName("Rotation"));
		AxeRotTimeline->AddInterpFloat(AxeSoundCurve, RotCallback, FName("Sound"));

		FOnTimelineEvent SoundCallback;
		SoundCallback.BindUFunction(this, "OnWhoosh1");
		AxeRotTimeline->AddEvent(0.f, SoundCallback);
		AxeRotTimeline->AddEvent(0.66f, SoundCallback);

		FOnTimelineEvent Sound2Callback;
		Sound2Callback.BindUFunction(this, "OnWhoosh2");
		AxeRotTimeline->AddEvent(0.33f, Sound2Callback);
	}

	if (AxeThrowTraceTimeline && AxeTraceCurve)
	{
		AxeThrowTraceTimeline->SetTimelineLength(5.f);
		AxeThrowTraceTimeline->SetLooping(true);

		FOnTimelineFloat TraceCallback;
		TraceCallback.BindUFunction(this, FName("UpdateAxeThrowTrace"));
		AxeThrowTraceTimeline->AddInterpFloat(AxeTraceCurve, TraceCallback);

		FOnTimelineEvent TraceFinishCallback;
		TraceFinishCallback.BindUFunction(this, FName("OnAxeThrowFinished"));
		AxeThrowTraceTimeline->SetTimelineFinishedFunc(TraceFinishCallback);
	}

	if (WiggleTimeline && WiggleCurve)
	{
		WiggleTimeline->SetTimelineLength(0.6f);
		WiggleTimeline->SetPlayRate(3.5f);

		FOnTimelineFloat WiggleCallback;
		WiggleCallback.BindUFunction(this, FName("UpdateAxeWiggle"));
		WiggleTimeline->AddInterpFloat(WiggleCurve, WiggleCallback);

		FOnTimelineEvent WiggleFinishCallback;
		WiggleFinishCallback.BindUFunction(this, FName("OnAxeWiggleFinished"));
		WiggleTimeline->SetTimelineFinishedFunc(WiggleFinishCallback);
	}

	if (AxeReturnTimeline && AxeRotationCurve && AxeRightVectorCurve && AxeReturnSpeedCurve && AxeRotation2Curve && AxeReturnSoundCurve)
	{
		AxeReturnTimeline->SetTimelineLength(1.f);

		FOnTimelineFloat UpdateCallback;
		UpdateCallback.BindUFunction(this, FName("UpdateAxeReturn"));
		AxeReturnTimeline->AddInterpFloat(AxeRotationCurve, UpdateCallback, FName("Rotation1"));
		AxeReturnTimeline->AddInterpFloat(AxeRightVectorCurve, UpdateCallback, FName("Right Vector"));
		AxeReturnTimeline->AddInterpFloat(AxeReturnSpeedCurve, UpdateCallback, FName("Return Speed"));
		AxeReturnTimeline->AddInterpFloat(AxeRotation2Curve, UpdateCallback, FName("Rotation2"));
		AxeReturnTimeline->AddInterpFloat(AxeReturnSoundCurve, UpdateCallback, FName("Sound"));

		FOnTimelineEvent ReturnFinishedCallback;
		ReturnFinishedCallback.BindUFunction(this, FName("OnAxeReturnFinished"));
		AxeReturnTimeline->SetTimelineFinishedFunc(ReturnFinishedCallback);
	}

	if (SpinTimeline && SpinRotationCurve)
	{
		SpinTimeline->SetTimelineLength(1.f);

		FOnTimelineFloat SpinCallback;
		SpinCallback.BindUFunction(this, FName("UpdateAxeReturnSpin"));
		SpinTimeline->AddInterpFloat(SpinRotationCurve, SpinCallback);

		FOnTimelineEvent SpinFinishedCallback;
		SpinFinishedCallback.BindUFunction(this, FName("OnSpinFinished"));
		SpinTimeline->SetTimelineFinishedFunc(SpinFinishedCallback);

		FOnTimelineEvent SoundCallback;
		SoundCallback.BindUFunction(this, "OnWhoosh1");
		SpinTimeline->AddEvent(0.f, SoundCallback);
		SpinTimeline->AddEvent(0.66f, SoundCallback);

		FOnTimelineEvent Sound2Callback;
		Sound2Callback.BindUFunction(this, "OnWhoosh2");
		SpinTimeline->AddEvent(0.33f, Sound2Callback);
	}

	if (AxeTraceReturnTimeline && AxeReturnTraceCurve)
	{
		AxeTraceReturnTimeline->SetTimelineLength(0.01f);
		AxeTraceReturnTimeline->SetLooping(true);

		FOnTimelineFloat TraceCallback;
		TraceCallback.BindUFunction(this, FName("UpdateAxeTraceReturn"));
		AxeTraceReturnTimeline->AddInterpFloat(AxeReturnTraceCurve, TraceCallback);
	}
}

void ALeviathan::StartAxeRotForward()
{
	if (AxeRotTimeline)
	{
		AxeRotTimeline->SetPlayRate(AxeSpinRate);
		AxeRotTimeline->PlayFromStart();
	}
}

void ALeviathan::StopAxeRotation()
{
	if (AxeRotTimeline)
	{
		AxeRotTimeline->Stop();
	}
}

void ALeviathan::StopAxeMoving()
{
	ProjectileMovement->Deactivate();

	StopAxeRotation();
}

void ALeviathan::LodgeAxe()
{
	StopAxeMoving();
	PivotPoint->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));

	SetActorRotation(CameraStartRotation);

	float InclinedSurfaceRange = FMath::FRandRange(-30.f, -55.f);
	float RegularSurfaceRange = FMath::FRandRange(-5.f, -25.f);

	FRotator RotationFromAxes = UKismetMathLibrary::MakeRotationFromAxes(
		ImpactNormal,
		FVector(1.f, 1.f, 1.f),
		FVector(1.f, 1.f, 1.f)
	);
	float Pitch = 0.f;
	if (RotationFromAxes.Pitch > 0.f)
		Pitch = RegularSurfaceRange - RotationFromAxes.Pitch;
	else
		Pitch = InclinedSurfaceRange - RotationFromAxes.Pitch;

	float Roll = FMath::FRandRange(-3.f, -8.f);
	LodgePoint->SetRelativeRotation(FRotator(Pitch, 0.f, Roll));

	Pitch =0.f;
	if (RotationFromAxes.Pitch > 0.f)
		Pitch = RotationFromAxes.Pitch;

	ZAdjustment = (90.f - Pitch) / 90.f * 10.f;

	FVector TargetLocation = ImpactLocation + FVector(0.f, 0.f, ZAdjustment);
	FVector LocationOffset = GetActorLocation() - LodgePoint->GetComponentLocation();
	FVector NewLocation = TargetLocation + LocationOffset;

	SetActorLocation(NewLocation);
	AxeState = EAxeState::LodgedInSomething;
}

void ALeviathan::AdjustAxeReturnLocation()
{
	FVector ActorLocation = GetActorLocation();
	float ScaledOffset = 30 * (1.f - ZAdjustment / 10.f);
	FVector OffsetZ = FVector(0.f, 0.f, ScaledOffset + 20.f);

	FVector NewLocation = ActorLocation + OffsetZ;
	SetActorLocation(NewLocation);
}

void ALeviathan::ReturnAxe()
{
	AxeState = EAxeState::Returning;

	// 도끼의 현재 위치에서 캐릭터의 도끼 소켓 위치까지의 벡터를 계산
	FVector DirectionToCharacter = GetActorLocation() - PlayerRef->GetMesh()->GetSocketLocation(FName("AxeSocket"));

	DistanceFromChar = FMath::Clamp(DirectionToCharacter.Size(), 0.f, MaxCalculationDistance);
	AdjustAxeReturnLocation();

	InitialLocation = GetActorLocation();
	InitialRotation = GetActorRotation();
	CameraStartRotation = PlayerRef->GetFollowCamera()->GetComponentRotation();

	LodgePoint->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));

	float NewRate = FMath::Clamp(OptimalDistance * AxeReturnSpeed / DistanceFromChar, 0.4f, 7.0f);
	AxeReturnTimeline->SetPlayRate(NewRate);
	AxeReturnTimeline->PlayFromStart();

	if (!ReturnNoBrownNoiseSound)
		return;

	float TimelineLength = 1.f / NewRate;

	FTimerHandle TimerHandle;
	if (TimelineLength - 0.87f > 0.f)
	{
		GetWorldTimerManager().SetTimer(
			TimerHandle,
			[this, TimelineLength]()
			{
				UGameplayStatics::SpawnSoundAttached(
					ReturnNoBrownNoiseSound,
					SkeletalMesh
				);
			},
			TimelineLength - 0.87f,
			false
		);
	}
	else
	{
		float StartTimeLength = 0.87f - TimelineLength;
		if (UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAttached(
			ReturnNoBrownNoiseSound,
			SkeletalMesh,
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTargetIncludingScale,
			true,
			1.f,
			1.f,
			StartTimeLength
		))
		{
			AudioComp->FadeIn(0.1f, 1.f, StartTimeLength);
		}
		
	}

	StopAxeRotation();

	NumOfSpins = FMath::RoundToInt32(TimelineLength / ReturnSpinRate);
	float SpinLength = (TimelineLength - 0.055f) / (float)NumOfSpins;

	SpinTimeline->SetPlayRate(1.f / SpinLength);
	SpinTimeline->PlayFromStart();

	AxeLocationLastTick = ReturnTargetLocation;
	AxeTraceReturnTimeline->PlayFromStart();
}

void ALeviathan::UpdateAxeRotation(float Value)
{
	float CurrentTime = AxeRotTimeline->GetPlaybackPosition();
	float RotationValue = AxeRotCurve ? AxeRotCurve->GetFloatValue(CurrentTime) : 0.f;

	if (SkeletalMesh)
	{
		FRotator NewRotation = FRotator(RotationValue * -360.f, 0.f, 0.f);
		PivotPoint->SetRelativeRotation(NewRotation);
	}
}

void ALeviathan::UpdateAxeThrowTrace(float Value)
{
	ProjectileMovement->ProjectileGravityScale = Value;

	FVector AxeThrowDirection = GetVelocity().GetSafeNormal(0.0001f);
	FVector TraceOffset = AxeThrowDirection * AxeThrowTraceDistance;

	FVector StartLocation = GetActorLocation();
	FVector EndLocation = StartLocation + TraceOffset;

	FCollisionQueryParams CollisionParams;
	CollisionParams.bTraceComplex = false;
	CollisionParams.AddIgnoredActor(this);

	FHitResult Hit;
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		StartLocation,
		EndLocation,
		ECollisionChannel::ECC_Visibility,
		CollisionParams
	);

	const FColor TraceColor = FColor::Red;
	const FColor HitColor = FColor::Green;
	const float DrawTime = 12.f;

	// DrawDebugLine(
	// 	GetWorld(),
	// 	StartLocation,
	// 	EndLocation,
	// 	bHit ? HitColor : TraceColor,
	// 	false,
	// 	DrawTime
	// );

	if (bHit)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, TEXT("Hit!!!"));
		ImpactLocation = Hit.ImpactPoint;;
		ImpactNormal = Hit.ImpactNormal;
		HitBoneName = Hit.BoneName;
		if (Hit.PhysMaterial != nullptr)
			HitSurface = Hit.PhysMaterial->SurfaceType;

		// TODO: Destructible
		if (ThrowParticle)
			ThrowParticle->EndTrails();

		StopAxeThrowTrace();
		ProjectileMovement->Deactivate();
		// TODO: Enemy
		if (ACharacter* CharacterRef = Cast<ACharacter>(Hit.GetActor()))
		{
			HitEnemyRef = CharacterRef;
			LodgeAxe();

			FVector TargetLocation = CharacterRef->GetActorLocation();
			FVector DirectionVector = TargetLocation - CameraLocationAtThrow;
			FVector NormalizedDirection = DirectionVector.GetSafeNormal(0.0001f);
			FVector ImpactVector = NormalizedDirection * ImpulseStrength;

			// TODO: HitEnemyRef->ReceiveHit(true, HitBoneName, ImpactVector);
			FAttachmentTransformRules AttachRules(
				EAttachmentRule::KeepWorld,
				EAttachmentRule::KeepWorld,
				EAttachmentRule::KeepWorld,
				false
			);

			AttachRules.bWeldSimulatedBodies = true;
			this->AttachToComponent(HitEnemyRef->GetMesh(), AttachRules, HitBoneName);

			// TODO: VFX
		}
		else
		{
			if (ImpactSound)
			{
				UGameplayStatics::PlaySoundAtLocation(
					this,
					ImpactSound,  // 초기화된 사운드 사용
					ImpactLocation
				);
			}

			if (DullThudSound)
			{
				UGameplayStatics::PlaySoundAtLocation(
					this,
					DullThudSound,  // 초기화된 사운드 사용
					ImpactLocation
				);
			}

			LodgeAxe();
		}
	}
}

void ALeviathan::StopAxeThrowTrace()
{
	AxeThrowTraceTimeline->Stop();
}

void ALeviathan::OnAxeThrowFinished()
{
	StopAxeMoving();

	SkeletalMesh->SetVisibility(false);
}

void ALeviathan::UpdateAxeWiggle(float Value)
{
	float Roll = Value * 12.f + LodgePointBaseRotation.Roll;
	LodgePoint->SetRelativeRotation(FRotator(LodgePointBaseRotation.Pitch, LodgePointBaseRotation.Yaw, Roll));
}

void ALeviathan::OnAxeWiggleFinished()
{
	if (ThrowParticle)
		ThrowParticle->BeginTrails(FName("BaseSocket"), FName("TipSocket"), ETrailWidthMode_FromCentre, 1.f);
	ReturnAxe();
}

void ALeviathan::UpdateAxeReturn(float Value)
{
	float CurrentTime = AxeReturnTimeline->GetPlaybackPosition();

	float Rotation1Value = AxeRotationCurve ? AxeRotationCurve->GetFloatValue(CurrentTime) : 0.f;
	float Rotation2Value = AxeRotation2Curve ? AxeRotation2Curve->GetFloatValue(CurrentTime) : 0.f;
	float SpeedValue = AxeReturnSpeedCurve ? AxeReturnSpeedCurve->GetFloatValue(CurrentTime) : 0.f;
	float RightVectorValue = AxeRightVectorCurve ? AxeRightVectorCurve->GetFloatValue(CurrentTime) : 0.f;
	float SoundValue = AxeReturnSoundCurve ? AxeReturnSoundCurve->GetFloatValue(CurrentTime) : 0.f;

	float RightVectorOffset = RightVectorValue * (DistanceFromChar / AxeReturnRightScale);
	FVector CameraRightDirection = PlayerRef->GetFollowCamera()->GetRightVector() * RightVectorOffset;
	FVector TargetSocketLocation = PlayerRef->GetMesh()->GetSocketLocation(FName("AxeSocket"));

	ReturnTargetLocation = FMath::Lerp(InitialLocation, TargetSocketLocation + CameraRightDirection, SpeedValue);

	FRotator TargetReturnRotation = FRotator(CameraStartRotation.Pitch, CameraStartRotation.Yaw, CameraStartRotation.Roll + AxeReturnTilt);
	FRotator LerpedRotation = UKismetMathLibrary::RLerp(InitialRotation, TargetReturnRotation, Rotation1Value, true);
	FRotator NewRotation = UKismetMathLibrary::RLerp(LerpedRotation, PlayerRef->GetMesh()->GetSocketRotation(FName("AxeSocket")), Rotation2Value, true);

	SetActorLocationAndRotation(ReturnTargetLocation, NewRotation);
	if (ReturnWhoosh)
		ReturnWhoosh->SetVolumeMultiplier(SoundValue);
}

void ALeviathan::OnAxeReturnFinished()
{
	AxeTraceReturnTimeline->Stop();

	if (ReturnWhoosh)
		ReturnWhoosh->FadeOut(0.4f, 0.f);

	StopAxeMoving();
	PlayerRef->Catch();

	if (AxeCatchParticle)
	{
		FTimerHandle CatchTimerHandle;
		GetWorldTimerManager().SetTimer(
			CatchTimerHandle,
			[this]()
			{
				AxeCatchParticle->Activate();
			},
			0.05f,
			false
		);
	}

	if (ThrowParticle)
	{
		FTimerHandle ThrowTimerHandle;
		GetWorldTimerManager().SetTimer(
			ThrowTimerHandle,
			[this]()
			{
				ThrowParticle->EndTrails();
			},
			0.07f,
			false
		);
	}
}

void ALeviathan::UpdateAxeReturnSpin(float Value)
{
	float CurrentTime = SpinTimeline->GetPlaybackPosition();
	bool bShouldTrigger = false;

	PivotPoint->SetRelativeRotation(FRotator(Value * 360.f, 0.f, 0.f));
}

void ALeviathan::OnSpinFinished()
{
	NumOfSpins = NumOfSpins - 1;
	if (NumOfSpins > 0)
		SpinTimeline->PlayFromStart();
}

void ALeviathan::UpdateAxeTraceReturn(float Value)
{
	FHitResult HitResult;
	bool bHit = UKismetSystemLibrary::SphereTraceSingle(
		this,
		ReturnTargetLocation,
		AxeLocationLastTick,
		25.f,
		UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility),
		false,
		TArray<AActor*>(),
		EDrawDebugTrace::None,
		HitResult,
		true
	);

	AxeLocationLastTick = ReturnTargetLocation;

	if (bHit)
	{
		ImpactLocation = HitResult.ImpactPoint;
		// TODO: Destructible
		// TODO: Enemy Damage
	}
}

void ALeviathan::OnWhoosh1()
{
	if (!Whoosh1Sound)
		return;
	UGameplayStatics::SpawnSoundAttached(
		Whoosh1Sound,
		SkeletalMesh
	);
}

void ALeviathan::OnWhoosh2()
{
	if (!Whoosh2Sound)
		return;
	UGameplayStatics::SpawnSoundAttached(
		Whoosh2Sound,
		SkeletalMesh
	);
}

void ALeviathan::Throw(FRotator CameraRotation, FVector ThrowDirectionVector, FVector CameraLocation)
{
	if (!ProjectileMovement)
		return;

	CameraStartRotation = CameraRotation;
	ThrowDirection = ThrowDirectionVector;
	CameraLocationAtThrow = CameraLocation;

	FRotator StartRotation = FRotator(CameraStartRotation.Pitch, CameraRotation.Yaw, CameraStartRotation.Roll + AxeSpinAxisOffset);
	SnapAxeToStartPosition(StartRotation, ThrowDirection, CameraLocationAtThrow);

	ProjectileMovement->Velocity =  ThrowDirection * AxeThrowSpeed;
	ProjectileMovement->Activate();
	
	StartAxeRotForward();

	AxeState = EAxeState::Launched;

	if (ThrowParticle)
	{
		ThrowParticle->BeginTrails(FName("BaseSocket"), FName("TipSocket"), ETrailWidthMode_FromCentre, 1.f);
	}

	ProjectileMovement->bSimulationEnabled = true;
	ProjectileMovement->ProjectileGravityScale = 0.f;

	if (!AxeThrowTraceTimeline)
		return;
	AxeThrowTraceTimeline->PlayFromStart();
}

void ALeviathan::Recall()
{
	StopAxeThrowTrace();

	SkeletalMesh->SetVisibility(true);

	if (BrownNoiseSound)
	{
		ReturnWhoosh = UGameplayStatics::SpawnSoundAttached(
			BrownNoiseSound,
			SkeletalMesh
		);
	}

	switch (AxeState)
	{
	case EAxeState::Launched:
		ZAdjustment = 10.f;
		
		if (ThrowParticle)
			ThrowParticle->BeginTrails(FName("BaseSocket"), FName("TipSocket"), ETrailWidthMode_FromCentre, 1.f);
		ReturnAxe();
		break;
	case EAxeState::LodgedInSomething:
		LodgePointBaseRotation = LodgePoint->GetRelativeRotation();
		WiggleTimeline->PlayFromStart();
		break;
	}
}

void ALeviathan::SnapAxeToStartPosition(FRotator StartRotation, FVector ThrowDirectionVector, FVector CameraLocation)
{
	FVector NewLocation = (ThrowDirectionVector * 250.f + CameraLocation) - PivotPoint->GetRelativeLocation();

	SetActorLocationAndRotation(NewLocation, StartRotation);
}

void ALeviathan::SetAxeState(int32 Index)
{
	switch (Index)
	{
	case 0:
		AxeState = EAxeState::Idle;
		break;
	case 1:
		AxeState = EAxeState::Launched;
		break;
	case 2:
		AxeState = EAxeState::LodgedInSomething;
		break;
	case 3:
		AxeState = EAxeState::Returning;
		break;	
	}
}

