// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleFunctionLibrary.h"

#include "ArkWarTags.h"
#include "ArkWars/Battle/System/SkillManagerSubsystem.h"
#include "ArkWars/Battle/System/CardManagerSubsystem.h"
#include "ArkWars/Battle/System/BattleGameFlowSubsystem.h"
#include "ArkWars/Battle/Toolkits/CardContainerInterface.h"

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

ICardContainerInterface* UBattleFunctionLibrary::ResolveContainer(AActor* Owner, const FGameplayTag& Area)
{
	//空归属：无容器可解析（桌面 Actor 由交易 Instigator 自身承载，玩家侧由 PlayerState 自身承载）
	if (!Owner) return nullptr;

	//自身优先：ACardTableManager / ABattlePlayerState 均自身实现接口，直接取自身
	if (Owner->Implements<UCardContainerInterface>())
		return Cast<ICardContainerInterface>(Owner);

	//后备：Owner 未实现接口时取其名下容器组件——Area 有效则优先命中该区域的组件，
	//否则退化为首个容器组件（当前两端 Actor 均自身实现，走不到此分支）
	ICardContainerInterface* Fallback = nullptr;
	for (UActorComponent* Component : Owner->GetComponentsByInterface(UCardContainerInterface::StaticClass()))
	{
		if (ICardContainerInterface* Container = Cast<ICardContainerInterface>(Component))
		{
			if (Area.IsValid() && Container->GetAreaKeys().Contains(Area))
				return Container;
			if (!Fallback)
				Fallback = Container;
		}
	}

	return Fallback;
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
