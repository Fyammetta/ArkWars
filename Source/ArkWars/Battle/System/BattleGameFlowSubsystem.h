// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArkWars/Battle/Toolkits/ArkWarDelegates.h"
#include "ArkWars/Battle/Toolkits/GameMessage.h"
//	带 BattleTransaction.h 而非 EventTypes.h：EventTypes.h 里 FNestedTransactionFrame 的构造会实例化
//	TStrongObjectPtr<UBattleTransaction>（要求类型完整），而它反向 include 不了定义该类的
//	BattleTransaction.h（后者已 include 它）。本头持 FResponseWindow/NestedStack 成员、是这组类型的宿主，
//	故由本头提供完整定义，不把 include 顺序契约甩给每个使用者（BattlePlayerController.cpp 曾因此编译失败）
#include "ArkWars/Battle/Transaction/BattleTransaction.h"
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
	TArray<FNestedTransactionFrame> NestedStack;
	
	///	当前结算者（忙锁 + GC 锚）：强持有且反射可见——交易出队后即由这里持有，存活不依赖调用栈；
	///	窗口的 TStrongObjectPtr 只是悬停期的第二重锚（EventTypes.h）
	///	嵌套（P5，卷 04 §3.2）：换手只发生在本字段——父挂起后改由 NestedStack 栈帧强持有，
	///	故栈帧的 Parent/Child 必须同为强引用，否则换手瞬间父/子裸奔，GC 一到即悬空
	UPROPERTY()
	TObjectPtr<UBattleTransaction> ActiveTransaction;
	
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

	/**
	 *	嵌套子交易的启动口（卷 04 §3.2；P3 §3.9 决策点定稿 = EnqueueNested，非 Start(bNested)）：
	 *	与 EnqueueTransaction 并列的第二条入队路径，唯一调用点 = PushNestedTransaction 换手之后。
	 *	子交易【不进 PendingTransactions】——它由父挂起换手后当场驱动，出栈时父原样回来；
	 *	若也塞进队列，父恢复之后队列会把它再驱动一次
	 *	@param Child	已挂起父、待启动的嵌套子交易
	 */
	void EnqueueNested(UBattleTransaction* Child);

	/**	执行队头的交易实例，并移除 */
	void DriveTransaction();
	
	/** 当交易实例被执行后，广播并准备执行下一条
	 *  @param bSuccess	本笔的结局：由 Finish(bSuccess) 透传而来——此前断在 BattleTransaction::Finish
	 *					（只传了 Tx），交易事件化委托要它，故补上这条通路 */
	void OnTransactionFinished(UBattleTransaction* Tx, bool bSuccess);

	///	查询窗口管理（P3：单层窗口 + 三策略骨架，卷 03 §4 / 卷 10 §2 / P3 §3.2）

	/**
	 *	唯一开窗入口：阶段机（Advance 遇 _Pre/_Post）与交易驱动（QueryTimings）共用，杜绝第二套窗口实现。
	 *	流程 = 查时机索引 → 合法预检 → 填 Responders → 广播 FWindowOpened → 上锁询问（发问侧三策略同形，
	 *	见 AskNextResponder）→ 本函数返回；此后的关窗与续行完全由回传（提交/放弃）驱动。
	 *	推进完全事件驱动：引擎不轮询、不阻塞等待。
	 *	@param EventTag		本次窗口的查询时机（Event.Timing.*，卷 03 §3.1）
	 *	@param Tx			窗口负载 = 交易引用（响应者从它拿 Tx 调 AppendModification）；纯阶段时机传 nullptr
	 *	@param Policy		窗口策略（开窗参数，不进入 FResponseWindow 数据形状，卷 10 §2）：三策略骨架已落，
	 *						差异只在"问几个人"与"何时收敛"——InOrder 已有真实推演，All/FirstOnly 待 P7 接线
	 */
	void OpenTimingWindow(const FGameplayTag& EventTag, UBattleTransaction* Tx, EWindowPolicy Policy);

	/**
	 *	窗口内响应提交（ServerRPC 的服务端落点，卷 11 §5.2）：
	 *	二次校验 = 窗口在场 + State.Responding 锁内 + 【句柄等于本窗】+ 提交者与所选技能双落在待回位
	 *	（授权粒度随策略分派，判法见 ResolvePendingIndex）→ 按定位到的条目取技能交其产出修改（OnRespond）
	 *	→ 销账，再按策略收敛（InOrder 推游标续问；All 等账平才关；FirstOnly 首响即关）。
	 *	【本函数是回传侧落点】：技能钩子（OnCanActivate）里的同步调用会构成重入，
	 *	由此产生的错位下发由发问侧的"重入守卫"拦截（见 AskNextResponder，钩子契约见卷 09 §10）
	 *	@param Responder	提交者：由调用方从连接推导，不接受客户端自报（零信任）
	 *	@param Req			上行意图 DTO——不是 ModRequest：修改内容由技能在服务端产出
	 */
	void SubmitWindowResponse(APlayerState* Responder, const FWindowResponseRequest& Req);

	/**
	 *	窗口内放弃（ServerRPC 的服务端落点）：与提交同一套校验、同一顺序（窗口在场 + 锁内 + 句柄 +
	 *	待回位），通过后销账并收敛。放弃不是"响应"、不构成首响抢占（卷 10 §2 抢的是响应，不是表态）——
	 *	同开类下其余人仍有响应权，两种策略在此都等账平；全员弃权同样关窗（账本清空即账平）
	 *	@param Responder	放弃者：由调用方从连接推导，不接受客户端自报（零信任）
	 *	@param WindowSerial	窗口句柄：须等于当前窗——UI 超时是自动发的，陈旧句柄必须挡在门外
	 */
	void DeclineWindowResponse(APlayerState* Responder, int32 WindowSerial);

	///	响应锁（State.Responding）单一收口（卷 10 §4）：
	///	P3 框架期 = 子系统 bool 闸门（服务器侧状态）；将来升级 GE 阻断（ASC + State.Responding
	///	GameplayTag）只换本组内部实现，调用方一律按 IsResponding() 查 → 零修改

	/**	当前是否处于响应中（窗口悬停期）——调用方判"无关输入是否被阻断"的唯一入口 */
	bool IsResponding() const { return bRespondingLock; }

	/**
	 *	尝试进入响应：置位并广播 FResponseLockChanged(true)
	 *	@return					false = 已在响应中（拒绝；锁状态与广播均不动），调用方据此拒绝 + 日志
	 */
	bool TryEnterResponseLock();

	/**	退出响应：撤锁并广播 FResponseLockChanged(false)；未上锁时为空操作（幂等） */
	void ExitResponseLock();

	///	C 面事件化委托实例（声明见 ArkWarDelegates.h，广播点登记见卷 12 §5.2）：
	///	服务器侧观察点，订阅者仅日志/表现/调试——广播是旁路，回调异常不得影响推进
	FWindowOpenedDelegate OnWindowOpened;
	FWindowClosedDelegate OnWindowClosed;
	FTransactionModifiedDelegate OnTransactionModified;
	FResponseLockChangedDelegate OnResponseLockChanged;
	FTransactionEnqueuedDelegate OnTransactionEnqueued;
	///	实例名带 Event 后缀是刻意的：OnTransactionFinished 已被上面的 C 面钩子函数占用，
	///	同名会遮蔽——钩子是引擎内部推进（直调），本委托是对外观察位，两者语义不同不可混
	FTransactionFinishedDelegate OnTransactionFinishedEvent;
	FTransactionNestedDelegate OnTransactionNested;
	
	///	嵌套插队口（卷 04 §3.2 / P3 §3.9）：挂起当前结算者、先完整结算 Child，再回到父。
	///	父就地取 ActiveTransaction——不由调用方传入（不变量 4：只有当前结算者能 Push，父的身份由子系统自证）
	///	@return		false = 被拒（防环 / 非活动上下文 / 空子 / 非服务器），ActiveTransaction 不变
	bool PushNestedTransaction(UBattleTransaction* Child);

	///	出栈换手（唯一调用点 = OnTransactionFinished 判栈顶命中）：弹栈 + ActiveTransaction 归父，
	///	不重发入队、不重发 Enqueued 事件（不变量 5）；父的续行点见卷 04 §3.2 决策项（P5）
	void PopNestedTransaction(UBattleTransaction* Child);

