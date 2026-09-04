// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameModeComponentBase.generated.h"

class UAbilitySystemComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARKWARS_API UGameModeComponentBase : public UActorComponent
{
	GENERATED_BODY()
public:
	static UGameModeComponentBase* GetCurrentGameMode(UObject* WorldContextObject);
	
	/**	开始游戏时，根据玩法配置Subsystem中的牌堆 */
	UFUNCTION(BlueprintCallable)
	virtual void InitCardDeck();
	
	/**	开始游戏时，根据玩法和人数为玩家分配身份 */
	UFUNCTION(BlueprintCallable)
	virtual void AllocateIdentity();
	
	UFUNCTION(BlueprintCallable)
	virtual int32 GetStartCardNum(UAbilitySystemComponent* Asc = nullptr);
};


