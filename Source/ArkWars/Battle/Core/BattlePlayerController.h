// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerController.h"
#include "BattlePlayerController.generated.h"

class ICardContainerInterface;
/**
 * 
 */
UCLASS()
class ARKWARS_API ABattlePlayerController : public APlayerController
{
	GENERATED_BODY()
	

public:
	
	///Try前缀均为UI接口，内部转发Server RPC调用


	///	=====================  回合  =====================
	
	/**
	 *	结束当前阶段
	 */
	void TryEndPhase();
	///	=====================  卡牌  =====================

	
	/**
	 *	选中卡牌,单次选中一张，可累加
	 *	@param Card		单次选中的卡牌
	 */
	void TrySelectCard(const FGameplayTagContainer& Card) const;
	/**
	 *	主动使用卡牌
	 *	@param Card		待使用的卡牌
	 */
	void TryUseCard(const FGameplayTagContainer& Card);
	/**
	 *	移动卡牌，可定义来源与去向，装备置入、弃牌等操作均由此触发
	 *	@param Card		待移动的卡牌
	 *	@param From		卡牌来源
	 *	@param To		卡牌去向
	 *	@param Msg		移动额外信息
	 */
	void TryMoveCard(const TArray<FGameplayTagContainer>& Cards, ICardContainerInterface* From, ICardContainerInterface* To, const FString& Msg);
	/**
	 *	响应他人使用卡牌的效果
	 *	@param Target	响应的来源玩家
	 *	@param Source	需要响应的卡牌
	 *	@param Card		用于响应的卡牌
	 */
	void TryResponse(APlayerState* Target, const FGameplayTagContainer& Source, const FGameplayTagContainer& Card);
	/**
	 *	展示多张卡牌
	 *	@param Cards	待展示的卡牌
	 */
	void TryShowCard(const TArray<FGameplayTagContainer>& Cards);
	/**
	 *	对多名目标发起拼点
	 *	@param Targets	拼点的目标
	 */
	void TryStartComparison(const TArray<APlayerState*>& Targets);
	/**
	 *	接收某一玩家的拼点
	 *	@param Target	拼点的目标
	 */
	void TryResponseComparison(APlayerState* Target);
	/**
	 *	确认在非使用、移动或响应的情况下确认卡牌、技能等操作(如拼点暗置、技能触发等)
	 *	@param CustomEvent	确认后触发的事件
	 */
	void TryConfirm(const TFunction<void(APlayerController*)>& CustomEvent);
	
	///	=====================  技能  =====================

	/**
	 *	触发技能
	 *	@param SkillTag		技能标签，用于触发对应的技能
	 */
	void TryActivateSkill(const FGameplayTag& SkillTag);

	
private:
	///===================== RPC =====================

	UFUNCTION(Server, Reliable)
	void Server_UseCard(APlayerState* Source, const TArray<APlayerState*>& Targets, const FGameplayTagContainer& Card);
	
	UFUNCTION(Server, Reliable)
	void Server_MoveCard(const TArray<FGameplayTagContainer>& Cards, const TScriptInterface<ICardContainerInterface>& From, const TScriptInterface<ICardContainerInterface>& To, const FString& Msg);
	
	UFUNCTION(Server, Reliable)
	void Server_Response(APlayerState* Target, const FGameplayTagContainer& Source, const FGameplayTagContainer& Card);
	
	UFUNCTION(Server, Reliable)
	void Server_ShowCard(const TArray<FGameplayTagContainer>& Cards);
	
	UFUNCTION(Server, Reliable)
	void Server_StartComparison(const TArray<APlayerState*>& Targets);
	
	UFUNCTION(Server, Reliable)
	void Server_ResponseComparison(APlayerState* Target);
	
	UFUNCTION(Server, Reliable)
	void Server_ConfirmComparison(const FGameplayTagContainer& Card, bool bIsInitiator);
};
