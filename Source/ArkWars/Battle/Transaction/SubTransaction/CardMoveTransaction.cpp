// Fill out your copyright notice in the Description page of Project Settings.


#include "CardMoveTransaction.h"

#include "ArkWars/Battle/Toolkits/GameMessage.h"

UCardMoveTransaction* UCardMoveTransaction::Create(const FString& Msg)
{
	auto RetVal = NewObject<UCardMoveTransaction>();
	RetVal->Message = MakeUnique<GameMessage::FMoveMessage>(Msg);
	return RetVal;
}
