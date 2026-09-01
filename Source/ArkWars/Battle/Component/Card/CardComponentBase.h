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
	///	卡牌容器，该容器会存放主标签精准匹配该卡牌种类标签的所有卡牌
	TArray<FCard> Container;
	
	///被转化的卡牌容器，该容器会存放主标签不匹配但存在其他标签精准匹配该卡牌种类标签的所有卡牌
	mutable TArray<FConvertedCard> ConvertedCards;
	
	///卡牌信息，储存常规状态下该类卡牌的共有信息，如卡牌种类标签、卡牌类型标签，当前类的类型等
	TUniquePtr<FCardInfo> Info;
public:
	UCardComponentBase();
	/**
	 *	给玩家添加手牌等需要获取卡牌组件时调用
	 *	@param Owner				组件的持有者，通常应该为玩家
	 *	@param Tag					卡牌的种类所对应的Tag
	 *	@param ForceCreate			是否强制附加到Owner，默认为是，若为否，则只会查询目标是否存在组件
	 */
	static UCardComponentBase* Get(AActor* Owner, const FGameplayTag& Tag, bool ForceCreate = true);
	
	/**
	 *	获取当前组件中所有的卡牌
	 *	@param OutCards				输出：所有的卡牌
	 *	@param bIncludeConverted	默认为假，若为真，则输出包括被转化或视为的卡牌
	 *	@return						输出：当前组件持有的所有卡牌的数量
	 */
	int32 GetCards(TArray<FCard>& OutCards, bool bIncludeConverted = false) const;
	
	/**
	 *	获取当前组件中所有指定花色的卡牌
	 *	@param Suit					指定筛选的花色
	 *	@param OutCards				输出：指定花色的卡牌
	 *	@param bIncludeConverted	默认为真，若为假，则只输出包括非转化和视为的卡牌
	 *	@return						输出：当前组件持有的所有指定花色的卡牌的数量
	 */
	int32 GetCardBySuit(ECardSuit Suit, TArray<FCard>& OutCards, bool bIncludeConverted = true) const;
	
	/**
	 *	获取当前组件中所有指定点数范围的卡牌
	 *	@param Min					指定筛选的最小点数(包含)
	 *	@param Max					指定筛选的最大点数(包含)
	 *	@param OutCards				输出：指定点数范围的卡牌
	 *	@param bIncludeConverted	默认为真，若为假，则只输出包括非转化和视为的卡牌
	 *	@return						输出：当前组件持有的指定点数范围的卡牌的数量
	 */
	int32 GetCardByPointRange(int32 Min, int32 Max, TArray<FCard>& OutCards, bool bIncludeConverted = true) const;

	/**
	 *	组合判别，获取当前组件中所有指定花色和点数范围的卡牌(包括被转化或视为的卡牌)
	 *	@param Suit					指定筛选的花色		
	 *	@param Min					指定筛选的最小点数(包含)
	 *	@param Max					指定筛选的最大点数(包含)
	 *	@param OutCards				输出：指定花色和点数范围卡牌(包括被转化或视为的卡牌)
	 *	@param bIncludeConverted	默认为真，若为假，则只输出包括非转化和视为的卡牌
	 *	@return						输出：当前组件持有的指定花色和点数范围的卡牌的数量
	 */
	int32 GetCardByMultiCondition(ECardSuit Suit, int32 Min, int32 Max, TArray<FCard>& OutCards, bool bIncludeConverted = true) const;

	/**
	 *	通用的卡牌获取方式，获取当前组件中满足指定条件的所有卡牌(包括被转化或视为的卡牌)
	 *	@param Predicate			筛选策略，若计算返回为真，则视为满足条件
	 *	@param OutCards				输出：当前组件中满足条件的所有卡牌(包括被转化或视为的卡牌)
	 *	@param bIncludeConverted	默认为真，若为假，则只输出包括非转化和视为的卡牌
	 *	@return						输出：当前组件满足条件的卡牌的数量
	 */
	int32 GetCardByPredicate(const TFunction<bool(const FCard&)>& Predicate, TArray<FCard>& OutCards, bool bIncludeConverted = true) const;
	
	/**
	 *	需要使用卡牌时调用, 可以指定多个目标, 逻辑由子类提供
	 *	@param Targets				卡牌的目标(若需要)
	 *	@param Card					被选中使用的卡牌，通常需要来源于该组件(Cards/ConvertedCards)或为“视为”等，否则无法使用
	 */
	virtual void Use(const TArray<AActor*>& Targets, FCard Card);
	
	/**
	 *	将卡牌移出手牌时调用，可以移动至牌堆、本人的其他区域、他人的区域等
	 *	@param Cards				待移动的手牌
	 *	@param To					移动的目标区域
	 */
	virtual void Move(const TArray<FCard>& Cards, ICardContainerInterface* To);

	/**
	 *	被告知需要使用/打出牌响应时调用，可以根据目标的卡牌决定当此响应是使用还是打出
	 *	@param Target				要求你响应的目标
	 *	@param Card					需要响应的目标牌
	 */
	virtual void Response(AActor* Target, const FCard& Card);

	/**
	 *	当卡牌选中后，需要选中目标时调用，用于判断目标是否可以被选中
	 *	@param Target				需要判断是否可以被选中的目标
	 *	@return						若返回真，则该目标可以被选中
	 */
	virtual bool TrySelectTarget(AActor* Target) {return false;};
	
protected:
	virtual int32 DataConverter(const FString& Data) { return 0; };

};
