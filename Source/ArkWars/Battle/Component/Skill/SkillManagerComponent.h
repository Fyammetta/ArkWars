// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ArkWars/Battle/Toolkits/ArkWarTypes.h"
#include "Components/ActorComponent.h"
#include "SkillManagerComponent.generated.h"


struct FSkillInfo;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARKWARS_API USkillManagerComponent : public UActorComponent
{
	GENERATED_BODY()
	
	TArray<TSharedPtr<FSkillInfo>> Skills;
	
protected:
	virtual void BeginPlay() override;
	
	void InitAsOperator(const FOperatorCardInfo& Info);
};
