// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePhaseManagerComponent.h"

#include "ArkWars/ArkWars.h"
#include "ArkWars/Battle/Toolkits/ArkWarGlobal.h"
#include "Net/UnrealNetwork.h"


UGamePhaseManagerComponent::UGamePhaseManagerComponent()
{
	SetIsReplicatedByDefault(true);
}


EGamePhase UGamePhaseManagerComponent::GetLastPhase()
{
	return CachedPhase;
}

EGamePhase UGamePhaseManagerComponent::GetPhase()
{
	return CurrentPhase;
}

void UGamePhaseManagerComponent::SetPhase(EGamePhase Phase)
{
	if (!GetOwner()->HasAuthority())
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GamePhaseManager][SetPhase] Modify should only be called on server"));
		return;
	}
	CachedPhase = CurrentPhase;
	CurrentPhase = Phase;
	
	BroadcastPhaseChange();
}

void UGamePhaseManagerComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGamePhaseManagerComponent, CurrentPhase);
	DOREPLIFETIME(UGamePhaseManagerComponent, CachedPhase);
	DOREPLIFETIME(UGamePhaseManagerComponent, PlayersInOrder);
	DOREPLIFETIME(UGamePhaseManagerComponent, ActivePlayerIndex);
}

void UGamePhaseManagerComponent::OnRep_Phase() const
{
	BroadcastPhaseChange();
}

void UGamePhaseManagerComponent::BroadcastPhaseChange() const
{
	OnGamePhaseChanged.Broadcast(GamePhase::GamePhaseToTagMap[CachedPhase], GamePhase::GamePhaseToTagMap[CurrentPhase]);
}


void UGamePhaseManagerComponent::SetNextPlayerActive()
{
	ActivePlayerIndex = (ActivePlayerIndex + 1)% PlayersInOrder.Num();
	BroadcastActivePlayerChange();
}

void UGamePhaseManagerComponent::OnRep_ActivePlayerIndex() const
{
	BroadcastActivePlayerChange();
}

void UGamePhaseManagerComponent::OnRep_PlayersInOrder() const
{
	
}

void UGamePhaseManagerComponent::BroadcastActivePlayerChange() const
{
	if (!PlayersInOrder.IsValidIndex(ActivePlayerIndex))
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[ABattleGameState][ActivePlayerChange] Index %d is not valid while Players containing %d elements"), ActivePlayerIndex, PlayersInOrder.Num());
		return;
	}
	OnActivePlayerChanged.Broadcast(PlayersInOrder[ActivePlayerIndex]);
}
