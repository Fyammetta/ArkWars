// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "ArkWars/Battle/Toolkits/CardContainerInterface.h"
#include "GameFramework/PlayerState.h"
#include "BattlePlayerState.generated.h"

/**
 * 
 */
UCLASS()
class ARKWARS_API ABattlePlayerState : public APlayerState, public IAbilitySystemInterface, public ICardContainerInterface
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> Asc;
	

public:
	ABattlePlayerState();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return Asc; };

	virtual TArray<FArkCard> GetCardsByKey(const FGameplayTag& Area) const override;
	virtual const FArkCard* GetCardById(int32 CardId) const override;
	virtual TArray<int32> Select(const FGameplayTag& Area, const FMessageType& Msg) const override;
	[[nodiscard]] virtual TArray<FArkCard> Consume(const FGameplayTag& Area, const TArray<int32>& CardIds) override;
	virtual EAreaWriteResult Add(const FGameplayTag& AreaKey, TArray<FArkCard>& Cards, const FMessageType& Msg) override;
	virtual TArray<FGameplayTag> GetAreaKeys() const override;
	virtual AActor* GetContainerActor() override { return this;};

protected:

	virtual void BeginPlay() override;
};
