// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SkillData.generated.h"

UENUM(BlueprintType)
enum class ESkillType : uint8
{
	Combo UMETA(DisplayName = "Combo Attack"),
	Ability UMETA(DisplayName = "Special Ability"),
	Passive UMETA(DisplayName = "Passive Upgrade")
};

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	Armed UMETA(DisplayName = "Armed (With Axe)"),
	Unarmed UMETA(DisplayName = "Unarmed (No Axe)")
};

USTRUCT(BlueprintType)
struct FSkillData : public FTableRowBase
{
	GENERATED_BODY()

	// 스킬 고유 ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Basic")
	FName SkillID;

	// 스킬 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Basic")
	FText SkillName;

	// 스킬 설명
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Basic")
	FText Description;

	// 스킬 타입
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Basic")
	ESkillType SkillType;

	// 무기 상태 (도끼 장착/비장착)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Basic")
	EWeaponState WeaponState;

	// 선행 스킬 (이 스킬들을 먼저 해제해야 함)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Requirements")
	TArray<FName> Prerequisites;

	// 해제 비용 (XP 또는 화폐)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Requirements")
	int32 UnlockCost;

	// 콤보 애니메이션 (SkillType이 Combo일 때)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Combat", meta = (EditCondition = "SkillType == ESkillType::Combo", EditConditionHides))
	UAnimMontage* ComboAnimation;

	// 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Combat")
	float Damage;

	// 콤보 인덱스 (연속 공격에서의 순서)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Combat", meta = (EditCondition = "SkillType == ESkillType::Combo", EditConditionHides))
	int32 ComboIndex;

	// 아이콘 (UI용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|UI")
	UTexture2D* SkillIcon;

	FSkillData()
		: SkillType(ESkillType::Combo)
		, WeaponState(EWeaponState::Armed)
		, UnlockCost(0)
		, ComboAnimation(nullptr)
		, Damage(10.f)
		, ComboIndex(0)
		, SkillIcon(nullptr)
	{
	}
};
