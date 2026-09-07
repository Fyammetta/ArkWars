// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "BattleTransaction.generated.h"

struct FGameplayTagContainer;
class APlayerState;
/**
 * 
 */
UCLASS(Abstract)
class ARKWARS_API UBattleTransaction : public UObject
{
	GENERATED_BODY()
public:
	enum class EState : uint8 { Created, Querying, Executed, Finished };

	void Begin();
	virtual void Execute() PURE_VIRTUAL(UBattleTransaction::Execute);
	void Cancel();
	void Finish();
	
	EState GetState() const { return State; };
	bool IsTransactionCanceled() const { return bIsCancelled; };

	AActor* GetInstigator();
	TArray<AActor*> GetTarget();
	
	void QueryTimings();
	void NotifyTimings();
	void OnWindowClosed();
protected:
	EState State = EState::Created;
	
	///	子类通过对应接口进行操作，规定使用Actor作为基类保证网络复制功能存在
	TWeakObjectPtr<AActor> Instigator;
	TArray<TWeakObjectPtr<AActor>> Target;
	bool bIsCancelled = false;
};

namespace GameMessage { struct FMoveMessage; }
UCLASS()
class ARKWARS_API UCardMoveTransaction : public UBattleTransaction
{
	GENERATED_BODY()
	TUniquePtr<GameMessage::FMoveMessage> Message;                // 解析后的意图（From/To/Num/Predicate/Order）
	TArray<FGameplayTagContainer> Cards;			
	
public:
	virtual void Execute() override {};
};