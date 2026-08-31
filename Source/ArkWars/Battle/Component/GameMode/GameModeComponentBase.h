// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArkWars/Battle/Toolkits/ArkWarTypes.h"
#include "Components/ActorComponent.h"
#include "GameModeComponentBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FChangePhaseDelegate);

class FChangePhaseDelegatePair
{
	FChangePhaseDelegate Delegates[3];
public:
	enum Type { PreBegin, PostBegin, End};
	
	FChangePhaseDelegate& Get(Type T){ return Delegates[T]; }
};

UCLASS(Abstract ,ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARKWARS_API UGameModeComponentBase : public UActorComponent
{
	GENERATED_BODY()
	
	TArray<FGameplayTagContainer> DiscardCache;
	
	UPROPERTY()
	TArray<APlayerState*> Players;
	
	int32 ActorIndex;
	
	EGamePhase::Type CurrentPhase;
	
	TArray<FChangePhaseDelegatePair> PhaseDelegates;
public:
	
	FChangePhaseDelegate& GetPhaseChangeDelegate(EGamePhase::Type Phase, FChangePhaseDelegatePair::Type Timing);
	
	UGameModeComponentBase();
	
	/**	开始游戏时，根据玩法配置Subsystem中的牌堆 */
	UFUNCTION(BlueprintCallable)
	virtual void InitCardDeck();
	
		
	/**	开始游戏时，根据玩法和人数为玩家分配身份 */
	UFUNCTION(BlueprintCallable)
	virtual void AllocateIdentity();
	
	/**	供特定玩家请求卡牌, 只进行服务器端的卡牌消耗，不负责分配到特定玩家
	 *	
	 *	@param Player	请求者的玩家控制器，用于区分请求的来源，不代表获得者
	 *	@param Msg		通过Msg传入具体的需求例如 "[Num]=2" 则为请求两张
	 */
	UFUNCTION(BlueprintCallable)
	virtual void RequestCard(APlayerState* Player, const FString& Msg);
	
	/**	供特定玩家弃牌，仅将牌弃至缓冲区，阶段结束后才会真正弃置
	 *	
	 *	@param Player	弃牌者的玩家控制器
	 *	@param Cards	被舍弃的牌
	 *	@param Msg		通过Msg传入具体的弃牌情况例如 "[End]" 则回合结束弃牌
	 */
	UFUNCTION(BlueprintCallable)
	virtual void Discard(APlayerState* Player, const TArray<FGameplayTagContainer>& Cards, const FString& Msg);
	
	/**	供特定玩家弃牌，仅将牌弃至缓冲区，阶段结束后才会真正弃置
	 *	
	 *	@param Msg		需要切换为特定阶段等需求时传入Msg实现
	 */
	UFUNCTION(BlueprintCallable)
	virtual void ChangeGamePhase(const FString& Msg);
	
	UFUNCTION(BlueprintCallable)
	APlayerState* GetStageOwner() const;
	
	
private:
	
	/**	特定玩家请求卡牌后分发至Client的实现
	 * 
	 *	@param Player	请求者的玩家控制器，用于区分请求的来源，不代表获得者
	 *	@param Cards	请求的结果，只负责服务端的消耗与下发，不负责分配
	 */
	void BroadcastCardToPlayer(APlayerState* Player, const TArray<FGameplayTagContainer>& Cards);
	
	void ClearDiscardCache();
	
};


