// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArkWars/Battle/Toolkits/ArkWarDelegates.h"
#include "ArkWars/Battle/Toolkits/ArkWarTypes.h"
#include "GameFramework/GameStateBase.h"
#include "BattleGameState.generated.h"

/**
 * 
 */
UCLASS()
class ARKWARS_API ABattleGameState : public AGameStateBase
{
	GENERATED_BODY()
	
	UPROPERTY(ReplicatedUsing=OnRep_Phase)
	EGamePhase CurrentPhase = EGamePhase::None;
	UPROPERTY(Replicated)
	EGamePhase CachedPhase = EGamePhase::None;
	
		
		
	UPROPERTY(ReplicatedUsing=OnRep_PlayersInOrder)
	TArray<APlayerState*> PlayersInOrder = {};
	
	UPROPERTY(ReplicatedUsing=OnRep_ActivePlayerIndex)
	int32 ActivePlayerIndex = INDEX_NONE;

public:
	///	============================  委托  ============================
	FGamePhaseChangeDelegate OnGamePhaseChanged;
	FActivePlayerChangeDelegate OnActivePlayerChanged;
	
	EGamePhase GetLastPhase();
	EGamePhase GetPhase();
	void SetPhase(EGamePhase Phase);
	
	void SetNextPlayerActive();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
private:	
	UFUNCTION()
	void OnRep_Phase() const;

	UFUNCTION()
	void OnRep_ActivePlayerIndex() const;
	
	UFUNCTION()
	void OnRep_PlayersInOrder() const;
	
	void BroadcastPhaseChange() const;
	void BroadcastActivePlayerChange() const;
};
