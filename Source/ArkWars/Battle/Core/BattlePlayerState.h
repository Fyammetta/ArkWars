// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "PlayerOperatorInterface.h"
#include "ArkWars/Battle/Toolkits/CardContainerInterface.h"
#include "GameFramework/PlayerState.h"
#include "BattlePlayerState.generated.h"

/**
 * 
 */
UCLASS()
class ARKWARS_API ABattlePlayerState : public APlayerState, public IAbilitySystemInterface, public ICardContainerInterface, public IPlayerOperatorInterface
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

	virtual void NotifySelectOperator(const TArray<FName>& OperatorList) override;
	virtual void OnOperatorSelected(const FName& Operator) override;

protected:
	virtual void BeginPlay() override;
	
private:
	UFUNCTION(Server, Reliable)
	void Server_OnOperatorSelected(const FName& Operator);
	
	UFUNCTION(Client, Reliable)
	void Client_NotifySelectOperator(const TArray<FName>& OperatorList);

	
};
