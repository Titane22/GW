// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Characters/Player_Base.h"

#include "EnhancedInputComponent.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Gameplay/Weapons/Leviathan.h"
#include "Gameplay/Components/PlayerProgressionComponent.h"
#include "Gameplay/Components/CombatComponent.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"

APlayer_Base::APlayer_Base()
{
	PrimaryActorTick.bCanEverTick = true;

	// Configure character rotation
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure camera boom (God of War style - close over-the-shoulder)
	CameraBoom->TargetArmLength = 120.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector(20.0f, 50.0f, 60.0f);
	CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraLagSpeed = 10.0f;
	CameraBoom->CameraRotationLagSpeed = 10.0f;
	CameraBoom->CameraLagMaxDistance = 3.0f;

	// Configure follow camera
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	LeviathanAxe = CreateDefaultSubobject<UChildActorComponent>(TEXT("Leviathan Axe"));
	LeviathanAxe->SetupAttachment(GetMesh(), FName("RightHandWeaponBoneSocket"));
	if (LeviathanAxe)
	{
		// 자식 액터 클래스 설정
		LeviathanAxe->SetChildActorClass(ALeviathan::StaticClass());
	}

	AxeCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Axe Capsule"));
	AxeCollision->SetupAttachment(LeviathanAxe);

	AimTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("AimTimeline"));
	DesiredSocketTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DesiredSocketTimeline"));
	RangedCameraTl = CreateDefaultSubobject<UTimelineComponent>(TEXT("RangedCameraTl"));

	// 스킬 시스템 컴포넌트 생성
	ProgressionComponent = CreateDefaultSubobject<UPlayerProgressionComponent>(TEXT("ProgressionComponent"));
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
}

void APlayer_Base::BeginPlay()
{
	Super::BeginPlay();

	GetMesh()->HideBoneByName(FName("hips_cloth_main_l"), EPhysBodyOp::PBO_None);
	GetMesh()->HideBoneByName(FName("hips_cloth_main_r"), EPhysBodyOp::PBO_None);
	
	if (DesiredSocketTimeline && DesireSocketCurve)
	{
		FOnTimelineFloat TimelineProgress;
		TimelineProgress.BindUFunction(this, FName("UpdateDesireSocket"));
		DesiredSocketTimeline->AddInterpFloat(DesireSocketCurve, TimelineProgress);
	}

	if (ALeviathan* Axe = Cast<ALeviathan>(LeviathanAxe->GetChildActor()))
	{
		LeviathanRef = Axe;
		// if (UPrimitiveComponent* WeaponMesh = Cast<UPrimitiveComponent>(Axe->GetRootComponent()))
		// {
		// 	WeaponMesh->SetSimulatePhysics(false);
		// 	WeaponMesh->SetEnableGravity(false);
		// }
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &APlayer_Base::OnNotifyBegin);
		AnimInstance->OnPlayMontageNotifyEnd.AddDynamic(this, &APlayer_Base::OnNotifyEnd);
		AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &APlayer_Base::OnCatchNotifyBegin);
		AnimInstance->OnPlayMontageNotifyEnd.AddDynamic(this, &APlayer_Base::OnCatchNotifyEnd);
	}
	
	// Initialize camera to default idle position
	if (CameraBoom)
	{
		CameraBoom->TargetArmLength = SpringArmLengthIdle;
		CameraBoom->SocketOffset = CameraVector;
		DesiredSocketOffset = CameraVector;
	}

	// Create AimHUD widget instance
	if (AimHUD_Class)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			AimHUD = CreateWidget<UUserWidget>(PC, AimHUD_Class);
		}
	}

	// Bind RangedCameraTimeline to LerpCameraPosition
	if (RangedCameraTl && RangedCameraCurve)
	{
		FOnTimelineFloat TimelineProgress;
		TimelineProgress.BindUFunction(this, FName("LerpCameraPosition"));
		RangedCameraTl->AddInterpFloat(RangedCameraCurve, TimelineProgress);
	}
}

void APlayer_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APlayer_Base::LerpCameraPosition(float Value)
{
	if (!CameraBoom)
		return;
	CameraBoom->TargetArmLength = FMath::Lerp(SpringArmLengthIdle, SpringArmLengthAim, Value);
	CameraBoom->SocketOffset = FMath::Lerp(CameraVector, RangedCameraVector, Value);
	DesiredSocketOffset = FMath::Lerp(CameraVector, RangedCameraVector, Value);
}

void APlayer_Base::UpdateAim(float Value)
{
}

void APlayer_Base::UpdateDesireSocket(float Value)
{
	float NewOffsetX = Value * 0.5f * 12.f;
	float NewOffsetY = Value * 0.5f * 4.f;
	float NewOffsetZ = Value * 0.5f * 3.f;
	CameraBoom->SocketOffset = FVector(
		DesiredSocketOffset.X - NewOffsetX,
		DesiredSocketOffset.Y - NewOffsetY,
		DesiredSocketOffset.Z - NewOffsetZ
	);
}

void APlayer_Base::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Bind Aim action for both Started (press) and Completed (release) events
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &APlayer_Base::PerformAttack);
		EnhancedInputComponent->BindAction(RecallAction, ETriggerEvent::Triggered, this, &APlayer_Base::ReturnAxe);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &APlayer_Base::AimPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &APlayer_Base::AimReleased);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void APlayer_Base::PerformAttack()
{
	if (bIsAim)
	{
		ThrowAxe();
	}
	else
	{
		if (CombatComponent)
		{
			CombatComponent->PerformAttack();
		}
	}
}

