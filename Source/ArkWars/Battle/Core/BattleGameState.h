// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArkWars/Battle/Toolkits/ArkWarDelegates.h"
#include "GameFramework/GameState.h"
#include "BattleGameState.generated.h"

/**
 * 
 */
UCLASS()
class ARKWARS_API ABattleGameState : public AGameStateBase
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	
		
		

};
