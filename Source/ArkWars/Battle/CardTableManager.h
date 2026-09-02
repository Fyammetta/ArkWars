// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Toolkits/CardContainerInterface.h"
#include "CardTableManager.generated.h"

UCLASS()
class ARKWARS_API ACardTableManager : public AActor , public ICardContainerInterface
{
	GENERATED_BODY()
	
	//展示、暂存等卡牌的临时区，
	UPROPERTY(ReplicatedUsing=OnRep_CachedAreaChanged)
	TArray<FGameplayTagContainer> CachedArea;
	
	//被打出的牌会进入此区域，弃牌阶段开始前以至弃牌区并不触发弃牌事件,UI设计上只显示最新的一张，可以手动展开显示全部
	UPROPERTY(ReplicatedUsing=OnRep_PlayedAreaChanged)
	TArray<FGameplayTagContainer> PlayedArea;
	
	//有牌进入使用区时，同步记入牌的来源(发生在PlayerArea更新前)
	UPROPERTY(Replicated)
	TArray<APlayerState*> CardSource;
	
public:
	// Sets default values for this actor's properties
	ACardTableManager();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_CachedAreaChanged();
	
	UFUNCTION()
	void OnRep_PlayedAreaChanged();
	
	virtual void MoveIn(ICardContainerInterface* From, TArray<FGameplayTagContainer>&& Cards, const FString& Msg) override;
	
	virtual void MoveOut(ICardContainerInterface* To, const TArray<FGameplayTagContainer>& Cards, const FString& Msg) override;
};