private:
	///	窗口私有状态（P3 单层窗口：多轮链/响应链嵌套留 P7，卷 10 §3）

	TOptional<FResponseWindow> ActiveWindow;					//	当前悬停窗口（TStrongObjectPtr 强持有 Tx，防窗口悬停期被 GC）
	EWindowPolicy ActivePolicy = EWindowPolicy::InOrder;		//	本窗策略（随开窗参数存，不进窗口形状）
	int32 ResponderCursor = 0;									//	InOrder 询问游标：指向"当前正被询问"的响应者
	TArray<bool> ResponsePending;								//	条目级待回账本：与 ActiveWindow->Responders 等长，
																//	true = 该条目已发问、尚无结局（响应/放弃/失联都算结局）。
																//	【条目】粒度而非人粒度：同一人的多个技能各占一条，同开类下各自独立决定。
																//	它的唯一职责是回答"这扇窗该不该关"——关窗条件是【账平】（发出几份 ↔ 收回几份），
																//	不是"发完了"：同开类若按"发完"关窗，响应窗口期为零（N2-b）
	bool bAnyResponded = false;									//	本窗是否有人响应过（FWindowClosed 广播负载）
	bool bRespondingLock = false;								//	State.Responding 锁本体：仅经 TryEnterResponseLock()/ExitResponseLock() 改写，查询走 IsResponding()（卷 10 §4）
	int32 WindowSerialCounter = 0;								//	窗口句柄发号器（仅服务器，单调递增；StartGame 归零 = 一局一句柄空间）：开窗取值，随候选下发、随提交/放弃回传比对（卷 11 §5.2 防陈旧/重放）。只增不减——窗口关闭不回收（发号即消耗）

	/** ①②③ 查时机索引 → 合法预检 → 排序填名单（卷 12 §4.1）；索引未建时返回空表 = "无监听直通"（P3 §7）
	 *  不带负载入参：名单与负载无关，交易引用由开窗方直接挂在窗口上（EventTypes.h） */
	TArray<FResponseEntry> CollectResponders(const FGameplayTag& TimingTag) const;

	/** 定位本次提交/放弃对应的待回条目——零信任下的"返回的是哪一条"（卷 11 §5.2）。
	 *  InOrder：游标处那一条——一次只问一人，条目由服务器记账推得，不采信客户端自报；
	 *           Key 亦须对上：同一人连占两条时，只认人会让同窗内一次陈旧重放顶替另一条的响应权。
	 *  All/FirstOnly：按 SkillTag 在【待回位】里查——一次问多人，服务端无从由身份推出是哪一条，
	 *                且须同时命中"该 Owner 名下"与"尚无结局"，否则拿别人的 Key 或重复提交都能销账。
	 *  @param SkillTag		放弃入口无技能参数（UI 对玩家只有一个放弃钮）：传空 Tag = 按人定位，
	 *						即该 Owner 名下全部待回条目视为一起放弃——分岔点见 .cpp 内 P7 标注
	 *  @return				INDEX_NONE = 不在待回位（越权 / 陈旧 / 已销账 / 非本窗响应者） */
	int32 ResolvePendingIndex(APlayerState* Responder, const FGameplayTag& SkillTag) const;

	/** 询问推进：按策略发问并记账（InOrder 问一个就停，All/FirstOnly 一次问全），
	 *  名单走完且【账平】→ 关窗（同开类走到"发完"只是送达完毕，不等于可以关）。
	 *  内含【重入守卫】（N7）：OnCanActivate 之后校"本层窗口是否还在"——钩子里同步推进（越权用法）
	 *  会递归到关窗、甚至换窗，本层余句随即作废（否则账本越界写 + 错位下发）；详见 .cpp 内注释 */
	void AskNextResponder();

	/** 关窗唯一收口：撤锁 → 复位窗口 → 广播 FWindowClosed → 回调持有者（交易 OnWindowClosed / 阶段机 Advance），任何路径不悬挂 */
	void CloseTimingWindow();

};

