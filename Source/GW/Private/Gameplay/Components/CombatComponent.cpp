// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Components/CombatComponent.h"
#include "Gameplay/Components/PlayerProgressionComponent.h"
#include "Gameplay/Weapons/Leviathan.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "Gameplay/Characters/Player_Base.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Variant_Combat/Interfaces/CombatDamageable.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// 기본 콤보 초기화
	bIsAttacking = false;
	bComboInputPressed = false;
	ComboCount = 0;
	MaxComboCount = 3;  // 3단 콤보

	// 공격 판정 초기화
	AttackRange = 150.f;
	AttackRadius = 50.f;
	bDebugAttackTrace = false;

	// 차징 시스템 초기화
	bIsCharging = false;
	ChargeTime = 0.f;
	MaxChargeTime = 2.f;
	ChargeMultiplier = 2.f;

	// 스킬 시스템 초기화
	CurrentComboIndex = 0;
	CurrentAttackDamage = 10.f;
	ComboResetTime = 1.5f;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerChar = Cast<APlayer_Base>(GetOwner());
	if (!OwnerChar)
	{
		UE_LOG(LogTemp, Error, TEXT("CombatComponent owner is not APlayer_Base!"));
		return;
	}

	ProgressionComp = OwnerChar->FindComponentByClass<UPlayerProgressionComponent>();
	if (!ProgressionComp)
	{
		UE_LOG(LogTemp, Error, TEXT("ProgressionComponent not found!"));
		return;
	}
}

void UCombatComponent::PerformAttack()
{
	if (!OwnerChar)
	{
		UE_LOG(LogTemp, Error, TEXT("OwnerChar not found in PerformAttack!"));
		return;
	}
	UAnimInstance* AnimInstance = OwnerChar->GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimInstance not found in PerformAttack!"));
		return;
	}

	if (bIsAttacking)
	{
		bComboInputPressed = true;
		UE_LOG(LogTemp, Log, TEXT("Combo Input Buffered"));
		return;
	}

	bIsAttacking = true;
	ComboCount = 0;
	bComboInputPressed = false;

	UAnimMontage* TargetMontage = GetCurrentWeaponState() == EWeaponState::Armed
		? ArmedComboMontage
		: UnarmedComboMontage;

	if (!TargetMontage)
	{
		UE_LOG(LogTemp, Error, TEXT("Target Montage not found!"));
		bIsAttacking = false;
		return;
	}

	const float MontageLength = AnimInstance->Montage_Play(
		TargetMontage,
		1.f,
		EMontagePlayReturnType::MontageLength,
		0.f,
		true
	);

	if (MontageLength > 0.f)
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUFunction(this, FName("OnComboMontageEnded"));
		AnimInstance->Montage_SetEndDelegate(EndDelegate, TargetMontage);

		UE_LOG(LogTemp, Log, TEXT("Started Combo Attack: %s"), *TargetMontage->GetName());
	}
	else
	{
		bIsAttacking = false;
	}
}

void UCombatComponent::CheckComboInput()
{
	if (!OwnerChar)
		return;
	UAnimInstance* AnimInstance = OwnerChar->GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimInstance not found in CheckComboInput!"));
		return;
	}

	if (bComboInputPressed && ComboCount < MaxComboCount - 1)
	{
		ComboCount++;
		bComboInputPressed = false;

		FName NextSection = ComboSectionNames[ComboCount];
		AnimInstance->Montage_JumpToSection(NextSection);

		UE_LOG(LogTemp, Log, TEXT("Combo Continue: Melee%d"), ComboCount + 1);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Combo Input Not Found - Ending"));
	}
}

EWeaponState UCombatComponent::GetCurrentWeaponState() const
{
	const ALeviathan* Axe = OwnerChar->GetLeviathanAxe();
	if (!Axe)
	{
		UE_LOG(LogTemp, Error, TEXT("LeviathanAxe not found!"));
		return EWeaponState::Unarmed;
	}

	return Axe->GetAxeState() == EAxeState::Launched ? EWeaponState::Unarmed : EWeaponState::Armed;
}

void UCombatComponent::ResetCombo()
{
	bIsAttacking = false;
	ComboCount = 0;
	bComboInputPressed = false;
	CurrentComboIndex = 0;
}

void UCombatComponent::OnComboMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsAttacking = false;
	ComboCount = 0;
	bComboInputPressed = false;
}

void UCombatComponent::ExcuteCombo(FName SkillID)
{
	// TODO: 스킬 시스템 구현
}

// ========================================
// 공격 로직 (Player_Base의 Interface에서 호출)
// ========================================

