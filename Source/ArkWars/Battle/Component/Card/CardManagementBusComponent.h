// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArkWars/Battle/Toolkits/ArkWarDelegates.h"
#include "ArkWars/Battle/Toolkits/ArkWarCardTypes.h"
#include "ArkWars/Battle/Toolkits/CardContainerInterface.h"
#include "Components/ActorComponent.h"
#include "CardManagementBusComponent.generated.h"

/**
 * 	设计附加到PlayerState上的组件，用于储存该玩家具有的所有卡牌及区域
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARKWARS_API UCardManagementBusComponent : public UActorComponent, public ICardContainerInterface
{
	GENERATED_BODY()
	///选中仅在客户端中暂存
	TArray<FGameplayTagContainer> SelectedCards;
	TArray<TWeakObjectPtr<APlayerState>> SelectedPlayers;
	
	UPROPERTY(Replicated)
	TArray<FCardAreaSlot> EquipmentArea;
	
	UPROPERTY(Replicated)
	TArray<FCardAreaSlot> JudgementArea;
	
	UPROPERTY(Replicated)
	TArray<FGameplayTagContainer> HandCards;
public:
	UCardManagementBusComponent();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	FPlayerSelectionChangeDelegate OnPlayerSelectionChanged;
	FCardSelectionChangeDelegate OnCardSelectionChanged;
	
	
	int32 GetIndexOfCard(const FGameplayTagContainer& Card) const;
	
	void SelectCard(const FGameplayTagContainer& Card);
	void ClearCardSelection(const FGameplayTagContainer& Card);
	const TArray<FGameplayTagContainer>& GetSelectedCards() { return SelectedCards; };
	TArray<FGameplayTagContainer> ConsumeCards();
	
	void SelectTarget(APlayerState* Target);
	void ClearTargetsSelection(APlayerState* Target);
	const TArray<TWeakObjectPtr<APlayerState>>& GetSelectedTargets() { return SelectedPlayers;};
	TArray<APlayerState*> ConsumeTargets();

	
	bool CanPutInJudgement(const FGameplayTagContainer& Card) const;
	
	void EquipCard(const FGameplayTagContainer& Card);
	void HandleJudgement();
	
	virtual void MoveIn(ICardContainerInterface* From, TArray<FGameplayTagContainer>&& Cards, const FString& Msg) override;
	
	virtual void MoveOut(ICardContainerInterface* To, const TArray<FGameplayTagContainer>& Cards, const FString& Msg) override;
	

	
	virtual TArray<FGameplayTag> GetAreaKeys() const override;
	
	virtual TArray<FGameplayTagContainer> GetCardsByKey(const FGameplayTag& Key) const override;
	
	void SetCardOrder(const TArray<int32>& NewOrder);
	
};
