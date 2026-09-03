// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleGameFlowSubsystem.h"
#include "ArkWars/Battle/Component/GameFlow/GamePhaseManagerComponent.h"
#include "ArkWars/Battle/Toolkits/ArkWarFlowTypes.h"
#include "ArkWars/Battle/Toolkits/GameMessage.h"
#include "GameFramework/GameStateBase.h"

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
}

void UBattleGameFlowSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UBattleGameFlowSubsystem::InitPlayerOrder(int32 StartIndex)
{
	auto Comp = GetPhaseManager();
	if (!Comp)
	{
		return;
	}
	Comp->InitPlayers(StartIndex);
}

int32 UBattleGameFlowSubsystem::GetPlayerIndex(APlayerState* Player)
{
	if (auto Comp = GetPhaseManager())
	{
		return Comp->GetPlayerIndex(Player);
	}
	
	return INDEX_NONE;
}

void UBattleGameFlowSubsystem::RegisterManagerActor(AActor* Actor)
{
	if (Actor && !ManagerActor)
		ManagerActor = Actor;
}

AActor* UBattleGameFlowSubsystem::GetManagerActor() const
{
	return ManagerActor;
}

bool UBattleGameFlowSubsystem::IsRunningOnServer() const
{
	return GetWorld() ? GetWorld()->GetNetMode() < NM_Client : false;
}

void UBattleGameFlowSubsystem::PushPhaseResolvation(const FString& Msg)
{	
	PhaseMsgQueue.Add(Msg);
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

void UBattleGameFlowSubsystem::Advance(EGamePhase Phase, int32 Index)
{
	if (!IsRunningOnServer()) return;
	
	auto Comp = GetPhaseManager();
	
	if (!Comp) return;
	
	Comp->SetNextPlayerActive(Index);
	Comp->SetPhase(Phase);
}

void UBattleGameFlowSubsystem::Advance()
{
	if (!IsRunningOnServer()) return;
	
	auto Comp = GetPhaseManager();
	
	if (!Comp) return;
	auto Next = ResolveNextPhase(Comp->GetPhase());
	while (true)
	{
		Comp->SetPhase(Next);
		
		if (Next == EGamePhase::Begin || Next == EGamePhase::Finish || !IsPreOrPost(Next))
		{
			Next = NextInSequence(Next);
			continue;
		}
		break;
	}
	
}
