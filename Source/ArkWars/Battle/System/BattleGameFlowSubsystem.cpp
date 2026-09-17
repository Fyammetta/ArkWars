// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleGameFlowSubsystem.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "SkillManagerSubsystem.h"
#include "ArkWars/ArkWars.h"
#include "ArkWars/Battle/ArkWarBattleSettings.h"
#include "ArkWars/Battle/Component/Card/CardManagementBusComponent.h"
#include "ArkWars/Battle/Component/GameFlow/GamePhaseManagerComponent.h"
#include "ArkWars/Battle/Component/GameMode/GameModeComponentBase.h"
#include "ArkWars/Battle/Component/Skill/SkillComponentBase.h"
#include "ArkWars/Battle/Core/BattlePlayerController.h"
#include "ArkWars/Battle/Toolkits/ArkWarFlowTypes.h"
#include "ArkWars/Battle/Toolkits/BattleFunctionLibrary.h"
#include "ArkWars/Battle/Toolkits/GameMessage.h"
#include "ArkWars/Battle/Transaction/EventTags.h"
#include "ArkWars/Battle/Transaction/SubTransaction/CardMoveTransaction.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

bool UBattleGameFlowSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (const UWorld* World = Cast<UWorld>(Outer))
	{
		if (const FWorldContext* Ctx = GEngine->GetWorldContextFromWorld(World))
		{
			if (World->GetNetMode() != NM_Standalone)
			{
				return true; 
			}
		}
	}
	return false; 
}

void UBattleGameFlowSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	CardManager = nullptr;
}

void UBattleGameFlowSubsystem::Deinitialize()
{
	Super::Deinitialize();
	
	CardManager = nullptr;
}

int32 UBattleGameFlowSubsystem::GetPlayerIndex(APlayerState* Player) const
{
	if (auto Comp = GetPhaseManager())
	{
		return Comp->GetPlayerIndex(Player);
	}
	
	return INDEX_NONE;
}

void UBattleGameFlowSubsystem::RegisterManagerActor(ICardContainerInterface* Mgr)
{
	if (Mgr && !CardManager)
		CardManager = Mgr->_getUObject();
}

ICardContainerInterface* UBattleGameFlowSubsystem::GetCardManager() const
{
	return CardManager ?  CardManager.GetInterface() : nullptr;
}

bool UBattleGameFlowSubsystem::IsRunningOnServer() const
{
	return GetWorld() ? GetWorld()->GetNetMode() < NM_Client : false;
}

void UBattleGameFlowSubsystem::PushPhaseResolvation(const FString& Msg)
{	
	PhaseMsgQueue.Add(Msg);
	UE_LOG(LogGamePlay, Log, TEXT("[GameFlow][PushPhaseResolvation] Enqueue a new phase message"))

}

UGamePhaseManagerComponent* UBattleGameFlowSubsystem::GetPhaseManager() const
{
	const AGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState() : nullptr;
	return GS ? GS->FindComponentByClass<UGamePhaseManagerComponent>() : nullptr;
}

namespace
{
	EGamePhase NextInSequence(EGamePhase Current)
	{
		return static_cast<EGamePhase>((static_cast<int32>(Current) + 1) % (GamePhase::PhaseCount + 1));
	}

	// 大阶段的门口：Begin、各大阶段的 Pre，以及 GameStart 轮转中转
	bool IsBigPhaseStart(EGamePhase Phase)
	{
		switch (Phase)
		{
		case EGamePhase::GameStart:
		case EGamePhase::Begin:
		case EGamePhase::Preparation_Pre:
		case EGamePhase::Judgment_Pre:
		case EGamePhase::Draw_Pre:
		case EGamePhase::Action_Pre:
		case EGamePhase::Discard_Pre:
			return true;
		default:
			return false;
		}
	}
	
	bool IsPreOrPost(EGamePhase Phase)
	{
		switch (Phase)
		{
		case EGamePhase::GameStart:
		case EGamePhase::Begin:
		case EGamePhase::Preparation:
		case EGamePhase::Judgment:
		case EGamePhase::Draw:
		case EGamePhase::Action:
		case EGamePhase::Discard:
		case EGamePhase::Finish:
			return false;
		default:
			return true;
		}
	}
}

EGamePhase UBattleGameFlowSubsystem::ResolveNextPhase(EGamePhase Current)
{
	using EOP = GameMessage::FPhaseMessage::EOperation;
	int32 Index = 0;
	while (PhaseMsgQueue.IsValidIndex(Index))
	{
		if (!PhaseMsgQueue[Index].CanExecute(Current, GetPlayerIndex()))
		{
			Index++;
			continue;
		}
		UE_LOG(LogGamePlay, Log, TEXT("[GameFlow][ResolveNextPhase] Found available Message"))

		const GameMessage::FPhaseMessage Mod = PhaseMsgQueue[Index];
		PhaseMsgQueue.RemoveAt(Index);

		switch (Mod._Op)
		{
		case EOP::Extra:
			{
				// 额外阶段先行；另插入一枚同玩家的 Jump 回归原本序列，
				// 插入位置为当前 Extra 的后一位（Extra 已出队，即队首）。
				// 回归跳跃不继承阶段限制，只继承玩家限制。
				GameMessage::FPhaseMessage Resume = Mod;
				Resume._Op = EOP::Jump;
				Resume._Target = NextInSequence(Current);
				Resume._Phase = GameMessage::FPhaseMessage::AnyPhase;
				PhaseMsgQueue.Insert(Resume, 0);
				return Mod._Target;
			}
		case EOP::Jump:		return Mod._Target;							// 直接跳至目标阶段
		case EOP::Skip:
			// 跳过大阶段：整组吞掉（Pre/主阶段/Post，或 Begin/Finish 独身阶段），
			// 直到下一个大阶段的门口为止
			Current = NextInSequence(Current);
			while (!IsBigPhaseStart(NextInSequence(Current)))
			{
				Current = NextInSequence(Current);
			}
			// 落入 default：返回下一个大阶段的首步
		default:			return NextInSequence(Current);
		}
	}
	return NextInSequence(Current);
}

