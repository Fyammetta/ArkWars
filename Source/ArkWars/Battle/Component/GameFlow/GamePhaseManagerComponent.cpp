// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePhaseManagerComponent.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "ArkWars/ArkWars.h"
#include "ArkWars/Battle/Toolkits/ArkWarTags.h"
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
		FinishedPlayerIndex.Add(ActivePlayerIndex);
		if (FinishedPlayerIndex.Num() == PlayersInOrder.Num())
		{
			FinishedPlayerIndex.Empty();
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
}

void UGamePhaseManagerComponent::OnRep_Phase() const
{
	BroadcastPhaseChange();
}

void UGamePhaseManagerComponent::BroadcastPhaseChange() const
{
	OnGamePhaseChanged.Broadcast(GamePhase::GetPhaseTag(CachedPhase), GamePhase::GetPhaseTag(CurrentPhase));
}


void UGamePhaseManagerComponent::SetNextPlayerActive(int32 Index)
{
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
	if (!GS)
	{
		return;
	}
	PlayersInOrder = GS->PlayerArray;
	auto Interface = Cast<IAbilitySystemInterface>(PlayersInOrder[Start]);
	
	if (Interface && Interface->GetAbilitySystemComponent())
	{
		auto ASC = Interface->GetAbilitySystemComponent();
		if (ASC->HasMatchingGameplayTag(IdentityTags::Commander))
			ActivePlayerIndex = Start;
		else
		{
			for (APlayerState* Player : PlayersInOrder)
			{
				auto Target = Cast<IAbilitySystemInterface>(Player);
				auto Comp = Target ? Target->GetAbilitySystemComponent() : nullptr;
				if (Comp && Comp->HasMatchingGameplayTag(IdentityTags::Commander))
				{
					ActivePlayerIndex = PlayersInOrder.Find(Player);
					return;
				}
			}
		}
	}
}

int32 UGamePhaseManagerComponent::GetPlayerIndex(APlayerState* Player) const
{
	if (!Player) return ActivePlayerIndex;
	
	return PlayersInOrder.Find(Player);
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
}
