// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleGameFlowSubsystem.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "ArkWars/ArkWars.h"
#include "ArkWars/Battle/ArkWarBattleSettings.h"
#include "ArkWars/Battle/Component/Card/CardManagementBusComponent.h"
#include "ArkWars/Battle/Component/GameFlow/GamePhaseManagerComponent.h"
#include "ArkWars/Battle/Component/GameMode/GameModeComponentBase.h"
#include "ArkWars/Battle/Toolkits/ArkWarFlowTypes.h"
#include "ArkWars/Battle/Toolkits/GameMessage.h"
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
		
		if (Next == EGamePhase::Begin || Next == EGamePhase::Finish || IsPreOrPost(Next))
		{
			Next = NextInSequence(Next);
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
	if (ActiveTransaction.IsValid()) return;
	
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
	if (ActiveTransaction != Tx)
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

	//	①②③ 查时机索引 → 合法预检 → 排序填 Responders 名单（卷 12 §4.1，实现见 CollectResponders）
	FResponseWindow Window;
	Window.Timing = EventTag;
	Window.Tx = TStrongObjectPtr(Tx);								//	TStrongObjectPtr 强持有：防窗口悬停期交易被 GC；纯阶段时机为空
	Window.Responders = CollectResponders(EventTag, Tx);
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
	bRespondingLock = true;
	AskNextResponder();
}

TArray<FResponseEntry> UBattleGameFlowSubsystem::CollectResponders(const FGameplayTag& TimingTag, UBattleTransaction* Tx) const
{
	//	未来形态（卷 12 §4.1 ①②③，P3 §3.3）：
	//	① 查 USkillManagerSubsystem 的"时机 → 监听技能"索引（TMap<FGameplayTag, TArray<...>>）得候选技能；
	//	② 逐技能合法预检：来源在场/可用；账本余量 P6 前常量放行（占位注释标注，卷 07）；
	//	③ 排序 = ResponsePriority 降序 × 座次（从当前回合玩家起顺/逆时针，ISeatLayout，卷 10 §2），
	//	   平手取注册稳定序 → 填 FResponseEntry{ Tx, Priority, Owner, Key }。
	//	现状：时机索引未建 → 返回空表，每窗走"无监听直通"路径（查表空表 = 最低成本，P3 §7）。
	//	TODO(P3 §3.3)：USkillManagerSubsystem 建好时机索引后接入此处
	return {};
}

void UBattleGameFlowSubsystem::AskNextResponder()
{
	if (!ActiveWindow.IsSet()) return;

	const TArray<FResponseEntry>& Responders = ActiveWindow->Responders;
	if (Responders.IsValidIndex(ResponderCursor))
	{
		//	InOrder：按序逐人问——客户端只收"你有响应权 + 候选"（P3 只定下发协议，UI 是 P7）。
		//	引擎在此返回、不轮询不阻塞：下一位由该玩家经 ServerRPC 提交/放弃后驱动游标推进。
		//	（All/FirstOnly 策略的差异只在"问几个人、何时收敛"，推进骨架不变——P3 不实现，卷 10 §2）
		const FResponseEntry& Entry = Responders[ResponderCursor];
		UE_LOG(LogGamePlay, Log, TEXT("[GameFlow][AskNextResponder] Asking responder %d/%d (Key:%s)"),
			ResponderCursor + 1, Responders.Num(), *Entry.Key.ToString())
		//	TODO(P3 §3.3)：候选下发给 Entry.Owner 的客户端（表现复制通道，卷 11）
		return;
	}

	//	全员响应/放弃完毕 → 空结果或已有结果关窗（修改已在提交阶段逐条落入 Tx->ModeRequests）
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
	if (!ActiveWindow.IsSet() || !bRespondingLock)
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
	ResponderCursor++;
	AskNextResponder();
}

void UBattleGameFlowSubsystem::DeclineWindowResponse(APlayerState* Responder)
{
	if (!IsRunningOnServer())
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][DeclineWindowResponse] Response should only be declined on server"))
		return;
	}

	//	与提交同一套二次校验：窗口在场 + 锁内 + 提交者 = 当前被询问者
	if (!ActiveWindow.IsSet() || !bRespondingLock)
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
	ResponderCursor++;
	AskNextResponder();
}

void UBattleGameFlowSubsystem::CloseTimingWindow()
{
	//	幂等保护：关窗只发生一次（防御多路径重复收口造成回调重入）
	if (!ActiveWindow.IsSet()) return;

	//	⑥-1 撤 State.Responding 锁：窗口期被阻断的无关输入自此恢复
	bRespondingLock = false;

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
