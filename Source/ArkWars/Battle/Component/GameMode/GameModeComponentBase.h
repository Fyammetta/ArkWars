// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameModeComponentBase.generated.h"

class UAbilitySystemComponent;

UCLASS(Abstract, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARKWARS_API UGameModeComponentBase : public UActorComponent
{
	GENERATED_BODY()
	
protected:
	TSet<TWeakObjectPtr<APlayerState>> UnRegisteredPlayers;
	TArray<FName> SelectedOperators;
	TArray<FName> TotalOperators;

public:
	static UGameModeComponentBase* GetCurrentGameMode(UObject* WorldContextObject);
	
	template <typename ComponentType>
	static ComponentType* GetCurrentGameMode(UObject* WorldContextObject)
	{
		return Cast<ComponentType>(GetCurrentGameMode(WorldContextObject));
	}
	
	/**	开始游戏时，根据玩法配置Subsystem中的牌堆 */
	UFUNCTION(BlueprintCallable)
	virtual void InitCardDeck() PURE_VIRTUAL(InitCardDeck);
	
	/**	开始游戏时，根据玩法和人数为玩家分配身份 */
	UFUNCTION(BlueprintCallable)
	virtual int32 AllocateIdentity() PURE_VIRTUAL(AllocateIdentity, return INDEX_NONE;);
	
	UFUNCTION(BlueprintCallable)
	virtual int32 GetStartCardNum(UAbilitySystemComponent* Asc = nullptr) PURE_VIRTUAL(GetStartCardNum, return INDEX_NONE;);
	
	UFUNCTION(BlueprintCallable)
	virtual void SentSelectOperatorNotify() PURE_VIRTUAL(SentSelectOperatorNotify);
	
	UFUNCTION(BlueprintCallable)
	virtual void CheckOperatorSelection(APlayerState* Player, const FName& Operator) PURE_VIRTUAL(CheckOperatorSelection);
	
	UFUNCTION(BlueprintCallable)
	virtual void ResetOperatorList() PURE_VIRTUAL(ResetOperatorList);
	
	UFUNCTION(BlueprintCallable)
	virtual bool ShouldResetOperatorList() PURE_VIRTUAL(ShouldResetOperatorList, return true;);
};


