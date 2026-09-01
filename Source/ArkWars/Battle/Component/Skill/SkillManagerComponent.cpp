// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillManagerComponent.h"

#include "ArkWars/Battle/System/SkillManagerSubsystem.h"
#include "ArkWars/Battle/Toolkits/BattleFunctionLibrary.h"




void USkillManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void USkillManagerComponent::InitAsOperator(const FOperatorCardInfo& Info)
{
	TArray<FGameplayTag> SkillTags;
	Info.SkillComps.GetGameplayTagArray(SkillTags);
	if (SkillTags.IsEmpty()) return;
	
	auto Subsystem = UBattleFunctionLibrary::GetSkillManager(this);
	if (!Subsystem) return;
	
	FOperatorSkillInfo SkInfo;
	FSkillAbilityMapping Mapping;
	for (const FGameplayTag& Skill : SkillTags)
	{
		Skills.Add(Subsystem->GetCurrentSkillByTag(Skill));
	}
}
