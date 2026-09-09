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

UCardComponentBase* UCardComponentBase::Get(AActor* Owner, const FGameplayTag& CardTag)
{
	if (!Owner || !Owner->HasAuthority())
	{
		UE_LOG(LogCard, Warning, TEXT("[UCardComponentBase][Get] Component can only be found on server"))
		return nullptr;		
	}
	auto RetVal = Cast<UCardComponentBase>(Owner->FindComponentByTag(StaticClass(), CardTag.GetTagName()));


	if (RetVal)
	{
		return RetVal;
	}
	
	auto System = UBattleFunctionLibrary::GetCardManager(Owner);
	
	if (!System)
	{
		UE_LOG(LogCard, Warning, TEXT("[UCardComponentBase][Get] Failed to get subsystem"))
		return nullptr;
	}
	if (!RetVal)
	{
		UE_LOG(LogCard, Log, TEXT("[UCardComponentBase][Get] Try to create component"))

		TSharedPtr<FCardInfo> Info = nullptr;
		if (!System->GetCardInfoByTag(CardTag, Info)) return nullptr;
		
		if (Info->IsValid())
		{
			RetVal = Cast<UCardComponentBase>(Owner->AddComponentByClass(Info->Class,false, FTransform(),true));
			RetVal->ComponentTags.Add(CardTag.GetTagName());
			RetVal->RegisterComponent();
			Info->DataGetter.BindUObject(RetVal, &UCardComponentBase::DataConverter);
			Info->Card = RetVal;
			RetVal->Key = CardTag;
			return RetVal;
		}
	}
	
	UE_LOG(LogCard, Warning, TEXT("[UCardComponentBase][Get] Tried to create component when not found but failed"))
	return nullptr;
}

UCardComponentBase* UCardComponentBase::Get(AActor* Owner, const FArkCard& Card)
{
	FGameplayTag Key;
	//TODO: 解码Key的方式
	
	return Get(Owner, Key);
}

void UCardComponentBase::Use(APlayerState* Source, const TArray<APlayerState*>& Targets, const FArkCard& Card)
{
}

void UCardComponentBase::Move(const TArray<FArkCard>& Cards, ICardContainerInterface* From,
	ICardContainerInterface* To)
{
}

void UCardComponentBase::Response(APlayerState* Source, APlayerState* Target, const FArkCard& Card)
{
}
