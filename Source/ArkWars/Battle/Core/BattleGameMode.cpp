// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleGamemode.h"
#include "ArkWars/Battle/ArkWarBattleSettings.h"
#include "ArkWars/Battle/Component/GameMode/GameModeComponentBase.h"


void ABattleGameMode::InitGameMode(const FGameplayTag& ModeTag)
{	
	if (auto Comp = FindComponentByClass(UGameModeComponentBase::StaticClass()))
		RemoveOwnedComponent(Comp);
	
	UGameModeComponentBase* Comp = nullptr;
	if (auto Settings = UArkWarBattleSettings::Get())
	{
		if (Settings->GameModes.Contains(ModeTag))
		{
			Comp = NewObject<UGameModeComponentBase>(this,Settings->GameModes[ModeTag]);
			Comp->RegisterComponent();
		}
	}
}
