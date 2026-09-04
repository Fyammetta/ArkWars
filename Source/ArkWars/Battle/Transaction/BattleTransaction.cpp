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

AActor* UBattleTransaction::GetTarget()
{
	return Target.IsValid() ? Target.Get() : nullptr;
}