void APlayer_Base::AimPressed()
{
	bIsAim = true;

	if (AimHUD)
	{
		AimHUD->AddToViewport();
	}
	bRangedAttackMode = true;
	bUseControllerRotationYaw = true;
	CameraTurnRate = 30.f;
	GetCharacterMovement()->MaxWalkSpeed = 250.f;

	if (RangedCameraTl)
	{
		RangedCameraTl->SetPlayRate(1.4f);
		RangedCameraTl->PlayFromStart();
	}
}

void APlayer_Base::AimReleased()
{
	bIsAim = false;

	if (AimHUD)
	{
		AimHUD->RemoveFromViewport();
	}
	bRangedAttackMode = false;
	bUseControllerRotationYaw = false;
	CameraTurnRate = 50.f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;

	if (RangedCameraTl)
	{
		RangedCameraTl->SetPlayRate(0.9f);
		RangedCameraTl->Reverse();
	}
}

void APlayer_Base::ThrowAxe()
{
	if (!bIsAim || !LeviathanRef || bAxeThrown)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, FString::Printf(TEXT("Throw Axe: %d %d"), bIsAim, bAxeThrown));
		if (!LeviathanRef)
			GEngine->AddOnScreenDebugMessage(-1, 1.f,FColor::Red, TEXT("LeviathanRef is null"));
		return;
	}
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		bAxeThrown = true;
		AnimInstance->Montage_Play(ThrowMontage, 1.f, EMontagePlayReturnType::MontageLength, 0.037f);
	}
}

void APlayer_Base::ReturnAxe()
{
	if (!LeviathanRef)
		return;
	if (!bAxeThrown || bAxeRecalling)
		return;

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		bAxeRecalling = true;
		AnimInstance->Montage_Play(RecallMontage, 1.1f, EMontagePlayReturnType::MontageLength, 0.037f);
		LeviathanRef->Recall();
	}
}

void APlayer_Base::Catch()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)
		return;

	AnimInstance->Montage_Play(
		CatchMontage,
		1.f,
		EMontagePlayReturnType::MontageLength,
		0.1f
	);

	LeviathanRef->AttachToComponent(
		GetMesh(),
		FAttachmentTransformRules::SnapToTargetIncludingScale,
		FName("RightHandWeaponBoneSocket")
	);

	bAxeThrown = false;
	bAxeRecalling = false;

	LeviathanRef->SetAxeState(0);

	if (!AimTimeline->IsPlaying())
	{
		if (APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0))
		{
			if (CameraManager && ShakeClass)
			{
				CameraManager->StartCameraShake(ShakeClass);
				DesiredSocketTimeline->SetPlayRate(0.7f);
				DesiredSocketTimeline->PlayFromStart();
			}
		}
	}
}

void APlayer_Base::OnNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload)
{
	if (NotifyName != FName("Throw") || !ThrowEffortSound)
		return;

	UGameplayStatics::SpawnSoundAttached(
		ThrowEffortSound,
		GetMesh()
	);
}

void APlayer_Base::OnNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload)
{
	if (NotifyName != FName("Throw"))
		return;

	LeviathanRef->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	FRotator CameraRotation = FollowCamera->GetComponentRotation();
	FVector ThrowDirectionVector = FollowCamera->GetForwardVector();
	FVector CameraLocation = FollowCamera->GetComponentLocation();

	LeviathanRef->Throw(CameraRotation, ThrowDirectionVector, CameraLocation);
}

void APlayer_Base::OnCatchNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if (NotifyName != FName("Catch"))
		return;
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_SetPlayRate(AnimInstance->GetCurrentActiveMontage(), 0.4f);
	}
}

void APlayer_Base::OnCatchNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if (NotifyName != FName("Catch"))
		return;
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_SetPlayRate(AnimInstance->GetCurrentActiveMontage(), 1.f);
	}
}

// ========== ICombatAttacker 인터페이스 구현 ==========

void APlayer_Base::DoAttackTrace(FName DamageSourceBone)
{
	if (CombatComponent)
	{
		CombatComponent->PerformAttackTrace(DamageSourceBone);
	}
}

void APlayer_Base::CheckCombo()
{
	if (CombatComponent)
	{
		CombatComponent->CheckComboInput();
	}
}

void APlayer_Base::CheckChargedAttack()
{
	if (CombatComponent)
	{
		CombatComponent->HandleChargedAttack();
	}
}

// ========== ICombatDamageable 인터페이스 구현 ==========

void APlayer_Base::ApplyDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation, const FVector& DamageImpulse)
{
	// TODO: HealthComponent 생성 시 해당 컴포넌트로 위임
	UE_LOG(LogTemp, Warning, TEXT("Player received %.1f damage from %s"),
		Damage, DamageCauser ? *DamageCauser->GetName() : TEXT("Unknown"));
}

void APlayer_Base::HandleDeath()
{
	// TODO: HealthComponent 생성 시 해당 컴포넌트로 위임
	UE_LOG(LogTemp, Warning, TEXT("Player died!"));
}

void APlayer_Base::ApplyHealing(float Healing, AActor* Healer)
{
	// TODO: HealthComponent 생성 시 해당 컴포넌트로 위임
	UE_LOG(LogTemp, Log, TEXT("Player healed %.1f"), Healing);
}
