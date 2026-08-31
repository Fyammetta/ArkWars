// Fill out your copyright notice in the Description page of Project Settings.


#include "CardManagerSubsystem.h"

#include "ArkWars/ArkWars.h"
#include "ArkWars/Battle/Component/HandCard/CardComponentBase.h"
#include "ArkWars/Battle/Toolkits/ArkWarTags.h"
#include "ArkWars/Battle/Toolkits/ArkWarTypes.h"
#include "ArkWars/Settings/BattleCardSettings.h"

TArray<FFormatArgumentData> FCardInfo::GetNumericalDatas()
{
	TArray<FFormatArgumentData> RetVal;
	FFormatArgumentData Arg;

	for (const FString& String : Data)
	{
		Arg.ArgumentName = FString::Printf(TEXT("{%d}"), Data.Find(String));
		Arg.ArgumentValueType = EFormatArgumentType::Int;
		if (String.IsNumeric())
		{
			Arg.ArgumentValueInt = FCString::Atoi64(*String);
		}
		else if (DataGetter.IsBound())
		{
			Arg.ArgumentValueInt = DataGetter.Execute(String);
		}
		else
		{
			Arg.ArgumentValueInt = 0;
		}
		
		RetVal.Add(MoveTemp(Arg));
	}
	
	return RetVal;
}

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
			Map.Add(Row.CardTag) = Row;
		});
	
	CardData->ForeachRow<FCardCompMapping>(TEXT("[UCardManagerSubsystem][Initialize][Component]"),
	[&Map = InfoMapping](const FName& Key, const FCardCompMapping& Row)->void
	{
		if (Row.Comp && Row.Comp->GetDefaultObject()->IsA(UCardComponentBase::StaticClass()) && Map.Contains(Row.Tag))
			Map.Find(Row.Tag)->Class = Row.Comp;
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
		if (!Tag.MatchesTag(CardTags::Root()))
			UE_LOG(LogCard, Warning, TEXT("[UCardManagerSubsystem][%s] Tag %s is not for Card"),Context, *Tag.ToString())
		else if (!Tag.MatchesTag(CardTags::Class()))
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

FCardInfo UCardManagerSubsystem::GetCardInfoByTag(const FGameplayTag& Tag)
{
	if (InfoMapping.Contains(Tag))
		return InfoMapping[Tag];
	
	Check(TEXT("GetCardInfoByTag"),Tag);
	return FCardInfo();
}

FGameplayTag UCardManagerSubsystem::GetCardType(const FGameplayTag& Tag)
{
	if (InfoMapping.Contains(Tag))
	{
		for (const auto& EachTag : InfoMapping[Tag].CardTags)
		{
			if (EachTag.MatchesTag(CardTags::Type()))
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
