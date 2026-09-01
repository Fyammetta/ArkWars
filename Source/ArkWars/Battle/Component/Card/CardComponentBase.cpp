// Fill out your copyright notice in the Description page of Project Settings.


#include "CardComponentBase.h"

#include "ArkWars/ArkWars.h"
#include "ArkWars/Battle/Toolkits/ArkWarTags.h"
#include "ArkWars/Battle/Toolkits/BattleFunctionLibrary.h"
#include "Net/UnrealNetwork.h"



void UCardComponentBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCardComponentBase, Key)
}

UCardComponentBase::UCardComponentBase()
{
	SetIsReplicatedByDefault(true);
}

UCardComponentBase* UCardComponentBase::Get(AActor* Owner, const FGameplayTag& CardTag)
{
	if (auto Component = Owner->FindComponentByTag(StaticClass(), CardTag.GetTagName()))
	{
		return Cast<UCardComponentBase>(Component);
	}
	
	if (auto System = UBattleFunctionLibrary::GetCardManager(Owner))
	{
		TSharedPtr<FCardInfo> Info = nullptr;
		if (!System->GetCardInfoByTag(CardTag, Info)) return nullptr;
		
		if (Info->IsValid())
		{
			auto Comp = NewObject<UCardComponentBase>(Owner,Info->Class, *Info->CardName.ToString());
			Comp->Key = CardTag;
			Comp->ComponentTags.Add(CardTag.GetTagName());
			Comp->ComponentTags.Add(CardTags::Root().GetTagName());
			Info->DataGetter.BindUObject(Comp, &UCardComponentBase::DataConverter);
			Comp->RegisterComponent();
			return Comp;
		}
	}
	
	UE_LOG(LogCard, Warning, TEXT("[UCardComponentBase][Get] Tried to create component when not found but failed"))
	return nullptr;
}

void UCardComponentBase::Use(AActor* Source, const TArray<AActor*>& Targets, const FGameplayTagContainer& Card)
{
}

void UCardComponentBase::Move(const TArray<FGameplayTagContainer>& Cards, ICardContainerInterface* From,
	ICardContainerInterface* To)
{
}

void UCardComponentBase::Response(AActor* Source, AActor* Target, const FGameplayTagContainer& Card)
{
}
