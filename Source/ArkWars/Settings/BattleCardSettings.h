// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "BattleCardSettings.generated.h"

/**
 * 
 */
UCLASS(Config = Game)
class ARKWARS_API UBattleCardSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	static UBattleCardSettings* Get();
	
	UPROPERTY(EditAnywhere,Category=Data)
	TSoftObjectPtr<UDataTable> CardInfoDataTable;
	UPROPERTY(EditAnywhere,Category=Data)
	TSoftObjectPtr<UDataTable> CardMappingDataTable;
};
