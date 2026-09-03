// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArkWars/Battle/Toolkits/GameMessage.h"
#include "Subsystems/WorldSubsystem.h"
#include "BattleGameFlowSubsystem.generated.h"


class UGamePhaseManagerComponent;
/**
 * 
 */



UCLASS()
class ARKWARS_API UBattleGameFlowSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	UPROPERTY()
	TObjectPtr<AActor> ManagerActor;
	
	TArray<GameMessage::FPhaseMessage> PhaseMsgQueue;

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	void InitPlayerOrder(int32 StartIndex);
	
	int32 GetPlayerIndex(APlayerState* Player = nullptr);
	
	void RegisterManagerActor(AActor* Actor);

	AActor* GetManagerActor() const;
	
	bool IsRunningOnServer() const;
	
	void PushPhaseResolvation(const FString& Msg);
	
	UGamePhaseManagerComponent* GetPhaseManager() const;
	
	EGamePhase ResolveNextPhase(EGamePhase Current);

	/**
	 *	指定玩家跳转到特定阶段，无视所有Message的修正
	 *	@param Phase			要跳转至的阶段
	 *	@param Player			获得下一个阶段的行动权的玩家, 默认为当前玩家
	 */
	void Advance(EGamePhase Phase, int32 Player = INDEX_NONE);

	/**
	 *	进入下一个阶段，优先执行Message的修正
	 */
	void Advance();
};

