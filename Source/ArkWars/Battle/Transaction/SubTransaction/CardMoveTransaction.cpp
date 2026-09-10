// Fill out your copyright notice in the Description page of Project Settings.


#include "CardMoveTransaction.h"

#include "ArkWars/ArkWars.h"
#include "ArkWars/Battle/Toolkits/BattleFunctionLibrary.h"
#include "ArkWars/Battle/Toolkits/CardContainerInterface.h"
#include "ArkWars/Battle/Toolkits/GameMessage.h"
#include "ArkWars/Battle/Transaction/EventTags.h"


void UCardMoveTransaction::Execute()
{
	//① 入参校验不通过直接失败收尾
	if (!Validate()) return Finish(false);
	//② 预选牌（Cards 非空）按实体定位；否则按消息 DSL 取序
	bool bIsSelected = !Cards.IsEmpty();
	

	
	FoldMods();
	
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

//TODO: 修正折叠当前为空实现（直通），修正链待接（P2 §2-D）
void UCardMoveTransaction::FoldMods()
{
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
	//收尾广播：当前仅取通知标签，未真正广播
	FGameplayTag Notify = Notify::Card::Moved;
		
	//TODO: 如果bSuccess = true, 广播通知
		
}

UCardMoveTransaction* UCardMoveTransaction::Create(const FString& Msg, const TArray<FArkCard>& CardsToMove)
{
	//解析消息串，并把预置牌（可为空）写入交易
	auto RetVal = NewObject<UCardMoveTransaction>();
	RetVal->Message = MakeUnique<GameMessage::FMoveMessage>(Msg);
	RetVal->Cards = CardsToMove;
	return RetVal;
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

