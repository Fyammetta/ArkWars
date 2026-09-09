// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleFunctionLibrary.h"

#include "ArkWarTags.h"
#include "ArkWars/Battle/System/SkillManagerSubsystem.h"
#include "ArkWars/Battle/System/CardManagerSubsystem.h"
#include "ArkWars/Battle/System/BattleGameFlowSubsystem.h"

UCardManagerSubsystem* UBattleFunctionLibrary::GetCardManager(const UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;
	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return nullptr;
	UGameInstance* GI = World->GetGameInstance();
	if (!GI) return nullptr;
	
	return GI->GetSubsystem<UCardManagerSubsystem>();
}

USkillManagerSubsystem* UBattleFunctionLibrary::GetSkillManager(const UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;
	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return nullptr;

	return World->GetSubsystem<USkillManagerSubsystem>();
}

UBattleGameFlowSubsystem* UBattleFunctionLibrary::GetBattleManager(const UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;
	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return nullptr;

	return World->GetSubsystem<UBattleGameFlowSubsystem>();
}

int32 UBattleFunctionLibrary::FilterCardByPredicate(const TArray<FArkCard>& Cards, const TFunction<bool(const FArkCard&)>& Predicate, TArray<FArkCard>& OutCards) 
{
	OutCards.Empty();
	for (auto Card : Cards)
	{
		if (Predicate(Card))
			OutCards.Add(Card);
	}
	
	return OutCards.Num();
}

int32 UBattleFunctionLibrary::FilterCardBySuit(const TArray<FArkCard>& Cards, const TArray<TEnumAsByte<ECardSuit>>& Suits, TArray<FArkCard>& OutCards)
{
	FGameplayTagContainer Arr {};
	
	if (Suits.Contains(Spade))
		Arr.AddTag(CardTags::Spade);
	if (Suits.Contains(Heart))
		Arr.AddTag(CardTags::Heart);
	if (Suits.Contains(Diamond))
		Arr.AddTag(CardTags::Diamond);
	if (Suits.Contains(Club))	
		Arr.AddTag(CardTags::Club);
	
	
	
	auto P = [&Arr](const FArkCard& Card)->bool
	{
		return  Card.Card.HasAny(Arr);
	};
	return FilterCardByPredicate(Cards, P, OutCards);
}

int32 UBattleFunctionLibrary::FilterCardByPoint(const TArray<FArkCard>& Cards, int32 Min, int32 Max, TArray<FArkCard>& OutCards)
{
	FGameplayTagContainer Arr {};

	for (int i = Min; i <= Max; ++i)
	{
		Arr.AddTag(CardTags::GetPointTag(i));
	}
	
	auto P = [&Arr](const FArkCard& Card)->bool
	{
		return Card.Card.HasAny(Arr);
	};
	return FilterCardByPredicate(Cards, P, OutCards);
}

int32 UBattleFunctionLibrary::FilterCardByTag(const TArray<FArkCard>& Cards, const FGameplayTag& Tag, TArray<FArkCard>& OutCards)
{
	auto P = [&Tag](const FArkCard& Card)->bool
	{
		return Card.Card.HasTag(Tag);
	};
	return FilterCardByPredicate(Cards, P, OutCards);
}
