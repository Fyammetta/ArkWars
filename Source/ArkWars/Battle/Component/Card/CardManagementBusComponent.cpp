// Fill out your copyright notice in the Description page of Project Settings.


#include "CardManagementBusComponent.h"
#include "GameFramework/PlayerState.h"
#include "CardComponentBase.h"
#include "HeadMountedDisplayTypes.h"
#include "ArkWars/Battle/CardTableManager.h"
#include "ArkWars/Battle/Toolkits/ArkWarTags.h"
#include "ArkWars/Battle/Transaction/SubTransaction/CardMoveTransaction.h"
#include "Net/UnrealNetwork.h"

#ifndef MATCH_SUIT_CASE
#define MATCH_SUIT_CASE(Suit)	\
	case Suit: if(!Card.HasTagExact(CardTags::Suit)) return false; break;
#endif

namespace 
{
	//按点数标签尾字符取数值（当前全库无调用方，属死代码，见 P2 §7 观察项）
	int32 PointToNumber(const FGameplayTag& PointTag)
	{
		auto TagStr = PointTag.ToString();
		auto Point = TagStr[TagStr.Len()-1];
		switch (Point)
		{
		case 'K':	return 13;
		case 'Q':	return 12;
		case 'J':	return 11;
		case 'A':	return 1;
		default:
			if (static_cast<int32>(Point) <= 2 || static_cast<int32>(Point) >= 10) return INDEX_NONE;
			return Point;
		}
	}
}


UCardManagementBusComponent::UCardManagementBusComponent()
{
	SetIsReplicatedByDefault(true);
}

void UCardManagementBusComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCardManagementBusComponent, EquipmentArea);
	DOREPLIFETIME(UCardManagementBusComponent, HandCards);
	DOREPLIFETIME(UCardManagementBusComponent, JudgementArea);
}

int32 UCardManagementBusComponent::GetIndexOfCard(const FArkCard& Card) const
{
	//按 FArkCard 相等（即 Identity 相同）在手牌区查找
	return HandCards.Find(Card);
}

void UCardManagementBusComponent::SelectCard(const FArkCard& Card)
{
	//本地待选集合：仅客户端暂存，不做去重
	SelectedCards.Add(Card);
}

void UCardManagementBusComponent::ClearCardSelection(const FArkCard& Card)
{
	//反选：按牌实体移除一次
	SelectedCards.Remove(Card);
}

TArray<FArkCard> UCardManagementBusComponent::ConsumeCards()
{
	//取走全部待选牌（移动语义清空本地集合）
	auto Arr = MoveTemp(SelectedCards);
	//广播一次"已取走"的选择变更，供 UI / 结算方刷新
	OnCardSelectionChanged.Broadcast(Arr, true);
	return MoveTemp(Arr);
}

TArray<APlayerState*> UCardManagementBusComponent::ConsumeTargets()
{
	TArray<APlayerState*> Arr {};
	//过滤失效弱引用，只返回仍然有效的目标
	for (TWeakObjectPtr SelectedPlayer : SelectedPlayers)
	{
		if ( SelectedPlayer.IsValid() )
			Arr.Add(SelectedPlayer.Get());
	}
	//取走即清空本地待选目标集合（与 ConsumeCards 一致）
	OnPlayerSelectionChanged.Broadcast(Arr, true);
	SelectedPlayers.Empty();
	return MoveTemp(Arr);
}

void UCardManagementBusComponent::SelectTarget(APlayerState* Target)
{
	//目标以弱引用暂存，取走时统一做有效性过滤
	SelectedPlayers.Add(Target);
}

void UCardManagementBusComponent::ClearTargetsSelection(APlayerState* Target)
{
	//反选：按指针移除一次
	SelectedPlayers.Remove(Target);
}

bool UCardManagementBusComponent::CanPutInJudgement(const FArkCard& Card) const
{
	//校验顺序：① 牌类（Card.Type.Judge）→ ② 槽下标解析 → ③ 槽占用
	if (!Card.Card.HasTagExact(CardTags::JudgeType)) return false;
	
	auto Index = CardAreaSlot::GetJudgementSlot(Card.GetClass());
	
	if (Index == INDEX_NONE) return false;
	
	//③ 槽未扩容到该下标（视为空槽）或该槽是空占位牌 → 可放
	return !JudgementArea.IsValidIndex(Index) || JudgementArea[Index].Identity == INDEX_NONE;
}

