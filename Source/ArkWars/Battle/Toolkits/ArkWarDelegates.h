#pragma once
#include "GameplayTagContainer.h"

struct FArkCard;
class APlayerState;
class AActor;

DECLARE_MULTICAST_DELEGATE_TwoParams(FGamePhaseChangeDelegate, const FGameplayTag& /* Before */, const FGameplayTag& /* Current */);
DECLARE_MULTICAST_DELEGATE_OneParam(FActivePlayerChangeDelegate, APlayerState* /* ActivePlayer */);
DECLARE_MULTICAST_DELEGATE(FGameRoundChangeDelegate);


DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerSelectionChangeDelegate, const TArray<APlayerState*>& /* Players */,	bool /* bLostSelection */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FCardSelectionChangeDelegate, TArray<FArkCard> /* Cards */, bool/* bLostSelection */);


///移动族事实通知：一次卡牌移动落子后的"已移动"事实（广播者 = UCardMoveTransaction::BroadcastFinish，卷 12 §5.1）
DECLARE_MULTICAST_DELEGATE_FourParams(FCardMovedDelegate, const FGameplayTag& /* Notify */, AActor* /* Instigator */, AActor* /* Target */, const TArray<FArkCard>& /* Cards */);
