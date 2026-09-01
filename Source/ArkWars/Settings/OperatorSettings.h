// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "OperatorSettings.generated.h"

/**
 * 
 */
UCLASS(Config = Game)
class ARKWARS_API UOperatorSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	static UOperatorSettings* Get();
	
	UPROPERTY(EditAnywhere,Category="Data|Skill")
	TSoftObjectPtr<UDataTable> SkillInfoDataTable;
	UPROPERTY(EditAnywhere,Category="Data|Skill")
	TSoftObjectPtr<UDataTable> SkillMappingDataTable;
	
	UPROPERTY(EditAnywhere,Category="Data|Operator")
	TSoftObjectPtr<UDataTable> OperatorDataTable;
	
	UDataTable* GetSkillInfoDataTable() const;
	UDataTable* GetSkillMappingDataTable() const;
	UDataTable* GetOperatorDataTable() const;
};
