// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "BattleGameFlowSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class ARKWARS_API UBattleGameFlowSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	UPROPERTY()
	TObjectPtr<AActor> ManagerActor;
	
public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	void InitPlayerOrder(int32 StartIndex);
	
	void RegisterManagerActor(AActor* Actor);

	AActor* GetManagerActor() const;
};
