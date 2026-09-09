// Fill out your copyright notice in the Description page of Project Settings.


#include "CardMoveTransaction.h"

#include "ArkWars/ArkWars.h"
#include "ArkWars/Battle/Toolkits/CardContainerInterface.h"
#include "ArkWars/Battle/Toolkits/GameMessage.h"
#include "ArkWars/Battle/Transaction/EventTags.h"


namespace 
{
	ICardContainerInterface* GetInterface(AActor* Target)
	{
		if (!Target) return nullptr;
		
		if (!Target->Implements<UCardContainerInterface>()) return Cast<ICardContainerInterface>(Target); 
		
		return Cast<ICardContainerInterface>(Target->FindComponentByInterface(UCardContainerInterface::StaticClass()));
	}
}

void UCardMoveTransaction::Execute()
{
	if (!Validate()) return Finish(false);
	bool bIsSelected = !Cards.IsEmpty();
	

	
	FoldMods();
	
	auto From = GetInterface(Instigator.Get());
	auto To = GetInterface(Targets[0].Get());
	
	if (From == To && Message->_From == Message->_To)
		return Finish(false);
	
	
	if (bIsSelected)
		HandleMovement_Selected(From);
	else
		HandleMovement(From);

	switch (To->Add(Message->_To, Cards, *Message))
	{
		case EAreaWriteResult::Accepted:	return Finish(true);
		case EAreaWriteResult::Full:		return Finish(false);
		case EAreaWriteResult::Mismatch:	return Finish(false);
	}
}

void UCardMoveTransaction::FoldMods()
{
}

bool UCardMoveTransaction::Validate() const
{
	if (!Instigator.IsValid() || !GetInterface(Instigator.Get()) )
	{
		UE_LOG(LogCard, Warning, TEXT("[UCardMoveTransaction][Validate] Instigator %s is illegal"), (Instigator.IsValid() ? *Instigator->GetName() : TEXT("nullptr")));
		return false;
	}
	
	
	if (Targets.Num() != 1) 	
	{
		UE_LOG(LogCard, Warning, TEXT("[UCardMoveTransaction][Validate] Targets count is not expected to be %d"), Targets.Num());
		return false;
	}
	if (!Targets[0].IsValid() || !GetInterface(Targets[0].Get()))
	{
		UE_LOG(LogCard, Warning, TEXT("[UCardMoveTransaction][Validate] Target %s is illegal"), (Targets[0].IsValid() ? *Targets[0]->GetName() : TEXT("nullptr")));
		return false;
	}
	
	
	
	return Message.IsValid() && Message->IsValid();
}

void UCardMoveTransaction::BroadcastFinish(bool bSuccess)
{
	FGameplayTag Notify = Notify::Card::Moved;
		
	///TODO: 如果bSuccess = true, 广播通知
		
}

UCardMoveTransaction* UCardMoveTransaction::Create(const FString& Msg, const TArray<FArkCard>& CardsToMove)
{
	auto RetVal = NewObject<UCardMoveTransaction>();
	RetVal->Message = MakeUnique<GameMessage::FMoveMessage>(Msg);
	RetVal->Cards = CardsToMove;
	return RetVal;
}

void UCardMoveTransaction::HandleMovement_Selected(ICardContainerInterface* From)
{
	TArray<int32> CardIds {};
	auto Area = From->GetCardsByKey(Message->_From);

	for (const FArkCard& Card : Cards)
	{
		auto Index = Area.Find(Card);
		if (Index != INDEX_NONE)
			CardIds.Add(Index);
	}
	
	Cards = From->Consume(Message->_From, CardIds);
}

void UCardMoveTransaction::HandleMovement(ICardContainerInterface* From)
{
	auto CardIds = From->Select(Message->_From, *Message);
	Cards = From->Consume(Message->_From, CardIds);
}

