// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillBaseComponent.h"
#include "GameFramework/PlayerState.h"
#include "ArkWars/ArkWars.h"
#include "ArkWars/Battle/System/SkillManagerSubsystem.h"
#include "ArkWars/Battle/Toolkits/BattleFunctionLibrary.h"


void USkillComponentBase::BeginPlay()
{
	Super::BeginPlay();
	RegisterComponent();
	
}

void USkillComponentBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	UnRegisterActivateTiming();
}

UActorComponent* USkillComponentBase::CreateSkill(APlayerState* Owner, const FGameplayTag& Skill)
{
	if (!Owner)
	{
		UE_LOG(LogSkill, Warning, TEXT("[USkillComponentBase][CreateSkill] Owner is invalid"))
		return nullptr;
	}
	
	if (Owner->FindComponentByTag(StaticClass(), Skill.GetTagName()))
	{
		UE_LOG(LogSkill, Warning, TEXT("[USkillComponentBase][CreateSkill] Skill %s had already registered to %s"), *Skill.ToString(), *Owner->GetName())
		return nullptr;
	}
	
	auto System = UBattleFunctionLibrary::GetSkillManager(Owner);

	if (!System)
	{
		UE_LOG(LogSkill, Warning, TEXT("[USkillComponentBase][CreateSkill] Skill manager was lost, failed to create skill %s"), *Skill.ToString())
		return nullptr;
	}

	auto&& SkillInfo = System->GetCurrentSkillByTag(Skill);

	if (!SkillInfo.IsValid() || !SkillInfo->SkillClass)
	{
		UE_LOG(LogSkill, Warning, TEXT("[USkillComponentBase][CreateSkill] Skill info or component class is missing for skill %s, creation aborted"), *Skill.ToString())
		return nullptr;
	}
	
	//	创建 + 延迟注册（bDeferred=true）：先把技能标签挂好，再 RegisterComponent 入场——
	//	运行期注册会立即触发组件 BeginPlay，入场前必须保证 Tag 就位（P3 §3.3 的时机索引登记要认它），
	//	与 CardComponentBase::CreateCard 同款顺序
	auto Comp = Owner->AddComponentByClass(SkillInfo->SkillClass, false, FTransform(), /*bDeferred=*/true);

	if (!Comp)
	{
		UE_LOG(LogSkill, Error, TEXT("[USkillComponentBase][CreateSkill] Component creation returned null for skill %s on owner %s"), *Skill.ToString(), *GetNameSafe(Owner))
		return nullptr;
	}

	Comp->ComponentTags.Add(Skill.GetTagName());
	Comp->RegisterComponent();
	
	
	UE_LOG(LogSkill, Log, TEXT("[USkillComponentBase][CreateSkill] Skill component %s created and registered on owner %s (Key:%s)"), *Comp->GetName(), *GetNameSafe(Owner), *Skill.ToString())

	return Comp;
}

FGameplayTag USkillComponentBase::GetSkillTag() const
{
	return SkillInfo.IsValid() ? SkillInfo->SkillTag : FGameplayTag::EmptyTag;
}
