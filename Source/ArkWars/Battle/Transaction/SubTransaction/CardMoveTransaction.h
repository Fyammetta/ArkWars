// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArkWars/Battle/Transaction/BattleTransaction.h"
#include "CardMoveTransaction.generated.h"


class ICardContainerInterface;
struct FArkCard;

namespace GameMessage { struct FMoveMessage; }
UCLASS()
class ARKWARS_API UCardMoveTransaction : public UBattleTransaction
{
	GENERATED_BODY()
	TUniquePtr<GameMessage::FMoveMessage> Message;                // 解析后的意图（From/To/Num/Predicate/Order）
	TArray<FArkCard> Cards;			// 本次移动的牌：预选牌入参 / Execute 后回写为实际取出的牌
	
public:
	virtual void FoldMods() override;			// 修正折叠（公开族钩子，供族 Execute 开头调；移动族为直通空实现）

protected:
	virtual void Execute() override;			// 交易主体：校验 → 源区取出 → 目标区写入 → 收尾（受保护族钩子，驱动器经基类 Drive() 进入）
	virtual bool Validate() const override;				// 入参校验：Instigator / Targets / 消息合法性
	virtual void BroadcastFinish(bool bSuccess) override;	// 收尾广播：落子成功即广播移动事实（Notify::Card::Moved）

public:
	///工厂：解析消息串得到移动意图，并预置本次要移动的牌（CardsToMove 为空则走 DSL 取序）
	static UCardMoveTransaction* Create(const FString& Msg, const TArray<FArkCard>& CardsToMove = {});
	
private:
	void HandleMovement_Selected(ICardContainerInterface* From);	// 源侧取牌：按预置牌实体定位下标
	void HandleMovement(ICardContainerInterface* From);				// 源侧取牌：按消息 DSL（Select）定位
};