void UBattleGameFlowSubsystem::Advance(EGamePhase Phase, int32 Index) const
{
	if (!IsRunningOnServer())
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][Advance] Game Phase should only be advanced on server"))
		return;
	};
	
	auto Comp = GetPhaseManager();
	
	if (!Comp)
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][Advance] Phase manager was lost, failed to advance"))
		return;
	};
	if (Index != INDEX_NONE)
		Comp->SetNextPlayerActive(Index);
	Comp->SetPhase(Phase);
}

void UBattleGameFlowSubsystem::Advance()
{
	if (!IsRunningOnServer())
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][Advance] Game Phase should only be advanced on server"))
		return;
	};
	
	auto Comp = GetPhaseManager();
	
	if (!Comp) return;
	auto Next = ResolveNextPhase(Comp->GetPhase());
	while (true)
	{
		Comp->SetPhase(Next);
		
		//	停窗范围（卷 03 §8 / P3 §3.4①）：_Pre/_Post 全体 + Begin/Finish 两个主阶段——
		//	后者是原方案的加宽（非笔误）；两族 tag 齐备、链路自洽：关窗后由 CloseTimingWindow 的
		//	纯阶段分支再 Advance()，ResolveNextPhase 先消费 PhaseMsgQueue 再给下一站，同一个点不二次停窗
		//	（无死循环）。索引判空即直通（"无监听直通"优先于"整点全开"，P3 §7）
		if (Next == EGamePhase::Begin || Next == EGamePhase::Finish || IsPreOrPost(Next))
		{
			auto Timing = Timing::Phase::PhaseEnumToTimingTag(Next);
			auto SkillMgr = UBattleFunctionLibrary::GetSkillManager(this);
			if (SkillMgr && !SkillMgr->GetSkillListeners(Timing).IsEmpty())
			{
				OpenTimingWindow(Timing,nullptr,EWindowPolicy::InOrder);
				return;
			}
			Next = ResolveNextPhase(Next);
			continue;
		}
		break;
	}
	
}

void UBattleGameFlowSubsystem::StartGame()
{
	if (!IsRunningOnServer())
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][StartGame] Game should NEVER be start by client"))
		return;
	};
	
	auto ModeComp = UGameModeComponentBase::GetCurrentGameMode(this);
	auto PhaseComp = GetPhaseManager();
	auto Manager = GetCardManager();
	
	if (!Manager || !PhaseComp || !ModeComp)
	{
		UE_LOG(LogGamePlay, Warning, 
			TEXT("[GameFlow][StartGame] Sub managers were lost, fail to start game.	\nDetails: <Mode:%hs>\t<Phase:%hs>\t<Card:%hs>"),
			(ModeComp ? "true" : "false"), (PhaseComp ? "true" : "false"), (Manager ? "true" : "false"));
		return;
	};

	//	一局一句柄空间：本局从 #1 起（卷 11 §5.2 防陈旧/重放）。
	//	归零前提 = 此刻无窗口在场：若上一局的窗还悬着，归零会让那个句柄被复用、把陈旧消息请回来——
	//	那条路径属异常（一局未收干净就开新局），报警以便暴露，但仍归零：不留半个跨局句柄空间
	if (ActiveWindow.IsSet())
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][StartGame] Window %d is still active at game start, serial space reset anyway"),
			ActiveWindow->WindowSerial)
	}
	WindowSerialCounter = 0;

	//分配身份
	int32 Start = ModeComp->AllocateIdentity();
	PhaseComp->InitPlayers(Start);
	UE_LOG(LogGamePlay, Log, TEXT("[GameFlow][StartGame] Identity allocated completely"))
	
	//通知选角
	ModeComp->SentSelectOperatorNotify();
	
	//根据角色选择结果初始化牌堆
	ModeComp->InitCardDeck();
	
	//分发起始手牌
	for (auto i = 0; i < PhaseComp->GetPlayerCount() ; ++i)
	{
		auto Player = PhaseComp->GetPlayerByIndex(i);
		using MSG = GameMessage::FMoveMessage;
		auto Num = ModeComp->GetStartCardNum(Player->Implements<UAbilitySystemInterface>() ? Cast<IAbilitySystemInterface>(Player)->GetAbilitySystemComponent() : nullptr);
	
		auto Transaction = UCardMoveTransaction::Create(FString::Printf(TEXT("%s=%d %s=%s %s=%s"), MSG::Num,Num, MSG::From,MSG::Pile,MSG::To,MSG::Hand));
		Transaction->Instigator = Manager->GetContainerActor();
		Transaction->Targets.Add(Player);
		Transaction->Start();
		
		UE_LOG(LogGamePlay, Log, TEXT("[GameFlow][StartGame] Initial hand card allocated to %s"), *Player->GetPlayerNameCustom())
	}

	//广播游戏开始阶段通知
	PhaseComp->OnCalledStartGame();
	UE_LOG(LogGamePlay, Log, TEXT("[GameFlow][StartGame] Broadcast start game phase notify"))

}

void UBattleGameFlowSubsystem::EnqueueTransaction(UBattleTransaction* Tx)
{
	UAbilitySystemComponent* Asc = nullptr;
	if (!IsRunningOnServer())
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][EnqueueTransaction] Transaction should only be added on server"))
		return;
	};
	
	if (!Tx) return;
	
	PendingTransactions.Add(Tx);

	//	入队观察点（卷 12 §5.2）：本笔已进队列，尚未决定是当场驱动还是等忙锁释放——广播点在忙锁分支
	//	【之前】，故两种情形订阅者都收得到。旁路纪律（§7）：回调抛错/耗时不得影响入队与驱动
	OnTransactionEnqueued.Broadcast(Tx);

	if (ActiveTransaction.Get() != nullptr) return;
	
	DriveTransaction();
}

void UBattleGameFlowSubsystem::EnqueueNested(UBattleTransaction* Child)
{
	//	门控与自守：PushNestedTransaction 已验过一遍，此处再验是为本函数作为独立公开入口的完整性
	//	（将来若新增调用点，不必回头补守卫）
	if (!IsRunningOnServer())
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][EnqueueNested] Nested transaction should only be started on server"))
		return;
	};
	if (!Child)
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][EnqueueNested] Null child transaction rejected"))
		return;
	};

	//	与 EnqueueTransaction 同口径的入队观察点：两条路径都是"一笔交易进入结算流程"，订阅者不该按路径分家
	//	（P3 §3.9 决策定稿为 EnqueueNested 而非 Start(bNested)，正是为了让这条观察点有落脚处）
	OnTransactionEnqueued.Broadcast(Child);

	//	不入 PendingTransactions：子交易的驱动权在换手那一刻已定，排队会与父的恢复撞车（见 .h 说明）
	Child->Drive();
}

