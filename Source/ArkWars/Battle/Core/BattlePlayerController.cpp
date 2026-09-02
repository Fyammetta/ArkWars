// Fill out your copyright notice in the Description page of Project Settings.


#include "BattlePlayerController.h"
#include "BattlePlayerState.h"
#include "ArkWars/Battle/Component/Card/CardComponentBase.h"
#include "ArkWars/Battle/Component/Card/CardManagementBusComponent.h"
#include "ArkWars/Battle/Toolkits/CardContainerInterface.h"

void ABattlePlayerController::TryEndPhase()
{
}

void ABattlePlayerController::TrySelectCard(const FGameplayTagContainer& Card) const
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

void ABattlePlayerController::TryUseCard(const FGameplayTagContainer& Card)
{
	auto PS = GetPlayerState<APlayerState>();
	auto Comp = PS->FindComponentByClass<UCardManagementBusComponent>();

	Server_UseCard(PS, Comp->GetSelectedTargets(), Card);
}

void ABattlePlayerController::TryMoveCard(const TArray<FGameplayTagContainer>& Cards, ICardContainerInterface* From, ICardContainerInterface* To, const FString& Msg)
{
	auto FromObj = From->_getUObject();
	auto ToObj = From->_getUObject();
	
	Server_MoveCard(Cards ,FromObj, ToObj, Msg);
}

void ABattlePlayerController::TryResponse(APlayerState* Target, const FGameplayTagContainer& Source, const FGameplayTagContainer& Card)
{
	Server_Response(Target, Source, Card);
}

void ABattlePlayerController::TryShowCard(const TArray<FGameplayTagContainer>& Cards)
{
	Server_ShowCard(Cards);
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

void ABattlePlayerController::Server_UseCard_Implementation(APlayerState* Source, const TArray<APlayerState*>& Targets, const FGameplayTagContainer& Card)
{
	if (auto Comp = UCardComponentBase::Get(this, Card))
	{
		Comp->Use(Source, Targets, Card);
	}
}

void ABattlePlayerController::Server_MoveCard_Implementation(const TArray<FGameplayTagContainer>& Cards,
	const TScriptInterface<ICardContainerInterface>& From, const TScriptInterface<ICardContainerInterface>& To, const FString& Msg)
{
	From->MoveOut(To.GetInterface(),Cards,Msg);
}

void ABattlePlayerController::Server_Response_Implementation(APlayerState* Target, const FGameplayTagContainer& Source, const FGameplayTagContainer& Card)
{
}

void ABattlePlayerController::Server_ShowCard_Implementation(const TArray<FGameplayTagContainer>& Cards)
{
}

void ABattlePlayerController::Server_StartComparison_Implementation(const TArray<APlayerState*>& Targets)
{
}

void ABattlePlayerController::Server_ResponseComparison_Implementation(APlayerState* Target)
{
}

void ABattlePlayerController::Server_ConfirmComparison_Implementation(const FGameplayTagContainer& Card, bool bIsInitiator)
{
}
