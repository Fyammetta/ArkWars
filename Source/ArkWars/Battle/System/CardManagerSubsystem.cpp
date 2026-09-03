// Fill out your copyright notice in the Description page of Project Settings.


#include "CardManagerSubsystem.h"

#include "ArkWars/ArkWars.h"
#include "ArkWars/Battle/Component/Card/CardComponentBase.h"
#include "ArkWars/Battle/Toolkits/ArkWarTags.h"
#include "ArkWars/Settings/BattleCardSettings.h"

FCardInfo& FCardInfo::operator=(const FCardInfo& Other)
{
	CardTags = Other.CardTags;
	CardName = Other.CardName;
	Description = Other.Description;
	Data = Other.Data;
	Class = Other.Class;
	DataGetter = Other.DataGetter;
	
	return *this;
}

FCardInfo& FCardInfo::operator=(const FHandCardInfo& Info)
{
	CardTags.AddTag(Info.CardTag);
	CardTags.AddTag(Info.CardType);
	CardName = Info.CardName;
	Description = Info.Description;
	Data = Info.Data;
	Class = nullptr;
	DataGetter = nullptr;
	return *this;
}

bool FCardInfo::IsValid()
{
	return Class != nullptr;
}

void UCardManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	auto Settings = UBattleCardSettings::Get();
	if (!Settings)
	{
		UE_LOG(LogCard, Warning, TEXT("[UCardManagerSubsystem][Initialize] Failed to find settings"))
		return;
	}
	
	auto CardData = Settings->CardInfoDataTable.LoadSynchronous();
	auto CardMapping = Settings->CardMappingDataTable.LoadSynchronous();
	
	if (!CardData || !CardMapping)
	{
		UE_LOG(LogCard, Warning, TEXT("[UCardManagerSubsystem][Initialize] Failed to load card data asset"))
		return;
	}
	
	CardData->ForeachRow<FHandCardInfo>(TEXT("[UCardManagerSubsystem][Initialize][Info]"),
		[&Map = InfoMapping](const FName& Key, const FHandCardInfo& Row)->void
		{
			check(!Map.Contains(Row.CardTag))
			*Map.Add(Row.CardTag) = Row;
		});
	
	CardData->ForeachRow<FCardComponentMapping>(TEXT("[UCardManagerSubsystem][Initialize][Component]"),
	[&Map = InfoMapping](const FName& Key, const FCardComponentMapping& Row)->void
	{
		if (Row.Comp && Row.Comp->GetDefaultObject()->IsA(UCardComponentBase::StaticClass()) && Map.Contains(Row.Tag))
			(*Map.Find(Row.Tag))->Class = Row.Comp;
	});
}

void UCardManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

namespace 
{
	FORCEINLINE void Check(const TCHAR* Context, const FGameplayTag& Tag)
	{
#if !UE_BUILD_SHIPPING
		if (!Tag.MatchesTag(CardTags::Root))
			UE_LOG(LogCard, Warning, TEXT("[UCardManagerSubsystem][%s] Tag %s is not for Card"),Context, *Tag.ToString())
		else if (!Tag.MatchesTag(CardTags::Class))
		{
			UE_LOG(LogCard, Warning, TEXT("[UCardManagerSubsystem][%s] Tag %s is not for Card Class"),Context, *Tag.ToString())
		}
		else
		{
			UE_LOG(LogCard, Warning, TEXT("[UCardManagerSubsystem][%s] Tag %s is not register in Subsystem"),Context, *Tag.ToString())
		}
#endif
	}
}

bool UCardManagerSubsystem::GetCardInfoByTag(const FGameplayTag& Tag, TSharedPtr<FCardInfo>& OutInfo)
{
	if (InfoMapping.Contains(Tag))
	{
		OutInfo = *InfoMapping.Find(Tag);
		return true;

	}
	
	Check(TEXT("GetCardInfoByTag"),Tag);
	return false;
}

UCardComponentBase* UCardManagerSubsystem::GetCardComponentByTag(const FGameplayTag& Tag)
{
	TSharedPtr<FCardInfo> Ptr = nullptr;
	if (GetCardInfoByTag(Tag, Ptr))
	{
		return Ptr->Card;
	}
	
	return nullptr;
}

FGameplayTag UCardManagerSubsystem::GetCardType(const FGameplayTag& Tag)
{
	if (InfoMapping.Contains(Tag))
	{
		for (const auto& EachTag : InfoMapping[Tag]->CardTags)
		{
			if (EachTag.MatchesTag(CardTags::Type))
			{
				return EachTag;
			}
		}
		UE_LOG(LogCard, Warning, TEXT("[UCardManagerSubsystem][GetCardType] Tag Type is not found"))
		return FGameplayTag();
	}
	
	Check(TEXT("GetCardType"),Tag);
	return FGameplayTag();
}
