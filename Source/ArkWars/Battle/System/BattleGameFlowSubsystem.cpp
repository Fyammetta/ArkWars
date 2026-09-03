// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleGameFlowSubsystem.h"

#include "ArkWars/Battle/Component/GameFlow/GamePhaseManagerComponent.h"
#include "ArkWars/Battle/Toolkits/ArkWarFlowTypes.h"
#include "GameFramework/GameStateBase.h"

bool UBattleGameFlowSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (const UWorld* World = Cast<UWorld>(Outer))
	{
		if (const FWorldContext* Ctx = GEngine->GetWorldContextFromWorld(World))
		{
			if (World->GetNetMode() != NM_Standalone)
			{
				return true; 
			}
		}
	}
	return false; 
}

void UBattleGameFlowSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UBattleGameFlowSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UBattleGameFlowSubsystem::InitPlayerOrder(int32 StartIndex)
{
}

void UBattleGameFlowSubsystem::RegisterManagerActor(AActor* Actor)
{
	if (Actor && !ManagerActor)
		ManagerActor = Actor;
}

AActor* UBattleGameFlowSubsystem::GetManagerActor() const
{
	return ManagerActor;
}

bool UBattleGameFlowSubsystem::IsRunningOnServer() const
{
	return GetWorld() ? GetWorld()->GetNetMode() < NM_Client : false;
}

void UBattleGameFlowSubsystem::PushPhase(const FString& Msg)
{
	UGamePhaseManagerComponent* Comp = nullptr;
	if (auto GS = GetWorld() ? GetWorld()->GetAuthGameMode() : nullptr)
	{
		Comp = GS->FindComponentByClass<UGamePhaseManagerComponent>();
	}
	
	if (!Comp)
	{
		return;
	}
	if (Msg.IsEmpty())
	{
		auto Phase = Comp->GetPhase();
		if (Phase == EGamePhase::GameStart)
		{
			Comp->SetPhase(EGamePhase::Begin);
		}
		else
		{
			auto CurPhase = static_cast<int32>(Comp->GetPhase());
			
			auto TarPhase = CurPhase % GamePhase::PhaseCount + 1;
			Phase = static_cast<EGamePhase>(TarPhase);
			if (Phase == EGamePhase::GameStart)
				Comp->SetNextPlayerActive();
			Comp->SetPhase(Phase);
		}
	}
		
}