void UBattleGameFlowSubsystem::DriveTransaction()
{
	if (!IsRunningOnServer())
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][DriveTransaction] Process should only run on server"))
		return;
	};
	
	if (PendingTransactions.IsEmpty())
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][DriveTransaction] No transaction is pending process"))
		return;
	};
	
	ActiveTransaction = PendingTransactions[0];
	PendingTransactions.RemoveAt(0);
	ActiveTransaction->Drive();
}

void UBattleGameFlowSubsystem::OnTransactionFinished(UBattleTransaction* Tx, bool bSuccess)
{
	if (!IsRunningOnServer())
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][OnTransactionFinished] Transaction finish notify should only be broadcast on server"))
		return;
	};

	//	完成观察点（卷 12 §5.2）：本笔的结局——子交易与顶层交易都算"完成"，故广播放在换手/收尾两条路径
	//	【之前】，订阅者不会漏掉栈里那些。旁路纪律（§7）：回调抛错/耗时不得影响下面的换手与队列推进
	OnTransactionFinishedEvent.Broadcast(Tx, bSuccess);

	//	嵌套分支（卷 04 §3.2 不变量 1/5）：先问栈——栈顶 Child == Tx 即"子交易跑完"，只换手：
	//	弹栈 + ActiveTransaction 归父，绝不可清空忙锁去拉下一笔（那会把挂起的父遗弃在栈里）。
	//	换手成立的前提是栈帧两字段同为强引用（否则换手瞬间父/子无锚，GC 一到即悬空）
	if (!NestedStack.IsEmpty() && NestedStack.Last().Child.Get() == Tx)
	{
		//	嵌套观察点（卷 12 §5.2"嵌套交易完成前"）：换手发生前广播，此刻父仍在栈顶可读——
		//	与 PushNestedTransaction 里那一次配对，订阅者因此能完整看到"子进子出"两个端点
		OnTransactionNested.Broadcast(Tx, NestedStack.Last().Parent.Get());
		PopNestedTransaction(Tx);
		return;
	};

	//	栈空（或本笔不在栈顶）→ 整条链收尾：校验当前结算者后清空忙锁 + 驱动下一笔。
	//	挂在栈里的父若莫名走到这里会被下面的守卫拦下（忙锁此刻指向栈顶子，不等于父）
	if (ActiveTransaction.Get() != Tx)
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][OnTransactionFinished] Transaction to remove is not active"))
		return;
	}

	ActiveTransaction = nullptr;
	DriveTransaction();
}

void UBattleGameFlowSubsystem::OpenTimingWindow(const FGameplayTag& EventTag, UBattleTransaction* Tx, EWindowPolicy Policy)
{
	//	⓪ 门控：开窗/收响应仅服务器（P3 §3.5，与 Enqueue/Drive 同纪律）
	if (!IsRunningOnServer())
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][OpenTimingWindow] Window should only be opened on server"))
		return;
	};

	//	单层窗口保护：P3 不做响应链嵌套（留 P7，卷 10 §3）。既有窗口悬停期间再开窗 = 非法调用，直接拒绝——
	//	交易侧忙锁（开窗即返、不清 ActiveTransaction）已保证正常链路不会走到这里，触发即 bug，留告警
	if (ActiveWindow.IsSet())
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][OpenTimingWindow] A window is already open, new window rejected"))
		return;
	}

	//	响应锁前置检查（卷 10 §4）：响应中不受理任何新窗口 = "窗口期无关输入被拒"在开窗侧的落点。
	//	与单层保护不合并：两者拒的是不同的账（前者 = 窗口已悬停，后者 = 锁被持有/泄漏），告警需可分辨。
	//	此处只查不置位——锁在空名单直通之后才正式持有（绝大多数移动窗走零延迟路径，不上锁）
	if (IsResponding())
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][OpenTimingWindow] Response lock is held, new window rejected"))
		return;
	}

	//	①②③ 查时机索引 → 合法预检 → 排序填 Responders 名单（卷 12 §4.1，实现见 CollectResponders）
	FResponseWindow Window;
	Window.WindowSerial = ++WindowSerialCounter;					//	句柄发号：本窗唯一；随候选下发、随提交/放弃回传比对（防陈旧/重放）
	Window.Timing = EventTag;
	Window.Tx = TStrongObjectPtr(Tx);								//	TStrongObjectPtr 强持有：防窗口悬停期交易被 GC；纯阶段时机为空
	Window.Responders = CollectResponders(EventTag);
	Window.Timeout = UArkWarBattleSettings::Get()->DefaultWindowTimeout;	//	时间预算从项目设置（Ark War Battle 页）取：关闭时机统一由 UI 决定，倒计时到点客户端直接请求关闭（= 放弃），引擎侧无定时器

	ActiveWindow = MoveTemp(Window);
	ActivePolicy = Policy;											//	策略是开窗参数、不进窗口形状（卷 10 §2），单独存
	ResponderCursor = 0;
	bAnyResponded = false;
	//	账本随名单同生（与 Responders 等长，全 false = "尚无一条在途"）：置位只发生在真正把候选
	//	发出去的那一处（AskNextResponder）——上面两条"视同放弃"的失联路径不置位，没问出去的不必等回传
	ResponsePending.Init(false, ActiveWindow->Responders.Num());

	UE_LOG(LogGamePlay, Log, TEXT("[GameFlow][OpenTimingWindow] Window opened on timing %s with %d responder(s)"),
		*EventTag.ToString(), ActiveWindow->Responders.Num())

	//	④ 广播 FWindowOpened——名单填出后、开始询问前（卷 12 §5.2）。
	//	旁路纪律（§7）：广播是观察点，推进不依赖任何订阅者的返回值，回调抛错/耗时不得影响窗口
	OnWindowOpened.Broadcast(ActiveWindow.GetValue());

	//	无监听直通（P3 §3.5/§7）：空名单（绝大多数移动窗）→ 立即关窗，零延迟路径，不上锁
	if (ActiveWindow->Responders.IsEmpty())
	{
		CloseTimingWindow();
		return;
	}

	//	⑤ State.Responding 上锁（阻断无关输入，关窗撤锁）→ InOrder 从游标 0 开始逐人询问。
	//	此后函数返回，推进完全由响应提交（SubmitWindowResponse/DeclineWindowResponse）事件驱动
	//	失败理论不可达（前置检查已查过锁）；真发生（订阅者在 FWindowOpened 回调里抢锁）= 窗口已开，
	//	必须配对关窗不留悬挂——此时无人响应过，走关窗收口等价于该窗从未开启
	if (!TryEnterResponseLock())
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][OpenTimingWindow] Response lock was taken during open, window aborted"))
		CloseTimingWindow();
		return;
	}
	AskNextResponder();
}

