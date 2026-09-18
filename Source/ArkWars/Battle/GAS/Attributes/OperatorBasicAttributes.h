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
	
	FGameplayAttributeData MaxHealth;

	FGameplayAttributeData Health;

	FGameplayAttributeData Shield;
	
	FGameplayAttributeData MaxHandCardAddition;
	
	FGameplayAttributeData AttackDistance;

	FGameplayAttributeData DefendDistance;
	
	FGameplayAttributeData AttackReach;

public:
	ATTRIBUTE_DECLARE(UOperatorBasicAttributes, MaxHealth);
	ATTRIBUTE_DECLARE(UOperatorBasicAttributes, Health);
	ATTRIBUTE_DECLARE(UOperatorBasicAttributes, Shield);
	ATTRIBUTE_DECLARE(UOperatorBasicAttributes, MaxHandCardAddition);
	ATTRIBUTE_DECLARE(UOperatorBasicAttributes, AttackDistance);
	ATTRIBUTE_DECLARE(UOperatorBasicAttributes, DefendDistance);
	ATTRIBUTE_DECLARE(UOperatorBasicAttributes, AttackReach);
};
