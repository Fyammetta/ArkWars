// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultModeComponent.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/GameState.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "ArkWars/ArkWars.h"
#include "ArkWars/Battle/Component/GameFlow/GamePhaseManagerComponent.h"
#include "ArkWars/Battle/System/BattleGameFlowSubsystem.h"
#include "ArkWars/Battle/Toolkits/ArkWarTags.h"
#include "ArkWars/Battle/Toolkits/BattleFunctionLibrary.h"

namespace 
{
	TArray<FGameplayTag> GetDesignedIdentity(int32 Num)
	{
		TArray<FGameplayTag> RetVal{};
		switch (Num)
		{
			case 10:	RetVal.Add(IdentityTags::Operator);
			case 9:		RetVal.Add(IdentityTags::Spy);
			case 8:		RetVal.Add(IdentityTags::Raider);
			case 7:		RetVal.Add(IdentityTags::Operator);
			case 6:		RetVal.Add(IdentityTags::Raider);
			case 5:		RetVal.Add(IdentityTags::Raider);
			case 4:		RetVal.Add(IdentityTags::Operator);
			case 3:		RetVal.Add(IdentityTags::Spy);
			case 2:		RetVal.Add(IdentityTags::Raider) ; break;
			default: return {};
		}
		
		RetVal.Add(IdentityTags::Commander);
		return RetVal;
	}
	
	bool IsCommander(APlayerState* Player)
	{
		if (auto Interface = Cast<IAbilitySystemInterface>(Player))
		{
			if (auto Comp = Interface->GetAbilitySystemComponent())
			{
				return Comp->HasMatchingGameplayTag(IdentityTags::Commander);
			}
		}
		
		return false;
	};
}

int32 UDefaultModeComponent::AllocateIdentity()
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
	
	//分配身份，当前设计末位（首个出栈的Tag)为主公
	TArray<FGameplayTag> Identities = GetDesignedIdentity(AllPlayers.Num());
	
	auto Identity = Identities.Pop();
	auto Start = FMath::RandRange(0, AllPlayers.Num() - 1);
	
	
	UAbilitySystemComponent* Comp = nullptr;
	//逐个分配身份直到身份分配数组无残留元素
	while (true)
	{
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
		//先添加身份，然后将下一个出栈的元素设为随机，保持出栈性能的同时不使得顺序死板
		Comp->AddLooseGameplayTag(Identity);
		UE_LOG(LogGamePlay, Log, TEXT("[UGameModeComponentBase][AllocateIdentity] Allocate identity %s to layer: %s"),*Identity.ToString(), *Comp->GetOwner()->GetName())

		//Start为当局的起始位，并通过身份数组的结构约束为主公
		if (Identities.IsEmpty()) return Start;
		
		auto Lasting = Identities.Num();
		Identities.Swap(Lasting - 1, FMath::RandRange(0, Lasting - 1));
		Identity = Identities.Pop();
	}

}

int32 UDefaultModeComponent::GetStartCardNum(UAbilitySystemComponent* Asc)
{
	return 4;
}

void UDefaultModeComponent::SentSelectOperatorNotify()
{
	TArray<APlayerState*> AllPlayers;
	{
		auto World = GetWorld();
		if (!World) return ;
		auto GameState = World->GetGameState();
		if (!GameState) return ;
		AllPlayers = GameState->PlayerArray;
		if (AllPlayers.IsEmpty()) return ;
	}
	
	for (APlayerState* Player : AllPlayers)
	{
		UnRegisteredPlayers.Add(Player);
		if (IsCommander(Player))
		{
			//TODO: 主公先选
		}
	}
}

void UDefaultModeComponent::CheckOperatorSelection(APlayerState* Player)
{
	Super::CheckOperatorSelection(Player);
	
	if (!IsCommander(Player)) return ;
	
	//TODO: 其他角色分别选
	
	
}

void UDefaultModeComponent::InitCardDeck()
{
	
}
