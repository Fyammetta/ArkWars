// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SkillBaseComponent.generated.h"


struct FSkillInfo;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARKWARS_API USkillComponentBase : public UActorComponent
{
	GENERATED_BODY()
	
	TSharedPtr<FSkillInfo> Skills;
	
protected:
	virtual void BeginPlay() override;
};
