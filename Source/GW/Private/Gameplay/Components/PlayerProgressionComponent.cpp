// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Components/PlayerProgressionComponent.h"

UPlayerProgressionComponent::UPlayerProgressionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	AvailablePoints = 0;
}

void UPlayerProgressionComponent::BeginPlay()
{
	Super::BeginPlay();

	UnlockInitialSkills();
}

bool UPlayerProgressionComponent::IsSkillUnlocked(FName SkillID) const
{
	return UnlockedSkills.Contains(SkillID);
}

bool UPlayerProgressionComponent::CanUnlockSkill(FName SkillID) const
{
	// 이미 해제되어 있으면 false
	if (IsSkillUnlocked(SkillID))
		return false;

	FSkillData SkillData = GetSkillData(SkillID);
	if (SkillData.SkillID.IsNone())
		return false;

	// 선행 스킬 확인
	for (const FName& Prereq : SkillData.Prerequisites)
	{
		if (!IsSkillUnlocked(Prereq))
			return false;
	}

	// 비용 확인
	return AvailablePoints >= SkillData.UnlockCost;
}

bool UPlayerProgressionComponent::UnlockSkill(FName SkillID)
{
	if (!CanUnlockSkill(SkillID))
		return false;

	FSkillData SkillData = GetSkillData(SkillID);
	if (SkillData.SkillID.IsNone())
		return false;

	// 포인트 차감
	AvailablePoints -= SkillData.UnlockCost;

	// 스킬 해제
	UnlockedSkills.Add(SkillID);

	// 이벤트 브로드캐스트
	OnSkillUnlocked.Broadcast(SkillID);
	OnPointsChanged.Broadcast(AvailablePoints);

	UE_LOG(LogTemp, Log, TEXT("Skill Unlocked: %s"), *SkillID.ToString());

	return true;
}

FSkillData UPlayerProgressionComponent::GetSkillData(FName SkillID) const
{
	if (!SkillDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillDataTable is not assigned in PlayerProgressionComponent!"));
		return FSkillData();
	}

	FSkillData* SkillData = SkillDataTable->FindRow<FSkillData>(SkillID, TEXT(""));
	if (!SkillData)
	{
		UE_LOG(LogTemp, Warning, TEXT("Skill not found in DataTable: %s"), *SkillID.ToString());
		return FSkillData();
	}

	return *SkillData;
}

TArray<FSkillData> UPlayerProgressionComponent::GetAllSkills() const
{
	TArray<FSkillData> AllSkills;

	if (!SkillDataTable)
		return AllSkills;

	TArray<FName> RowNames = SkillDataTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		if (FSkillData* SkillData = SkillDataTable->FindRow<FSkillData>(RowName, TEXT("")))
		{
			AllSkills.Add(*SkillData);
		}
	}

	return AllSkills;
}

TArray<FSkillData> UPlayerProgressionComponent::GetSkillsByType(ESkillType SkillType) const
{
	TArray<FSkillData> FilteredSkills;

	TArray<FSkillData> AllSkills = GetAllSkills();
	for (const FSkillData& Skill : AllSkills)
	{
		if (Skill.SkillType == SkillType)
		{
			FilteredSkills.Add(Skill);
		}
	}

	return FilteredSkills;
}

TArray<FSkillData> UPlayerProgressionComponent::GetSkillsByWeaponState(EWeaponState WeaponState) const
{
	TArray<FSkillData> FilteredSkills;

	TArray<FSkillData> AllSkills = GetAllSkills();
	for (const FSkillData& Skill : AllSkills)
	{
		if (Skill.WeaponState == WeaponState)
		{
			FilteredSkills.Add(Skill);
		}
	}

	return FilteredSkills;
}

void UPlayerProgressionComponent::AddPoints(int32 Points)
{
	AvailablePoints += Points;
	OnPointsChanged.Broadcast(AvailablePoints);

	UE_LOG(LogTemp, Log, TEXT("Points Added: %d | Total: %d"), Points, AvailablePoints);
}

void UPlayerProgressionComponent::ForceUnlockSkill(FName SkillID)
{
	if (!IsSkillUnlocked(SkillID))
	{
		UnlockedSkills.Add(SkillID);
		OnSkillUnlocked.Broadcast(SkillID);

		UE_LOG(LogTemp, Warning, TEXT("Skill Force Unlocked (Debug): %s"), *SkillID.ToString());
	}
}

void UPlayerProgressionComponent::ResetAllSkills()
{
	UnlockedSkills.Empty();
	UnlockInitialSkills();

	UE_LOG(LogTemp, Warning, TEXT("All skills have been reset!"));
}

void UPlayerProgressionComponent::UnlockInitialSkills()
{
	// 기본 공격 스킬들을 자동으로 해제
	// DataTable에서 UnlockCost가 0인 스킬들을 자동 해제할 수도 있음

	TArray<FSkillData> AllSkills = GetAllSkills();
	for (const FSkillData& Skill : AllSkills)
	{
		if (Skill.UnlockCost == 0)
		{
			UnlockedSkills.Add(Skill.SkillID);
			UE_LOG(LogTemp, Log, TEXT("Initial Skill Unlocked: %s"), *Skill.SkillID.ToString());
		}
	}
}
