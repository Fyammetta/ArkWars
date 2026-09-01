// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArkWars/Battle/Toolkits/CardContainerInterface.h"
#include "Subsystems/WorldSubsystem.h"
#include "CardPileManagerSubsystem.generated.h"

struct FGameplayTagContainer;
/**
 * 
 */
UCLASS()
class ARKWARS_API UCardPileManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	TArray<FGameplayTagContainer> DiscardCache;
	TArray<FGameplayTagContainer> Discard;
	TArray<FGameplayTagContainer> DrawCardsPile;
	

};
