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

public:
	TArray<FArkCard> Cards;			// 本次移动的牌：预选牌入参 / Execute 后回写为实际取出的牌

protected:
	virtual void FoldMods() override;

	///	移动族登记的查询时机：Event.Timing.Card.PreMove（卷 04 §3"在哪些时机被查询"）——
	///	Execute 前开查询窗，是窗口内修改 Num/From/To 的唯一干预点（P3 验收 §4.2）
	virtual TArray<FGameplayTag> GetQueryTimingTags() const override;

	virtual void Execute() override;			// 交易主体：校验 → 源区取出 → 目标区写入 → 收尾（受保护族钩子，驱动器经基类 Drive() 进入）
	virtual bool Validate() const override;				// 入参校验：Instigator / Targets / 消息合法性
	virtual void BroadcastFinish(bool bSuccess) override;	// 收尾广播：落子成功即广播移动事实（Notify::Card::Moved）

public:
	///工厂：解析消息串得到移动意图，并预置本次要移动的牌（CardsToMove 为空则走 DSL 取序）
	static UCardMoveTransaction* Create(const FString& Msg, const TArray<FArkCard>& CardsToMove = {});

	///	只读访问解析后的移动意图（预检 / 候选下发用）：只读侧不设窗口闸门，随时可看
	const GameMessage::FMoveMessage& GetMessage() const;

	///	结构干预点（卷 04 §4.1 结构直写通道）：仅 State == Querying 受理，窗外告警拒（与 AppendModification 同款闸门）。
	///	两条纪律守卫——违规字段还原原值 + 告警，Modifier 的其余合法结构修改仍然生效：
	///	① _Count 只走 ModRequest 折叠通道：数值位不开第二通道（双通道 = 并发写歧义、不可回放）；
	///	② 预置牌（Cards 非空）锁定 _From：改源区会让预置牌与源区脱钩（HandleMovement_Selected 查无牌
	///	  即走"直送不 Consume"分支，牌凭空落目标区而源区不扣）；DSL 取序（Cards 空）改 _From 合法——
	///	  Execute 会按新源区重新 Select。
	///	@return Modifier 是否被受理执行（窗外 / Message 非法则拒；守卫还原另行告警、不影响返回值）
	bool ModifyMessage(TFunctionRef<void(GameMessage::FMoveMessage&)> Modifier);

private:
	void HandleMovement_Selected(ICardContainerInterface* From);	// 源侧取牌：按预置牌实体定位下标
	void HandleMovement(ICardContainerInterface* From);				// 源侧取牌：按消息 DSL（Select）定位
	
};
