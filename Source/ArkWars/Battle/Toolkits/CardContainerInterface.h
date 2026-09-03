// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "CardContainerInterface.generated.h"
// This class does not need to be modified.
UINTERFACE()
class UCardContainerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARKWARS_API ICardContainerInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void MoveIn(ICardContainerInterface* From, TArray<FGameplayTagContainer>&& Cards, const FString& Msg) = 0;
	virtual void MoveOut(ICardContainerInterface* To, const TArray<FGameplayTagContainer>& Cards, const FString& Msg) = 0;
};
