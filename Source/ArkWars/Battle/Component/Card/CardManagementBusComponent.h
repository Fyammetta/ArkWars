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
	TArray<FArkCard> SelectedCards;
	TArray<TWeakObjectPtr<APlayerState>> SelectedPlayers;
	
	UPROPERTY(Replicated)
	TArray<FArkCard> EquipmentArea;
	
	UPROPERTY(Replicated)
	TArray<FArkCard> JudgementArea;
	
	UPROPERTY(Replicated)
	TArray<FArkCard> HandCards;
public:
	UCardManagementBusComponent();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	FPlayerSelectionChangeDelegate OnPlayerSelectionChanged;
	FCardSelectionChangeDelegate OnCardSelectionChanged;
	
	
	int32 GetIndexOfCard(const FArkCard& Card) const;
	
	void SelectCard(const FArkCard& Card);
	void ClearCardSelection(const FArkCard& Card);
	const TArray<FArkCard>& GetSelectedCards() { return SelectedCards; };
	TArray<FArkCard> ConsumeCards();
	
	void SelectTarget(APlayerState* Target);
	void ClearTargetsSelection(APlayerState* Target);
	const TArray<TWeakObjectPtr<APlayerState>>& GetSelectedTargets() { return SelectedPlayers;};
	TArray<APlayerState*> ConsumeTargets();

	
	bool CanPutInJudgement(const FArkCard& Card) const;
	
	void EquipCard(const FArkCard& Card);
	void HandleJudgement();


	virtual TArray<FArkCard> GetCardsByKey(const FGameplayTag& Area) const override;
	virtual const FArkCard* GetCardById(int32 CardId) const override;
	virtual TArray<int32> Select(const FGameplayTag& Area, const FString& Msg) const override;
	[[nodiscard]] virtual TArray<FArkCard> Consume(const FGameplayTag& Area, const TArray<int32>& CardIds) override;
	virtual EAreaWriteResult Add(const FGameplayTag& AreaKey, TArray<FArkCard>&& Cards, const FString& Msg) override;
	virtual TArray<FGameplayTag> GetAreaKeys() const override;
	virtual AActor* GetContainerActor() override {return GetOwner();};
	
	
	void SetCardOrder(const TArray<int32>& NewOrder);
	
};
