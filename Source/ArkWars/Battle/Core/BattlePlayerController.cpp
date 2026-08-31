// Fill out your copyright notice in the Description page of Project Settings.


#include "BattlePlayerController.h"

void ABattlePlayerController::TryEndPhase()
{
}

void ABattlePlayerController::TrySelectCard(const FCard& Card)
{
}

void ABattlePlayerController::TryUseCard(const FCard& Card)
{
}

void ABattlePlayerController::TryMoveCard(const FCard& Card, ICardContainerInterface* From, ICardContainerInterface* To)
{
}

void ABattlePlayerController::TryResponse(APlayerState* Target, const FCard& Source, const FCard& Card)
{
}

void ABattlePlayerController::TryShowCard(const TArray<FCard>& Cards)
{
}

void ABattlePlayerController::TryStartComparison(TArray<APlayerState>* Targets)
{
}

void ABattlePlayerController::TryResponseComparison(APlayerState* Target)
{
}

void ABattlePlayerController::TryConfirm(const TFunction<void()>& CustomEvent)
{
}

void ABattlePlayerController::TryActivateSkill(const FGameplayTag& SkillTag)
{
}

void ABattlePlayerController::Server_UseCard_Implementation(int32 Index)
{
}

void ABattlePlayerController::Server_MoveCard_Implementation(int32 Index, const TScriptInterface<ICardContainerInterface>& From, const TScriptInterface<ICardContainerInterface>& To)
{
}

