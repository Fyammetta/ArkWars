// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"
#include "ArkWarBattleSettings.generated.h"

class IBattleModeInterface;
/**
 * 
 */
UCLASS(Config = Game, DisplayName = "Ark War Battle")
class ARKWARS_API UArkWarBattleSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	static UArkWarBattleSettings* Get();
	
	UPROPERTY(EditAnywhere)
	TMap<FGameplayTag, TSubclassOf<UActorComponent>> GameModes;
	
	UPROPERTY(EditAnywhere)
	TMap<FGameplayTag, int32> DesignedCardDeck;
};
