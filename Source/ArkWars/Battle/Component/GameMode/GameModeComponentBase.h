// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameModeComponentBase.generated.h"

UCLASS(Abstract ,ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARKWARS_API UGameModeComponentBase : public UActorComponent
{
	GENERATED_BODY()
public:
	/**	开始游戏时，根据玩法配置Subsystem中的牌堆 */
	UFUNCTION(BlueprintCallable)
	virtual void InitCardDeck();
	
		
	/**	开始游戏时，根据玩法和人数为玩家分配身份 */
	UFUNCTION(BlueprintCallable)
	virtual void AllocateIdentity();
};


