// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillManagerSubsystem.h"
#include "ArkWars/ArkWars.h"
#include "ArkWars/Battle/ArkWarBattleSettings.h"
#include "GameplayAbilitySpec.h"
#include "ArkWars/Settings/OperatorSettings.h"

FSkillInfo::FSkillInfo()
{
	SkillTag = {};
	SkillType = {};
	SkillName = FText::GetEmpty();
	Description = FText::GetEmpty();
	Data = {};
	SkillClass = nullptr;
}

FSkillInfo::FSkillInfo(const TSubclassOf<USkillComponentBase>& Comp)
{
	SkillClass = Comp;
}

FSkillInfo& FSkillInfo::operator=(const FOperatorSkillInfo& Info)
{	
	SkillTag = Info.SkillTag;
	SkillType = Info.SkillType;
	SkillName = Info.SkillName;
	Description = Info.Description;
	Data = Info.Data;
	
	return *this;
}

FSkillInfo& FSkillInfo::operator=(const FSkillInfo& Info)
{
	SkillTag = Info.SkillTag;
	SkillType = Info.SkillType;
	SkillName = Info.SkillName;
	Description = Info.Description;
	Data = Info.Data;
	
	return *this;
}


void USkillManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	UE_LOG(LogSkill, Log, TEXT("[SkillManager][Initialize]%s"), *GetLogNetContext());

	Super::Initialize(Collection);
}

void USkillManagerSubsystem::Deinitialize()
{
	UE_LOG(LogSkill, Log, TEXT("[SkillManager][Deinitialize]%s"), *GetLogNetContext());

	Super::Deinitialize();
	SkillMapping.Empty();
}

TSharedPtr<FSkillInfo> USkillManagerSubsystem::GetCurrentSkillByTag(const FGameplayTag& Tag)
{
	return SkillMapping.Contains(Tag) ? SkillMapping[Tag] : nullptr;
}

void USkillManagerSubsystem::OnAllOperatorSelected(const TArray<FOperatorCardInfo>& Operators)
{
	
	UE_LOG(LogSkill, Log, TEXT("[SkillManager][OnAllOperatorSelected]%s Start to init skills for all operator in pending"), *GetLogNetContext());
	TArray<FGameplayTag> SkillTags;
	for (const FOperatorCardInfo& Operator : Operators)
	{
		SkillTags.Append(Operator.SkillComps.GetGameplayTagArray());
	}
	if (SkillTags.IsEmpty())
	{
		UE_LOG(LogSkill, Warning, TEXT("[SkillManager][OnAllOperatorSelected]%s No skill tag was found, skip out"), *GetLogNetContext());
		return;
	}
	auto Settings = UOperatorSettings::Get();
	if (!Settings)
	{
		UE_LOG(LogSkill, Warning, TEXT("[SkillManager][OnAllOperatorSelected]%s Failed to find OperatorSettings"),*GetLogNetContext());
		return;
	}

	Settings->GetSkillMappingDataTable()->ForeachRow<FSkillComponentMapping>(TEXT("[SkillManager][InitAbility]"), 
		[&List = SkillMapping, &SkillTags ,InitAll = SkillTags.IsEmpty()](const FName& Key, const FSkillComponentMapping& Row)->void
	{
		if (InitAll || SkillTags.Contains(Row.Tag))
		{
			*List.Add(Row.Tag) = Row.Comp;
		}
	});
	if (SkillMapping.IsEmpty())
	{
		UE_LOG(LogSkill, Warning, TEXT("[SkillManager][OnAllOperatorSelected]%s No operator was filled into container,skip out"), *GetLogNetContext());
		return;
	}
	
	Settings->GetSkillInfoDataTable()->ForeachRow<FOperatorSkillInfo>(TEXT("[SkillManager][InitInfo]"), 
		[&List = SkillMapping](const FName& Key, const FOperatorSkillInfo& Row)->void
	{
		if (List.Contains(Row.SkillTag))
		{
			*List[Row.SkillTag] = Row;
		}
	});

}

TSharedPtr<FSkillInfo> USkillManagerSubsystem::AppendSkill(const FGameplayTag& Tag)
{
	UE_LOG(LogSkill, Log , TEXT("[SkillManager][AppendSkill]%s Start to append skill %s to container"), *GetLogNetContext(), *Tag.ToString());

	TArray<FSkillComponentMapping*> Mappings;
	TArray<FOperatorSkillInfo*> Infos;
	
	auto Settings = UOperatorSettings::Get();
	if (!Settings)
	{
		UE_LOG(LogSkill, Warning, TEXT("[SkillManager][AppendSkill]%s Failed to find OperatorSettings"), *GetLogNetContext());
		return nullptr;
	}
	
	Settings->GetSkillMappingDataTable()->GetAllRows<FSkillComponentMapping>(TEXT("[SkillManager][FindAbility]"),Mappings);
	Settings->GetSkillInfoDataTable()->GetAllRows<FOperatorSkillInfo>(TEXT("[SkillManager][FindInfo]"),Infos);

	if (Infos.IsEmpty() || Mappings.IsEmpty())
	{
		UE_LOG(LogSkill, Warning, TEXT("[SkillManager][AppendSkill]%s Infos has %d elements, Mappings has %d elements"), *GetLogNetContext(), Infos.Num(), Mappings.Num());
		return nullptr;
	}
	
	TSharedPtr<FSkillInfo> Ptr = nullptr;
	for (FSkillComponentMapping* Mapping : Mappings)
	{
		if (Mapping->Tag == Tag)
		{
			Ptr = MakeShared<FSkillInfo>(Mapping->Comp);
			break;
		}
	}
	if (!Ptr)
	{
		UE_LOG(LogSkill, Warning, TEXT("[SkillManager][AppendSkill]%s Skill %s was not constructed successfully"), *GetLogNetContext(), *Tag.ToString());
		return nullptr;
	}
	
	for (FOperatorSkillInfo* Info : Infos)
	{
		if (Info->SkillTag == Tag)
		{
			*Ptr = *Info;
			break;
		}
	}
	
	return Ptr;
}