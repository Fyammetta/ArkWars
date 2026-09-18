#pragma once
#include "GameplayTagContainer.h"

struct FArkCard;
struct FResponseWindow;
struct FClientResponseWindow;
struct FTransactionModRequest;
class APlayerState;
class AActor;
class UBattleTransaction;

///	====================================================================================================
///	通知面分类宪法（卷 12 §2；第一原则见卷 03 §2）——系统里所有"通知"归三个面，绝不混用：
///
///	A 事实通知面   —— 事情【已发生】：同步多播委托（本文件）+ 复制 OnRep。订阅者只读、只反应，不可否决。
///	B 查询通知面   —— 事情【将发生】：载体不是委托，而是响应窗口（OpenTimingWindow + Responders + 负载，
///	                  宿主 = UBattleGameFlowSubsystem）。响应者可修改/阻止/替代/追加（唯一收口 AppendModification）——
///	                  故本文件没有 B 面声明，B 面的一切在窗口机制里。
///	C 生命周期通知面 —— 引擎【内部推进】：直调钩子（BroadcastFinish / OnWindowClosed / OnTransactionFinished），
///	                  默认不可订阅；外部观察必须走文件末尾的事件化委托（卷 12 §5.2），只看不动手。
///
///	评审守则（新增事件先回答"它已被事实化了吗"）：
///	  已发生          → A 面只读广播；
///	  将发生且可干预   → B 面开窗；
///	  引擎内务        → C 面直调 + 可选观察位。
///	混用的代价：可干预的查询塞进 A 面 = 响应者失去介入权；已发生的事实开成 B 面窗口 = 全局卡死等否决（卷 03 §2）。
///	====================================================================================================

///	A 面：流程事实（阶段已切换 / 行动玩家已变更 / 一轮已结束）——只读反应，不可否决
DECLARE_MULTICAST_DELEGATE_TwoParams(FGamePhaseChangeDelegate, const FGameplayTag& /* Before */, const FGameplayTag& /* Current */);
DECLARE_MULTICAST_DELEGATE_OneParam(FActivePlayerChangeDelegate, APlayerState* /* ActivePlayer */);
DECLARE_MULTICAST_DELEGATE(FGameRoundChangeDelegate);


///	A 面：选择集合已被取走（表现侧 UI 订阅；"取走即广播"口径见卷 12 §3.1 现状差异）
DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerSelectionChangeDelegate, const TArray<APlayerState*>& /* Players */,	bool /* bLostSelection */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FCardSelectionChangeDelegate, TArray<FArkCard> /* Cards */, bool/* bLostSelection */);


///	A 面·移动族事实通知：一次卡牌移动落子后的"已移动"事实（广播者 = UCardMoveTransaction::BroadcastFinish，卷 12 §5.1）
DECLARE_MULTICAST_DELEGATE_FourParams(FCardMovedDelegate, const FGameplayTag& /* Notify */, AActor* /* Instigator */, AActor* /* Target */, const TArray<FArkCard>& /* Cards */);

///	C 面事件化委托（卷 12 §5.2 / 附录 B）：把引擎内部推进链路开放为"只增观察点"的订阅位——只看不动手。
///	纪律：广播是旁路——订阅者回调抛错/耗时不得改变交易与窗口推进（卷 12 §7）；均为服务器侧观察点。
///	注意：窗口的"询问/响应"本身是 B 面（窗口机制），下面四条只是它的 C 面观察位，订阅它们得不到任何干预权。

///	窗口开启：Responders 名单填出后、开始询问前广播（广播者 = UBattleGameFlowSubsystem::OpenTimingWindow）
DECLARE_MULTICAST_DELEGATE_OneParam(FWindowOpenedDelegate, const FResponseWindow& /* Window */);
///	窗口关闭：结果回写完成后、回调持有者（阶段机 Advance / 交易 OnWindowClosed）之前广播（广播者 = CloseTimingWindow）
DECLARE_MULTICAST_DELEGATE_TwoParams(FWindowClosedDelegate, const FResponseWindow& /* Window */, bool /* bAnyResponded */);
///	响应锁切换：锁进入/退出瞬间广播，bool 形参为新值（广播者 = TryEnterResponseLock / ExitResponseLock，卷 10 §4 / 卷 12 §5.2）
DECLARE_MULTICAST_DELEGATE_OneParam(FResponseLockChangedDelegate, bool /* bResponding */);
///	交易被修改：ModeRequests.Add 之后广播，仅日志/表现/回放用，不参与折叠（广播者 = UBattleTransaction::AppendModification）
DECLARE_MULTICAST_DELEGATE_TwoParams(FTransactionModifiedDelegate, UBattleTransaction* /* Tx */, const FTransactionModRequest& /* Req */);
/// 交易入队，忙锁分支之前广播
DECLARE_MULTICAST_DELEGATE_OneParam(FTransactionEnqueuedDelegate, UBattleTransaction* /* Tx */);
/// 交易完成，被清除忙锁前广播
DECLARE_MULTICAST_DELEGATE_TwoParams(FTransactionFinishedDelegate, UBattleTransaction* /* Tx */, bool /* bSuccess */);
/// 交易被附加嵌套交易后或嵌套交易完成前广播
DECLARE_MULTICAST_DELEGATE_TwoParams(FTransactionNestedDelegate, UBattleTransaction* /* Child */, UBattleTransaction* /* Parent */);

///	====================================================================================================
///	客户端侧（非观察点）：服务器把窗口【定向】下发给被询问者本人后，由该玩家的 PC 广播（卷 11 §5.2）。
///	与上面的服务器侧观察位分属两端，刻意不复用 FWindowOpenedDelegate：
///	  · 形状不同——后者带 FResponseWindow（含交易强引用与 Responders 名单），客户端一个都填不出；
///	    客户端那份裁剪视图是 FClientResponseWindow。
///	  · 受众不同——后者是【全局观察点】（"这一轮谁在响应"，旁观/日志/调试）；
///	    本委托是【定向消息】（"我该不该弹面板"）。改成世界广播即丢失"给谁"的语义
///	    （FClientResponseWindow 无 Target 字段——收件人由 Client RPC 的定向性白送）。
///	  · 语义陷阱——硬塞进 FWindowOpenedDelegate 只能填出空 Responders，而空 Responders
///	    在服务器侧恰好是"无监听直通"（OpenTimingWindow 判 IsEmpty 即关窗）：同一形状、同样为空，两端意思相反。
///	====================================================================================================

///	响应窗口已下发（定向）：订阅者 = 本玩家的 UI（展开面板 + 自起倒计时）与需要表现的技能组件
DECLARE_MULTICAST_DELEGATE_OneParam(FWindowResponseRequestedDelegate, const FClientResponseWindow& /* Window */);

///	选角候选已下发（定向，同族：客户端侧非观察点）：订阅者 = 本玩家的 UI（展开选角界面）。
///	宿主同为 PC，理由与上一条一致——候选名单只属于"被问的这个人"，广播到世界即丢失"给谁"的语义
DECLARE_MULTICAST_DELEGATE_OneParam(FOperatorSelectRequestedDelegate, const TArray<FName>& /* OperatorList */);
