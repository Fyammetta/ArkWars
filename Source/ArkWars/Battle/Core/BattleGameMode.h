// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BattleGameMode.generated.h"

struct FGameplayTag;
/**
 * 
 */
UCLASS()
class ARKWARS_API ABattleGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
	void InitGameMode(const FGameplayTag& ModeTag);
	
	
};
