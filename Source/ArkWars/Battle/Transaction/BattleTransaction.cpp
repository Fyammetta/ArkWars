// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleTransaction.h"

void UBattleTransaction::Begin()
{
	
}

void UBattleTransaction::Cancel()
{
}

void UBattleTransaction::Finish()
{
}

void UBattleTransaction::QueryTimings()
{
}

void UBattleTransaction::NotifyTimings()
{
}

void UBattleTransaction::OnWindowClosed()
{
}

AActor* UBattleTransaction::GetInstigator()
{
	return Instigator.IsValid() ? Instigator.Get() : nullptr;
}

TArray<AActor*> UBattleTransaction::GetTarget()
{
	TArray<AActor*> Result;
	for (TWeakObjectPtr<AActor> Actor : Target)
	{
		if (Actor.IsValid())
			Result.Add(Actor.Get());
	}
	
	return Result;
}
