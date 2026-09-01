// Fill out your copyright notice in the Description page of Project Settings.


#include "BattlePlayerState.h"
#include "AbilitySystemComponent.h"
#include "ArkWars/Battle/Component/Card/CardManagementBusComponent.h"

ABattlePlayerState::ABattlePlayerState()
{
	Asc = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
}

void ABattlePlayerState::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		auto Comp = NewObject<UCardManagementBusComponent>(this,UCardManagementBusComponent::StaticClass(), TEXT("CardManagementBus"));
		Comp->RegisterComponent();
	}
}


