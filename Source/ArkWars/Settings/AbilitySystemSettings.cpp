// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystemSettings.h"
#include "GameplayEffect.h"

UAbilitySystemSettings* UAbilitySystemSettings::Get()
{
	return GetMutableDefault<UAbilitySystemSettings>();
}

UGameplayEffect* UAbilitySystemSettings::GetDamageEffectTemplate() const
{
	check(DamageEffectTemplate.IsValid());
	return DamageEffectTemplate.LoadSynchronous();
}