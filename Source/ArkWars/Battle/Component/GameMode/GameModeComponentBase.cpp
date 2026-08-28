
#include "GameModeComponentBase.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "ArkWars/Battle/Toolkits/ArkWarTags.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

namespace 
{
	TArray<FGameplayTag> GetDesignedIdentity(int32 Num)
	{
		TArray<FGameplayTag> RetVal{};
		switch (Num)
		{
			case 10:	RetVal.Add(IdentityTags::Operator());
			case 9:		RetVal.Add(IdentityTags::Spy());
			case 8:		RetVal.Add(IdentityTags::Raider());
			case 7:		RetVal.Add(IdentityTags::Operator());
			case 6:		RetVal.Add(IdentityTags::Raider());
			case 5:		RetVal.Add(IdentityTags::Raider());
			case 4:		RetVal.Add(IdentityTags::Operator());
			case 3:		RetVal.Add(IdentityTags::Spy());
			case 2:		RetVal.Add(IdentityTags::Raider()) ; break;
			default: return {};
		}
		
		RetVal.Add(IdentityTags::Commander());
		return RetVal;
	}
}

FChangePhaseDelegate& UGameModeComponentBase::GetPhaseChangeDelegate(EGamePhase::Type Phase, FChangePhaseDelegatePair::Type Timing)
{
	return PhaseDelegates[Phase].Get(Timing);
}

UGameModeComponentBase::UGameModeComponentBase()
{
	PhaseDelegates.Init(FChangePhaseDelegatePair(),4);
}

void UGameModeComponentBase::InitCardDeck()
{
}

void UGameModeComponentBase::AllocateIdentity()
{
	TArray<APlayerState*> TempPlayers;
	{
		auto World = GetWorld();
		if (!World) return;
		auto GameState = World->GetGameState();
		if (!GameState) return;
		Players = GameState->PlayerArray;
		TempPlayers = Players;
		if (Players.IsEmpty()) return;
	}
	TArray<FGameplayTag> Identities = GetDesignedIdentity(Players.Num());
	
	APlayerState* CommanderPlayer = nullptr;
	while (!TempPlayers.IsEmpty())
	{
		UAbilitySystemComponent* Comp = nullptr;
		{
			auto Target = Players[FMath::RandRange(0, Players.Num() - 1)];
			if (!CommanderPlayer) CommanderPlayer = Target;
			Players.Remove(Target);
			if (auto Interface = Cast<IAbilitySystemInterface>(Target))
			Comp = Interface->GetAbilitySystemComponent();
			if (!Comp) return;
		}
		
		Comp->AddLooseGameplayTag(Identities.Pop());
	}
	
	check(CommanderPlayer);
	
	ActorIndex = Players.Find(CommanderPlayer);

	for (int i = ActorIndex; i < ActorIndex + Players.Num(); ++i)
	{
		RequestCard(Players[i%Players.Num()],TEXT("[Num]=4"));
	}
}

void UGameModeComponentBase::RequestCard(APlayerState* Player, const FString& Msg)
{
	TArray<FGameplayTag> Cards;
	//TODO: 解析Msg，根据需求填入卡牌
	
	
	BroadcastCardToPlayer(Player, Cards);
}

void UGameModeComponentBase::Discard(APlayerState* Player, const TArray<FGameplayTag>& Cards, const FString& Msg)
{
	DiscardCache.Append(Cards);
}

void UGameModeComponentBase::ChangeGamePhase(const FString& Msg)
{
	EGamePhase::Roll(CurrentPhase);
}

APlayerState* UGameModeComponentBase::GetStageOwner() const
{
	return Players[ActorIndex];
}

void UGameModeComponentBase::BroadcastCardToPlayer(APlayerState* Player, const TArray<FGameplayTag>& Cards)
{
	
}

void UGameModeComponentBase::ClearDiscardCache()
{
	//TODO: 将Cache中的卡牌移交给Subsystem的弃牌区
}