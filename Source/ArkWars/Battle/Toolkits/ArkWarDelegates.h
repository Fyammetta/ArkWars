#pragma once
#include "ArkWarTypes.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FGamePhaseChangeDelegate, EGamePhase /* Before */, EGamePhase /* Current */);
DECLARE_MULTICAST_DELEGATE_OneParam(FActivePlayerChangeDelegate, APlayerState* /* ActivePlayer */);


DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerSelectionChangeDelegate, TArray<TWeakObjectPtr<APlayerState>> /* Players */, bool /* bLostSelection */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FCardSelectionChangeDelegate, TArray<FCard> /* Cards */, bool/* bLostSelection */);
