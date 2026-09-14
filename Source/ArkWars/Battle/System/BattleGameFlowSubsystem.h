// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArkWars/Battle/Toolkits/ArkWarDelegates.h"
#include "ArkWars/Battle/Toolkits/GameMessage.h"
#include "ArkWars/Battle/Transaction/EventTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "BattleGameFlowSubsystem.generated.h"


class UBattleTransaction;
class ICardContainerInterface;
class ACardTableManager;
class UGamePhaseManagerComponent;
/**
 * 
 */



UCLASS()
class ARKWARS_API UBattleGameFlowSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
	UPROPERTY()
	TScriptInterface<ICardContainerInterface> CardManager;
	
	UPROPERTY()
	TArray<UBattleTransaction*> PendingTransactions;
	
	TWeakObjectPtr<UBattleTransaction> ActiveTransaction;
	
	/** 仅服务器存在，FIFO */
	TArray<GameMessage::FPhaseMessage> PhaseMsgQueue;

public:
	///	子系统通用
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	///	返回当前子系统是否在服务器上运行
	bool IsRunningOnServer() const;
	
	///	子管理模块获取接口
	
	/**	获取牌桌管理器接口实例 */
	ICardContainerInterface* GetCardManager() const;
	/**	获取阶段管理器	*/
	UGamePhaseManagerComponent* GetPhaseManager() const;

	///	阶段管理方法

	/**
	 *	获取特定玩家的下标
	 *	@param Player			需要查询下标的玩家，若为空，则视为当前回合的行动者
	 *	@return					该玩家的下标
	 */
	int32 GetPlayerIndex(APlayerState* Player = nullptr) const;
	
	/**
	 *	将卡牌管理器注册至子系统
	 *	@param Mgr				待注册的卡牌管理器，若已存在管理器则忽略
	 */
	void RegisterManagerActor(ICardContainerInterface* Mgr);

	/**
	 *	插入延时性阶段切换修正信息，仅无参Advance方法调用时会查询且生效前不能被移除，先入先出
	 *	@param Msg				将要插入的信息，会被解码成特定的结构储存
	 */
	void PushPhaseResolvation(const FString& Msg);

	/**
	 *	通过已传入的修正信息队列，修正阶段信息
	 *	@param Current			当前阶段
	 *	@return					被修正后输出的阶段，每次只执行一个满足的修正信息
	 */
	EGamePhase ResolveNextPhase(EGamePhase Current);

	/**
	 *	指定玩家跳转到特定阶段，无视所有Message的修正
	 *	@param Phase			要跳转至的阶段
	 *	@param Player			获得下一个阶段的行动权的玩家, 默认为当前玩家
	 */
	void Advance(EGamePhase Phase, int32 Player = INDEX_NONE) const;

	/**	进入下一个阶段，优先执行Message的修正 */
	void Advance();

	/**	启动游戏，依次执行：身份分配、发牌、进入游戏开始阶段并广播 */
	void StartGame();
	
	///事实通知（A 面）：卡牌移动落成后的"已移动"事实，广播者 = 族钩子
	///UCardMoveTransaction::BroadcastFinish（订阅方只读；广播旁路，回调异常不影响交易推进，卷 12 §7）
	FCardMovedDelegate OnCardMoved;

	///子阶段内交易行为管理
	
	/**	
	 *	将一个新建的交易实例加入队列等待执行
	 *	@param Tx			将要入队的交易实例
	 */
	void EnqueueTransaction(UBattleTransaction* Tx);

	/**	执行队头的交易实例，并移除 */
	void DriveTransaction();
	
	/** 当交易实例被执行后，广播并准备执行下一条 */
	void OnTransactionFinished(UBattleTransaction* Tx);

	///	查询窗口管理（P3：单层窗口 + InOrder 最小策略，卷 03 §4 / 卷 10 §2 / P3 §3.2）

	/**
	 *	唯一开窗入口：阶段机（Advance 遇 _Pre/_Post）与交易驱动（QueryTimings）共用，杜绝第二套窗口实现。
	 *	流程 = 查时机索引 → 合法预检 → 填 Responders → 广播 FWindowOpened → 上锁询问（InOrder 逐人）→ 关窗回调。
	 *	推进完全事件驱动（响应提交/放弃驱动游标），引擎不轮询、不阻塞等待。
	 *	@param EventTag		本次窗口的查询时机（Event.Timing.*，卷 03 §3.1）
	 *	@param Tx			窗口负载 = 交易引用（响应者从它拿 Tx 调 AppendModification）；纯阶段时机传 nullptr
	 *	@param Policy		窗口策略（开窗参数，不进入 FResponseWindow 数据形状，卷 10 §2）；P3 只实现 InOrder
	 */
	void OpenTimingWindow(const FGameplayTag& EventTag, UBattleTransaction* Tx, EWindowPolicy Policy);

	/**
	 *	窗口内响应提交（未来 ServerRPC 的服务端落点）：
	 *	二次校验 = 窗口在场 + State.Responding 锁内 + 提交者在名单内且正被询问 → Tx->AppendModification(Req) → InOrder 推进下一位
	 */
	void SubmitWindowResponse(APlayerState* Responder, const FTransactionModRequest& Req);

	/** 窗口内放弃（未来 ServerRPC 的服务端落点）：校验当前被询问者后直接推进下一位 */
	void DeclineWindowResponse(APlayerState* Responder);

	///	C 面事件化委托实例（声明见 ArkWarDelegates.h，广播点登记见卷 12 §5.2）：
	///	服务器侧观察点，订阅者仅日志/表现/调试——广播是旁路，回调异常不得影响推进
	FWindowOpenedDelegate OnWindowOpened;
	FWindowClosedDelegate OnWindowClosed;
	FTransactionModifiedDelegate OnTransactionModified;

private:
	///	窗口私有状态（P3 单层窗口：多轮链/响应链嵌套留 P7，卷 10 §3）

	TOptional<FResponseWindow> ActiveWindow;					//	当前悬停窗口（TStrongObjectPtr 强持有 Tx，防窗口悬停期被 GC）
	EWindowPolicy ActivePolicy = EWindowPolicy::InOrder;		//	本窗策略（随开窗参数存，不进窗口形状）
	int32 ResponderCursor = 0;									//	InOrder 询问游标：指向"当前正被询问"的响应者
	bool bAnyResponded = false;									//	本窗是否有人响应过（FWindowClosed 广播负载）
	bool bRespondingLock = false;								//	State.Responding 锁：窗口期阻断无关输入，关窗撤锁（P3 §3.3）

	/** ①②③ 查时机索引 → 合法预检 → 排序填名单（卷 12 §4.1）；索引未建时返回空表 = "无监听直通"（P3 §7） */
	TArray<FResponseEntry> CollectResponders(const FGameplayTag& TimingTag, UBattleTransaction* Tx) const;

	/** InOrder 推进：询问游标处响应者（下发候选），问完全员 → 关窗 */
	void AskNextResponder();

	/** 关窗唯一收口：撤锁 → 复位窗口 → 广播 FWindowClosed → 回调持有者（交易 OnWindowClosed / 阶段机 Advance），任何路径不悬挂 */
	void CloseTimingWindow();

};

