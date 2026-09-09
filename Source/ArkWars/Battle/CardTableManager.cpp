// Fill out your copyright notice in the Description page of Project Settings.


#include "CardTableManager.h"
#include "Net/UnrealNetwork.h"
#include "Toolkits/ArkWarTags.h"
#include "Toolkits/GameMessage.h"

TStrongObjectPtr<ACardTableManager> ACardTableManager::Instance = nullptr;

// Sets default values
ACardTableManager::ACardTableManager()
{
	SetReplicates(true);
}

void ACardTableManager::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ACardTableManager, CachedArea)
	DOREPLIFETIME(ACardTableManager, PlayedArea)
	DOREPLIFETIME(ACardTableManager, PileArea)
	DOREPLIFETIME(ACardTableManager, DiscardArea)
	DOREPLIFETIME(ACardTableManager, JudgementArea)
}

ACardTableManager* ACardTableManager::Get(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}
	
	if (Instance.IsValid() && Instance->GetWorld() == World) return Instance.Get();

	for (auto Actor : World->GetCurrentLevel()->Actors)
	{
		if (Actor && Actor->IsA(StaticClass()))
		{
			Instance = TStrongObjectPtr(Cast<ACardTableManager>(Actor)) ;
			return Instance.Get();
		}
	}
	{
		ENetMode Mode = World->GetNetMode();
		if (Mode != NM_DedicatedServer && Mode != NM_ListenServer) return nullptr;
	}
	
	Instance = TStrongObjectPtr(Cast<ACardTableManager>(World->SpawnActor(StaticClass())));
	return Instance.Get();
}

void ACardTableManager::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACardTableManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	Instance.Reset();
}

void ACardTableManager::OnRep_CachedAreaChanged()
{
}

void ACardTableManager::OnRep_PlayedAreaChanged()
{
}

void ACardTableManager::OnRep_PileAreaChanged()
{
}

void ACardTableManager::OnRep_DiscardAreaChanged()
{
}

void ACardTableManager::OnRep_JudgementAreaChanged()
{
}

const FArkCard* ACardTableManager::GetCardById(int32 CardId) const
{
	for (const FArkCard& Card : CachedArea)
	{
		if (Card.Identity == CardId) return &Card;
	}
	
	for (const FArkCard& Card : JudgementArea)
	{
		if (Card.Identity == CardId) return &Card;
	}
	
	for (const FArkCard& Card : PlayedArea)
	{
		if (Card.Identity == CardId) return &Card;
	}
	
	for (const FArkCard& Card : PileArea)
	{
		if (Card.Identity == CardId) return &Card;
	}
	for (const FArkCard& Card : DiscardArea)
	{
		if (Card.Identity == CardId) return &Card;
	}
	
	return nullptr;
}

TArray<int32> ACardTableManager::Select(const FGameplayTag& Area, const FMessageType& Msg) const
{
	using namespace CardTags;
	using EOrder = GameMessage::FMoveMessage::EOrder;
	TArray<FArkCard> Temp {};
	if (Area == Pile)
		Temp = PileArea;
	else if (Area == Discard)
		Temp = DiscardArea;
	else if (Area == Judgement)
		Temp = JudgementArea;
	else if (Area == Used)
		Temp = PlayedArea;
	else if (Area == Cache)
		Temp = CachedArea;
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

TArray<FArkCard> ACardTableManager::Consume(const FGameplayTag& Area, const TArray<int32>& CardIndexes)
{
	using namespace CardTags;
	TArray<FArkCard> OutCards;
	
	TArray<FArkCard>* CardArea = nullptr;
	if (Area == Pile)
		CardArea = &PileArea;
	else if (Area == Discard)
		CardArea = &DiscardArea;
	else if (Area == Judgement)
		CardArea = &JudgementArea;
	else if (Area == Used)
		CardArea = &PlayedArea;
	else if (Area == Cache)
		CardArea = &CachedArea;
	else return {};
	
	for (int32 Index : CardIndexes)
	{
		OutCards.Add((*CardArea)[Index]);
	}
	
	for (const FArkCard& OutCard : OutCards)
	{
		CardArea->RemoveAt(OutCards.Find(OutCard));
	}
	
	return OutCards;
}

EAreaWriteResult ACardTableManager::Add(const FGameplayTag& AreaKey, TArray<FArkCard>& Cards, const FMessageType& Msg)
{
	using namespace CardTags;
	using EOrder = GameMessage::FMoveMessage::EOrder;
	if (AreaKey == Pile)
	{
		switch (Msg._Order)
		{
			case EOrder::Bottom :
			{
				Cards.Append(PileArea);
				PileArea = MoveTemp(Cards);
				break;
			}
			case EOrder::Top :
			{
				PileArea.Append(MoveTemp(Cards));
				break;
			}
			case EOrder::Random :
			{
				while (!Cards.IsEmpty())
				{
					PileArea.Insert(Cards.Pop(),FMath::RandRange(0,PileArea.Num() - 1));
				}
			}
		}
	}

	else if (AreaKey == Discard)
		DiscardArea.Append(MoveTemp(Cards));
	else if (AreaKey == Judgement)
		JudgementArea.Append(MoveTemp(Cards));
	else if (AreaKey == Used)
		PlayedArea.Append(MoveTemp(Cards));
	else if (AreaKey == Cache)
		CachedArea.Append(MoveTemp(Cards));
	return EAreaWriteResult::Accepted;
}


TArray<FArkCard> ACardTableManager::GetCardsByKey(const FGameplayTag& Key) const
{
	using namespace CardTags;

	if (Key == Pile)	return PileArea;
	if (Key == Discard) return DiscardArea;
	if (Key == Cache)	return CachedArea;
	if (Key == Used)	return PlayedArea;
	
	return {};
}

TArray<FGameplayTag> ACardTableManager::GetAreaKeys() const
{
	using namespace CardTags;
	return {Pile, Discard, Cache, Used};
}
