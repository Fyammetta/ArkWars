// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillManagerSubsystem.h"
#include "ArkWars/ArkWars.h"
#include "ArkWars/Battle/ArkWarBattleSettings.h"
#include "GameFramework/PlayerState.h"
#include "ArkWars/Battle/Component/Skill/SkillComponentBase.h"
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
	UE_LOG(LogSkill, Log, TEXT("[SkillManager][Initialize] %s"), *GetLogNetContext());

	Super::Initialize(Collection);
}

void USkillManagerSubsystem::Deinitialize()
{
	UE_LOG(LogSkill, Log, TEXT("[SkillManager][Deinitialize] %s"), *GetLogNetContext());

	Super::Deinitialize();
	SkillMapping.Empty();
}

TSharedPtr<FSkillInfo> USkillManagerSubsystem::GetCurrentSkillByTag(const FGameplayTag& Tag)
{
	if (const TSharedPtr<FSkillInfo>* Found = SkillMapping.Find(Tag))
	{
		return *Found;
	}

	//	索引未预载时的兜底：批量预载（OnAllOperatorSelected）要等"全部干员选完"，
	//	而玩家是各自选完即刻初始化自己的——先落座的那位必然扑空。
	//	此处按需补载，载入即入索引（见 AppendSkill 末尾），后续查询不再扫表
	return AppendSkill(Tag);
}

TArray<FSkillListenerEntry> USkillManagerSubsystem::GetSkillListeners(const FGameplayTag& Timing) const
{
	//	门控：索引只在服务器被填写（B 面开窗的点名来源），客户端查询属脏查询——
	//	拦下并回空表，正好落在窗口侧"空表直通"的零延迟路径上
	if (!IsRunningOnServer())
	{
		UE_LOG(LogSkill, Warning, TEXT("[SkillManager][GetSkillListeners] %s Timing index should only be queried on server (Timing:%s)"), *GetLogNetContext(), *Timing.ToString());
		return {};
	}

	//	按值返回 = 快照：调用方（CollectResponders）遍历期间源表可自由增删，不踩迭代器失效
	return SkillTimings.Contains(Timing) ? SkillTimings[Timing] : TArray<FSkillListenerEntry>{};
}

void USkillManagerSubsystem::RegisterSkillTiming(const FGameplayTag& Timing, FSkillListenerEntry&& Entry)
{
	//	门控：客户端组件若也走 BeginPlay，登记进本地索引纯属脏账（索引只服务开窗，开窗仅服务器）
	if (!IsRunningOnServer())
	{
		UE_LOG(LogSkill, Warning, TEXT("[SkillManager][RegisterSkillTiming] %s Timing index should only be written on server (Timing:%s)"), *GetLogNetContext(), *Timing.ToString());
		return;
	}

	//	条目验活：组件已失效的登记没有意义（开窗预检第一关就是验活，收进来也是白占一行）
	if (!Entry.IsValid())
	{
		UE_LOG(LogSkill, Warning, TEXT("[SkillManager][RegisterSkillTiming] %s Listener entry carries no valid skill component, registration of timing %s rejected"), *GetLogNetContext(), *Timing.ToString());
		return;
	}
	
	auto& Listeners = SkillTimings.FindOrAdd(Timing);

	//	日志先行、移动在后——MoveTemp 之后 Entry 已被掏空，报不出名字
	auto SKillName = Entry.Skill->GetName();
	auto OwnerName = Entry.Owner->GetName();
	
	Listeners.AddUnique(MoveTemp(Entry));

	UE_LOG(LogSkill, Log, TEXT("[SkillManager][RegisterSkillTiming] %s Skill %s (Owner:%s) registered to timing %s"), *GetLogNetContext(), *SKillName, *OwnerName, *Timing.ToString());
}

