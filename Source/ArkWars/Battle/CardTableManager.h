// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Toolkits/CardContainerInterface.h"
#include "CardTableManager.generated.h"

///	管理当局游戏的非角色卡牌区域，如抽牌堆、弃牌堆等
///	服务器权威，并复制到每个客户端



UCLASS()
class ARKWARS_API ACardTableManager : public AActor , public ICardContainerInterface
{
	GENERATED_BODY()
	
	static TStrongObjectPtr<ACardTableManager> Instance;
	
	//展示、暂存等卡牌的临时区，
	UPROPERTY(ReplicatedUsing=OnRep_CachedAreaChanged)
	TArray<FGameplayTagContainer> CachedArea;
	
	//被打出的牌会进入此区域，弃牌阶段开始前以至弃牌区并不触发弃牌事件,UI设计上只显示最新的一张，可以手动展开显示全部
	UPROPERTY(ReplicatedUsing=OnRep_PlayedAreaChanged)
	TArray<FGameplayTagContainer> PlayedArea;
	
	//有牌进入使用区时，同步记入牌的来源(发生在PlayerArea更新前)
	UPROPERTY(Replicated)
	TArray<APlayerState*> CardSource;
	
	//抽牌堆，默认以数组尾端作为牌堆顶
	UPROPERTY(ReplicatedUsing=OnRep_PileAreaChanged)
	TArray<FGameplayTagContainer> PileArea;
	
	//被打出的牌会进入此区域，弃牌阶段开始前以至弃牌区并不触发弃牌事件,UI设计上只显示最新的一张，可以手动展开显示全部
	UPROPERTY(ReplicatedUsing=OnRep_DiscardAreaChanged)
	TArray<FGameplayTagContainer> DiscardArea;
	
public:
	// Sets default values for this actor's properties
	ACardTableManager();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;


	/**
	 *	获取当前世界的管理器单例
	 *	@param World			需要获取管理器的世界
	 *	@return					管理器实例，若不存在则新建一个
	 */
	static ACardTableManager* Get(UWorld* World);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	virtual void OnRep_CachedAreaChanged();
	
	UFUNCTION()
	virtual void OnRep_PlayedAreaChanged();
	
	UFUNCTION()
	virtual void OnRep_PileAreaChanged();
	
	UFUNCTION()
	virtual void OnRep_DiscardAreaChanged();
	
	virtual void MoveIn(ICardContainerInterface* From, TArray<FGameplayTagContainer>&& Cards, const FString& Msg) override;
	
	virtual void MoveOut(ICardContainerInterface* To, const TArray<FGameplayTagContainer>& Cards, const FString& Msg) override;
	
	virtual TArray<FGameplayTagContainer> GetCardsByKey(const FGameplayTag& Key) const override;
	virtual TArray<FGameplayTag> GetAreaKeys() const override;
};

