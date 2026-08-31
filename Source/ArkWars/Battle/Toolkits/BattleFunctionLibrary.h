// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BattleFunctionLibrary.generated.h"

class UCardManagerSubsystem;
/**
 * 
 */
UCLASS()
class ARKWARS_API UBattleFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Card|Mgr")
	static UCardManagerSubsystem* GetCardManager(UObject* WorldContextObject);
};


