// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "OperatorBasicAttributes.generated.h"

/**
 * 
 */


#define ATTRIBUTE_DECLARE(ClassName, PropertyName)					\
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName)			\
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName)						\
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName)						\
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName) 

UCLASS()
class ARKWARS_API UOperatorBasicAttributes : public UAttributeSet
{
	GENERATED_BODY()
	
	
	///	最大生命值：生命的上限
	UPROPERTY(ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;

	///	当前生命值
	UPROPERTY(ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;

	///	护盾值：伤害结算时先行扣除的缓冲层
	UPROPERTY(ReplicatedUsing = OnRep_Shield)
	FGameplayAttributeData Shield;

	///	手牌上限加成：叠加在基础手牌上限之上的增减量
	UPROPERTY(ReplicatedUsing = OnRep_MaxHandCardAddition)
	FGameplayAttributeData MaxHandCardAddition;

	///	攻击距离修正：自己计算「到目标的距离」时叠加——类比三国杀的进攻马（-1 马），让自己够得更远
	UPROPERTY(ReplicatedUsing = OnRep_AttackDistance)
	FGameplayAttributeData AttackDistance;

	///	防御距离修正：他人计算「到自己的距离」时叠加——类比三国杀的防御马（+1 马），让别人够不到自己
	UPROPERTY(ReplicatedUsing = OnRep_DefendDistance)
	FGameplayAttributeData DefendDistance;

	///	攻击范围：由武器等来源提供的可及距离（打得到多远），与上面两项距离修正不是一回事
	UPROPERTY(ReplicatedUsing = OnRep_AttackReach)
	FGameplayAttributeData AttackReach;

public:
	ATTRIBUTE_DECLARE(UOperatorBasicAttributes, MaxHealth);
	ATTRIBUTE_DECLARE(UOperatorBasicAttributes, Health);
	ATTRIBUTE_DECLARE(UOperatorBasicAttributes, Shield);
	ATTRIBUTE_DECLARE(UOperatorBasicAttributes, MaxHandCardAddition);
	ATTRIBUTE_DECLARE(UOperatorBasicAttributes, AttackDistance);
	ATTRIBUTE_DECLARE(UOperatorBasicAttributes, DefendDistance);
	ATTRIBUTE_DECLARE(UOperatorBasicAttributes, AttackReach);
	
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	///	复制登记表（实现里逐个 DOREPLIFETIME_CONDITION_NOTIFY）；复制通道由 ASC 的
	///	ReplicateSubobjects 把本属性集作为 subobject 塞进 ActorChannel 提供
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	///	复制回写口：七个属性各一个，签名固定 const + 旧值；
	///	内部走 GAMEPLAYATTRIBUTE_REPNOTIFY，落点 ASC::SetBaseAttributeValueFromReplication
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;

	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth) const;

	UFUNCTION()
	void OnRep_Shield(const FGameplayAttributeData& OldShield) const;

	UFUNCTION()
	void OnRep_MaxHandCardAddition(const FGameplayAttributeData& OldMaxHandCardAddition) const;

	UFUNCTION()
	void OnRep_AttackDistance(const FGameplayAttributeData& OldAttackDistance) const;

	UFUNCTION()
	void OnRep_DefendDistance(const FGameplayAttributeData& OldDefendDistance) const;

	UFUNCTION()
	void OnRep_AttackReach(const FGameplayAttributeData& OldAttackReach) const;
};
