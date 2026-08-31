// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArkWarTypes.h"
#include "UObject/Interface.h"
#include "CardContainerInterface.generated.h"

struct FGameplayTagContainer;
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
	virtual void MoveIn(ICardContainerInterface* From, TArray<TSharedPtr<FCard>>&& Cards) = 0;

};