TArray<FResponseEntry> UBattleGameFlowSubsystem::CollectResponders(const FGameplayTag& TimingTag) const
{
	if (!IsRunningOnServer())
	{
		return {};
	}

	//	① 查时机索引（卷 12 §4.1，P3 §3.3）：USkillManagerSubsystem 的"时机 → 监听技能"表；
	//	索引为空表时按值返回空快照 → 窗口侧走"无监听直通"零延迟路径（P3 §7）
	auto SkillMgr = UBattleFunctionLibrary::GetSkillManager(this);
	if (!SkillMgr)
	{
		return {};
	}

	//	座次轮转依赖阶段管理器：座次信息缺失时名单无从排序，与索引缺失同待遇（宁空勿乱）
	auto PhaseMgr = GetPhaseManager();
	if (!PhaseMgr)
	{
		return {};
	}

	const TArray<FSkillListenerEntry> Listeners = SkillMgr->GetSkillListeners(TimingTag);

	//	② 合法预检：来源在场/可用（IsValid = Skill 与 Owner 双双存活）；
	//	账本余量 P6 前常量放行（卷 07 占位）。逐条转换 FSkillListenerEntry → FResponseEntry
	TArray<FResponseEntry> RetArr;
	RetArr.Reserve(Listeners.Num());
	for (const FSkillListenerEntry& Listener : Listeners)
	{
		if (!Listener.IsValid())
		{
			continue;
		}

		FResponseEntry Entry;
		Entry.Priority = Listener.Priority;
		Entry.Owner = Listener.Owner;
		Entry.Key = Listener.Skill->GetSkillTag();
		RetArr.Add(MoveTemp(Entry));
	}

	//	③ 排序 = Priority 降序 × 座次（从当前回合玩家起轮转，卷 10 §2），平手取注册稳定序。
	//	必须 StableSort：Sort 是 introsort 不稳定，会吃掉注册序承诺；
	//	比较器严守严格弱序（相等返回 false），非法比较器会让 introsort 越界
	const int32 CurPlayer = PhaseMgr->GetPlayerIndex();
	const int32 PlayerCount = PhaseMgr->GetPlayerCount();
	auto SeatOf = [PhaseMgr, CurPlayer, PlayerCount](const FResponseEntry& Entry)
	{
		const int32 Index = PhaseMgr->GetPlayerIndex(Entry.Owner.Get());
		//	不在座次表（INDEX_NONE）或座次表未就绪 → 沉底
		if (PlayerCount <= 0 || Index < 0 || Index >= PlayerCount)
		{
			return PlayerCount;
		}
		//	轮转为"从当前玩家起"的相对座次：差值恒在 (-PlayerCount, PlayerCount)，单次加模即非负
		return (Index - CurPlayer + PlayerCount) % PlayerCount;
	};

	RetArr.StableSort([&SeatOf](const FResponseEntry& A, const FResponseEntry& B)
	{
		if (A.Priority != B.Priority)
		{
			return A.Priority > B.Priority;		//	优先级降序
		}
		return SeatOf(A) < SeatOf(B);			//	座次升序，严格弱序禁用 <=
	});

	return RetArr;
}

int32 UBattleGameFlowSubsystem::ResolvePendingIndex(APlayerState* Responder, const FGameplayTag& SkillTag) const
{
	if (!ActiveWindow.IsSet() || !Responder)
	{
		return INDEX_NONE;
	}

	//	InOrder：一次只问一人——权在游标处那一条。条目由服务器自己的账推得（游标 + 名单），
	//	这就是"用谁响应由名单裁决"的实现（卷 11 §5.2）：自报的 Key 不作授权，只参与"在本人名下
	//	的待回条目里挑一条"这个动作——此处候选只有一个（游标即答案），故 Key 只作相符性校验
	if (ActivePolicy == EWindowPolicy::InOrder)
	{
		if (!ActiveWindow->Responders.IsValidIndex(ResponderCursor)
			|| ActiveWindow->Responders[ResponderCursor].Owner.Get() != Responder)
		{
			return INDEX_NONE;
		}

		//	游标处的 Key 也对上（与同开类同一判法，只是判据来源不同——那边靠"找"，这边游标即答案）：
		//	CollectResponders 不按 Owner 去重，同一人可连占两条；只认人的话，本窗内一次陈旧重放
		//	（他第一条技能的提交）会顶替第二条的响应权——销掉第二条的账、把第一条的修改记在它名下。
		//	正常 UI 路径不可达（面板与提交一一对应），只有重放才串扰。空 Tag = 放弃入口的按人定位，跳过；
		//	"同一条被重复提交"不在此列（要两条 (Owner, Key) 全同的条目才可能，另论）
		if (SkillTag.IsValid() && ActiveWindow->Responders[ResponderCursor].Key != SkillTag)
		{
			return INDEX_NONE;
		}

		return ResponderCursor;
	}

	//	All/FirstOnly：一次问多人——由"提交者 + 所选技能"两步定位，三条件缺一不可：
	//	· 在【待回位】上：已有结局的条目不可再销账，否则同一 Key 重复提交会反复销账，把"账平"
	//	  判成假、窗口提前关闭（同开类下，这一判同时就是"重复提交"的唯一拦点）；
	//	· 属于该 Owner：Key 是客户端自报的，不校验归属则同一扇窗里的任何人都能拿别人的 Key 销自己的账；
	//	· Key 命中（空 Tag 时跳过）：见下。
	//	不采信自报字段的纪律不变——自报的 Key 只用来"在本人名下的待回条目里挑一条"，挑不出就是无权
	const TArray<FResponseEntry>& Responders = ActiveWindow->Responders;
	for (int32 Index = 0; Index < Responders.Num(); ++Index)
	{
		if (!ResponsePending.IsValidIndex(Index) || !ResponsePending[Index])
		{
			continue;
		}
		if (Responders[Index].Owner.Get() != Responder)
		{
			continue;
		}
		//	空 Tag = 放弃入口的按人定位：取本人名下第一条待回即可，调用方据此清该 Owner 的全部条目
		if (SkillTag.IsValid() && Responders[Index].Key != SkillTag)
		{
			continue;
		}
		return Index;
	}

	return INDEX_NONE;
}

