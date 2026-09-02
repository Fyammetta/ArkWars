// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleGameFlowSubsystem.h"

bool UBattleGameFlowSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (const UWorld* World = Cast<UWorld>(Outer))
	{
		if (const FWorldContext* Ctx = GEngine->GetWorldContextFromWorld(World))
		{
			if (Ctx->LastURL.HasOption(TEXT("listen")))
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
