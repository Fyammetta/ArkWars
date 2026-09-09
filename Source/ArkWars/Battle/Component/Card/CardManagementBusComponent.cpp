// Fill out your copyright notice in the Description page of Project Settings.


#include "CardManagementBusComponent.h"
#include "GameFramework/PlayerState.h"
#include "CardComponentBase.h"
#include "HeadMountedDisplayTypes.h"
#include "ArkWars/Battle/Toolkits/ArkWarTags.h"
#include "Net/UnrealNetwork.h"

#ifndef MATCH_SUIT_CASE
#define MATCH_SUIT_CASE(Suit)	\
	case Suit: if(!Card.HasTagExact(CardTags::Suit)) return false; break;
#endif

namespace 
{
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
	return HandCards.Find(Card);
}

void UCardManagementBusComponent::SelectCard(const FArkCard& Card)
{
	SelectedCards.Add(Card);
}

void UCardManagementBusComponent::ClearCardSelection(const FArkCard& Card)
{
	SelectedCards.Remove(Card);
}

TArray<FArkCard> UCardManagementBusComponent::ConsumeCards()
{
	auto Arr = MoveTemp(SelectedCards);
	OnCardSelectionChanged.Broadcast(Arr, true);
	return MoveTemp(Arr);
}

TArray<APlayerState*> UCardManagementBusComponent::ConsumeTargets()
{
	TArray<APlayerState*> Arr {};
	for (TWeakObjectPtr SelectedPlayer : SelectedPlayers)
	{
		if ( SelectedPlayer.IsValid() )
			Arr.Add(SelectedPlayer.Get());
	}
	OnPlayerSelectionChanged.Broadcast(Arr, true);
	return MoveTemp(Arr);
}

void UCardManagementBusComponent::SelectTarget(APlayerState* Target)
{
	SelectedPlayers.Add(Target);
}

void UCardManagementBusComponent::ClearTargetsSelection(APlayerState* Target)
{
	SelectedPlayers.Remove(Target);
}

bool UCardManagementBusComponent::CanPutInJudgement(const FArkCard& Card) const
{
	return true;//根据Card的Tag容器中的信息确定Card所在的槽位，若槽位不为空则return false;
}

bool UCardManagementBusComponent::CanEquipCard(const FArkCard& Card) const
{
	
	return true;//根据Card的Tag容器中的信息确定Card所在的槽位，若槽位不为空且内部的Card具有"废除"标记，则return false;
}

void UCardManagementBusComponent::Equip(const FArkCard& Card) const
{
	//根据Card的Tag容器中的信息确定Card所在的槽位, 插入
}

void UCardManagementBusComponent::PutIntoJudgement(const FArkCard& Card) const
{
	//根据Card的Tag容器中的信息确定Card所在的槽位, 插入
}

void UCardManagementBusComponent::HandleJudgement()
{
}

void UCardManagementBusComponent::SetCardOrder(const TArray<int32>& NewOrder)
{
}

TArray<FGameplayTag> UCardManagementBusComponent::GetAreaKeys() const
{
	using namespace CardTags;

	TArray<FGameplayTag> Arr = {Hand, Equipment, Judgement};
	
	auto Comps = GetOwner()->GetComponentsByInterface(UCardContainerInterface::StaticClass());
	
	for (auto Comp : Comps)
	{
		if (auto Interface = Cast<ICardContainerInterface>(Comp))
		{
			Arr.Append(Interface->GetAreaKeys());
		}
	}
	
	return Arr;
}

TArray<FArkCard> UCardManagementBusComponent::GetCardsByKey(const FGameplayTag& Key) const
{
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
			while (RetArr.Num() < Msg._Count)
			{	
				auto Max = Temp.Num() - 1;
				auto Rand = FMath::RandRange(0, Max);
				Temp.Swap(Max, Rand);
				
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
	}
	
	return RetArr;
		
}

TArray<FArkCard> UCardManagementBusComponent::Consume(const FGameplayTag& Area, const TArray<int32>& CardIndexes)
{
	if (!GetOwner()->HasAuthority()) return {};
	
	using namespace CardTags;
	TArray<FArkCard> OutCards;
	
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
	
	for (const FArkCard& OutCard : OutCards)
	{
		int32 Index = OutCards.Find(OutCard);
		if (Index != INDEX_NONE)
			CardArea->RemoveAt(Index);
	}
	
	return OutCards;
}

EAreaWriteResult UCardManagementBusComponent::Add(const FGameplayTag& AreaKey, TArray<FArkCard>& Cards, const FMessageType& Msg)
{
	if (!GetOwner()->HasAuthority()) return {};
	
	using namespace CardTags;
	
	if (AreaKey == Hand)
	{
		HandCards.Append(MoveTemp(Cards));
		return EAreaWriteResult::Accepted;
	}
	if (AreaKey == Equipment)
	{
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
