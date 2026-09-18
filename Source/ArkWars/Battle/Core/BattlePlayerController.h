// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerController.h"
#include "ArkWars/Battle/Toolkits/ArkWarDelegates.h"
#include "BattlePlayerController.generated.h"

struct FClientResponseWindow;
struct FWindowResponseRequest;
struct FArkCard;
class ICardContainerInterface;
/**
 * 
 */
UCLASS()
class ARKWARS_API ABattlePlayerController : public APlayerController
{
	GENERATED_BODY()
	
	FTimerHandle DeclineHandle;
public:
	
	///Try前缀均为UI接口，内部转发Server RPC调用


	///	=====================  选角  =====================

	/**
	 *	选定干员（UI 入口）：纯转发 Server RPC，不做任何裁决（零信任，卷 11 §5.2）
	 *	@param Operator	选中的干员档案键（= 干员名，与干员表行键同源）
	 */
	void TrySelectOperator(const FName& Operator);

	///	客户端侧候选下发（定向，非观察点，声明见 ArkWarDelegates.h）：
	///	订阅者 = 本玩家的 UI（展开选角界面）。
	///	宿主是 PC 而非 PlayerState——同 OnWindowResponseRequested：定向消息挂个体；
	///	且 UI 的网关只有 PC 一个（Try* 入口皆在此），PlayerState 只留业务，不留收发口
	FOperatorSelectRequestedDelegate OnOperatorSelectRequested;


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
	void TrySelectCard(const FArkCard& Card) const;
	/**
	 *	主动使用卡牌
	 *	@param Card		待使用的卡牌
	 */
	void TryUseCard(const FArkCard& Card);
	/**
	 *	移动卡牌，可定义来源与去向，装备置入、弃牌等操作均由此触发
	 *	@param Card		待移动的卡牌
	 *	@param From		卡牌来源
	 *	@param To		卡牌去向
	 *	@param Msg		移动额外信息
	 */
	void TryMoveCard(const TArray<FArkCard>& Cards, ICardContainerInterface* From, ICardContainerInterface* To, const FString& Msg);
	/**
	 *	响应他人使用卡牌的效果
	 *	@param Target	响应的来源玩家
	 *	@param Source	需要响应的卡牌
	 *	@param Card		用于响应的卡牌
	 */
	void TryResponse(APlayerState* Target, const FArkCard& Source, const FArkCard& Card);
	/**
	 *	展示多张卡牌
	 *	@param Cards	待展示的卡牌
	 */
	void TryShowCard(const TArray<FArkCard>& Cards);
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

	///	=====================  响应窗口  =====================

	/**
	 *	窗口内响应提交（UI 入口）：纯转发 Server RPC，不做任何裁决（零信任，卷 11 §5.2）
	 *	@param Req	上行意图 DTO：窗口句柄 + 所选技能
	 */
	void TrySubmitWindowResponse(const FWindowResponseRequest& Req);

	/**
	 *	窗口内放弃（UI 入口）：UI 倒计时到点也走本入口（超时 = 直接请求关闭，卷 10 §4）
	 *	@param WindowSerial	窗口句柄
	 */
	void TryDeclineWindowResponse(int32 WindowSerial);

	///	客户端侧窗口下发（定向，非观察点，声明见 ArkWarDelegates.h）：
	///	订阅者 = 本玩家的 UI（展开面板 + 自起倒计时）与需要表现的技能组件。
	///	宿主是 PC 而非子系统——这条消息只属于"被询问的这个人"：定向消息挂个体，
	///	全局观察点（Subsystem::OnWindowOpened）挂世界级对象，两者受众与形状都不同
	FWindowResponseRequestedDelegate OnWindowResponseRequested;


private:
	///===================== RPC =====================

	UFUNCTION(Server, Reliable)
	void Server_SelectOperator(const FName& Operator);

	UFUNCTION(Server, Reliable)
	void Server_UseCard(APlayerState* Source, const TArray<APlayerState*>& Targets, const FArkCard& Card);
	
	UFUNCTION(Server, Reliable)
	void Server_MoveCard(const TArray<FArkCard>& Cards, const TScriptInterface<ICardContainerInterface>& From, const TScriptInterface<ICardContainerInterface>& To, const FString& Msg);
	
	UFUNCTION(Server, Reliable)
	void Server_Response(APlayerState* Target, const FArkCard& Source, const FArkCard& Card);
	
	UFUNCTION(Server, Reliable)
	void Server_ShowCard(const TArray<FArkCard>& Cards);
	
	UFUNCTION(Server, Reliable)
	void Server_StartComparison(const TArray<APlayerState*>& Targets);
	
	UFUNCTION(Server, Reliable)
	void Server_ResponseComparison(APlayerState* Target);
	
	UFUNCTION(Server, Reliable)
	void Server_ConfirmComparison(const FArkCard& Card, bool bIsInitiator);
	
	UFUNCTION(Server, Reliable)
	void Server_SubmitWindowResponse(const FWindowResponseRequest& Req);
	
	UFUNCTION(Server, Reliable)
	void Server_DeclineWindowResponse(int32 WindowSerial);
	
public:
	UFUNCTION(Client, Reliable)
	void Client_OpenResponseWindow(const FClientResponseWindow& Window);

	///	候选名单下发（服务器侧调用，由模式组件在选角阶段发出）：收件人 = 该 PC 的属主玩家
	UFUNCTION(Client, Reliable)
	void Client_NotifySelectOperator(const TArray<FName>& OperatorList);
};
