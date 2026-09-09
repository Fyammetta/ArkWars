// Fill out your copyright notice in the Description page of Project Settings.


#include "BattlePlayerState.h"
#include "AbilitySystemComponent.h"
#include "ArkWars/Battle/Component/Card/CardManagementBusComponent.h"

ABattlePlayerState::ABattlePlayerState()
{
	Asc = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
}

TArray<FArkCard> ABattlePlayerState::GetCardsByKey(const FGameplayTag& Area) const
{
	ICardContainerInterface* Container = nullptr;
	for (UActorComponent* Component : GetComponentsByInterface(UCardManagementBusComponent::StaticClass()))
	{
		Container = Cast<ICardContainerInterface>(Component);
		if (Container->GetAreaKeys().Contains(Area))
			return Container->GetCardsByKey(Area);
	}
	
	return {};
}

const FArkCard* ABattlePlayerState::GetCardById(int32 CardId) const
{
	ICardContainerInterface* Container = nullptr;
	for (UActorComponent* Component : GetComponentsByInterface(UCardManagementBusComponent::StaticClass()))
	{
		Container = Cast<ICardContainerInterface>(Component);
		if (Container->GetCardById(CardId))
			return Container->GetCardById(CardId);
	}
	
	return nullptr;
}

TArray<int32> ABattlePlayerState::Select(const FGameplayTag& Area, const FMessageType& Msg) const
{
	ICardContainerInterface* Container = nullptr;
	for (UActorComponent* Component : GetComponentsByInterface(UCardManagementBusComponent::StaticClass()))
	{
		Container = Cast<ICardContainerInterface>(Component);
		if (Container->GetAreaKeys().Contains(Area))
			return Container->Select(Area, Msg);
	}
	
	return {};}

TArray<FArkCard> ABattlePlayerState::Consume(const FGameplayTag& Area, const TArray<int32>& CardIds)
{
	ICardContainerInterface* Container = nullptr;
	for (UActorComponent* Component : GetComponentsByInterface(UCardManagementBusComponent::StaticClass()))
	{
		Container = Cast<ICardContainerInterface>(Component);
		if (Container->GetAreaKeys().Contains(Area))
			return Container->Consume(Area, CardIds);
	}
	
	return {};
}

EAreaWriteResult ABattlePlayerState::Add(const FGameplayTag& AreaKey, TArray<FArkCard>& Cards, const FMessageType& Msg)
{
	ICardContainerInterface* Container = nullptr;
	for (UActorComponent* Component : GetComponentsByInterface(UCardManagementBusComponent::StaticClass()))
	{
		Container = Cast<ICardContainerInterface>(Component);
		if (Container->GetAreaKeys().Contains(AreaKey))
			return Container->Add(AreaKey, Cards, Msg);
	}
	
	return {};
}

TArray<FGameplayTag> ABattlePlayerState::GetAreaKeys() const
{
	TArray<FGameplayTag> AreaKeys{};
	ICardContainerInterface* Container = nullptr;
	for (UActorComponent* Component : GetComponentsByInterface(UCardManagementBusComponent::StaticClass()))
	{
		AreaKeys.Append(Cast<ICardContainerInterface>(Component)->GetAreaKeys());
	}
	
	return AreaKeys;
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


