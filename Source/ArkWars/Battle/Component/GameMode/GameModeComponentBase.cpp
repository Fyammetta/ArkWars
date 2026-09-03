
#include "GameModeComponentBase.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "ArkWars/ArkWars.h"
#include "ArkWars/Battle/Core/BattleGameState.h"
#include "ArkWars/Battle/System/BattleGameFlowSubsystem.h"
#include "ArkWars/Battle/Toolkits/ArkWarTags.h"
#include "ArkWars/Battle/Toolkits/BattleFunctionLibrary.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

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
}

void UGameModeComponentBase::InitCardDeck()
{
}

void UGameModeComponentBase::AllocateIdentity()
{
	//获取所有玩家，顺序按照GameState中的玩家数组顺序，以服务器中的数组为准
	TArray<APlayerState*> AllPlayers;
	{
		auto World = GetWorld();
		if (!World) return;
		auto GameState = World->GetGameState();
		if (!GameState) return;
		AllPlayers = GameState->PlayerArray;
		if (AllPlayers.IsEmpty()) return;
	}
	
	//分配身份，当前设计末位（首个出栈的Tag)为主公
	TArray<FGameplayTag> Identities = GetDesignedIdentity(AllPlayers.Num());
	
	auto Identity = Identities.Pop();
	auto Start = FMath::RandRange(0, AllPlayers.Num() - 1);
	
	//Start为当局的起始位，并通过身份数组的结构约束为主公
	if (auto Subsystem = UBattleFunctionLibrary::GetBattleManager(this))
	{
		Subsystem->InitPlayerOrder(Start);
	}
	{
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
					return;
				}
			}
			else
			{
				UE_LOG(LogGamePlay, Warning, TEXT("[UGameModeComponentBase][AllocateIdentity] Found a player do not exist"))
				return;
			}
			//先添加身份，然后将下一个出栈的元素设为随机，保持出栈性能的同时不使得顺序死板
			Comp->AddLooseGameplayTag(Identity);
			UE_LOG(LogGamePlay, Log, TEXT("[UGameModeComponentBase][AllocateIdentity] Allocate identity %s to layer: %s"),*Identity.ToString(), *Comp->GetOwner()->GetName())

			if (Identities.IsEmpty()) return;
			
			auto Lasting = Identities.Num();
			Identities.Swap(Lasting - 1, FMath::RandRange(0, Lasting - 1));
			Identity = Identities.Pop();
		}
	}
}

