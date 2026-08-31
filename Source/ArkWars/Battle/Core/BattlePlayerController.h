// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArkWars/Battle/Toolkits/ArkWarTypes.h"
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
	void TrySelectCard(const FCard& Card);
	/**
	 *	主动使用卡牌
	 *	@param Card		待使用的卡牌
	 */
	void TryUseCard(const FCard& Card);
	/**
	 *	移动卡牌，可定义来源与去向，装备置入、弃牌等操作均由此触发
	 *	@param Card		待移动的卡牌
	 *	@param From		卡牌来源
	 *	@param To		卡牌去向, 为空时视为进入弃牌堆
	 */
	void TryMoveCard(const FCard& Card, ICardContainerInterface* From, ICardContainerInterface* To = nullptr);
	/**
	 *	响应他人使用卡牌的效果
	 *	@param Target	响应的来源玩家
	 *	@param Source	需要响应的卡牌
	 *	@param Card		用于响应的卡牌
	 */
	void TryResponse(APlayerState* Target, const FCard& Source, const FCard& Card);
	/**
	 *	展示多张卡牌
	 *	@param Cards	待展示的卡牌
	 */
	void TryShowCard(const TArray<FCard>& Cards);
	/**
	 *	对多名目标发起拼点
	 *	@param Targets	拼点的目标
	 */
	void TryStartComparison(TArray<APlayerState>* Targets);
	/**
	 *	接收某一玩家的拼点
	 *	@param Target	拼点的目标
	 */
	void TryResponseComparison(APlayerState* Target);
	/**
	 *	确认在非使用、移动或响应的情况下确认卡牌、技能等操作(如拼点暗置、技能触发等)
	 *	@param CustomEvent	确认后触发的事件,若为空,则默认为拼点确认
	 */
	void TryConfirm(const TFunction<void()>& CustomEvent = nullptr);
	
	///	=====================  技能  =====================

	/**
	 *	触发技能
	 *	@param SkillTag		技能标签，用于触发对应的技能
	 */
	void TryActivateSkill(const FGameplayTag& SkillTag);

	
private:
	///===================== RPC =====================

	UFUNCTION(Server, Reliable)
	void Server_UseCard(int32 Index);
	
	UFUNCTION(Server, Reliable)
	void Server_MoveCard(int32 Index, const TScriptInterface<ICardContainerInterface>& From, const TScriptInterface<ICardContainerInterface>& To = nullptr);
	
};
