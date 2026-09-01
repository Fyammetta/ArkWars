// Fill out your copyright notice in the Description page of Project Settings.


#include "BattlePlayerController.h"
#include "BattlePlayerState.h"
#include "ArkWars/Battle/Component/Card/CardManagementBusComponent.h"
#include "ArkWars/Battle/Toolkits/CardContainerInterface.h"

void ABattlePlayerController::TryEndPhase()
{
}

void ABattlePlayerController::TrySelectCard(const FCard& Card) const
{
	auto PS = GetPlayerState<AActor>();
	if (!PS)
	{
		return;
	}
	auto Comp = PS->FindComponentByClass<UCardManagementBusComponent>();
	if (!Comp)
	{
		return;
	}
	Comp->SelectCard(Card);
}

void ABattlePlayerController::TryUseCard(const FCard& Card)
{
	auto PS = GetPlayerState<AActor>();
	if (!PS)
	{
		return;
	}
	auto Comp = PS->FindComponentByClass<UCardManagementBusComponent>();
	if (!Comp)
	{
		return;
	}

	auto Index = Comp->GetIndexOfCard(Card);
	if (Index == INDEX_NONE)
	{
		return;
	}
	Server_UseCard(Index);
	
}

void ABattlePlayerController::TryMoveCard(const TArray<FGameplayTagContainer>& Cards, ICardContainerInterface* From, ICardContainerInterface* To, const FString& Msg)
{
	auto FromObj = From ? From->_getUObject() : nullptr;
	auto ToObj = To ? To->_getUObject() : nullptr;

	Server_MoveCard(Cards ,FromObj, ToObj, Msg);
}

void ABattlePlayerController::TryResponse(APlayerState* Target, const FGameplayTagContainer& Source, const FCard& Card)
{
	auto PS = GetPlayerState<AActor>();
	if (!PS)
	{
		return;
	}
	auto Comp = PS->FindComponentByClass<UCardManagementBusComponent>();
	if (!Comp)
	{
		return;
	}

	auto Index = Comp->GetIndexOfCard(Card);
	
	Server_Response(Target, Source, Index);
}

void ABattlePlayerController::TryShowCard(const TArray<FCard>& Cards)
{
	if (Cards.IsEmpty())
	{
		return;
	}
	auto PS = GetPlayerState<AActor>();
	if (!PS)
	{
		return;
	}
	auto Comp = PS->FindComponentByClass<UCardManagementBusComponent>();
	if (!Comp)
	{
		return;
	}

	TArray<int32> IndexArray{};
	auto CachedCards = Comp->GetAllHandCard();
	for (FCard C : Cards)
	{
		auto Index = CachedCards.Find(C);
		if (Index != INDEX_NONE)
			IndexArray.Add(CachedCards.Find(C));
	}
	
	Server_ShowCard(IndexArray);
}

void ABattlePlayerController::TryStartComparison(const TArray<APlayerState*>& Targets)
{
	if (Targets.IsEmpty())
	{
		return;
	}
	Server_StartComparison(Targets);
}

void ABattlePlayerController::TryResponseComparison(APlayerState* Target)
{
	Server_ResponseComparison(Target);
}

void ABattlePlayerController::TryConfirm(const TFunction<void(APlayerController*)>& CustomEvent)
{
	if (CustomEvent)
	{
		CustomEvent(this);
	}
}

void ABattlePlayerController::TryActivateSkill(const FGameplayTag& SkillTag)
{
}

void ABattlePlayerController::Server_UseCard_Implementation(int32 Index)
{
	auto PS = GetPlayerState<AActor>();
	if (!PS)
	{
		return;
	}
	auto Comp = PS->FindComponentByClass<UCardManagementBusComponent>();
	if (!Comp)
	{
		return;
	}
	
	Comp->UseCard(Index);
	
}

void ABattlePlayerController::Server_MoveCard_Implementation(const TArray<FGameplayTagContainer>& Cards,
	const TScriptInterface<ICardContainerInterface>& From, const TScriptInterface<ICardContainerInterface>& To, const FString& Msg)
{
}

void ABattlePlayerController::Server_Response_Implementation(APlayerState* Target, const FGameplayTagContainer& Source, int32 Index)
{
}

void ABattlePlayerController::Server_ShowCard_Implementation(const TArray<int32>& Cards)
{
}

void ABattlePlayerController::Server_StartComparison_Implementation(const TArray<APlayerState*>& Targets)
{
}

void ABattlePlayerController::Server_ResponseComparison_Implementation(APlayerState* Target)
{
}

void ABattlePlayerController::Server_ConfirmComparison_Implementation(int32 Index, bool bIsInitiator)
{
}
