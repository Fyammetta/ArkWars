// Fill out your copyright notice in the Description page of Project Settings.


#include "CardTableManager.h"
#include "Net/UnrealNetwork.h"
#include "Toolkits/ArkWarTags.h"
#include "Toolkits/GameMessage.h"

TStrongObjectPtr<ACardTableManager> ACardTableManager::Instance = nullptr;

// Sets default values
ACardTableManager::ACardTableManager()
{
	SetReplicates(true);
}

void ACardTableManager::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ACardTableManager, CachedArea)
	DOREPLIFETIME(ACardTableManager, PlayedArea)
	DOREPLIFETIME(ACardTableManager, PileArea)
	DOREPLIFETIME(ACardTableManager, DiscardArea)
	DOREPLIFETIME(ACardTableManager, JudgementArea)
}

ACardTableManager* ACardTableManager::Get(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}
	
	if (Instance.IsValid() && Instance->GetWorld() == World) return Instance.Get();

	//其次在世界中查找已存在的实例（关卡内手工摆放的管理器）
	for (auto Actor : World->GetCurrentLevel()->Actors)
	{
		if (Actor && Actor->IsA(StaticClass()))
		{
			Instance = TStrongObjectPtr(Cast<ACardTableManager>(Actor)) ;
			return Instance.Get();
		}
	}
	{
		//客户端不允许新建：必须是 Dedicated / Listen Server
		ENetMode Mode = World->GetNetMode();
		if (Mode != NM_DedicatedServer && Mode != NM_ListenServer) return nullptr;
	}
	
	//服务器侧新建单例
	Instance = TStrongObjectPtr(Cast<ACardTableManager>(World->SpawnActor(StaticClass())));
	return Instance.Get();
}

void ACardTableManager::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACardTableManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	Instance.Reset();
}

void ACardTableManager::OnRep_CachedAreaChanged()
{
}

void ACardTableManager::OnRep_PlayedAreaChanged()
{
}

void ACardTableManager::OnRep_PileAreaChanged()
{
}

void ACardTableManager::OnRep_DiscardAreaChanged()
{
}

void ACardTableManager::OnRep_JudgementAreaChanged()
{
}

const FArkCard* ACardTableManager::GetCardById(int32 CardId) const
{
	//按实体 Id 依次在 缓存 → 判定流向 → 已出 → 牌堆 → 弃牌 中反查
	for (const FArkCard& Card : CachedArea)
	{
		if (Card.Identity == CardId) return &Card;
	}
	
	for (const FArkCard& Card : JudgementArea)
	{
		if (Card.Identity == CardId) return &Card;
	}
	
	for (const FArkCard& Card : PlayedArea)
	{
		if (Card.Identity == CardId) return &Card;
	}
	
	for (const FArkCard& Card : PileArea)
	{
		if (Card.Identity == CardId) return &Card;
	}
	for (const FArkCard& Card : DiscardArea)
	{
		if (Card.Identity == CardId) return &Card;
	}
	
	return nullptr;
}

TArray<int32> ACardTableManager::Select(const FGameplayTag& Area, const FMessageType& Msg) const
{
	//只读：先拷出目标区域（不改动源区），再按 _Order 扫描，命中返回源区下标
	using namespace CardTags;
	using EOrder = GameMessage::FMoveMessage::EOrder;
	TArray<FArkCard> Temp {};
	if (Area == Pile)
		Temp = PileArea;
	else if (Area == Discard)
		Temp = DiscardArea;
	else if (Area == Judgement)
		Temp = JudgementArea;
	else if (Area == Used)
		Temp = PlayedArea;
	else if (Area == Cache)
		Temp = CachedArea;
	else
		return {};

	if (Temp.IsEmpty()) return {};
	
	TArray<int32> RetArr {};
	switch (Msg._Order)
	{
	case EOrder::Top :
		{
			while (RetArr.Num() < Msg._Count)
			{	
				auto Card = Temp.Last();
				if (Msg(Card))
				{
					RetArr.Add(Temp.Find(Card));
				}
				Temp.Pop();
				if (Temp.IsEmpty()) break;
			}
			break;
		}
	case EOrder::Bottom :
		{
			for (int Index = 0; RetArr.Num() < Msg._Count;)
			{
				auto Card = Temp[Index];
				if (Msg(Card))
				{
					RetArr.Add(Index);
				}
				Index++;
				if (!Temp.IsValidIndex(Index)) break;
			}
			break;
		}
	case EOrder::Random :
		{
			auto Source = Temp;
			while (RetArr.Num() < Msg._Count)
			{	
				auto Max = Temp.Num() - 1;
				auto Rand = FMath::RandRange(0, Max);
				Temp.Swap(Max, Rand);
				
				auto Card = Temp.Last();
				if (Msg(Card))
				{
					RetArr.Add(Source.Find(Card));
				}
				Temp.Pop();
				if (Temp.IsEmpty()) break;
			}
			break;
		}
	}
	
	return RetArr;
}

