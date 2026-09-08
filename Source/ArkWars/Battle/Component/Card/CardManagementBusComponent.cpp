// Fill out your copyright notice in the Description page of Project Settings.


#include "CardManagementBusComponent.h"
#include "GameFramework/PlayerState.h"
#include "CardComponentBase.h"
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
	return false;
}

void UCardManagementBusComponent::EquipCard(const FArkCard& Card)
{
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
	{
		return HandCards;
	}
	if (Key == Equipment)
	{
		TArray<FArkCard> OutCards;

		for (const FArkCard& Slot : EquipmentArea)
		{
			OutCards.Add(Slot);
		}
		return OutCards;

	}

	if (Key == Judgement)
	{
		TArray<FArkCard> OutCards;

		for (const FArkCard& Slot : JudgementArea)
		{
			OutCards.Add(Slot);
		}
		return OutCards;
	}
	
	if (auto Comp = GetOwner()->FindComponentByTag(UActorComponent::StaticClass(), Key.GetTagName()))
	{
		if (auto Interface = Cast<ICardContainerInterface>(Comp))
		{
			return Interface->GetCardsByKey(Key);
		}
	}
	
	for (auto Comp : GetOwner()->GetComponentsByInterface(UCardContainerInterface::StaticClass()))
	{
		if (auto Interface = Cast<ICardContainerInterface>(Comp))
		{
			if (!Interface->GetAreaKeys().Contains(Key)) continue;
			
			return Interface->GetCardsByKey(Key);
		}
	}
	
	return {};
}

const FArkCard* UCardManagementBusComponent::GetCardById(int32 CardId) const
{
	return nullptr;
}

TArray<int32> UCardManagementBusComponent::Select(const FGameplayTag& Area, const FString& Msg) const
{
	return {};
}

TArray<FArkCard> UCardManagementBusComponent::Consume(const FGameplayTag& Area, const TArray<int32>& CardIds)
{
	return {};
}

EAreaWriteResult UCardManagementBusComponent::Add(const FGameplayTag& AreaKey, TArray<FArkCard>&& Cards,
	const FString& Msg)
{
	return EAreaWriteResult::Accepted;
}


#undef MATCH_SUIT_CASE
