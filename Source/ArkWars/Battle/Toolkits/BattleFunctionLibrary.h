// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BattleFunctionLibrary.generated.h"

struct FGameplayTag;
enum ECardSuit : uint8;
struct FGameplayTagContainer;
class USkillManagerSubsystem;
class UCardManagerSubsystem;
class UBattleGameFlowSubsystem;
/**
 * 
 */
UCLASS()
class ARKWARS_API UBattleFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Card|Mgr")
	static UCardManagerSubsystem* GetCardManager(const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Skill|Mgr")
	static USkillManagerSubsystem* GetSkillManager(const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameFlow")
	static UBattleGameFlowSubsystem* GetBattleManager(const UObject* WorldContextObject);
	
	
	/**
	 *	通用的卡牌检索方式，获取输入中满足指定条件的所有卡牌
	 *	@param Cards				待筛选的牌组
	 *	@param Predicate			筛选策略，若计算返回为真，则视为满足条件
	 *	@param OutCards				输出：输入中满足条件的所有卡牌(包括被转化或视为的卡牌)
	 *	@return						输出：输入满足条件的卡牌的数量
	 */
	static int32 FilterCardByPredicate(const TArray<FGameplayTagContainer>& Cards, const TFunction<bool(const FGameplayTagContainer&)>& Predicate, TArray<FGameplayTagContainer>& OutCards);
	
	
	/**
	 *	依赖花色的卡牌检索方式，获取输入中特定花色的所有卡牌
	 *	@param Cards				待筛选的牌组
	 *	@param Suits				检索指定的花色，若满足任一花色则视为通过
	 *	@param OutCards				输出：输入中满足条件的所有卡牌(包括被转化或视为的卡牌)
	 *	@return						输出：输入满足条件的卡牌的数量
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Card|Filter")
	static int32 FilterCardBySuit(const TArray<FGameplayTagContainer>& Cards, const TArray<TEnumAsByte<ECardSuit>>& Suits, TArray<FGameplayTagContainer>& OutCards);
	
	/**
	 *	指定点数范围的卡牌检索方式，获取输入中点数在[Min, Max]的所有卡牌
	 *	@param Cards				待筛选的牌组
	 *	@param Min					检索指定的最小点数范围（包含）
	 *	@param Max					检索指定的最大点数范围（包含）
	 *	@param OutCards				输出：输入中满足条件的所有卡牌(包括被转化或视为的卡牌)
	 *	@return						输出：输入满足条件的卡牌的数量
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Card|Filter")
	static int32 FilterCardByPoint(const TArray<FGameplayTagContainer>& Cards, int32 Min, int32 Max, TArray<FGameplayTagContainer>& OutCards);
	
	/**
	 *	指定特定标签的卡牌检索方式，获取输入中具有指定Tag的所有卡牌
	 *	@param Cards				待筛选的牌组
	 *	@param Tag					卡牌需要具有的Tag
	 *	@param OutCards				输出：输入中满足条件的所有卡牌(包括被转化或视为的卡牌)
	 *	@return						输出：输入满足条件的卡牌的数量
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Card|Filter")
	static int32 FilterCardByTag(const TArray<FGameplayTagContainer>& Cards, const FGameplayTag& Tag, TArray<FGameplayTagContainer>& OutCards);
	
};
