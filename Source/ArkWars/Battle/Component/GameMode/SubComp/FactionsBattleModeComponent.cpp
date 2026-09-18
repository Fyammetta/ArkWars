// Fill out your copyright notice in the Description page of Project Settings.


#include "FactionsBattleModeComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "ArkWars/ArkWars.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemComponent.h"
#include "ArkWars/Battle/Core/BattlePlayerController.h"
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
	
	constexpr int32 OperatorToSelect = 5;
	constexpr int32 StartCountAddition = 3;
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
	if (!StartPlayer.IsValid()) return;
	
	//	候选经本人 PC 定向下发（卷 11 §5.2）：取不到 PC = 界面收不到候选，玩家卡在选角，故留一条 Warning
	if (auto PC = Cast<ABattlePlayerController>(StartPlayer->GetPlayerController()))
	{
		TArray<FName> ToSelect {};
		for (int i = 0; i < OperatorToSelect + StartCountAddition; ++i)
		{
			auto Max = TotalOperators.Num() - 1;
			TotalOperators.Swap(Max, FMath::RandRange(0, Max));

			ToSelect.Add(TotalOperators.Pop());
		}
		PC->Client_NotifySelectOperator(ToSelect);
	}
	else
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[UFactionsBattleModeComponent][SentSelectOperatorNotify] No battle player controller for %s, candidate dispatch skipped"), *GetNameSafe(StartPlayer.Get()))
	}
}

void UFactionsBattleModeComponent::CheckOperatorSelection(APlayerState* Player, const FName& Operator)
{
	UnRegisteredPlayers.Remove(Player);
	SelectedOperators.Add(Operator);
	if (StartPlayer.IsValid() && Player == StartPlayer.Get())
	{
		
		//TODO: 其他角色分别选
		for (auto UnRegisteredPlayer : UnRegisteredPlayers)
		{
			//	候选经本人 PC 定向下发（卷 11 §5.2）：取不到 PC = 界面收不到候选，玩家卡在选角，故留一条 Warning
			if (auto PC = UnRegisteredPlayer.IsValid() ? Cast<ABattlePlayerController>(UnRegisteredPlayer->GetPlayerController()) : nullptr)
			{
				TArray<FName> ToSelect {};
				for (int i = 0; i < OperatorToSelect; ++i)
				{
					auto Max = TotalOperators.Num() - 1;
					TotalOperators.Swap(Max, FMath::RandRange(0, Max));

					ToSelect.Add(TotalOperators.Pop());
				}
				PC->Client_NotifySelectOperator(ToSelect);
			}
			else
			{
				UE_LOG(LogGamePlay, Warning, TEXT("[UFactionsBattleModeComponent][CheckOperatorSelection] No battle player controller for %s, candidate dispatch skipped"), *GetNameSafe(UnRegisteredPlayer.Get()))
			}
		}
		return;
	}
	
	
	if (UnRegisteredPlayers.IsEmpty())
	{
		
	}
		
}
