// Fill out your copyright notice in the Description page of Project Settings.


#include "CardComponentBase.h"

#include "ArkWars/ArkWars.h"
#include "ArkWars/Battle/Toolkits/ArkWarTags.h"
#include "ArkWars/Battle/Toolkits/BattleFunctionLibrary.h"
#ifndef MATCH_SUIT_CASE
#define MATCH_SUIT_CASE(Suit)	\
	case Suit: if(!Card->HasTagExact(CardTags::Suit())) return false; break;
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


UCardComponentBase::UCardComponentBase()
{
	SetIsReplicatedByDefault(true);
}

UCardComponentBase* UCardComponentBase::Get(AActor* Owner, const FGameplayTag& CardTag, bool ForceCreate)
{
	if (auto Component = Owner->FindComponentByTag(StaticClass(), CardTag.GetTagName()))
	{
		return Cast<UCardComponentBase>(Component);
	}
	
	if (!ForceCreate) return nullptr;
	
	if (auto System = UBattleFunctionLibrary::GetCardManager(Owner))
	{
		TUniquePtr Info = MakeUnique<FCardInfo>(System->GetCardInfoByTag(CardTag));
		
		if (Info->IsValid())
		{
			auto Comp = NewObject<UCardComponentBase>(Owner,Info->Class, *Info->CardName.ToString());
			Comp->ComponentTags.Add(CardTag.GetTagName());
			Comp->ComponentTags.Add(CardTags::Root().GetTagName());
			Comp->Info = MoveTemp(Info);
			Comp->Info->DataGetter.BindUObject(Comp, &UCardComponentBase::DataConverter);
			Comp->RegisterComponent();
			return Comp;
		}
	}
	
	UE_LOG(LogCard, Warning, TEXT("[UCardComponentBase][Get] Tried to create component when not found but failed"))
	return nullptr;
}

int32 UCardComponentBase::GetCards(TArray<FCard>& OutCards, bool bIncludeConverted) const
{
	if (!bIncludeConverted)
	{
		OutCards = Container;
		return Container.Num();
	}
	
	return GetCardByPredicate([](auto)->bool{return true; },OutCards);
}

int32 UCardComponentBase::GetCardBySuit(ECardSuit Suit, TArray<FCard>& OutCards, bool bIncludeConverted) const
{
	return GetCardByPredicate(
		[Suit](const FCard& Card)->bool
		{
			switch (Suit)
			{
				MATCH_SUIT_CASE(Diamond)
				MATCH_SUIT_CASE(Heart)
				MATCH_SUIT_CASE(Club)
				MATCH_SUIT_CASE(Spade)
			}

			return true;
		},OutCards);
}

int32 UCardComponentBase::GetCardByPointRange(int32 Min, int32 Max, TArray<FCard>& OutCards, bool bIncludeConverted) const
{
	return GetCardByPredicate(
		[Min, Max](const FCard& Card)->bool
		{
			auto Point = PointToNumber(Card->Filter(FGameplayTagContainer{CardTags::Point()}).First());
					
			return Point >Min && Point < Max;
		},OutCards);
}

int32 UCardComponentBase::GetCardByMultiCondition(ECardSuit Suit, int32 Min, int32 Max, TArray<FCard>& OutCards, bool bIncludeConverted) const
{
	return GetCardByPredicate(
	[Suit, Min, Max](const FCard& Card)->bool
	{
		switch (Suit)
		{
			MATCH_SUIT_CASE(Diamond)
			MATCH_SUIT_CASE(Heart)
			MATCH_SUIT_CASE(Club)
			MATCH_SUIT_CASE(Spade)
		}
			
		auto Point = PointToNumber(Card->Filter(FGameplayTagContainer{CardTags::Point()}).First());
			
		return Point >Min && Point < Max;
	},OutCards);
}

int32 UCardComponentBase::GetCardByPredicate(const TFunction<bool(const FCard&)>& Predicate, TArray<FCard>& OutCards, bool bIncludeConverted) const
{
	OutCards.Empty();
	for (auto Card : Container)
	{
		if (Predicate(Card))
			OutCards.Add(Card);
	}
	TArray<FConvertedCard> WeakArr = ConvertedCards;
	for (auto Card : WeakArr)
	{
		if (!Card.IsValid())
		{
			ConvertedCards.Remove(Card);
			continue;
		}
		
		if (Predicate(Card.Pin()))
			OutCards.Add(Card.Pin());
	}
	
	return OutCards.Num();
}

void UCardComponentBase::Move(const TArray<FCard>& Cards, ICardContainerInterface* To)
{
}

void UCardComponentBase::Response(AActor* Target, const FCard& Card)
{
}

void UCardComponentBase::Use(const TArray<AActor*>& Targets, TSharedPtr<FGameplayTagContainer> Card)
{
}


#undef MATCH_SUIT_CASE