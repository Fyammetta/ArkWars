// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleFunctionLibrary.h"
#include "ArkWars/Battle/System/CardManagerSubsystem.h"

UCardManagerSubsystem* UBattleFunctionLibrary::GetCardManager(UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;
	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return nullptr;
	UGameInstance* GI = World->GetGameInstance();
	if (!GI) return nullptr;
	
	return GI->GetSubsystem<UCardManagerSubsystem>();
}
