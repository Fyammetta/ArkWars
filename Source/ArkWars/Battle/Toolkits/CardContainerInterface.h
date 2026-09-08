// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArkWarCardTypes.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "CardContainerInterface.generated.h"
struct FArkCard;
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
	///Read Only
	
	virtual TArray<FArkCard> GetCardsByKey(const FGameplayTag& Area) const = 0;
	
	virtual const FArkCard* GetCardById(int32 CardId) const = 0;
	
	virtual TArray<int32> Select(const FGameplayTag& Area, const FString& Msg) const = 0;
	
	///Write
	
	[[nodiscard]] virtual TArray<FArkCard> Consume(const FGameplayTag& Area, const TArray<int32>& CardIds) = 0;
	
	virtual EAreaWriteResult Add(const FGameplayTag& AreaKey, TArray<FArkCard>&& Cards, const FString& Msg) = 0;
	
	virtual TArray<FGameplayTag> GetAreaKeys() const = 0;
	
	virtual AActor* GetContainerActor() = 0;
};
