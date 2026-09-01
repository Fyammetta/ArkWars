// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArkWars/Battle/Toolkits/ArkWarDelegates.h"
#include "ArkWars/Battle/Toolkits/ArkWarTypes.h"
#include "ArkWars/Battle/Toolkits/CardContainerInterface.h"
#include "Components/ActorComponent.h"
#include "CardManagementBusComponent.generated.h"

USTRUCT(BlueprintType)
struct FEquipmentPair
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Area;
	
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag CardTag;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARKWARS_API UCardManagementBusComponent : public UActorComponent, public ICardContainerInterface
{
	GENERATED_BODY()
	///选中仅在客户端中暂存
	TArray<FCard> SelectedCards;
	TArray<TWeakObjectPtr<APlayerState>> SelectedPlayers;
	
	UPROPERTY(Replicated)
	TArray<FEquipmentPair> EquipmentArea;
public:
	UCardManagementBusComponent();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	FPlayerSelectionChangeDelegate OnPlayerSelectionChanged;
	FCardSelectionChangeDelegate OnCardSelectionChanged;
	
	TArray<FCard> GetAllHandCard(FGameplayTagContainer* Filter = nullptr) const;
	int32 GetIndexOfCard(const FCard& Card) const;
	
	void SelectCard(const FCard& Card);
	void ClearCardSelection(const FCard& Card);
	TArray<FCard> GetSelectedCards();
	
	void SelectTarget(APlayerState* Target);
	void ClearTargetsSelection(APlayerState* Target);
	TArray<TWeakObjectPtr<APlayerState>> GetSelectedTargets();
	
	void UseCard(int32 Index);
	
	virtual void MoveIn(ICardContainerInterface* From, TArray<FGameplayTagContainer>&& Cards, const FString& Msg) override;
	
	virtual void MoveOut(ICardContainerInterface* To, TArray<FGameplayTagContainer>&& Cards, const FString& Msg) override;
	
	
};