TArray<FArkCard> ACardTableManager::Consume(const FGameplayTag& Area, const TArray<int32>& CardIndexes)
{
	//仅服务器权威可写
	if (!HasAuthority()) return {};
	
	using namespace CardTags;
	TArray<FArkCard> OutCards;
	
	//定位源区（桌面五区）
	TArray<FArkCard>* CardArea = nullptr;
	if (Area == Pile)
		CardArea = &PileArea;
	else if (Area == Discard)
		CardArea = &DiscardArea;
	else if (Area == Judgement)
		CardArea = &JudgementArea;
	else if (Area == Used)
		CardArea = &PlayedArea;
	else if (Area == Cache)
		CardArea = &CachedArea;
	else return {};
	
	for (int32 Index : CardIndexes)
	{
		OutCards.Add((*CardArea)[Index]);
	}
	
	TArray<FArkCard> Recover {};

	for (int32 i = 0; i < CardArea->Num(); i++)
	{
		if (CardIndexes.Find(i) != INDEX_NONE) continue;
		Recover.Add((*CardArea)[i]);
	}
	
	*CardArea = MoveTemp(Recover);
	
	return OutCards;
}

EAreaWriteResult ACardTableManager::Add(const FGameplayTag& AreaKey, TArray<FArkCard>& Cards, const FMessageType& Msg)
{
	//仅服务器权威可写
	if (!HasAuthority()) return EAreaWriteResult::Mismatch;
	
	using namespace CardTags;
	using EOrder = GameMessage::FMoveMessage::EOrder;
	if (AreaKey == Pile)
	{
		//牌堆：数组尾为牌堆顶 → Top = 追加尾部；Bottom = 旧牌堆接在新牌之后（新牌在底）；Random = 逐张随机插入
		switch (Msg._Order)
		{
			case EOrder::Bottom :
			{
				Cards.Append(PileArea);
				PileArea = MoveTemp(Cards);
				break;
			}
			case EOrder::Top :
			{
				PileArea.Append(MoveTemp(Cards));
				break;
			}
			case EOrder::Random :
			{
				while (!Cards.IsEmpty())
				{
					PileArea.Insert(Cards.Pop(),FMath::RandRange(0,PileArea.Num() - 1));
				}
			}
		}
	}

	//以下四区均无顺序语义，一律追加到数组尾
	else if (AreaKey == Discard)
		DiscardArea.Append(MoveTemp(Cards));
	else if (AreaKey == Judgement)
		JudgementArea.Append(MoveTemp(Cards));
	else if (AreaKey == Used)
		PlayedArea.Append(MoveTemp(Cards));
	else if (AreaKey == Cache)
		CachedArea.Append(MoveTemp(Cards));
	else return EAreaWriteResult::Mismatch;		//未识别区域键：兜底（09-10 补）
	
	return EAreaWriteResult::Accepted;
}


TArray<FArkCard> ACardTableManager::GetCardsByKey(const FGameplayTag& Key) const
{
	//只读：桌面五区（Judgement 于 09-10 补入），其余区域返回空数组
	using namespace CardTags;

	if (Key == Pile)	return PileArea;
	if (Key == Discard) return DiscardArea;
	if (Key == Cache)	return CachedArea;
	if (Key == Used)	return PlayedArea;
	if (Key == Judgement)	return JudgementArea;
	
	return {};
}

TArray<FGameplayTag> ACardTableManager::GetAreaKeys() const
{
	//与 GetCardsByKey 的清单保持一致
	using namespace CardTags;
	return {Pile, Discard, Cache, Used, Judgement};
}
