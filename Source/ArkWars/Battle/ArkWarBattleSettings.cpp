// Fill out your copyright notice in the Description page of Project Settings.


#include "ArkWarBattleSettings.h"

UArkWarBattleSettings* UArkWarBattleSettings::Get()
{
	return GetMutableDefault<UArkWarBattleSettings>();
}
