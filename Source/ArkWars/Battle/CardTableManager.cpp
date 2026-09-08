// Fill out your copyright notice in the Description page of Project Settings.


#include "CardTableManager.h"

#include "HeadMountedDisplayTypes.h"
#include "Net/UnrealNetwork.h"
#include "Toolkits/ArkWarTags.h"
#include "Toolkits/GameMessage.h"

TStrongObjectPtr<ACardTableManager> ACardTableManager::Instance = nullptr;

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
	DOREPLIFETIME(ACardTableManager, CardSource)
	DOREPLIFETIME(ACardTableManager, PileArea)
	DOREPLIFETIME(ACardTableManager, DiscardArea)
}

ACardTableManager* ACardTableManager::Get(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}
	
	if (Instance.IsValid() && Instance->GetWorld() == World) return Instance.Get();

	for (auto Actor : World->GetCurrentLevel()->Actors)
	{
		if (Actor && Actor->IsA(StaticClass()))
		{
			Instance = TStrongObjectPtr(Cast<ACardTableManager>(Actor)) ;
			return Instance.Get();
		}
	}
	{
		ENetMode Mode = World->GetNetMode();
		if (Mode != NM_DedicatedServer && Mode != NM_ListenServer) return nullptr;
	}
	
	Instance = TStrongObjectPtr(Cast<ACardTableManager>(World->SpawnActor(StaticClass())));
	return Instance.Get();
}

void ACardTableManager::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACardTableManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	Instance.Reset();
}

void ACardTableManager::OnRep_CachedAreaChanged()
{
}

void ACardTableManager::OnRep_PlayedAreaChanged()
{
}

void ACardTableManager::OnRep_PileAreaChanged()
{
}

void ACardTableManager::OnRep_DiscardAreaChanged()
{
}

const FArkCard* ACardTableManager::GetCardById(int32 CardId) const
{
	return nullptr;
}

TArray<int32> ACardTableManager::Select(const FGameplayTag& Area, const FString& Msg) const
{
	return {};
}

TArray<FArkCard> ACardTableManager::Consume(const FGameplayTag& Area, const TArray<int32>& CardIds)
{
	return {};
}

EAreaWriteResult ACardTableManager::Add(const FGameplayTag& AreaKey, TArray<FArkCard>&& Cards, const FString& Msg)
{
	return EAreaWriteResult::Accepted;
}


TArray<FArkCard> ACardTableManager::GetCardsByKey(const FGameplayTag& Key) const
{
	using namespace CardTags;

	if (Key == Pile)	return PileArea;
	if (Key == Discard) return DiscardArea;
	if (Key == Cache)	return CachedArea;
	if (Key == Used)	return PlayedArea;
	
	return {};
}

TArray<FGameplayTag> ACardTableManager::GetAreaKeys() const
{
	using namespace CardTags;
	return {Pile, Discard, Cache, Used};
}
