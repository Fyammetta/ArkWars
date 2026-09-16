// Fill out your copyright notice in the Description page of Project Settings.


#include "CardMoveTransaction.h"

#include "ArkWars/ArkWars.h"
#include "ArkWars/Battle/System/BattleGameFlowSubsystem.h"
#include "ArkWars/Battle/Toolkits/BattleFunctionLibrary.h"
#include "ArkWars/Battle/Toolkits/CardContainerInterface.h"
#include "ArkWars/Battle/Toolkits/GameMessage.h"
#include "ArkWars/Battle/Transaction/EventTags.h"


void UCardMoveTransaction::Execute()
{
	//① 入参校验在外部进行
	

	//② 预选牌（Cards 非空）按实体定位；否则按消息 DSL 取序
	bool bIsSelected = !Cards.IsEmpty();

	//折零 = "被防"（折零即被防）：DSL 路径折叠后一张不取即未发生移动——不落子、不广播 Moved 事实
	//（否则"被防"会被记成一次成功移动）；预置牌路径 _Count 不参与取序，不受此判
	if (!bIsSelected && Message->_Count <= 0) return Finish(false);
	
	//③ 折叠修正后解析两端容器（区域解析唯一入口，卷 08 §4 / P2 §2-F）
	auto From = UBattleFunctionLibrary::ResolveContainer(Instigator.Get(), Message->_From);
	auto To = UBattleFunctionLibrary::ResolveContainer(Targets[0].Get(), Message->_To);
	
	//④ 同容器同区自移无意义，直接判失败
	if (From == To && Message->_From == Message->_To)
		return Finish(false);
	
	
	//⑤ 源侧"移出"半跳：两条路径都会把实际取出的牌写回 Cards
	if (bIsSelected)
		HandleMovement_Selected(From);
	else
		HandleMovement(From);

	//⑥ 目标侧"移入"半跳：写入结果即交易结果（Accepted 之外一律判失败）
	switch (To->Add(Message->_To, Cards, *Message))
	{
		case EAreaWriteResult::Accepted:	return Finish(true);
		case EAreaWriteResult::Full:		return Finish(false);
		case EAreaWriteResult::Mismatch:	return Finish(false);
	}
}

TArray<FGameplayTag> UCardMoveTransaction::GetQueryTimingTags() const
{
	//	族登记（卷 04）：移动交易在落子前只开一个查询窗（PreMove）；
	//	基类 QueryTimings 会以此开 Tx = this 的窗口，策略 InOrder（P3 最小集）
	return { Timing::Card::PreMove };
}

//	折叠口径（U2 定稿）：全程 float 域累加，只在"_Count = 整数张数"这一个语义点上做一次
//	RoundToInt 回填——避免逐条截断的累积误差（两条 Add(-0.5)：float 域累出 -1，逐条截断得 0）。
//	Override 立即覆写并短路（其前累计抹除、其后修正全废），幅值同样过 RoundToInt（定稿甲：
//	与末次回填同一函数、同一语义点；负值落 _Count <= 0，与折零同归"被防"）；
//	预置牌（Cards 非空）不参与折叠。
void UCardMoveTransaction::FoldMods()
{
	
	if (!Cards.IsEmpty()) return;
	
	float Addition = 0.f;
	float Multiplication = 1.f;
	TOptional<float> ClampCap;
	
	for (const FTransactionModRequest& Req : ModeRequests)
	{
		switch (Req.Operation)
		{
			using EOP = FTransactionModRequest::EModOp;
			case EOP::Add:			Addition += Req.Magnitude; break;
			case EOP::Multiply:		Multiplication *= Req.Magnitude; break;
			case EOP::Clamp:		ClampCap = ClampCap.IsSet() ? FMath::Min(Req.Magnitude, ClampCap.GetValue()) : Req.Magnitude; break;
			case EOP::Override:		Message->_Count = FMath::RoundToInt(Req.Magnitude); return ModeRequests.Empty();
		}
	}
	
	ModeRequests.Empty();
	float Result = Message->_Count;
	(Result += Addition) *= Multiplication;
	
	if (ClampCap.IsSet())
		Result = FMath::Clamp(Result, 0, FMath::RoundToInt(ClampCap.GetValue()));
	
	Message->_Count = FMath::RoundToInt(Result);
}

bool UCardMoveTransaction::Validate() const
{
	//发起方必须有效且可解析为容器（此时 Message 尚未校验，区域参数按空标签传入）
	if (!Instigator.IsValid() || !UBattleFunctionLibrary::ResolveContainer(Instigator.Get(), FGameplayTag::EmptyTag) )
	{
		UE_LOG(LogCard, Warning, TEXT("[UCardMoveTransaction][Validate] Instigator %s is illegal"), (Instigator.IsValid() ? *Instigator->GetName() : TEXT("nullptr")));
		return false;
	}
	
	
	//目标必须恰好一个（当前仅支持单目标移动）
	if (Targets.Num() != 1) 	
	{
		UE_LOG(LogCard, Warning, TEXT("[UCardMoveTransaction][Validate] Targets count is not expected to be %d"), Targets.Num());
		return false;
	}
	//目标必须有效且可解析为容器
	if (!Targets[0].IsValid() || !UBattleFunctionLibrary::ResolveContainer(Targets[0].Get(), FGameplayTag::EmptyTag))
	{
		UE_LOG(LogCard, Warning, TEXT("[UCardMoveTransaction][Validate] Target %s is illegal"), (Targets[0].IsValid() ? *Targets[0]->GetName() : TEXT("nullptr")));
		return false;
	}
	
	
	
	//最后校验消息本身（工厂解析失败时 Message 为空或非法）
	return Message.IsValid() && Message->IsValid();
}

