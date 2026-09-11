#pragma once
#include "GameplayTagContainer.h"

struct FArkCard;
struct FResponseWindow;
struct FTransactionModRequest;
class APlayerState;
class AActor;
class UBattleTransaction;

DECLARE_MULTICAST_DELEGATE_TwoParams(FGamePhaseChangeDelegate, const FGameplayTag& /* Before */, const FGameplayTag& /* Current */);
DECLARE_MULTICAST_DELEGATE_OneParam(FActivePlayerChangeDelegate, APlayerState* /* ActivePlayer */);
DECLARE_MULTICAST_DELEGATE(FGameRoundChangeDelegate);


DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerSelectionChangeDelegate, const TArray<APlayerState*>& /* Players */,	bool /* bLostSelection */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FCardSelectionChangeDelegate, TArray<FArkCard> /* Cards */, bool/* bLostSelection */);


///移动族事实通知：一次卡牌移动落子后的"已移动"事实（广播者 = UCardMoveTransaction::BroadcastFinish，卷 12 §5.1）
DECLARE_MULTICAST_DELEGATE_FourParams(FCardMovedDelegate, const FGameplayTag& /* Notify */, AActor* /* Instigator */, AActor* /* Target */, const TArray<FArkCard>& /* Cards */);

///	C 面事件化委托（卷 12 §5.2 / 附录 B）：把引擎内部推进链路开放为"只增观察点"的订阅位。
///	纪律：广播是旁路——订阅者回调抛错/耗时不得改变交易与窗口推进（卷 12 §7）；均为服务器侧观察点。

///	窗口开启：Responders 名单填出后、开始询问前广播（广播者 = UBattleGameFlowSubsystem::OpenTimingWindow）
DECLARE_MULTICAST_DELEGATE_OneParam(FWindowOpenedDelegate, const FResponseWindow& /* Window */);
///	窗口关闭：结果回写完成后、回调持有者（阶段机 Advance / 交易 OnWindowClosed）之前广播（广播者 = CloseTimingWindow）
DECLARE_MULTICAST_DELEGATE_TwoParams(FWindowClosedDelegate, const FResponseWindow& /* Window */, bool /* bAnyResponded */);
///	交易被修改：ModeRequests.Add 之后广播，仅日志/表现/回放用，不参与折叠（广播者 = UBattleTransaction::AppendModification）
DECLARE_MULTICAST_DELEGATE_TwoParams(FTransactionModifiedDelegate, UBattleTransaction* /* Tx */, const FTransactionModRequest& /* Req */);
