// Fill out your copyright notice in the Description page of Project Settings.


#include "CardManagementBusComponent.h"
#include "GameFramework/PlayerState.h"
#include "CardComponentBase.h"
#include "Net/UnrealNetwork.h"


UCardManagementBusComponent::UCardManagementBusComponent()
{
	SetIsReplicatedByDefault(true);
}

void UCardManagementBusComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCardManagementBusComponent, EquipmentArea);
}

TArray<FCard> UCardManagementBusComponent::GetAllHandCard(FGameplayTagContainer* Filter) const
{
	auto OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return {};
	}
	
	TArray<FCard> RetVal = {};
	TArray<FCard> Append = {};
	
	if (Filter)
	{
		for (const FGameplayTag& Tag : Filter->GetGameplayTagArray())
		{
			FName CompTag = Tag.GetTagName();
			if (auto CardComp = Cast<UCardComponentBase>(OwnerActor->FindComponentByTag(UCardComponentBase::StaticClass(), CompTag)))
			{
				if (CardComp->GetCards(Append))
					RetVal.Append(MoveTemp(Append));
			}
		}
	}
	else
	{
		for (UActorComponent* Comp : OwnerActor->GetComponents())
		{
			if (auto CardComp = Cast<UCardComponentBase>(Comp))
			{
				if (CardComp->GetCards(Append) > 0)
					RetVal.Append(MoveTemp(Append));
			}
		}
	}
	
	return RetVal;
}

int32 UCardManagementBusComponent::GetIndexOfCard(const FCard& Card) const
{
	return GetAllHandCard().Find(Card);
}

void UCardManagementBusComponent::SelectCard(const FCard& Card)
{
	SelectedCards.Add(Card);
}

void UCardManagementBusComponent::ClearCardSelection(const FCard& Card)
{
	SelectedCards.Remove(Card);
}

TArray<FCard> UCardManagementBusComponent::GetSelectedCards()
{
	auto Arr = MoveTemp(SelectedCards);
	OnCardSelectionChanged.Broadcast(Arr, true);
	return MoveTemp(Arr);
}

void UCardManagementBusComponent::SelectTarget(APlayerState* Target)
{
	SelectedPlayers.Add(Target);
}

void UCardManagementBusComponent::ClearTargetsSelection(APlayerState* Target)
{
	SelectedPlayers.Remove(Target);
}

TArray<TWeakObjectPtr<APlayerState>> UCardManagementBusComponent::GetSelectedTargets()
{
	auto Arr = MoveTemp(SelectedPlayers);
	OnPlayerSelectionChanged.Broadcast(Arr, true);
	return MoveTemp(Arr);
}

void UCardManagementBusComponent::UseCard(int32 Index)
{
}

void UCardManagementBusComponent::MoveIn(ICardContainerInterface* From, TArray<FGameplayTagContainer>&& Cards, const FString& Msg)
{
}

void UCardManagementBusComponent::MoveOut(ICardContainerInterface* To, TArray<FGameplayTagContainer>&& Cards, const FString& Msg)
{
}
