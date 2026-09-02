// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArkWars/Battle/Toolkits/ArkWarDelegates.h"
#include "ArkWars/Battle/Toolkits/ArkWarTypes.h"
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
	TArray<FGameplayTagContainer> GetSelectedCards();
	
	void SelectTarget(APlayerState* Target);
	void ClearTargetsSelection(APlayerState* Target);
	TArray<APlayerState*> GetSelectedTargets();
	
	bool CanPutInJudgement(const FGameplayTagContainer& Card) const;
	
	void EquipCard(const FGameplayTagContainer& Card);
	
	virtual void MoveIn(ICardContainerInterface* From, TArray<FGameplayTagContainer>&& Cards, const FString& Msg) override;
	
	virtual void MoveOut(ICardContainerInterface* To, const TArray<FGameplayTagContainer>& Cards, const FString& Msg) override;
	
	
	///Filter
	/**
	 *	获取当前组件中所有的卡牌
	 *	@param OutCards				输出：所有的卡牌
	 *	@return						输出：当前组件持有的所有卡牌的数量
	 */
	int32 GetCards(TArray<FGameplayTagContainer>& OutCards) const;

	/**
	 *	通用的卡牌获取方式，获取当前组件中满足指定条件的所有卡牌(包括被转化或视为的卡牌)
	 *	@param Predicate			筛选策略，若计算返回为真，则视为满足条件
	 *	@param OutCards				输出：当前组件中满足条件的所有卡牌(包括被转化或视为的卡牌)
	 *	@return						输出：当前组件满足条件的卡牌的数量
	 */
	int32 GetCardByPredicate(const TFunction<bool(const FGameplayTagContainer&)>& Predicate, TArray<FGameplayTagContainer>& OutCards) const;
	
	void SetCardOrder(const TArray<int32>& NewOrder);
	
};
