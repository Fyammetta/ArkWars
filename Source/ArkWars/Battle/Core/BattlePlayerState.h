// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "ArkWars/Battle/Toolkits/CardContainerInterface.h"
#include "GameFramework/PlayerState.h"
#include "BattlePlayerState.generated.h"

struct FGameplayTagContainer;
class UGameplayEffect;
struct FGameplayTag;
struct FGameplayEventData;
/**
 * 
 */
UCLASS()
class ARKWARS_API ABattlePlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> Asc;
	

public:
	ABattlePlayerState();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return Asc; };
	

	
protected:

	virtual void BeginPlay() override;
};
