// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArkWars/Battle/Toolkits/ArkWarDelegates.h"
#include "ArkWars/Battle/Toolkits/GameMessage.h"
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

};

