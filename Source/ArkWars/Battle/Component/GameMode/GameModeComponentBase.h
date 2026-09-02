// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArkWars/Battle/Toolkits/ArkWarTypes.h"
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

	/**	用于切换状态，若Msg为空时按默认形式切换
	 *	
	 *	@param Msg		需要切换为特定阶段等需求时传入Msg实现
	 */
	UFUNCTION(BlueprintCallable)
	virtual void ChangeGamePhase(const FString& Msg);
	
	
private:
	
	/**	特定玩家请求卡牌后分发至Client的实现
	 * 
	 *	@param Player	请求者的玩家控制器，用于区分请求的来源，不代表获得者
	 *	@param Cards	请求的结果，只负责服务端的消耗与下发，不负责分配
	 */
	void BroadcastCardToPlayer(APlayerState* Player, const TArray<FGameplayTagContainer>& Cards);
	
	void ClearDiscardCache();
	
};


