
#include "GameModeComponentBase.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "ArkWars/ArkWars.h"
#include "ArkWars/Battle/System/BattleGameFlowSubsystem.h"
#include "ArkWars/Battle/Toolkits/ArkWarTags.h"
#include "ArkWars/Battle/Toolkits/BattleFunctionLibrary.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"


UGameModeComponentBase* UGameModeComponentBase::GetCurrentGameMode(UObject* WorldContextObject)
{
	auto GM = WorldContextObject->GetWorld() ? WorldContextObject->GetWorld()->GetAuthGameMode() : nullptr;
	if (!GM)
	{
		return nullptr;
	}
	return GM->FindComponentByClass<UGameModeComponentBase>();

}
