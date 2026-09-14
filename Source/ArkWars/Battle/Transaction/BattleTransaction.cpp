// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleTransaction.h"
#include "ArkWars/Battle/System/BattleGameFlowSubsystem.h"
#include "ArkWars/Battle/Toolkits/BattleFunctionLibrary.h"

void UBattleTransaction::Start()
{
	//仅在服务器中入队
	if (GetWorld()->GetNetMode() != NM_ListenServer && GetWorld()->GetNetMode() != NM_DedicatedServer) return;

	if (!Validate() || State != EState::Spawned) return;
	
	State = EState::Started;
	auto System = UBattleFunctionLibrary::GetBattleManager(this);
	
	if (!System) return;
	
	System->EnqueueTransaction(this);
}


void UBattleTransaction::Drive()
{
	//	驱动器唯一入口（非虚，卷 04 §2）：P3 起改调 QueryTimings——开窗即返、不清 ActiveTransaction
	//	（忙锁继续，窗口悬停期入队的交易不重入）；后续链路全靠关窗回调 OnWindowClosed 续走。
	//	入口名与调用方 DriveTransaction 不变，驱动器不再直接触碰族钩子 Execute（P3 §3.4）
	QueryTimings();
}


void UBattleTransaction::AppendModification(const FTransactionModRequest& Req)
{
	//	唯一收口（卷 04 §4.1）：窗口内的修改只在查询期被收——
	//	这是 State.Responding 锁之外、交易侧的第二道闸门，锁外/窗外的提交一律静默拒绝
	if (State != EState::Querying) return;

	ModeRequests.Add(Req);

	//	C 面事件化广播 FTransactionModified（卷 12 §5.2，广播点 = 本函数）：仅日志/表现/回放观察位，
	//	不参与折叠——折叠由族 FoldMods 在 Execute 开头一次性收敛，与 RPC 到达序无关（卷 10 §2）
	if (auto System = UBattleFunctionLibrary::GetBattleManager(this))
	{
		System->OnTransactionModified.Broadcast(this, Req);
	}
}

void UBattleTransaction::Finish(bool bSuccess)
{
	State = EState::Finished;
	auto System = UBattleFunctionLibrary::GetBattleManager(this);
	
	if (!System) return;
	
	BroadcastFinish(bSuccess);
	System->OnTransactionFinished(this);
}

void UBattleTransaction::QueryTimings()
{
	//	开窗链入口：先落 Querying——这是 AppendModification 的状态闸门（窗内收、窗外拒）
	State = EState::Querying;

	const TArray<FGameplayTag> Timings = GetQueryTimingTags();
	auto System = UBattleFunctionLibrary::GetBattleManager(this);

	//	直通族（无登记的查询时机）：不开窗直接 Execute——行为与 P2 完全一致，零延迟路径
	if (Timings.IsEmpty() || !System)
	{
		Execute();
		return;
	}

	//	经唯一开窗入口挂窗（Tx = this 即窗口负载，响应者从负载拿回交易引用追加修改）。
	//	多时机登记时逐个开（P3 §3.4）：本函数开首窗，后续每扇由 OnWindowClosed 关窗后推游标再开——
	//	窗口悬停期忙锁照旧、队列不重入；开窗即返，本函数不等待。
	//	（注意：逐个开多时机 ≠ 响应链——后者是"响应可被再响应"的递归窗栈，仍留 P7，卷 10 §3）
	//	InOrder 为 P3 §3.2 策略最小集的显式硬编码（子系统当前也只实现这一种走法）；
	//	P7 落 All/FirstOnly 时改为族登记策略对（GetQueryTimingTags → FQueryTiming{Tag, Policy}），此处随族数据传入
	QueryTimingCursor = 0;
	System->OpenTimingWindow(Timings[0], this, EWindowPolicy::InOrder);
}

void UBattleTransaction::OnWindowClosed()
{
	//	关窗回调（C 面钩子，唯一调用方 = UBattleGameFlowSubsystem::CloseTimingWindow，friend 准入）：
	//	本扇窗的响应修改已逐条收入 ModeRequests。状态闸门兼幂等保护：
	//	只有仍在查询期的交易接受关窗回调，防重复收口重入 Execute
	if (State != EState::Querying) return;

	//	逐个开多查询时机（P3 §3.4）：清单还有没问完的 → 开下一扇窗继续悬停（忙锁不破）。
	//	每扇窗收的修改都落进同一个 ModeRequests，最终由族 FoldMods 在 Execute 开头一次性折叠，
	//	先到窗与后到窗的修改互不覆盖、收敛可复现（卷 10 §2）。
	//	窗口侧"先复位、后回调"（CloseTimingWindow 的顺序约定），此处开新窗是合法的
	const TArray<FGameplayTag> Timings = GetQueryTimingTags();
	auto System = UBattleFunctionLibrary::GetBattleManager(this);
	if (System && Timings.IsValidIndex(++QueryTimingCursor))
	{
		System->OpenTimingWindow(Timings[QueryTimingCursor], this, EWindowPolicy::InOrder);
		return;
	}

	//	全部时机问完（或子系统缺席）→ 交族裁决：Execute 开头 FoldMods() 一次性折叠修正
	//	（Ignore 短路即"被防"）→ 族自收尾 Finish(true/false)；基类不懂具体失效语义
	Execute();
}