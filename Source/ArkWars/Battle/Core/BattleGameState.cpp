// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleGameState.h"
#include "ArkWars/ArkWars.h"
#include "ArkWars/Battle/Toolkits/ArkWarGlobal.h"
#include "Net/UnrealNetwork.h"

EGamePhase ABattleGameState::GetLastPhase()
{
	return CachedPhase;
}

EGamePhase ABattleGameState::GetPhase()
{
	return CurrentPhase;
}

void ABattleGameState::SetPhase(EGamePhase Phase)
{
	if (!HasAuthority())
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[ABattleGameState][SetPhase] Modify should only be called on server"));
		return;
	}
	CachedPhase = CurrentPhase;
	CurrentPhase = Phase;
	
	BroadcastPhaseChange();
}

void ABattleGameState::SetNextPlayerActive()
{
	ActivePlayerIndex = (ActivePlayerIndex + 1)% PlayersInOrder.Num();
	BroadcastActivePlayerChange();
}

void ABattleGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ABattleGameState, CurrentPhase);
	DOREPLIFETIME(ABattleGameState, CachedPhase);
	DOREPLIFETIME(ABattleGameState, PlayersInOrder);
	DOREPLIFETIME(ABattleGameState, ActivePlayerIndex);
	
}

void ABattleGameState::OnRep_ActivePlayerIndex() const
{
	BroadcastActivePlayerChange();
}

void ABattleGameState::OnRep_PlayersInOrder() const
{
	
}

void ABattleGameState::OnRep_Phase() const
{
	BroadcastPhaseChange();
}

void ABattleGameState::BroadcastPhaseChange() const
{
	OnGamePhaseChanged.Broadcast(GamePhase::GamePhaseToTagMap[CachedPhase], GamePhase::GamePhaseToTagMap[CurrentPhase]);
}

void ABattleGameState::BroadcastActivePlayerChange() const
{
	if (!PlayersInOrder.IsValidIndex(ActivePlayerIndex))
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[ABattleGameState][ActivePlayerChange] Index %d is not valid while Players containing %d elements"), ActivePlayerIndex, PlayersInOrder.Num());
		return;
	}
	OnActivePlayerChanged.Broadcast(PlayersInOrder[ActivePlayerIndex]);
}


