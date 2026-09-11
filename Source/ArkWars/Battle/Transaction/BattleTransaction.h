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
protected:
	/* ======= ↓↓↓↓ ======= */
	/**
	 *	驱动器唯一入口（非虚）：由 UBattleGameFlowSubsystem::DriveTransaction 调用。
	 *	当前直调族钩子 Execute()；P3 起改调 QueryTimings()（开窗链入口）——入口名与调用方不变，
	 *	族钩子保持 protected、驱动器不再直接触碰族钩子（卷 04 §2/§3）。
	 */
	void Drive();

	/* ======= ↓↓↓↓ ======= */
	virtual void Execute() PURE_VIRTUAL(UBattleTransaction::Execute);
	/* ======= ↓↓↓↓ ======= */
	void QueryTimings();
	/* ======= ↓↓↓↓ ======= */
	void OnWindowClosed();
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
	virtual void FoldMods() PURE_VIRTUAL(UBattleTransaction::FoldMods)

	
	virtual bool Validate() const PURE_VIRTUAL(UBattleTransaction::Validate, return false;);
	
	virtual void BroadcastFinish(bool bSuccess) PURE_VIRTUAL(UBattleTransaction::BroadcastFinish);
	
	EState State = EState::Spawned;
	
	TArray<FTransactionModRequest> ModeRequests = {};

	///	查询时机游标：GetQueryTimingTags() 清单已开到第几扇窗。
	///	QueryTimings 开首窗，OnWindowClosed 关一扇推一格、问完才 Execute（逐个开，P3 §3.4）
	int32 QueryTimingCursor = 0;
};
