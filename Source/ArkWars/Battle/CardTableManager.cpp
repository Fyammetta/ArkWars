// Fill out your copyright notice in the Description page of Project Settings.


#include "CardTableManager.h"

#include "Net/UnrealNetwork.h"


// Sets default values
ACardTableManager::ACardTableManager()
{
	SetReplicates(true);
}

void ACardTableManager::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ACardTableManager, CachedArea)
	DOREPLIFETIME(ACardTableManager, PlayedArea)
}

void ACardTableManager::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACardTableManager::OnRep_CachedAreaChanged()
{
}

void ACardTableManager::OnRep_PlayedAreaChanged()
{
}

void ACardTableManager::MoveIn(ICardContainerInterface* From, TArray<FGameplayTagContainer>&& Cards,
	const FString& Msg)
{
}

void ACardTableManager::MoveOut(ICardContainerInterface* To, const TArray<FGameplayTagContainer>& Cards,
	const FString& Msg)
{
}