void UCombatComponent::PerformAttackTrace(FName DamageSourceBone)
{
	if (!OwnerChar)
		return;

	// 공격 시작 위치 결정
	FVector Origin;
	if (DamageSourceBone.IsNone())
	{
		// 본 이름이 없으면 무기나 주먹 위치 사용
		if (GetCurrentWeaponState() == EWeaponState::Armed)
		{
			// 도끼 위치
			ALeviathan* Axe = OwnerChar->GetLeviathanAxe();
			Origin = Axe ? Axe->GetActorLocation() : OwnerChar->GetActorLocation();
		}
		else
		{
			// 주먹 위치 (오른손 소켓)
			Origin = OwnerChar->GetMesh()->GetSocketLocation(FName("RightHandSocket"));
		}
	}
	else
	{
		// 지정된 본 위치 사용
		Origin = OwnerChar->GetMesh()->GetSocketLocation(DamageSourceBone);
	}

	// 트레이스 방향 및 범위
	FVector Forward = OwnerChar->GetActorForwardVector();
	FVector Start = Origin;
	FVector End = Origin + (Forward * AttackRange);

	// 충돌 무시 액터
	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(OwnerChar);

	// Sphere Trace 실행
	TArray<FHitResult> HitResults;
	bool bHit = UKismetSystemLibrary::SphereTraceMulti(
		GetWorld(),
		Start,
		End,
		AttackRadius,
		UEngineTypes::ConvertToTraceType(ECC_Pawn),
		false,
		IgnoredActors,
		bDebugAttackTrace ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		HitResults,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		2.f
	);

	if (bHit)
	{
		TSet<AActor*> HitActors;  // 중복 방지

		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (!HitActor || HitActors.Contains(HitActor))
				continue;

			HitActors.Add(HitActor);

			// ICombatDamageable 인터페이스 확인
			if (HitActor->Implements<UCombatDamageable>())
			{
				// 데미지 계산 (차징 배율 적용)
				float FinalDamage = CurrentAttackDamage;
				if (bIsCharging)
				{
					float ChargeRatio = FMath::Clamp(ChargeTime / MaxChargeTime, 0.f, 1.f);
					FinalDamage *= (1.f + (ChargeMultiplier - 1.f) * ChargeRatio);
				}

				// 넉백 임펄스 계산
				FVector ImpulseDirection = (Hit.ImpactPoint - Origin).GetSafeNormal();
				FVector Impulse = ImpulseDirection * 1000.f;  // 넉백 강도

				// 데미지 적용
				ICombatDamageable* DamageableActor = Cast<ICombatDamageable>(HitActor);
				if (DamageableActor)
				{
					DamageableActor->ApplyDamage(FinalDamage, OwnerChar, Hit.ImpactPoint, Impulse);
				}

				UE_LOG(LogTemp, Log, TEXT("Hit %s for %.1f damage (Charged: %s)"),
					*HitActor->GetName(), FinalDamage, bIsCharging ? TEXT("Yes") : TEXT("No"));
			}
		}

		// 차징 해제
		bIsCharging = false;
		ChargeTime = 0.f;
	}
}

void UCombatComponent::HandleChargedAttack()
{
	if (!OwnerChar)
		return;

	// 공격 버튼이 계속 눌려있는지 확인
	APlayerController* PC = Cast<APlayerController>(OwnerChar->GetController());
	if (!PC)
		return;

	// TODO: 입력 확인 로직
	// 예: Enhanced Input System의 IsPressed() 사용

	if (bIsCharging)
	{
		// 차징 시간 증가
		ChargeTime += GetWorld()->GetDeltaSeconds();
		ChargeTime = FMath::Min(ChargeTime, MaxChargeTime);

		// 최대 차징 도달 시 자동 공격
		if (ChargeTime >= MaxChargeTime)
		{
			UE_LOG(LogTemp, Log, TEXT("Max Charge Reached!"));
			// 차징 애니메이션 루프 종료하고 공격 실행
			// AnimInstance->Montage_JumpToSection(FName("ChargedAttackRelease"));
		}
		else
		{
			// 차징 애니메이션 루프 계속
			// AnimInstance->Montage_JumpToSection(FName("ChargeLoop"));
			UE_LOG(LogTemp, Log, TEXT("Charging... %.2f / %.2f"), ChargeTime, MaxChargeTime);
		}
	}
}

void UCombatComponent::StartCharging()
{
	bIsCharging = true;
	ChargeTime = 0.f;
	UE_LOG(LogTemp, Log, TEXT("Started Charging"));
}

void UCombatComponent::StopCharging()
{
	bIsCharging = false;
	ChargeTime = 0.f;
	UE_LOG(LogTemp, Log, TEXT("Stopped Charging"));
}
