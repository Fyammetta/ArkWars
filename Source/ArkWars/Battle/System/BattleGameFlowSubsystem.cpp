// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleGameFlowSubsystem.h"

#include "AbilitySystemInterface.h"
#include "ArkWars/ArkWars.h"
#include "ArkWars/Battle/Component/Card/CardManagementBusComponent.h"
#include "ArkWars/Battle/Component/GameFlow/GamePhaseManagerComponent.h"
#include "ArkWars/Battle/Component/GameMode/GameModeComponentBase.h"
#include "ArkWars/Battle/Toolkits/ArkWarFlowTypes.h"
#include "ArkWars/Battle/Toolkits/GameMessage.h"
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

void UBattleGameFlowSubsystem::InitPlayerOrder(int32 StartIndex)
{
	
	if (!IsRunningOnServer())
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][InitPlayerOrder] Initialization should only be run on server"))
		return;
	};
	auto Comp = GetPhaseManager();
	if (!Comp)
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][InitPlayerOrder] Fail to init player order, phase manager was not found"))

		return;
	}
	Comp->InitPlayers(StartIndex);
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
	ModeComp->AllocateIdentity();
	UE_LOG(LogGamePlay, Log, TEXT("[GameFlow][StartGame] Identity allocated completely"))
	//分发起始手牌
	for (auto i = PhaseComp->GetPlayerIndex(); i < PhaseComp->GetPlayerIndex() + PhaseComp->GetPlayerCount() ; ++i)
	{
		auto Player = PhaseComp->GetPlayerByIndex(i);
		auto Hand = Player->GetComponentByClass<UCardManagementBusComponent>();
		using MSG = GameMessage::FMoveMessage;
		auto Num = ModeComp->GetStartCardNum(Player->Implements<UAbilitySystemInterface>() ? Cast<IAbilitySystemInterface>(Player)->GetAbilitySystemComponent() : nullptr);
		Manager->MoveOut(Hand, {}, FString::Printf(TEXT("%s=%d %s=%s %s=%s"),MSG::Num, Num, MSG::From, MSG::Pile, MSG::To, MSG::Hand));
		
		UE_LOG(LogGamePlay, Log, TEXT("[GameFlow][StartGame] Initial hand card allocated to %s"), *Player->GetPlayerNameCustom())
	}

	//广播游戏开始阶段通知
	PhaseComp->OnCalledStartGame();
	UE_LOG(LogGamePlay, Log, TEXT("[GameFlow][StartGame] Broadcast start game phase notify"))

}

void UBattleGameFlowSubsystem::EnqueueTransaction(UBattleTransaction* Tx)
{
	if (!IsRunningOnServer())
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][EnqueueTransaction] Transaction should only be added on server"))
		return;
	};
}

void UBattleGameFlowSubsystem::DriveTransaction()
{
	if (!IsRunningOnServer())
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][DriveTransaction] Process should only run on server"))
		return;
	};
}

void UBattleGameFlowSubsystem::OnTransactionFinished(UBattleTransaction* Tx)
{
	if (!IsRunningOnServer())
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[GameFlow][OnTransactionFinished] Transaction finish notify should only be broadcast on server"))
		return;
	};
}
