// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArkWars/Battle/Transaction/BattleTransaction.h"
#include "CardMoveTransaction.generated.h"


class ICardContainerInterface;
struct FArkCard;

namespace GameMessage { struct FMoveMessage; }
UCLASS()
class ARKWARS_API UCardMoveTransaction : public UBattleTransaction
{
	GENERATED_BODY()
	TUniquePtr<GameMessage::FMoveMessage> Message;                // 解析后的意图（From/To/Num/Predicate/Order）
	TArray<FArkCard> Cards;			
	
public:
	virtual void Execute() override;
	virtual void FoldMods() override;

protected:
	virtual bool Validate() const override;
	virtual void BroadcastFinish(bool bSuccess) override;

public:
	static UCardMoveTransaction* Create(const FString& Msg, const TArray<FArkCard>& CardsToMove = {});
	
private:
	void HandleMovement_Selected(ICardContainerInterface* From);
	void HandleMovement(ICardContainerInterface* From);
};
