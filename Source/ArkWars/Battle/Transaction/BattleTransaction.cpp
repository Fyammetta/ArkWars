// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleTransaction.h"
#include "ArkWars/Battle/System/BattleGameFlowSubsystem.h"
#include "ArkWars/Battle/Toolkits/BattleFunctionLibrary.h"

void UBattleTransaction::Start()
{
	//仅在服务器中入队
	if (GetWorld()->GetNetMode() != NM_ListenServer && GetWorld()->GetNetMode() != NM_DedicatedServer) return;

	if (!Validate() || State != EState::Spawned) return;
	
	State = EState::Started;
	auto System = UBattleFunctionLibrary::GetBattleManager(this);
	
	if (!System) return;
	
	System->EnqueueTransaction(this);
}


void UBattleTransaction::AppendModification(const FTransactionModRequest& Req)
{
	if (State != EState::Querying) return;
	
	ModeRequests.Add(Req);
}

void UBattleTransaction::Finish(bool bSuccess)
{
	State = EState::Finished;
	auto System = UBattleFunctionLibrary::GetBattleManager(this);
	
	if (!System) return;
	
	BroadcastFinish(bSuccess);
	System->OnTransactionFinished(this);
}

void UBattleTransaction::QueryTimings()
{
	State = EState::Querying;
}

void UBattleTransaction::OnWindowClosed()
{
	
}