// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "AbilitySystemSettings.generated.h"

class UAttributeSet;
class UGameplayEffect;

/**
 *	GAS 执行载体的资产目录（卷 06 §6：GE/ASC 承担执行载体，GAS 不裁决时序）。
 *	只收"要被引用"的资产槽位，不含运行期逻辑与校验——取用与判定归各自的卷。
 *	P4 §1 决策项落案 A（GAS 满配）时，本类即其配置面。
 */
UCLASS(Config = Game, DisplayName = "Ark War Ability System")
class ARKWARS_API UAbilitySystemSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	static UAbilitySystemSettings* Get();

	/// 玩家 ASC 挂载的属性集类（卷 06 §6：AttributeSet 只放属性与随属性后置逻辑）
	UPROPERTY(EditAnywhere, Config, Category = "Data|Attribute")
	TSubclassOf<UAttributeSet> AttributeSetClass;

	/// 伤害结算 GE 模板（P4 §3）：动态构造的底，数值经 SetByCaller 携带
	UPROPERTY(EditAnywhere, Config, Category = "Data|Effect")
	TSoftObjectPtr<UGameplayEffect> DamageEffectTemplate;

	UGameplayEffect* GetDamageEffectTemplate() const;
};