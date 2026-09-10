// Fill out your copyright notice in the Description page of Project Settings.


#include "FactionsBattleModeComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "ArkWars/ArkWars.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemComponent.h"
#include "ArkWars/Battle/Toolkits/ArkWarTags.h"
#include "GameFramework/GameStateBase.h"


namespace 
{
	TArray<FGameplayTag> GetDesignedIdentity()
	{
		static TArray<FGameplayTag> RetVal
		{
			IdentityTags::Commander,
			IdentityTags::Operator,
			IdentityTags::Operator,
			IdentityTags::Spy,
			IdentityTags::Raider,
			IdentityTags::Raider
		};
		return RetVal;
	}
}

void UFactionsBattleModeComponent::InitCardDeck()
{
}

int32 UFactionsBattleModeComponent::AllocateIdentity()
{
	//获取所有玩家，顺序按照GameState中的玩家数组顺序，以服务器中的数组为准
	TArray<APlayerState*> AllPlayers;
	{
		auto World = GetWorld();
		if (!World) return INDEX_NONE;
		auto GameState = World->GetGameState();
		if (!GameState) return INDEX_NONE;
		AllPlayers = GameState->PlayerArray;
		if (AllPlayers.IsEmpty()) return INDEX_NONE;
	}
	
	TArray<FGameplayTag> Identities = GetDesignedIdentity();
	
	auto Start = FMath::RandRange(0, AllPlayers.Num() - 1);
	
	StartPlayer = AllPlayers[Start];
	UAbilitySystemComponent* Comp = nullptr;
	//逐个分配身份直到身份分配数组无残留元素
	while (!Identities.IsEmpty())
	{
		auto Lasting = Identities.Num();
		Identities.Swap(Lasting - 1, FMath::RandRange(0, Lasting - 1));
		auto Identity = Identities.Pop();
		if (auto Target = AllPlayers[(Start++)%AllPlayers.Num()])
		{
			if (auto Interface = Cast<IAbilitySystemInterface>(Target))
				Comp = Interface->GetAbilitySystemComponent();
			if (!Comp)
			{
				UE_LOG(LogGamePlay, Warning, TEXT("[UGameModeComponentBase][AllocateIdentity] Player: %s do not own AbilitySystemComponent"),*Target->GetName())
				return INDEX_NONE;
			}
		}
		else
		{
			UE_LOG(LogGamePlay, Warning, TEXT("[UGameModeComponentBase][AllocateIdentity] Found a player do not exist"))
			return INDEX_NONE;
		}
		Comp->AddLooseGameplayTag(Identity);
		UE_LOG(LogGamePlay, Log, TEXT("[UGameModeComponentBase][AllocateIdentity] Allocate identity %s to layer: %s"),*Identity.ToString(), *Comp->GetOwner()->GetName())
		
	}
	
	return Start;

}

int32 UFactionsBattleModeComponent::GetStartCardNum(UAbilitySystemComponent* Asc)
{
	return 4;
}

void UFactionsBattleModeComponent::SentSelectOperatorNotify()
{
}

void UFactionsBattleModeComponent::CheckOperatorSelection(APlayerState* Player)
{
	UnRegisteredPlayers.Remove(Player);

	if (StartPlayer.IsValid() && Player == StartPlayer.Get())
	{
		//TODO: 其他角色分别选
		
		return;
	}
	
	
	if (UnRegisteredPlayers.IsEmpty())
	{
		
	}
		
}