bool UCardManagementBusComponent::CanEquipCard(const FArkCard& Card) const
{
	//校验顺序：① 牌类（Card.Type.Equip）→ ② 槽下标解析 → ③ 槽占用
	//TODO: ② 的槽下标映射表（CardAreaSlot::GetEquipmentSlot）尚未填充，恒 INDEX_NONE → 当前恒返回 false（P2 §2-E 缺陷⑦）
	if (!Card.Card.HasTagExact(CardTags::EquipType)) return false;
	
	auto Index = CardAreaSlot::GetEquipmentSlot(Card.GetClass());
	
	if (Index == INDEX_NONE) return false;
	
	//③ 槽未扩容（视为空）或占用牌已被标记 Card.Area.Deprecated（可被顶替）→ 可装备
	return !EquipmentArea.IsValidIndex(Index) || !EquipmentArea[Index].Card.HasTag(CardTags::Deprecated);
}

void UCardManagementBusComponent::Equip(const FArkCard& Card)
{
	//仅服务器权威可写
	if (!GetOwner()->HasAuthority()) return;
	//根据Card的Tag容器中的信息确定Card所在的槽位, 插入
	//默认调用前已通过验证，不在额外验证
	auto Index = CardAreaSlot::GetEquipmentSlot(Card.GetClass());
	//槽位不足时以空占位牌扩容到目标下标（空槽以 Identity == INDEX_NONE 标识）
	while (!EquipmentArea.IsValidIndex(Index))
	{
		EquipmentArea.Add(FArkCard{INDEX_NONE,{}});
	}
	if (EquipmentArea[Index].Identity == INDEX_NONE)
		EquipmentArea[Index] = Card;
	else
	{
		//槽已占用：新牌顶替旧牌，并让旧牌走一次移动交易（Equipment → Used）
		//旧牌此刻已离开装备区，交易侧按"源区未命中即沿用传入牌"处理（见 HandleMovement_Selected）
		auto Temp = EquipmentArea[Index];
		EquipmentArea[Index] = Card;
		
		using MSG = GameMessage::FMoveMessage;
		static const FString Msg = FString::Printf(TEXT("%s=%s %s=%s"), MSG::From, MSG::Equipment, MSG::To, MSG::Used);
		auto Tx = UCardMoveTransaction::Create(Msg, {Temp});
		Tx->Instigator = GetOwner();
		Tx->Targets.Add(ACardTableManager::Get(GetWorld()));
		Tx->Start();
	}
}

void UCardManagementBusComponent::PutIntoJudgement(const FArkCard& Card)
{
	//仅服务器权威可写
	if (!GetOwner()->HasAuthority()) return;
	//根据Card的Tag容器中的信息确定Card所在的槽位, 插入
	auto Index = CardAreaSlot::GetJudgementSlot(Card.GetClass());
	//槽位不足时以空占位牌扩容到目标下标
	while (!JudgementArea.IsValidIndex(Index))
	{
		JudgementArea.Add(FArkCard{INDEX_NONE,{}});
	}
	
	//默认调用前已通过验证，不在额外验证
	JudgementArea[Index] = Card;
}

void UCardManagementBusComponent::HandleJudgement()
{
	//TODO: 判定结算待阶段体接入（卷 08 §8 / P2 §2-E）
}

TArray<FGameplayTag> UCardManagementBusComponent::GetAreaKeys() const
{
	using namespace CardTags;
	return {Hand, Equipment, Judgement};
}

TArray<FArkCard> UCardManagementBusComponent::GetCardsByKey(const FGameplayTag& Key) const
{
	//只读：仅本组件三区可读，其余区域返回空数组
	using namespace CardTags;
	
	if (Key == Hand)
		return HandCards;
	if (Key == Equipment)
		return EquipmentArea;
	if (Key == Judgement)
		return JudgementArea;
	
	
	return {};
}

const FArkCard* UCardManagementBusComponent::GetCardById(int32 CardId) const
{
	//按实体 Id 依次在 手牌 → 判定 → 装备 中反查
	for (const FArkCard& Card : HandCards)
	{
		if (Card.Identity == CardId) return &Card;
	}
	
	for (const FArkCard& Card : JudgementArea)
	{
		if (Card.Identity == CardId) return &Card;
	}
	
	for (const FArkCard& Card : EquipmentArea)
	{
		if (Card.Identity == CardId) return &Card;
	}
	
	return nullptr;
}

