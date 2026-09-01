// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BattleFunctionLibrary.generated.h"

class USkillManagerSubsystem;
class UCardManagerSubsystem;
class UBattleGameFlowSubsystem;
/**
 * 
 */
UCLASS()
class ARKWARS_API UBattleFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Card|Mgr")
	static UCardManagerSubsystem* GetCardManager(const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Skill|Mgr")
	static USkillManagerSubsystem* GetSkillManager(const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameFlow")
	static UBattleGameFlowSubsystem* GetBattleManager(const UObject* WorldContextObject);
};
