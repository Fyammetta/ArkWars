// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleFunctionLibrary.h"
#include "ArkWars/Battle/System/SkillManagerSubsystem.h"
#include "ArkWars/Battle/System/CardManagerSubsystem.h"
#include "ArkWars/Battle/System/BattleGameFlowSubsystem.h"

UCardManagerSubsystem* UBattleFunctionLibrary::GetCardManager(const UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;
	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return nullptr;
	UGameInstance* GI = World->GetGameInstance();
	if (!GI) return nullptr;
	
	return GI->GetSubsystem<UCardManagerSubsystem>();
}

USkillManagerSubsystem* UBattleFunctionLibrary::GetSkillManager(const UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;
	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return nullptr;

	return World->GetSubsystem<USkillManagerSubsystem>();
}

UBattleGameFlowSubsystem* UBattleFunctionLibrary::GetBattleManager(const UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;
	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return nullptr;

	return World->GetSubsystem<UBattleGameFlowSubsystem>();
}
