// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleCardSettings.h"

UBattleCardSettings* UBattleCardSettings::Get()
{
	return GetMutableDefault<UBattleCardSettings>();
}
