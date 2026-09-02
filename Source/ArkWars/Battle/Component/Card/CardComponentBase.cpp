// Fill out your copyright notice in the Description page of Project Settings.


#include "CardComponentBase.h"

#include "ArkWars/ArkWars.h"
#include "ArkWars/Battle/System/CardManagerSubsystem.h"
#include "ArkWars/Battle/Toolkits/ArkWarTags.h"
#include "ArkWars/Battle/Toolkits/BattleFunctionLibrary.h"


UCardComponentBase::~UCardComponentBase()
{
	auto System = UBattleFunctionLibrary::GetCardManager(this);
	
	if (!System) return;
	
	TSharedPtr<FCardInfo> Info = nullptr;
	if (System->GetCardInfoByTag(Key,Info))
	{
		Info->DataGetter.Unbind();
		Info->Card = nullptr;
	}

}

UCardComponentBase* UCardComponentBase::Get(const UObject* WorldContextObject, const FGameplayTag& CardTag)
{
	auto System = UBattleFunctionLibrary::GetCardManager(WorldContextObject);
	
	if (!System)
	{
		UE_LOG(LogCard, Warning, TEXT("[UCardComponentBase][Get] Failed to get subsystem"))
		return nullptr;
	}
	auto RetVal = System->GetCardComponentByTag(CardTag);
	if (!RetVal)
	{
		UE_LOG(LogCard, Log, TEXT("[UCardComponentBase][Get] Try to create component"))

		TSharedPtr<FCardInfo> Info = nullptr;
		if (!System->GetCardInfoByTag(CardTag, Info)) return nullptr;
		
		if (Info->IsValid())
		{
			auto Comp = NewObject<UCardComponentBase>(System ,Info->Class, *Info->CardName.ToString());
			Info->DataGetter.BindUObject(Comp, &UCardComponentBase::DataConverter);
			Info->Card = Comp;
			Comp->Key = CardTag;
			return Comp;
		}
	}
	
	UE_LOG(LogCard, Warning, TEXT("[UCardComponentBase][Get] Tried to create component when not found but failed"))
	return nullptr;
}

UCardComponentBase* UCardComponentBase::Get(const UObject* WorldContextObject, const FGameplayTagContainer& Card)
{
	FGameplayTag Key;
	//TODO: 解码Key的方式
	
	return Get(WorldContextObject, Key);
}

void UCardComponentBase::Use(APlayerState* Source, const TArray<APlayerState*>& Targets, const FGameplayTagContainer& Card)
{
}

void UCardComponentBase::Move(const TArray<FGameplayTagContainer>& Cards, ICardContainerInterface* From,
	ICardContainerInterface* To)
{
}

void UCardComponentBase::Response(APlayerState* Source, APlayerState* Target, const FGameplayTagContainer& Card)
{
}