void UBattleGameFlowSubsystem::AskNextResponder()
{
	if (!ActiveWindow.IsSet()) return;

	
	
	const TArray<FResponseEntry>& Responders = ActiveWindow->Responders;
	while (Responders.IsValidIndex(ResponderCursor))
	{
		const FResponseEntry& Entry = Responders[ResponderCursor];

		//	Owner 弱指针先验活：收集与询问之间隔着事件时间，悬停期离场者不可裸解引用（卷 10 §3）
		APlayerState* Owner = Entry.Owner.Get();
		UActorComponent* Comp = Owner
			? Owner->FindComponentByTag(USkillComponentBase::StaticClass(), Entry.Key.GetTagName())
			: nullptr;

		if (auto Skill = Cast<USkillComponentBase>(Comp))
		{
			//	发问：客户端只收"你有响应权 + 候选"（P3 只定下发协议，UI 是 P7）。引擎在此返回、
			//	不轮询不阻塞：后续推进完全由该玩家经 ServerRPC 提交/放弃驱动。
			//	三策略的差异只在【问几个人】与【何时收敛】（卷 10 §2）：InOrder 问一个就停、
			//	回传才推游标；All/FirstOnly 一次问全（文档：两者"无序——现实层面同时发生"），
			//	收敛靠下面的账本——All 等账平，FirstOnly 首响即关。发问侧则三策略同形

			//	【本层窗口身份 · 快照】N7 重入守卫的上半：钩子可能同步推进行为（旧口径曾认其合法）
			//	→ 递归 → 关窗、甚至立刻开新窗。而上面 :590 的 Responders 与 :593 的 Entry 都是
			//	【引用】，绑在 ActiveWindow 内部的数组上——窗口一换即悬空。故快照必须取在钩子之前
			const int32 LayerSerial = ActiveWindow->WindowSerial;

			UE_LOG(LogGamePlay, Log, TEXT("[GameFlow][AskNextResponder] Asking responder %d/%d (Key:%s)"),
				ResponderCursor + 1, Responders.Num(), *Entry.Key.ToString())

			//	钩子契约（卷 09 §10）：本钩子只做【就位】——接住接缝（Timing / Tx）、自判要不要响、
			//	把准备状态缓存好等 OnRespond 来取。【不得在此提交/放弃】：那是回传侧动作，在此调用会把
			//	发问与回传两个阶段叠进同一帧，并抢在客户端收到面板之前落地生效（纯自动技能应走名单层
			//	"不进询问通道"，不是进名单后靠钩子提交模拟自动——卷 09 §8 契约行同款分工）
			Skill->OnCanActivate(ActiveWindow->Timing, ActiveWindow->Tx.IsValid() ? ActiveWindow->Tx.Get() : nullptr);

			//	【重入守卫】N7 下半：钩子返回后，本层服务的窗口可能已被换掉——两判缺一不可：
			//	· 无窗（!IsSet）——内层推进关了窗且未开新窗，本层余句已无对象（GetValue 撞 check）；
			//	· 句柄不同——内层开了新窗，继续执行会【错位下发】：拿新窗的句柄/时机配旧窗的 Entry，
			//	  客户端据此提交、服务端三道校验全过（句柄确实是新窗的），而服务端认的"被问条目"
			//	  是新窗的另一条——两端对不上且日志上无从分辨；同开类的 continue 还会回边读悬空的
			//	  Responders。命中即【整层作废】：本层曾持有的账已由内层递归结清，这里只须 return。
			//	本条告警刻意不重复打 Key——它上面紧挨着的那句 Asking 日志就是本次被问的技能
			if (!ActiveWindow.IsSet() || ActiveWindow->WindowSerial != LayerSerial)
			{
				UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][AskNextResponder] Window replaced during OnCanActivate (layer serial:%d, cursor:%d), this pass aborted"),
					LayerSerial, ResponderCursor)
				return;
			}
			//	P3 §3.3 候选下发（表现复制通道，卷 11）：把"轮到你了 + 本次问的那一条"发给该 Owner 的 PC。
			//	PC 是纯落点——UI 展开与倒计时由订阅者自理（OnWindowResponseRequested），它不认识窗口
			auto PC = Cast<ABattlePlayerController>(Owner->GetPlayerController());
			if (!PC)
			{
				//	PC 失联（掉线 / 无主 PlayerState）：视同放弃推进下一位。
				//	此处直接 return 会把窗口悬在半空——锁不撤、游标不动、无人推进，违反"任何路径不悬挂"（P3 §3.2）；
				//	与下方技能组件失联同一处置
				UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][AskNextResponder] Responder %d/%d has no player controller (Key:%s), treated as decline"),
					ResponderCursor + 1, Responders.Num(), *Entry.Key.ToString())
				ResponderCursor++;
				continue;
			}
			switch (ActivePolicy)
			{
			case EWindowPolicy::InOrder:
				//	逐人问：问一个就停，等他的回传驱动游标（现阶段唯一有真实推演的策略）
				ResponsePending[ResponderCursor] = true;	//	记账：这一条已发问、尚无结局
				PC->Client_OpenResponseWindow(FClientResponseWindow(ActiveWindow.GetValue(), Entry));
				return;		//	询问已送达；推进交由回传驱动——钩子里的同步提交已被上面的重入守卫挡住，本层不再碰窗口
			case EWindowPolicy::All:
			case EWindowPolicy::FirstOnly:
				//	同开类：一次问全，游标只作发问进度、此后不再参与推进。
				//	两者发问侧同形（文档：All/FirstOnly 无序），差异全在回传后的收敛条件——
				//	All 等账平、FirstOnly 首响即关，见 SubmitWindowResponse/DeclineWindowResponse
				ResponsePending[ResponderCursor] = true;	//	记账：这一条已发问、尚无结局
				PC->Client_OpenResponseWindow(FClientResponseWindow(ActiveWindow.GetValue(), Entry));
				ResponderCursor++;
				continue;
			}
		}

		//	技能组件失联（已销毁/未挂 Tag）：视同放弃推进下一位，不留悬挂窗口（任何路径不悬挂，P3 §3.2）
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][AskNextResponder] Responder %d/%d unreachable (Key:%s), treated as decline"),
			ResponderCursor + 1, Responders.Num(), *Entry.Key.ToString())
		ResponderCursor++;
	}

	//	发完 ≠ 可以关（N2-b）：同开类走到这里只是"询问全部送达"，账本里还有在途的条目，关窗条件未达成。
	//	真正的条件是【账平】——发出多少份、收回多少份，两边对上才关。InOrder 退出本循环时账本必空
	//	（一次只问一人，且问完即 return，唯回传能推游标再回到这里），故本判断对它零行为变化
	if (ResponsePending.Contains(true))
	{
		return;
	}

	//	全员响应/放弃/失联完毕 → 空结果或已有结果关窗（修改已在提交阶段逐条落入 Tx->ModeRequests）
	CloseTimingWindow();
}

