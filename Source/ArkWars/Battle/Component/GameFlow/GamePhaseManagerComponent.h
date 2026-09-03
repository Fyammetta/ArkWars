// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArkWars/Battle/Toolkits/ArkWarDelegates.h"
#include "ArkWars/Battle/Toolkits/ArkWarFlowTypes.h"
#include "Components/ActorComponent.h"
#include "GamePhaseManagerComponent.generated.h"


/**
 *	设计附加在Server的GameState上，并复制到Client
 *	管理每个游戏阶段的变化，与BattleGameFlowSubsystem双向依赖,作为数据源和转
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARKWARS_API UGamePhaseManagerComponent : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY(ReplicatedUsing=OnRep_Phase)
	EGamePhase CurrentPhase = EGamePhase::GameStart;
	UPROPERTY(Replicated)
	EGamePhase CachedPhase = EGamePhase::GameStart;
	
	UPROPERTY(ReplicatedUsing=OnRep_PlayersInOrder)
	TArray<APlayerState*> PlayersInOrder = {};
	
	UPROPERTY(ReplicatedUsing=OnRep_ActivePlayerIndex)
	int32 ActivePlayerIndex = INDEX_NONE;
	
	TSet<int32> FinishedPlayerIndex;
public:
	// Sets default values for this component's properties
	UGamePhaseManagerComponent();
	EGamePhase GetLastPhase();
	EGamePhase GetPhase();
	void SetPhase(EGamePhase Phase);
	
	///	============================  委托  ============================
	FActivePlayerChangeDelegate OnActivePlayerChanged;
	FGamePhaseChangeDelegate OnGamePhaseChanged;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	void SetNextPlayerActive(int32 Index = INDEX_NONE);
	
	void InitPlayers(int32 Start);
	int32 GetPlayerIndex(APlayerState* Player = nullptr) const;
private:
	UFUNCTION()
	void OnRep_Phase() const;
	
	void BroadcastPhaseChange() const;

	UFUNCTION()
	void OnRep_ActivePlayerIndex() const;
	
	UFUNCTION()
	void OnRep_PlayersInOrder() const;
	
	void BroadcastActivePlayerChange() const;
	
	void BroadcastRoundRefresh() const;
};
