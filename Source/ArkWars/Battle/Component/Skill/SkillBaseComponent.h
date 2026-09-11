// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SkillBaseComponent.generated.h"


struct FGameplayTag;
struct FSkillInfo;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARKWARS_API USkillComponentBase : public UActorComponent
{
	GENERATED_BODY()
	
	TSharedPtr<FSkillInfo> SkillInfo;
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
public:
	UFUNCTION(BlueprintCallable,BlueprintPure, Category=Skill)
	static UActorComponent* CreateSkill(APlayerState* Owner, const FGameplayTag& Skill);
	
	template <typename T = USkillComponentBase>
	static T* CreateSkill(APlayerState* Owner, const FGameplayTag& Skill) { return Cast<T>(CreateSkill(Owner, Skill)); };
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category=Skill)
	FGameplayTag GetSkillTag() const;
	
	UFUNCTION(BlueprintCallable)
	virtual void OnCanActivate(const FGameplayTag& Timing) PURE_VIRTUAL(OnCanActivate);
protected:
	
	virtual void RegisterActivateTiming() PURE_VIRTUAL(RegisterActivateTiming);
	virtual void UnRegisterActivateTiming() PURE_VIRTUAL(UnRegisterActivateTiming);
};
