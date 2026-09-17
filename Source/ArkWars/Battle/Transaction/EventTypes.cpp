#include "EventTypes.h"
#include "BattleTransaction.h"

FClientResponseWindow::FClientResponseWindow(const FResponseWindow& Window, const FResponseEntry& Entry)
	: WindowSerial(Window.WindowSerial)
	, Timing(Window.Timing)
	, Timeout(Window.Timeout)
	, bHasTransaction(Window.Tx.IsValid())
{
	//	只下发【本次询问的那一条】——InOrder 逐条问，游标指向哪条就是哪条：
	//	· 与上游取值对齐：SubmitWindowResponse 以 Responders[ResponderCursor].Key 为准（零信任，不采信
	//	  Req.Skill），此处下发的正是同一条，"客户端挑的"与"服务器办的"才是同一件事。
	//	  若改为下发本人的全部候选，玩家选第 k 条而服务器按游标那条办，选择会被静默换掉——
	//	  更糟的是同一人被问多次时，每次下发的面板一模一样，玩家无从分辨是哪一次；
	//	· 不泄露：Responders 是全窗监听者快照（含其他玩家的 Key 与 Priority），全量下发等于摊开对手底牌。
	//	  将来若要一次给全（All 策略 / 按人聚合），服务器取值也得同步改成认 Req.Skill，
	//	  且须先校验所选 Key 属于该 Owner 的合法集合——那时再一并放开
	Candidates.AddTag(Entry.Key);
}
