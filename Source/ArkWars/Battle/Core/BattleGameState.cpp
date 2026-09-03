// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleGameState.h"
#include "ArkWars/Battle/Component/GameFlow/GamePhaseManagerComponent.h"



void ABattleGameState::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		auto Comp = NewObject<UGamePhaseManagerComponent>(this, UGamePhaseManagerComponent::StaticClass(), TEXT("GamePhaseManager"));
		Comp->RegisterComponent();
	}
}