void UBattleGameFlowSubsystem::SubmitWindowResponse(APlayerState* Responder, const FWindowResponseRequest& Req)
{
	if (!IsRunningOnServer())
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][SubmitWindowResponse] Response should only be submitted on server"))
		return;
	}

	//	二次校验（P3 §3.3）：窗口在场 + State.Responding 锁内——锁外的任何提交都是无关输入，拒绝
	if (!ActiveWindow.IsSet() || !IsResponding())
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][SubmitWindowResponse] No active responding window, submission rejected (WindowSerial:%d)"), Req.WindowSerial)
		return;
	}

	//	二次校验：句柄须等于当前窗——陈旧/重放的上行（如上一扇窗迟到的提交）不得作用于本窗（卷 11 §5.2）。
	//	句柄判刻意排在身份判【之前】：先验"这是不是本扇窗"，再验"你是不是本条的正主"——陈旧上行
	//	连参与定位的资格都没有。下发侧已随候选填句柄（FClientResponseWindow 构造的 WindowSerial
	//	即本字段来源），客户端原样回传；对不上即拒，不设"0 就放行"的暗门
	if (Req.WindowSerial != ActiveWindow->WindowSerial)
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][SubmitWindowResponse] Stale WindowSerial:%d (current:%d), submission rejected"),
			Req.WindowSerial, ActiveWindow->WindowSerial)
		return;
	}

	//	二次校验：提交者与所选技能须双双落在【待回位】上——授权粒度随策略（判法见 ResolvePendingIndex）：
	//	InOrder 认游标处那一条（一次只问一人）；All/FirstOnly 认"你在待回位里"（一次问多人，若仍只用
	//	游标判，除最后一人外的提交会被全部误拒，而日志还反着说"你不是被询问者"——归因错到底）
	const int32 PendingIndex = ResolvePendingIndex(Responder, Req.Skill);
	if (PendingIndex == INDEX_NONE)
	{
		//	措辞须同时覆盖两族判据的落空：①你不是被问的那一个；②是被问的人，但自报 Key 在其名下
		//	待回条目里挑不出（陈旧重放 / 客户端没回填 Key）。旧措辞只说了①，在②上就是又一条反着说的日志
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][SubmitWindowResponse] Responder %s has no pending entry for this window (Key:%s), submission rejected"),
			*GetNameSafe(Responder), *Req.Skill.ToString())
		return;
	}

	//	被问的技能 = 窗口点名的条目（定位见 ResolvePendingIndex），不是客户端自报的 Req.Skill——
	//	上行只声明"我答应的是哪一条"，用谁响应仍由名单裁决（卷 11 §5.2）：自报的 Key 仅参与
	//	"从多条待回里挑一条"（InOrder 下以游标处那条为准，Key 须与之相符——同样是"挑"，不是"定"），
	//	且必须命中该 Owner 名下的待回条目。技能组件按 Tag 挂在 PlayerState 上，
	//	故从"提交者 + 被问条目"两步可及
	const FResponseEntry& Entry = ActiveWindow->Responders[PendingIndex];
	UActorComponent* Comp = Responder
		? Responder->FindComponentByTag(USkillComponentBase::StaticClass(), Entry.Key.GetTagName())
		: nullptr;
	auto Skill = Cast<USkillComponentBase>(Comp);

	//	技能失联（已销毁/未挂 Tag）：拒绝提交且不动游标——不替玩家弃权（弃权只认 Decline 入口），
	//	窗口仍由 UI 倒计时收场；此处有 Warning 即为"悬停期技能被销毁"的诊断线索（失效传播属 P7）
	if (!Skill)
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][SubmitWindowResponse] Responder %s has no live skill for Key:%s, submission rejected"),
			*GetNameSafe(Responder), *Entry.Key.ToString())
		return;
	}

	//	校验通过后才把负载交给技能：技能自行产出修改并调 Tx->AppendModification（交易侧唯一收口：
	//	State==Querying 才收，卷 04 §4.1；FTransactionModified 在交易侧广播）。修改不直写结果位、
	//	不覆盖共享值——Execute 前由族 FoldMods 统一折叠。校验在前、副作用在后，顺序即安全
	Skill->OnRespond(ActiveWindow->Tx.IsValid() ? ActiveWindow->Tx.Get() : nullptr, Req);
	//	纯阶段时机（Tx 为空）：技能对阶段的修正（Skip/Extra/Jump）走 PushPhaseResolvation 写入
	//	PhaseMsgQueue，不经 ModRequest 通道——两条回写路径在 CloseTimingWindow 的路由处汇合
	bAnyResponded = true;

	//	销账：本条已有结局（响应），自待回表移除——此后该不该关窗由策略各自的收敛条件判
	ResponsePending[PendingIndex] = false;

	switch (ActivePolicy)
	{
	case EWindowPolicy::InOrder:
		//	逐人问：响应完 → 下一位（名单走完时 AskNextResponder 内部收口关窗）
		ResponderCursor++;
		return AskNextResponder();
	case EWindowPolicy::All:
		//	同开类：不推游标（它在发问阶段已走完，此后只作账），等其余在途者回传——账平才关窗。
		//	此前这里是空块贯穿到 FirstOnly，且"发完即关"使响应窗口期为零：玩家的提交永远撞
		//	"没有活动窗口"。现在是"发出多少份 ↔ 收回多少份，两边对上才关"
		if (ResponsePending.Contains(true)) return;
		return CloseTimingWindow();
	case EWindowPolicy::FirstOnly:
		//	首响抢占（卷 10 §2"谁先响应生效，其余作废"）：有人响应即生效，不必等账平——
		//	在途者的修改本就不该再生效（本窗已被首响者定局）
		return CloseTimingWindow();
	}

}