TArray<int32> UCardManagementBusComponent::Select(const FGameplayTag& Area, const FMessageType& Msg) const
{
	//只读：先拷出目标区域（不改动源区），再按 _Order 扫描，命中返回源区下标
	using namespace CardTags;
	using EOrder = GameMessage::FMoveMessage::EOrder;
	TArray<FArkCard> Temp {};
	if (Area == Hand)
		Temp = HandCards;
	else if (Area == Equipment)
		Temp = EquipmentArea;
	else if (Area == Judgement)
		Temp = JudgementArea;
	else
		return {};
	
	if (Temp.IsEmpty()) return {};


	TArray<int32> RetArr {};
	switch (Msg._Order)
	{
	case EOrder::Top :
		{
			while (RetArr.Num() < Msg._Count)
			{	
				auto Card = Temp.Last();
				if (Msg(Card))
				{
					RetArr.Add(Temp.Find(Card));
				}
				Temp.Pop();
				if (Temp.IsEmpty()) break;
			}
			break;
		}
	case EOrder::Bottom :
		{
			for (int Index = 0; RetArr.Num() < Msg._Count;)
			{
				auto Card = Temp[Index];
				if (Msg(Card))
				{
					RetArr.Add(Index);
				}
				Index++;
				if (!Temp.IsValidIndex(Index)) break;
			}
			break;
		}
	case EOrder::Random :
		{
			auto Source = Temp;
			while (RetArr.Num() < Msg._Count)
			{	
				auto Max = Temp.Num() - 1;
				auto Rand = FMath::RandRange(0, Max);
				Temp.Swap(Max, Rand);
				
				auto Card = Temp.Last();
				if (Msg(Card))
				{
					RetArr.Add(Source.Find(Card));
				}
				Temp.Pop();
				if (Temp.IsEmpty()) break;
			}
			break;
		}
	}
	
	return RetArr;
		
}

TArray<FArkCard> UCardManagementBusComponent::Consume(const FGameplayTag& Area, const TArray<int32>& CardIndexes)
{
	//仅服务器权威可写
	if (!GetOwner()->HasAuthority()) return {};
	
	using namespace CardTags;
	TArray<FArkCard> OutCards;
	
	//定位源区（仅本组件三区，其余区域直接返回空）
	TArray<FArkCard>* CardArea = nullptr;
	if (Area == Hand)
		CardArea = &HandCards;
	else if (Area == Equipment)
		CardArea = &EquipmentArea;
	else if (Area == Judgement)
		CardArea = &JudgementArea;
	else return {};
	
	for (int32 Index : CardIndexes)
	{
		OutCards.Add((*CardArea)[Index]);
	}
	
	TArray<FArkCard> Recover {};
	
	
	
	//反向重建：未选中的牌按原序回填，等价于按 CardIndexes 原地删除
	for (int32 i = 0; i < CardArea->Num(); i++)
	{
		if (CardIndexes.Find(i) != INDEX_NONE) continue;
		Recover.Add((*CardArea)[i]);
	}
	
	*CardArea = MoveTemp(Recover);
	
	return OutCards;
}

EAreaWriteResult UCardManagementBusComponent::Add(const FGameplayTag& AreaKey, TArray<FArkCard>& Cards, const FMessageType& Msg)
{
	//仅服务器权威可写
	if (!GetOwner()->HasAuthority()) return EAreaWriteResult::Mismatch;
	
	using namespace CardTags;
	if (AreaKey == Hand)
	{
		//手牌：整壳追加到数组尾（尾部即牌顶）
		HandCards.Append(MoveTemp(Cards));
		return EAreaWriteResult::Accepted;
	}
	if (AreaKey == Equipment)
	{
		//装备：单张落槽 → 槽位校验不过按 Full 回报，避免调用方把未落位的牌当成功
		if (Cards.Num() != 1) return EAreaWriteResult::Mismatch;
		
		if (CanEquipCard(Cards[0]))
		{
			Equip(Cards[0]);
			Cards.RemoveAt(0);
			return EAreaWriteResult::Accepted;
		}
		
		return EAreaWriteResult::Full;
	}
	if (AreaKey == Judgement)
	{
		//判定：单张落槽（同装备）
		if (Cards.Num() != 1) return EAreaWriteResult::Mismatch;
		
		if (CanPutInJudgement(Cards[0]))
		{
			PutIntoJudgement(Cards[0]);
			Cards.RemoveAt(0);
			return EAreaWriteResult::Accepted;
		}
		return EAreaWriteResult::Full;
	}	
	
	
	return EAreaWriteResult::Mismatch;
}


#undef MATCH_SUIT_CASE
