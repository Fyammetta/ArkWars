// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArkWars/Battle/System/CardManagerSubsystem.h"
#include "ArkWars/Battle/Toolkits/ArkWarTypes.h"
#include "Components/ActorComponent.h"
#include "ArkWars/Battle/Toolkits/CardContainerInterface.h"
#include "CardComponentBase.generated.h"

/**
 *	抽象类，只允许创建子类的实例
 *	
 *	设计附加到PlayerState上的组件，用于储存该玩家具有的所有特定种类的卡牌
 *	
 *	除开储存指定种类的卡牌，组件还持有被转化/视为种类卡牌的其他卡牌的弱引用
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Abstract)
class ARKWARS_API UCardComponentBase : public UActorComponent
{
	GENERATED_BODY()
	UPROPERTY(Replicated)
	FGameplayTag Key;
public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UCardComponentBase();
	/**
	 *	添加手牌等需要获取卡牌组件时调用
	 *	@param Owner				组件的持有者
	 *	@param Tag					卡牌的种类所对应的Tag
	 */
	static UCardComponentBase* Get(AActor* Owner, const FGameplayTag& Tag);
	/**
	 *	需要使用卡牌时调用, 可以指定多个目标, 逻辑由子类提供
	 *	@param Source				卡牌的使用者
	 *	@param Targets				卡牌的目标(若需要)
	 *	@param Card					被选中使用的卡牌，通常需要来源于该组件(Cards/ConvertedCards)或为“视为”等，否则无法使用
	 */
	virtual void Use(AActor* Source, const TArray<AActor*>& Targets, const FGameplayTagContainer& Card);
	
	/**
	 *	将卡牌移出手牌时调用，可以移动至牌堆、本人的其他区域、他人的区域等
	 *	@param Cards				待移动的手牌
	 *	@param From					移动的来源区域
	 *	@param To					移动的目标区域
	 */
	virtual void Move(const TArray<FGameplayTagContainer>& Cards, ICardContainerInterface* From, ICardContainerInterface* To);

	/**
	 *	被告知需要使用/打出牌响应时调用，可以根据目标的卡牌决定当此响应是使用还是打出
	 *	@param Source				要求响应的来源
	 *	@param Target				需要响应的目标
	 *	@param Card					需要响应的目标牌
	 */
	virtual void Response(AActor* Source, AActor* Target, const FGameplayTagContainer& Card);

	/**
	 *	当卡牌选中后，需要选中目标时调用，用于判断目标是否可以被选中
	 *	@param Source				发动选中的人
	 *	@param Target				需要判断是否可以被选中的目标
	 *	@return						若返回真，则该目标可以被选中
	 */
	virtual bool TrySelectTarget(AActor* Source, AActor* Target) {return false;};
	
protected:
	virtual int32 DataConverter(const FString& Data) { return 0; };
	
};
