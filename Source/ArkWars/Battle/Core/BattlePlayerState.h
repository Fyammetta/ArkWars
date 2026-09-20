// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "PlayerOperatorInterface.h"
#include "ArkWars/Battle/Toolkits/CardContainerInterface.h"
#include "GameFramework/PlayerState.h"
#include "BattlePlayerState.generated.h"

/**
 *
 */
UCLASS()
class ARKWARS_API ABattlePlayerState : public APlayerState, public IAbilitySystemInterface, public ICardContainerInterface, public IPlayerOperatorInterface
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAbilitySystemComponent> Asc;

	UPROPERTY(ReplicatedUsing=OnRep_OperatorSelected)
	FName Operator;

public:
	ABattlePlayerState();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return Asc; };

	virtual TArray<FArkCard> GetCardsByKey(const FGameplayTag& Area) const override;
	virtual const FArkCard* GetCardById(int32 CardId) const override;
	virtual TArray<int32> Select(const FGameplayTag& Area, const FMessageType& Msg) const override;
	[[nodiscard]] virtual TArray<FArkCard> Consume(const FGameplayTag& Area, const TArray<int32>& CardIds) override;
	virtual EAreaWriteResult Add(const FGameplayTag& AreaKey, TArray<FArkCard>& Cards, const FMessageType& Msg) override;
	virtual TArray<FGameplayTag> GetAreaKeys() const override;
	virtual AActor* GetContainerActor() override { return this;};

	virtual const FName& GetOperator() const override { return Operator; };

	///	选角落定的业务入口（服务器侧，非 RPC）：由本人 PC 的 Server_SelectOperator 调用。
	///	不带 Server_ 前缀——它不是 RPC，是"服务器侧已授权的动作"；RPC 收发口一律在 PC（卷 11 §5.2）
	void ApplyOperatorSelection(const FName& SelectedOperator);

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	
private:
	void InitOperator(const FName& SelectedOperator);
	
	UFUNCTION()
	void OnRep_OperatorSelected();
};
