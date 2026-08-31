// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArkWars/Battle/System/CardManagerSubsystem.h"
#include "CardInfoBase.generated.h"

/**
 * 
 */
UCLASS()
class ARKWARS_API UCardInfoBase : public UObject
{
	GENERATED_BODY()
	
	TUniquePtr<FCardInfo> Info;
	
};
