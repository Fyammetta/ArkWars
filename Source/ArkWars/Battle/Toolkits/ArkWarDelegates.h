#pragma once
#include "GameplayTagContainer.h"

class APlayerState;

DECLARE_MULTICAST_DELEGATE_TwoParams(FGamePhaseChangeDelegate, const FGameplayTag& /* Before */, const FGameplayTag& /* Current */);
DECLARE_MULTICAST_DELEGATE_OneParam(FActivePlayerChangeDelegate, APlayerState* /* ActivePlayer */);


DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerSelectionChangeDelegate, const TArray<APlayerState*>& /* Players */,	bool /* bLostSelection */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FCardSelectionChangeDelegate, TArray<FGameplayTagContainer> /* Cards */, bool/* bLostSelection */);
