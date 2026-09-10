// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EventTypes.h"
#include "UObject/Object.h"
#include "BattleTransaction.generated.h"

class APlayerState;
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

	/**
	 *	驱动器唯一入口（非虚）：由 UBattleGameFlowSubsystem::DriveTransaction 调用。
	 *	当前直调族钩子 Execute()；P3 起改调 QueryTimings()（开窗链入口）——入口名与调用方不变，
	 *	族钩子保持 protected、驱动器不再直接触碰族钩子（卷 04 §2/§3）。
	 */
	void Drive();

protected:
	/* ======= ↓↓↓↓ ======= */
	virtual void Execute() PURE_VIRTUAL(UBattleTransaction::Execute);
	/* ======= ↓↓↓↓ ======= */
	void QueryTimings();
	/* ======= ↓↓↓↓ ======= */
	void OnWindowClosed();
	/* ======= ↓↓↓↓ ======= */
	void Finish(bool bSuccess);
	
public:
	EState GetState() const { return State; };
	
	///	子类通过对应接口进行操作，规定使用Actor作为基类保证网络复制功能存在
	TWeakObjectPtr<AActor> Instigator = nullptr;
	TArray<TWeakObjectPtr<AActor>> Targets = {};

	void AppendModification(const FTransactionModRequest& Req);
	
	virtual void FoldMods() PURE_VIRTUAL(UBattleTransaction::FoldMods)
	
protected:
	virtual bool Validate() const PURE_VIRTUAL(UBattleTransaction::Validate, return false;);
	
	virtual void BroadcastFinish(bool bSuccess) PURE_VIRTUAL(UBattleTransaction::BroadcastFinish);
	
	EState State = EState::Spawned;
	
	TArray<FTransactionModRequest> ModeRequests = {};
};
