// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArkWars/Battle/Transaction/BattleTransaction.h"
#include "CardMoveTransaction.generated.h"


namespace GameMessage { struct FMoveMessage; }
UCLASS()
class ARKWARS_API UCardMoveTransaction : public UBattleTransaction
{
	GENERATED_BODY()
	TUniquePtr<GameMessage::FMoveMessage> Message;                // 解析后的意图（From/To/Num/Predicate/Order）
	TArray<FGameplayTagContainer> Cards;			
	
public:
	virtual void Execute() override {};
	
	static UCardMoveTransaction* Create(const FString& Msg);
};