void USkillManagerSubsystem::UnregisterSkillTiming(const FGameplayTag& Timing, USkillComponentBase* Owner)
{
	if (!IsRunningOnServer())
	{
		UE_LOG(LogSkill, Warning, TEXT("[SkillManager][UnregisterSkillTiming] %s Timing index should only be written on server (Timing:%s)"), *GetLogNetContext(), *Timing.ToString());
		return;
	}

	if (!Owner)
	{
		UE_LOG(LogSkill, Warning, TEXT("[SkillManager][UnregisterSkillTiming] %s Unregistering with null skill component (Timing:%s), nothing to match"), *GetLogNetContext(), *Timing.ToString());
		return;
	}

	if (SkillTimings.Contains(Timing))
	{
		//	按组件指针销账（登记拿 this 来、销账拿 this 走）：同一 SkillTag 可能挂在多名玩家身上，
		//	指针是唯一无歧义的钥匙；IsValid 前置确保已死条目不会误配
		auto& Array = SkillTimings[Timing];
		int32 Removed = 0;
		for (int32 Index = 0; Index < Array.Num();)
		{
			if (Array[Index].Skill.IsValid() && Array[Index].Skill.Get() == Owner)
			{
				Array.RemoveAt(Index);
				Removed++;
			}
			else
			{
				++Index;
			}
		}

		if (Removed > 0)
		{
			UE_LOG(LogSkill, Log, TEXT("[SkillManager][UnregisterSkillTiming] %s Skill %s unregistered from timing %s (%d entry removed)"), *GetLogNetContext(), *GetNameSafe(Owner), *Timing.ToString(), Removed);
		}
		else
		{
			//	销账查无此人 = 登记/退订两本账对不上，是排查悬案的第一现场，值得一条 Warning
			UE_LOG(LogSkill, Warning, TEXT("[SkillManager][UnregisterSkillTiming] %s Skill %s was not listening to timing %s, nothing removed"), *GetLogNetContext(), *GetNameSafe(Owner), *Timing.ToString());
		}
	}
}

void USkillManagerSubsystem::OnAllOperatorSelected(const TArray<FOperatorCardInfo>& Operators)
{
	UE_LOG(LogSkill, Log, TEXT("[SkillManager][OnAllOperatorSelected] %s Start to init skills for all operator in pending"), *GetLogNetContext());
	TArray<FGameplayTag> SkillTags;
	for (const FOperatorCardInfo& Operator : Operators)
	{
		SkillTags.Append(Operator.SkillComps.GetGameplayTagArray());
	}
	if (SkillTags.IsEmpty())
	{
		UE_LOG(LogSkill, Warning, TEXT("[SkillManager][OnAllOperatorSelected] %s No skill tag was found, skip out"), *GetLogNetContext());
		return;
	}
	auto Settings = UOperatorSettings::Get();
	if (!Settings)
	{
		UE_LOG(LogSkill, Warning, TEXT("[SkillManager][OnAllOperatorSelected] %s Failed to find OperatorSettings"),*GetLogNetContext());
		return;
	}

	Settings->GetSkillMappingDataTable()->ForeachRow<FSkillComponentMapping>(TEXT("[SkillManager][InitAbility] "), 
		[&List = SkillMapping, &SkillTags ,InitAll = SkillTags.IsEmpty()](const FName& Key, const FSkillComponentMapping& Row)->void
	{
		if (InitAll || SkillTags.Contains(Row.Tag))
		{
			*List.Add(Row.Tag) = Row.Comp;
		}
	});
	if (SkillMapping.IsEmpty())
	{
		UE_LOG(LogSkill, Warning, TEXT("[SkillManager][OnAllOperatorSelected] %s No operator was filled into container,skip out"), *GetLogNetContext());
		return;
	}
	
	Settings->GetSkillInfoDataTable()->ForeachRow<FOperatorSkillInfo>(TEXT("[SkillManager][InitInfo] "), 
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
	UE_LOG(LogSkill, Log , TEXT("[SkillManager][AppendSkill] %s Start to append skill %s to container"), *GetLogNetContext(), *Tag.ToString());

	TArray<FSkillComponentMapping*> Mappings;
	TArray<FOperatorSkillInfo*> Infos;
	
	auto Settings = UOperatorSettings::Get();
	if (!Settings)
	{
		UE_LOG(LogSkill, Warning, TEXT("[SkillManager][AppendSkill] %s Failed to find OperatorSettings"), *GetLogNetContext());
		return nullptr;
	}
	
	Settings->GetSkillMappingDataTable()->GetAllRows<FSkillComponentMapping>(TEXT("[SkillManager][FindAbility] "),Mappings);
	Settings->GetSkillInfoDataTable()->GetAllRows<FOperatorSkillInfo>(TEXT("[SkillManager][FindInfo] "),Infos);

	if (Infos.IsEmpty() || Mappings.IsEmpty())
	{
		UE_LOG(LogSkill, Warning, TEXT("[SkillManager][AppendSkill] %s Infos has %d elements, Mappings has %d elements"), *GetLogNetContext(), Infos.Num(), Mappings.Num());
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
		UE_LOG(LogSkill, Warning, TEXT("[SkillManager][AppendSkill] %s Skill %s was not constructed successfully"), *GetLogNetContext(), *Tag.ToString());
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

	//	回写索引：本函数是"按需补载"入口，不回写则每次调用都全表重扫；
	//	且 CreateSkill 走的是只读索引的 GetCurrentSkillByTag——不写回等于白载
	SkillMapping.Add(Tag, Ptr);

	return Ptr;
}