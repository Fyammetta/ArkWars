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
	if (ActiveTransaction.Get() != nullptr) return;
	
	DriveTransaction();
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

void UBattleGameFlowSubsystem::OnTransactionFinished(UBattleTransaction* Tx)
{
	if (!IsRunningOnServer())
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][OnTransactionFinished] Transaction finish notify should only be broadcast on server"))
		return;
	};
	//	嵌套分支（卷 04 §3.2 不变量 1/5）：先问栈——栈顶 Child == Tx 即"子交易跑完"，只换手：
	//	弹栈 + ActiveTransaction 归父，绝不可清空忙锁去拉下一笔（那会把挂起的父遗弃在栈里）。
	//	换手成立的前提是栈帧两字段同为强引用（否则换手瞬间父/子无锚，GC 一到即悬空）
	if (!NestedStack.IsEmpty() && NestedStack.Last().Child.Get() == Tx)
	{
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
	Window.Timing = EventTag;
	Window.Tx = TStrongObjectPtr(Tx);								//	TStrongObjectPtr 强持有：防窗口悬停期交易被 GC；纯阶段时机为空
	Window.Responders = CollectResponders(EventTag);
	Window.Timeout = UArkWarBattleSettings::Get()->DefaultWindowTimeout;	//	时间预算从项目设置（Ark War Battle 页）取：关闭时机统一由 UI 决定，倒计时到点客户端直接请求关闭（= 放弃），引擎侧无定时器

	ActiveWindow = MoveTemp(Window);
	ActivePolicy = Policy;											//	策略是开窗参数、不进窗口形状（卷 10 §2），单独存
	ResponderCursor = 0;
	bAnyResponded = false;

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
			//	InOrder：按序逐人问——客户端只收"你有响应权 + 候选"（P3 只定下发协议，UI 是 P7）。
			//	引擎在此返回、不轮询不阻塞：下一位由该玩家经 ServerRPC 提交/放弃后驱动游标推进。
			//	（All/FirstOnly 策略的差异只在"问几个人、何时收敛"，推进骨架不变——P3 不实现，卷 10 §2）
			UE_LOG(LogGamePlay, Log, TEXT("[GameFlow][AskNextResponder] Asking responder %d/%d (Key:%s)"),
				ResponderCursor + 1, Responders.Num(), *Entry.Key.ToString())
			//	TODO(P3 §3.3)：候选下发给 Entry.Owner 的客户端（表现复制通道，卷 11）

			Skill->OnCanActivate(ActiveWindow->Timing, ActiveWindow->Tx.IsValid() ? ActiveWindow->Tx.Get() : nullptr);
			switch (ActivePolicy)
			{
			case EWindowPolicy::InOrder: 	
				return;		//	询问已送达；即便技能在钩子里同步提交/放弃（重入推进），本帧也只需返回
			case EWindowPolicy::All:
				ResponderCursor++;
				continue;
			case EWindowPolicy::FirstOnly:
				CloseTimingWindow();
			}
		}

		//	技能组件失联（已销毁/未挂 Tag）：视同放弃推进下一位，不留悬挂窗口（任何路径不悬挂，P3 §3.2）
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][AskNextResponder] Responder %d/%d unreachable (Key:%s), treated as decline"),
			ResponderCursor + 1, Responders.Num(), *Entry.Key.ToString())
		ResponderCursor++;
	}

	//	全员响应/放弃/失联完毕 → 空结果或已有结果关窗（修改已在提交阶段逐条落入 Tx->ModeRequests）
	CloseTimingWindow();
}

void UBattleGameFlowSubsystem::SubmitWindowResponse(APlayerState* Responder, const FTransactionModRequest& Req)
{
	if (!IsRunningOnServer())
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][SubmitWindowResponse] Response should only be submitted on server"))
		return;
	}

	//	二次校验（P3 §3.3）：窗口在场 + State.Responding 锁内——锁外的任何提交都是无关输入，拒绝
	if (!ActiveWindow.IsSet() || !IsResponding())
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][SubmitWindowResponse] No active responding window, submission rejected"))
		return;
	}

	//	二次校验：提交者必须是"当前正被询问者"（InOrder 下只有游标处的人有权提交；名单内但没轮到 = 拒绝）
	if (!ActiveWindow->Responders.IsValidIndex(ResponderCursor)
		|| ActiveWindow->Responders[ResponderCursor].Owner.Get() != Responder)
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][SubmitWindowResponse] Responder is not the one being asked, submission rejected"))
		return;
	}

	//	响应者从窗口负载拿 Tx → AppendModification（交易侧唯一收口：State==Querying 才收，卷 04 §4.1；
	//	FTransactionModified 在交易侧广播）。修改不直写结果位、不覆盖共享值——Execute 前由族 FoldMods 统一折叠
	if (UBattleTransaction* Tx = ActiveWindow->Tx.Get())
	{
		Tx->AppendModification(Req);
	}
	//	纯阶段时机（Tx 为空）：技能对阶段的修正（Skip/Extra/Jump）走 PushPhaseResolvation 写入
	//	PhaseMsgQueue，不经 ModRequest 通道——两条回写路径在 CloseTimingWindow 的路由处汇合
	bAnyResponded = true;

	//	InOrder 推进：响应完 → 下一位（问完全员时 AskNextResponder 内部收口关窗）
	switch (ActivePolicy)
	{
		case EWindowPolicy::InOrder: 	
			ResponderCursor++;
			return AskNextResponder();
		case EWindowPolicy::All:
			{
				//TODO: 
			}
		case EWindowPolicy::FirstOnly:
			CloseTimingWindow();
	}

}

void UBattleGameFlowSubsystem::DeclineWindowResponse(APlayerState* Responder)
{
	if (!IsRunningOnServer())
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][DeclineWindowResponse] Response should only be declined on server"))
		return;
	}

	//	与提交同一套二次校验：窗口在场 + 锁内 + 提交者 = 当前被询问者
	if (!ActiveWindow.IsSet() || !IsResponding())
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][DeclineWindowResponse] No active responding window, decline rejected"))
		return;
	}
	if (!ActiveWindow->Responders.IsValidIndex(ResponderCursor)
		|| ActiveWindow->Responders[ResponderCursor].Owner.Get() != Responder)
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][DeclineWindowResponse] Responder is not the one being asked, decline rejected"))
		return;
	}

	//	放弃 = 不落任何修改，直接推进下一位；全员弃 → AskNextResponder 收口 → 空结果关窗。
	//	UI 倒计时到点也走本入口（客户端直接请求关闭）——关闭时机统一只通过 UI 决定，引擎侧不挂定时器
	switch (ActivePolicy)
	{
	case EWindowPolicy::FirstOnly:
	case EWindowPolicy::InOrder: 	
		ResponderCursor++;
		return AskNextResponder();
	case EWindowPolicy::All:
		{
			//TODO:
		}
	}
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
	//	先入栈再换手——入栈时忙锁仍指向父，帧记下来的就是父
	NestedStack.Push({ActiveTransaction , Child});
	ActiveTransaction = Child;

	//	子交易开自己的窗口；合规插入点上 ActiveWindow 必为空，不撞单层窗口保护（卷 04 §3.2）。
	//	注意：子交易入队方式为 P3 决策点（Start(bNested) vs EnqueueNested()，P3 §3.9）——此处按骨架取"直接驱动"
	Child->Drive();
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
