// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePhaseManagerComponent.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemComponent.h"
#include "ArkWars/ArkWars.h"
#include "ArkWars/Battle/Toolkits/ArkWarDelegates.h"
#include "GameFramework/GameStateBase.h"
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
	
	if (Phase == EGamePhase::Finish)
	{
		SetNextPlayerActive();
		FinishedPlayerIndex.AddUnique(ActivePlayerIndex);
		if (FinishedPlayerIndex.Num() == PlayersInOrder.Num())
		{
			FinishedPlayerIndex.Reset(PlayersInOrder.Num());
			BroadcastRoundRefresh();
		}
	}
	
	BroadcastPhaseChange();
}

void UGamePhaseManagerComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGamePhaseManagerComponent, CurrentPhase);
	DOREPLIFETIME(UGamePhaseManagerComponent, CachedPhase);
	DOREPLIFETIME(UGamePhaseManagerComponent, PlayersInOrder);
	DOREPLIFETIME(UGamePhaseManagerComponent, ActivePlayerIndex);
	DOREPLIFETIME(UGamePhaseManagerComponent, FinishedPlayerIndex);
}

void UGamePhaseManagerComponent::OnRep_Phase()
{
	//游戏开始的广播由 NetMulticast 执行
	if (CurrentPhase == EGamePhase::GameStart)
	{
		NetMulticast_OnCalledStartGame();
	}
	else
	{
		BroadcastPhaseChange();
	}
}

void UGamePhaseManagerComponent::BroadcastPhaseChange() const
{
	OnGamePhaseChanged.Broadcast(GamePhase::GetPhaseTag(CachedPhase), GamePhase::GetPhaseTag(CurrentPhase));
}

void UGamePhaseManagerComponent::SetNextPlayerActive(int32 Index)
{
	if (PlayersInOrder.IsEmpty())
	{
		
		return;
	}
	
	ActivePlayerIndex = Index == INDEX_NONE ? (ActivePlayerIndex + 1) % PlayersInOrder.Num() : Index % PlayersInOrder.Num();
	BroadcastActivePlayerChange();
}

void UGamePhaseManagerComponent::InitPlayers(int32 Start)
{
	auto GS = Cast<AGameStateBase>(GetOwner());

	if (!GS)
	{
		GS = GetWorld() ? GetWorld()->GetGameState() : nullptr;
	}
	if (!GS || GS->PlayerArray.IsEmpty())
	{
		return;
	}

	PlayersInOrder.Reset(PlayersInOrder.Num());
	FinishedPlayerIndex.Reset(PlayersInOrder.Num());
	for (int i = Start; i < Start + GS->PlayerArray.Num(); ++i)
	{
		PlayersInOrder.Add(GS->PlayerArray[i %  GS->PlayerArray.Num()]);
	}
	ActivePlayerIndex = 0;

}

int32 UGamePhaseManagerComponent::GetPlayerIndex(APlayerState* Player) const
{
	if (!Player) return ActivePlayerIndex;
	
	return PlayersInOrder.Find(Player);
}

APlayerState* UGamePhaseManagerComponent::GetPlayerByIndex(int32 Index) const
{
	if (PlayersInOrder.IsEmpty()) return nullptr;
	
	return PlayersInOrder.IsValidIndex(Index) ? PlayersInOrder[Index % PlayersInOrder.Num()] : PlayersInOrder[ActivePlayerIndex];
}

int32 UGamePhaseManagerComponent::GetPlayerCount() const
{
	return PlayersInOrder.Num();
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

void UGamePhaseManagerComponent::BroadcastRoundRefresh() const
{
	OnGameRoundChanged.Broadcast();
}

void UGamePhaseManagerComponent::NetMulticast_OnCalledStartGame_Implementation()
{
	BroadcastPhaseChange();
}

void UGamePhaseManagerComponent::OnCalledStartGame()
{
	if (!GetOwner()->HasAuthority()) return;

	if (CurrentPhase != EGamePhase::GameStart || CachedPhase != EGamePhase::GameStart )
	{
		CurrentPhase = EGamePhase::GameStart;
		CachedPhase = EGamePhase::GameStart;
		NetMulticast_OnCalledStartGame_Implementation();
	}
	else
	{
		NetMulticast_OnCalledStartGame();
	}
}