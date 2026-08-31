// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SkillManagerSubsystem.generated.h"

/**
 *	用于管理当局内的所有技能
 *	便于通过技能Tag获取技能的具体信息
 */
UCLASS()
class ARKWARS_API USkillManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
};
