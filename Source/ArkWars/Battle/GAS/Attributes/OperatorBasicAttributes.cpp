// Fill out your copyright notice in the Description page of Project Settings.


#include "OperatorBasicAttributes.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"



void UOperatorBasicAttributes::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
		if (Attribute == GetAttackReachAttribute() || Attribute == GetAttackDistanceAttribute() || Attribute == GetDefendDistanceAttribute())
    	{
    		//攻击范围与距离修正不小于0，无上限
    		NewValue = FMath::Max(0.0f, NewValue);
    	}
    	
    	
    	if (Attribute == GetMaxHealthAttribute())
    	{
    		if (auto GM = GetWorld() ? GetWorld()->GetGameState() : nullptr)
    		{
    			//最大生命值不大于两倍玩家数，不小于0
    			auto Max = GM->PlayerArray.Num() * 2;
    			NewValue = FMath::Clamp(NewValue ,0.0f, Max);
    		}
    	}
    	
    	if (Attribute == GetHealthAttribute())
    	{
    		//血量不超过血量上限
    		NewValue = FMath::Min(GetMaxHealth(), NewValue);
    	}
}

void UOperatorBasicAttributes::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaxHealthAttribute())
	{
		//血量不超过血量上限
		if (NewValue < GetHealth())
		SetHealth(NewValue);
	}
}

void UOperatorBasicAttributes::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//	REPNOTIFY_Always：数值未变也回写，与 GAS 惯例一致（叠层/免疫等场景下值可能相等但语义已变）
	DOREPLIFETIME_CONDITION_NOTIFY(UOperatorBasicAttributes, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOperatorBasicAttributes, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOperatorBasicAttributes, Shield, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOperatorBasicAttributes, MaxHandCardAddition, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOperatorBasicAttributes, AttackDistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOperatorBasicAttributes, DefendDistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOperatorBasicAttributes, AttackReach, COND_None, REPNOTIFY_Always);
}

void UOperatorBasicAttributes::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOperatorBasicAttributes, MaxHealth, OldMaxHealth);
}

void UOperatorBasicAttributes::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOperatorBasicAttributes, Health, OldHealth);
}

void UOperatorBasicAttributes::OnRep_Shield(const FGameplayAttributeData& OldShield) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOperatorBasicAttributes, Shield, OldShield);
}

void UOperatorBasicAttributes::OnRep_MaxHandCardAddition(const FGameplayAttributeData& OldMaxHandCardAddition) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOperatorBasicAttributes, MaxHandCardAddition, OldMaxHandCardAddition);
}

void UOperatorBasicAttributes::OnRep_AttackDistance(const FGameplayAttributeData& OldAttackDistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOperatorBasicAttributes, AttackDistance, OldAttackDistance);
}

void UOperatorBasicAttributes::OnRep_DefendDistance(const FGameplayAttributeData& OldDefendDistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOperatorBasicAttributes, DefendDistance, OldDefendDistance);
}

void UOperatorBasicAttributes::OnRep_AttackReach(const FGameplayAttributeData& OldAttackReach) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOperatorBasicAttributes, AttackReach, OldAttackReach);
}
