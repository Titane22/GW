// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/Data/SkillData.h"
#include "PlayerProgressionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillUnlocked, FName, SkillID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPointsChanged, int32, NewPoints);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GW_API UPlayerProgressionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerProgressionComponent();

protected:
	virtual void BeginPlay() override;

	// 해제된 스킬 목록
	UPROPERTY(SaveGame, VisibleAnywhere, Category = "Progression")
	TSet<FName> UnlockedSkills;

	// 스킬 데이터 테이블
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	UDataTable* SkillDataTable;

	// 사용 가능한 스킬 포인트
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int32 AvailablePoints;

public:
	// 스킬 해제 여부 확인
	UFUNCTION(BlueprintCallable, Category = "Progression")
	bool IsSkillUnlocked(FName SkillID) const;

	// 스킬 해제 가능 여부 (선행 스킬 + 비용 확인)
	UFUNCTION(BlueprintCallable, Category = "Progression")
	bool CanUnlockSkill(FName SkillID) const;

	// 스킬 해제
	UFUNCTION(BlueprintCallable, Category = "Progression")
	bool UnlockSkill(FName SkillID);

	// 스킬 데이터 가져오기
	UFUNCTION(BlueprintCallable, Category = "Progression")
	FSkillData GetSkillData(FName SkillID) const;

	// 모든 스킬 데이터 가져오기
	UFUNCTION(BlueprintCallable, Category = "Progression")
	TArray<FSkillData> GetAllSkills() const;

	// 특정 타입의 스킬 가져오기
	UFUNCTION(BlueprintCallable, Category = "Progression")
	TArray<FSkillData> GetSkillsByType(ESkillType SkillType) const;

	// 특정 무기 상태의 스킬 가져오기
	UFUNCTION(BlueprintCallable, Category = "Progression")
	TArray<FSkillData> GetSkillsByWeaponState(EWeaponState WeaponState) const;

	// 포인트 추가
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void AddPoints(int32 Points);

	// 포인트 가져오기
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Progression")
	int32 GetAvailablePoints() const { return AvailablePoints; }

	// 스킬 강제 해제 (디버그/치트용)
	UFUNCTION(BlueprintCallable, Category = "Progression|Debug")
	void ForceUnlockSkill(FName SkillID);

	// 모든 스킬 초기화
	UFUNCTION(BlueprintCallable, Category = "Progression|Debug")
	void ResetAllSkills();

	// 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Progression")
	FOnSkillUnlocked OnSkillUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Progression")
	FOnPointsChanged OnPointsChanged;

private:
	// 초기 스킬 해제 (게임 시작 시 기본 스킬)
	void UnlockInitialSkills();
};