void UCardMoveTransaction::BroadcastFinish(bool bSuccess)
{
	//族事实广播位（卷 04 §2 / 卷 12 §5.1）：仅成功落子产生"已移动"事实；
	//失败 / 被防 / 同区短路均不落子，本族无对应通知标签，不广播（MovedIn / MovedOut 细拆待用）
	if (!bSuccess) return;

	auto System = UBattleFunctionLibrary::GetBattleManager(this);
	if (!System) return;

	//唯一事实：Notify::Card::Moved + 双端 Actor + 本次实际移动的牌（订阅方只读，旁路，卷 12 §7）
	System->OnCardMoved.Broadcast(Notify::Card::Moved, Instigator.Get(), Targets.IsEmpty() ? nullptr : Targets[0].Get(), Cards);
}

UCardMoveTransaction* UCardMoveTransaction::Create(const FString& Msg, const TArray<FArkCard>& CardsToMove)
{
	//解析消息串，并把预置牌（可为空）写入交易
	auto RetVal = NewObject<UCardMoveTransaction>();
	RetVal->Message = MakeUnique<GameMessage::FMoveMessage>(Msg);
	RetVal->Cards = CardsToMove;
	return RetVal;
}

const GameMessage::FMoveMessage& UCardMoveTransaction::GetMessage() const
{
	//	只读侧不设窗口闸门（卷 04 §4.1）：预检 / 候选下发随时可看，窗外亦然；
	//	返回引用以省拷贝，约束不变：调用方不得留存（交易随窗口存活，跨窗持有即悬垂）
	return *Message;
}

bool UCardMoveTransaction::ModifyMessage(TFunctionRef<void(GameMessage::FMoveMessage&)> Modifier)
{
	//	窗外告警拒（卷 04 §4.1，与 AppendModification 同款闸门）：
	//	结构直写只发生在查询期——窗外提交意味着响应者不在任何窗口负载里，属接线错误而非玩法事件
	if (State != EState::Querying)
	{
		UE_LOG(LogCard, Warning, TEXT("[UCardMoveTransaction][ModifyMessage] State is not Querying, structure modification rejected"));
		return false;
	}

	//	意图载体缺席（工厂解析失败）时无可改之物；
	//	语义合法性不在此处判——Execute 前 Validate() 是唯一安全网（卷 04 §4.1），
	//	此处放行的改动活不到落子，不落子即由族自收尾
	if (!Message.IsValid())
	{
		UE_LOG(LogCard, Warning, TEXT("[UCardMoveTransaction][ModifyMessage] Message is null, structure modification rejected"));
		return false;
	}

	//	纪律快照：守卫只还原违规字段，Modifier 的其余合法结构修改仍然生效
	const int32 CountBefore = Message->_Count;
	const FGameplayTag FromBefore = Message->_From;
	const bool bFromLocked = !Cards.IsEmpty();

	//	放行结构直写：_To / _Order / Predicate / _Meta 及 DSL 取序下的 _From
	Modifier(*Message);

	//	纪律①（卷 04 §4.1）：数值位不开第二通道——_Count 只经 ModRequest 折叠，
	//	双通道 = 多响应者并发写歧义、不可回放，故此处直改一律还原
	if (Message->_Count != CountBefore)
	{
		UE_LOG(LogCard, Warning, TEXT("[UCardMoveTransaction][ModifyMessage] _Count is fold-only, reverted from %d to %d"),
			Message->_Count, CountBefore);
		Message->_Count = CountBefore;
	}

	//	纪律②（P3 §7）：预置牌（Cards 非空）锁定 _From——改源区会让预置牌与源区脱钩，
	//	HandleMovement_Selected 查无牌即走"直送不 Consume"分支，牌凭空落目标区而源区不扣
	if (bFromLocked && Message->_From != FromBefore)
	{
		UE_LOG(LogCard, Warning, TEXT("[UCardMoveTransaction][ModifyMessage] _From is locked by selected cards, reverted to %s"),
			*FromBefore.ToString());
		Message->_From = FromBefore;
	}

	//	结构修改是直写，不进 ModeRequests、不广播 FTransactionModified（卷 12 §5.2 该项只登记数值追加）
	return true;
}

void UCardMoveTransaction::HandleMovement_Selected(ICardContainerInterface* From)
{
	//源侧取牌（显式指定）：按牌实体在源区副本中反查源区下标
	TArray<int32> CardIds {};
	auto Area = From->GetCardsByKey(Message->_From);

	for (const FArkCard& Card : Cards)
	{
		//找不到（不在源区 / 已被移走）则跳过该牌，不报错
		auto Index = Area.Find(Card);
		if (Index != INDEX_NONE)
			CardIds.Add(Index);
	}
	//一个都没命中：传入的牌已不在源区（如装备槽顶替，旧牌在发交易前就被顶出槽位），
	//此时沿用传入的 Cards 直送目标区，不做源侧 Consume（否则 Cards 会被空数组覆盖而静默丢弃）
	if (!CardIds.IsEmpty())
		Cards = From->Consume(Message->_From, CardIds);
}

void UCardMoveTransaction::HandleMovement(ICardContainerInterface* From)
{
	//源侧取牌（DSL）：先按谓词/顺序选出下标，再一次性取出（取出结果覆盖 Cards）
	auto CardIds = From->Select(Message->_From, *Message);
	Cards = From->Consume(Message->_From, CardIds);
}