void UBattleGameFlowSubsystem::DeclineWindowResponse(APlayerState* Responder, int32 WindowSerial)
{
	if (!IsRunningOnServer())
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][DeclineWindowResponse] Response should only be declined on server"))
		return;
	}

	//	与提交同一套校验，顺序也一致：窗口在场 + 锁内 → 句柄 → 身份落在待回位
	if (!ActiveWindow.IsSet() || !IsResponding())
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][DeclineWindowResponse] No active responding window, decline rejected (WindowSerial:%d)"), WindowSerial)
		return;
	}

	//	句柄校验对放弃比提交更要紧：UI 超时是自动发的，上一扇窗迟到的放弃若被放行，会静默吞掉玩家在新窗里的
	//	响应权——其余校验全过、日志不留痕，只在行为上表现为"窗口一闪而过"。此处是唯一的拦截点
	if (WindowSerial != ActiveWindow->WindowSerial)
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][DeclineWindowResponse] Stale WindowSerial:%d (current:%d), decline rejected"),
			WindowSerial, ActiveWindow->WindowSerial)
		return;
	}

	//	身份校验：同开类下"你在待回位里"即有权（与提交同一判法，理由见 ResolvePendingIndex）。
	//	空 Tag = 按人定位——放弃入口没有技能参数，这是【当前形状的缺口】而非疏漏，见下面的销账说明。
	//	空 Tag 同时使 InOrder 那侧的 Key 判跳过：放弃本就无需说出"弃的是哪一条"
	const int32 PendingIndex = ResolvePendingIndex(Responder, FGameplayTag());
	if (PendingIndex == INDEX_NONE)
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][DeclineWindowResponse] Responder %s has no pending entry for this window, decline rejected"),
			*GetNameSafe(Responder))
		return;
	}

	//	放弃 = 不落任何修改，直接推进；全弃 → 收口 → 空结果关窗。
	//	UI 倒计时到点也走本入口（客户端直接请求关闭）——关闭时机统一只通过 UI 决定，引擎侧不挂定时器
	if (ActivePolicy == EWindowPolicy::InOrder)
	{
		//	逐人问：一次只有一条在途，按条即按人
		ResponsePending[PendingIndex] = false;
		ResponderCursor++;
		return AskNextResponder();
	}

	//	同开类的销账：按【人】清——该 Owner 名下的待回条目一起销。
	//	为什么不是按条：放弃入口拿不到"弃的是哪一条"（Server_DeclineWindowResponse 只带句柄），
	//	客户端侧也没有这个信息——PC 的超时句柄是单个成员，同一 PC 收到多条候选时后一扇会覆盖
	//	前一扇的兜底（一 PC 一次只撑得住一扇窗）。在"下发单位"定下来之前，按人是唯一自洽的默认。
	//	TODO(P7 · 下发单位待裁)：若定为按条目（每面板各有放弃钮），本入口须加 SkillTag 参数改按条销账——
	//	那时"弃技能A、仍响应技能B"才成立；若定为按人聚合下发，则本处即最终形态
	for (int32 Index = 0; Index < ResponsePending.Num(); ++Index)
	{
		if (ResponsePending[Index] && ActiveWindow->Responders[Index].Owner.Get() == Responder)
		{
			ResponsePending[Index] = false;
		}
	}

	//	同开类的放弃侧同形：放弃不是"响应"，不构成首响抢占（卷 10 §2 抢的是响应，不是表态），
	//	故其余人仍有响应权——两种策略在这里都等账平。FirstOnly 此前是"一有人放弃就关窗"：
	//	第一个人弃权，剩下的人连看一眼的机会都没有，与文档语义相反
	if (ResponsePending.Contains(true)) return;
	return CloseTimingWindow();
}

bool UBattleGameFlowSubsystem::TryEnterResponseLock()
{
	//	已在响应中：拒绝——锁状态未变，故不重复广播；调用方据点拒绝 + 日志（卷 10 §4）
	if (bRespondingLock)
	{
		return false;
	}
	bRespondingLock = true;

	//	旁路广播（卷 12 §5.2/§7）：锁切换观察点，订阅者回调抛错/耗时不得影响上锁与窗口推进
	UE_LOG(LogGamePlay, Log, TEXT("[GameFlow][ResponseLock] Response lock entered"))
	OnResponseLockChanged.Broadcast(true);
	return true;
}

void UBattleGameFlowSubsystem::ExitResponseLock()
{
	//	幂等：未上锁时空操作（关窗路径可能重复到达，不重复广播）
	if (!bRespondingLock)
	{
		return;
	}
	bRespondingLock = false;

	UE_LOG(LogGamePlay, Log, TEXT("[GameFlow][ResponseLock] Response lock exited"))
	OnResponseLockChanged.Broadcast(false);
}

