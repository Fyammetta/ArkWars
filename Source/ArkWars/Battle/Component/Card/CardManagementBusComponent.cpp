// Fill out your copyright notice in the Description page of Project Settings.


#include "CardManagementBusComponent.h"
#include "GameFramework/PlayerState.h"
#include "CardComponentBase.h"
#include "ArkWars/Battle/Toolkits/ArkWarTags.h"
#include "Net/UnrealNetwork.h"

#ifndef MATCH_SUIT_CASE
#define MATCH_SUIT_CASE(Suit)	\
	case Suit: if(!Card.HasTagExact(CardTags::Suit())) return false; break;
#endif

namespace 
{
	int32 PointToNumber(const FGameplayTag& PointTag)
	{
		auto Point = *PointTag.ToString().end();
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
}

int32 UCardManagementBusComponent::GetIndexOfCard(const FGameplayTagContainer& Card) const
{
	return HandCards.Find(Card);
}

void UCardManagementBusComponent::SelectCard(const FGameplayTagContainer& Card)
{
	SelectedCards.Add(Card);
}

void UCardManagementBusComponent::ClearCardSelection(const FGameplayTagContainer& Card)
{
	SelectedCards.Remove(Card);
}

TArray<FGameplayTagContainer> UCardManagementBusComponent::GetSelectedCards()
{
	auto Arr = MoveTemp(SelectedCards);
	OnCardSelectionChanged.Broadcast(Arr, true);
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

TArray<APlayerState*> UCardManagementBusComponent::GetSelectedTargets()
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

bool UCardManagementBusComponent::CanPutInJudgement(const FGameplayTagContainer& Card) const
{
	return false;
}

void UCardManagementBusComponent::EquipCard(const FGameplayTagContainer& Card)
{
}




void UCardManagementBusComponent::SetCardOrder(const TArray<int32>& NewOrder)
{
}

void UCardManagementBusComponent::MoveIn(ICardContainerInterface* From, TArray<FGameplayTagContainer>&& Cards, const FString& Msg)
{
}

void UCardManagementBusComponent::MoveOut(ICardContainerInterface* To, const TArray<FGameplayTagContainer>& Cards, const FString& Msg)
{
}



int32 UCardManagementBusComponent::GetCards(TArray<FGameplayTagContainer>& OutCards) const
{
	return GetCardByPredicate([](auto)->bool{return true; },OutCards);
}

int32 UCardManagementBusComponent::GetCardByPredicate(const TFunction<bool(const FGameplayTagContainer&)>& Predicate, TArray<FGameplayTagContainer>& OutCards) const
{
	OutCards.Empty();
	for (auto Card : HandCards)
	{
		if (Predicate(Card))
			OutCards.Add(Card);
	}
	
	return OutCards.Num();
}



#undef MATCH_SUIT_CASE