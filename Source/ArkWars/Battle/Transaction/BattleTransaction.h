// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EventTypes.h"
#include "UObject/Object.h"
#include "BattleTransaction.generated.h"

class APlayerState;
class UBattleGameFlowSubsystem;
/**
 *
 */
UCLASS(Abstract)
class ARKWARS_API UBattleTransaction : public UObject
{
	GENERATED_BODY()
public:
	enum class EState : uint8 {Spawned, Started, Querying, Executed, Finished };
	
	void Start();
	/* ======= ↓↓↓↓ ======= */
	/**
	 *	驱动器唯一入口（非虚）：由 UBattleGameFlowSubsystem::DriveTransaction 调用。
	 *	直调 QueryTimings()（开窗链入口）——入口名与调用方不变，
	 *	族钩子保持 protected、驱动器不再直接触碰族钩子（卷 04 §2/§3）。
	 */
private:
	void Drive();
	/* ======= ↓↓↓↓ ======= */
	void QueryTimings();
	/* ======= ↓↓↓↓ ======= */
	void OnWindowClosed();
	/* ======= ↓↓↓↓ ======= */
	void TryExecute();
protected:
	/* ======= ↓↓↓↓ ======= */
	virtual void Execute() PURE_VIRTUAL(UBattleTransaction::Execute);
	/* ======= ↓↓↓↓ ======= */
	void Finish(bool bSuccess);

	///	族登记"在哪些时机被查询"（卷 04 §2 两件登记之一）：QueryTimings 按该清单开窗；
	///	空 = 直通族——不开窗直接 Execute（与窗口侧"无监听直通"同精神，P3 §7）
	virtual TArray<FGameplayTag> GetQueryTimingTags() const { return {}; }

	///	关窗回调唯一调用方 = 窗口管理器（C 面引擎直调钩子，外部不可订阅，卷 12 §5.1）
	friend class UBattleGameFlowSubsystem;

public:
	EState GetState() const { return State; };
	
	///	子类通过对应接口进行操作，规定使用Actor作为基类保证网络复制功能存在
	TWeakObjectPtr<AActor> Instigator = nullptr;
	TArray<TWeakObjectPtr<AActor>> Targets = {};

	void AppendModification(const FTransactionModRequest& Req);
protected:
	virtual void FoldMods() PURE_VIRTUAL(UBattleTransaction::FoldMods);
	
	virtual bool Validate() const PURE_VIRTUAL(UBattleTransaction::Validate, return false;);
	
	virtual void BroadcastFinish(bool bSuccess) PURE_VIRTUAL(UBattleTransaction::BroadcastFinish);
	
	EState State = EState::Spawned;
	
	TArray<FTransactionModRequest> ModeRequests = {};

	///	查询时机游标：GetQueryTimingTags() 清单已开到第几扇窗。
	///	QueryTimings 开首窗，OnWindowClosed 关一扇推一格、问完才 Execute（逐个开，P3 §3.4）
	int32 QueryTimingCursor = 0;
};

//	↓↓↓ 以下两个形状引用交易本体（TStrongObjectPtr<UBattleTransaction>），故必须落在 UBattleTransaction
//	定义之后：TStrongObjectPtr 的构造要求类型完整（static_assert），这与"谁持有它"无关，是类型系统的账
//
//	不留在原位 EventTypes.h——硬约束而非偏好：该头带 USTRUCT，UHT 为它生成 EventTypes.gen.cpp，
//	而那个生成单元【只 include EventTypes.h 自己】（不借任何邻居）；于是头里任何需要外部完整类型的
//	内联代码都无处借力——FNestedTransactionFrame 的构造正是内联在此——改一次 EventTypes.h 即编译失败。
//	补 include 治不了：反向成环（本头已 include EventTypes.h）
//
//	也不另立 WindowTypes.h、不挪进 ArkWarFlowTypes.h——技术上两者都可行（后者 include 本头即可、不成环），
//	代价却是把"完整类型"扩散给该头的全体使用者：凡是 include 流程类型的 TU 都要跟着背上整个
//	UBattleTransaction。而本结构的消费者当前只有两处——UBattleGameFlowSubsystem（持
//	TOptional<FResponseWindow> ActiveWindow）与 ArkWarDelegates.h 的只读委托签名（那里前置声明即可）。
//	放这里，代价只由真正构造它的 TU 付。且它本身是窗口机制的私有状态（纯 server 私产）、不是通用流程类型，
//	归交易的头恰与"契约归宿主"一致——用了交易本体的形状，就由交易的头提供定义

//	窗口基座（纯 server 私产，无复制语义，故为普通结构体）
struct FResponseWindow
{
	//	窗口句柄：开窗时自增取值（服务器单调递增、StartGame 归零 = 一局一句柄空间），随候选下发、
	//	随提交/放弃回传比对——防陈旧/重放（卷 11 §5.2）：上一扇窗迟到的"超时放弃"不得作用于本窗。
	//	恒 ≥ 1（0 是"未填"哨兵：任何合法句柄都不等于它）；只比对"是否等于当前窗"，不承担鉴权，故无需不可猜测
	int32 WindowSerial = 0;

	FGameplayTag Timing;

	//	负载本体 + 悬停期强持有：交易出队后 PendingTransactions 不再持有它，
	//	这里是窗口悬停期唯一的 GC 锚（TStrongObjectPtr 强引用，不依赖反射追踪）；纯阶段时机为空
	TStrongObjectPtr<UBattleTransaction> Tx;

	TArray<FResponseEntry> Responders;

	float Timeout;
};

//	嵌套交易帧（卷 04 §3.2）：父/子必须同为强引用——换手后父的强引用只剩本帧，
//	弱引用会让父在子交易悬停期被 GC，出栈即悬空（不变量 1）
struct FNestedTransactionFrame
{
	TStrongObjectPtr<UBattleTransaction> Parent;
	TStrongObjectPtr<UBattleTransaction> Child;

	FNestedTransactionFrame() : Parent(nullptr), Child(nullptr) {}
	FNestedTransactionFrame(UBattleTransaction* Parent, UBattleTransaction* Child) : Parent(TStrongObjectPtr(Parent)), Child(TStrongObjectPtr(Child)) {}
};