bool UBattleGameFlowSubsystem::PushNestedTransaction(UBattleTransaction* Child)
{
	//	门控：与 Enqueue/Drive 同纪律，嵌套只在服务器发起（P3 §3.9）
	if (!IsRunningOnServer())
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][PushNestedTransaction] Nested push should only happen on server"))
		return false;
	};
	if (!Child)
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][PushNestedTransaction] Null child transaction rejected"))
		return false;
	};

	//	深度上限（卷 04 §3.2 不变量 2）只作设计层约定，不落运行期校验：栈不设硬闸，深度由调用方账目约束
	//	（P3 §3.9 的"超深拒绝分支"随之取消，2026-09-16 定）

	//	拒绝分支①·非活动上下文（不变量 4）：父 = 当前结算者，就地取——父的身份在 Push 这一刻由子系统
	//	自证，不由调用方传入（传参等于把"只有当前结算者能 Push"降级为调用方的口供）
	if (!ActiveTransaction)
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][PushNestedTransaction] No active transaction, push rejected"))
		return false;
	};

	//	拒绝分支②·防环（不变量 3）：子不得是当前结算者，也不得出现在栈内任一帧的 Parent/Child 链上——
	//	两格都要查：被挂起的父只以 Parent 的形式留在帧里
	if (Child == ActiveTransaction)
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][PushNestedTransaction] Self nested push rejected"))
		return false;
	}

	for (const FNestedTransactionFrame& Frame : NestedStack)
	{
		if (Frame.Parent.Get() == Child || Frame.Child.Get() == Child)
		{
			UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][PushNestedTransaction] Cyclic nested push rejected"))
			return false;
		}
	}
	

	//	挂起父 + 换手：帧的两个强引用是父此后唯一的锚（锚点硬约束，不变量 1）。
	//	先入栈再换手——入栈时忙锁仍指向父，帧记下来的就是父。
	//	父另存一份裸指针供广播：换手之后 ActiveTransaction 已是子，父只存在于栈顶帧里，
	//	而广播要发在【入栈之后】（订阅者从栈顶就能验到"父确实挂起"）
	UBattleTransaction* Parent = ActiveTransaction;
	NestedStack.Push({Parent, Child});
	ActiveTransaction = Child;

	//	嵌套观察点（卷 12 §5.2"被附加嵌套交易后"）：父已入栈挂起、忙锁已换手给子、子尚未启动——
	//	此刻是本笔嵌套唯一可观察的起点（子一起来就可能同步跑完，那时只剩"完成前"那一次）
	OnTransactionNested.Broadcast(Child, Parent);

	//	子交易开自己的窗口；合规插入点上 ActiveWindow 必为空，不撞单层窗口保护（卷 04 §3.2）。
	//	入队走 EnqueueNested 而非直接 Drive：子交易与顶层交易在"进入结算流程"这件事上共用同一个
	//	观察点（P3 §3.9 决策定稿），而它不进 PendingTransactions——排队的账见 .h 说明
	EnqueueNested(Child);
	return true;
}

void UBattleGameFlowSubsystem::PopNestedTransaction(UBattleTransaction* Child)
{
	//	栈顶自校：唯一调用点已判过栈顶，这里再校一遍，防将来新增调用点弹错帧（弹错 = 换手给错父）
	if (NestedStack.IsEmpty() || NestedStack.Last().Child.Get() != Child)
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][PopNestedTransaction] Transaction to pop is not the stack top"))
		return;
	};

	//	按值取出：本地副本即换手落地前父/子的锚，别用引用接（引用会在弹栈后悬空）
	FNestedTransactionFrame Frame = NestedStack.Pop();
	ActiveTransaction = Frame.Parent.Get();		//	换手回父（不重发入队、不重发 Enqueued 事件，不变量 5）

	//	TODO(P5 · 卷 04 §3.2「父的续行点」决策项)：父的续行尚未定稿——子交易若开窗（P5 濒死必开），
	//	父必须走 ResumeFromNested() 显式续行（由本函数调起）；P3 无真实使用者，父停在挂起点
}

void UBattleGameFlowSubsystem::CloseTimingWindow()
{
	//	幂等保护：关窗只发生一次（防御多路径重复收口造成回调重入）
	if (!ActiveWindow.IsSet()) return;

	//	⑥-1 撤 State.Responding 锁（单一收口 ExitResponseLock，广播 FResponseLockChanged(false)）：
	//	窗口期被阻断的无关输入自此恢复
	ExitResponseLock();

	//	⑥-2 先摘出负载、复位窗口状态，再广播/回调——顺序关键：回调里可能立刻开新窗
	//	（阶段机 Advance 续走遇下一个 _Pre/_Post），若不先复位会被 OpenTimingWindow 的"单层窗口保护"拒绝
	FResponseWindow ClosedWindow = MoveTemp(ActiveWindow.GetValue());
	ActiveWindow.Reset();
	ResponderCursor = 0;
	ResponsePending.Empty();		//	账本随窗同灭：必须赶在 ⑥-4 回调之前——回调里可能立刻开新窗，晚一步就把它刚填的账清掉

	UE_LOG(LogGamePlay, Log, TEXT("[GameFlow][CloseTimingWindow] Window on timing %s closed (AnyResponded:%s)"),
		*ClosedWindow.Timing.ToString(), (bAnyResponded ? TEXT("true") : TEXT("false")))

	//	⑥-3 广播 FWindowClosed（卷 12 §5.2）——旁路：订阅者异常不得影响 ⑥-4 的持有者回调
	OnWindowClosed.Broadcast(ClosedWindow, bAnyResponded);
	bAnyResponded = false;

	//	⑥-4 回调持有者路由（P3 §3.2：关窗 = 汇总结果回写 → 回调，任何路径不悬挂）：
	//	交易路径（Tx 非空）→ Tx->OnWindowClosed()：C 面引擎直调钩子（friend 准入），
	//	  族在里面裁决被防/无效 → Finish(false)，否则 → Execute()（开头 FoldMods 一次性消费修改）；
	//	纯阶段路径（Tx 为空）→ 响应结果已由技能写入 PhaseMsgQueue（PushPhaseResolvation），
	//	  这里 Advance() 续走，修正裁决照旧（ResolveNextPhase 消费队列）
	if (UBattleTransaction* Tx = ClosedWindow.Tx.Get())
	{
		Tx->OnWindowClosed();
	}
	else
	{
		Advance();
	}
}
