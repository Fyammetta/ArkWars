#pragma once
#include "GameplayTagContainer.h"

class UBattleTransaction;






struct FTransactionModRequest
{
	enum class EModOp : uint8 { Ignore, Add, Multiply, Clamp };
	
	EModOp Operation;
	
	float Magnitude;
	
	TWeakObjectPtr<APlayerState> Actor;
	
	int32 Priority;
};


struct FResponseEntry
{
	TStrongObjectPtr<UBattleTransaction> Tx;
	
	int32 Priority;
	
	TWeakObjectPtr<APlayerState> Owner;
	
	FGameplayTag Key;
};

struct FResponseWindow
{
	FGameplayTag Timing;
	
	TStrongObjectPtr<UBattleTransaction> Tx;
	
	TArray<FResponseEntry> Responders;
	
	float Timeout;
};