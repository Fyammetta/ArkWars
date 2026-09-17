// Fill out your copyright notice in the Description page of Project Settings.


#include "BattlePlayerController.h"
#include "BattlePlayerState.h"
#include "ArkWars/ArkWars.h"
#include "ArkWars/Battle/System/BattleGameFlowSubsystem.h"
//	取 FClientResponseWindow 完整类型（Client_OpenResponseWindow 要读 WindowSerial/Timeout）：
//	子系统头当前会经 BattleTransaction.h 间接带进来，但那是别人的 include 链——本 TU 要完整类型就自己带
#include "ArkWars/Battle/Transaction/EventTypes.h"
#include "ArkWars/Battle/Component/Card/CardComponentBase.h"
#include "ArkWars/Battle/Component/Card/CardManagementBusComponent.h"
#include "ArkWars/Battle/Toolkits/BattleFunctionLibrary.h"
#include "ArkWars/Battle/Toolkits/CardContainerInterface.h"

void ABattlePlayerController::TryEndPhase()
{
}

void ABattlePlayerController::TrySelectCard(const FArkCard& Card) const
{
	auto PS = GetPlayerState<AActor>();
	if (!PS)
	{
		return;
	}
	auto Comp = PS->FindComponentByClass<UCardManagementBusComponent>();
	if (!Comp)
	{
		return;
	}
	Comp->SelectCard(Card);
}

void ABattlePlayerController::TryUseCard(const FArkCard& Card)
{
	auto PS = GetPlayerState<APlayerState>();
	auto Comp = PS->FindComponentByClass<UCardManagementBusComponent>();

	Server_UseCard(PS, Comp->ConsumeTargets(), Card);
}

void ABattlePlayerController::TryMoveCard(const TArray<FArkCard>& Cards, ICardContainerInterface* From, ICardContainerInterface* To, const FString& Msg)
{
	auto FromObj = From->_getUObject();
	auto ToObj = From->_getUObject();
	
	Server_MoveCard(Cards ,FromObj, ToObj, Msg);
}

void ABattlePlayerController::TryResponse(APlayerState* Target, const FArkCard& Source, const FArkCard& Card)
{
	Server_Response(Target, Source, Card);
}

void ABattlePlayerController::TryShowCard(const TArray<FArkCard>& Cards)
{
	Server_ShowCard(Cards);
}

void ABattlePlayerController::TryStartComparison(const TArray<APlayerState*>& Targets)
{
	if (Targets.IsEmpty())
	{
		return;
	}
	Server_StartComparison(Targets);
}

void ABattlePlayerController::TryResponseComparison(APlayerState* Target)
{
	Server_ResponseComparison(Target);
}

void ABattlePlayerController::TryConfirm(const TFunction<void(APlayerController*)>& CustomEvent)
{
	if (CustomEvent)
	{
		CustomEvent(this);
	}
}

void ABattlePlayerController::TryActivateSkill(const FGameplayTag& SkillTag)
{
}

void ABattlePlayerController::TrySubmitWindowResponse(const FWindowResponseRequest& Req)
{
	GetWorld()->GetTimerManager().ClearTimer(DeclineHandle);
	Server_SubmitWindowResponse(Req);
}

void ABattlePlayerController::TryDeclineWindowResponse(int32 WindowSerial)
{
	GetWorld()->GetTimerManager().ClearTimer(DeclineHandle);
	Server_DeclineWindowResponse(WindowSerial);
}

void ABattlePlayerController::Client_OpenResponseWindow_Implementation(const FClientResponseWindow& Window)
{
	//	超时兜底先立：它是"玩家全程不管"时唯一的收窗路径，必须先于广播——
	//	广播是旁路，订阅者回调若抛错/耗时，不得把兜底一起带走。
	//	句柄按值进委托（不是现场读成员）：超时那一刻发出的是【开窗时那一扇】的句柄，陈旧消息由服务器句柄比对挡下
	auto& Mgr = GetWorld()->GetTimerManager();
	FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &ABattlePlayerController::TryDeclineWindowResponse,Window.WindowSerial);
	Mgr.SetTimer(DeclineHandle, Delegate, Window.Timeout,false);

	//	再同步分发：UI 展开面板并自起倒计时、需表现的技能组件各取所需——
	//	PC 不 FindComponentByTag（PC 不认窗口，同理不认技能；服务器侧那次"找组件"在子系统内，
	//	因为子系统是窗口的主人、知道名单与游标）。谁要完整类型谁自己带 EventTypes.h
	OnWindowResponseRequested.Broadcast(Window);
}

void ABattlePlayerController::Server_SubmitWindowResponse_Implementation(const FWindowResponseRequest& Req)
{
	//	零信任（卷 11 §5.2）：身份只从连接推导——本 RPC 的 this = 发起者本人的 PC，
	//	绝不接受客户端自报的提交者。同理"提交的是哪个技能"也不作数：它只是【定位线索】，
	//	用来在本人名下的待回条目里挑一条（挑不出即拒），被问的条目始终以服务器账为准
	//	（Req.Skill 须由客户端回填被点名条目的 Key——面板拿到手的那一条，见 FClientResponseWindow）
	auto Responder = GetPlayerState<APlayerState>();
	if (!Responder)
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[BattlePlayerController][Server_SubmitWindowResponse] PlayerState is missing, submission dropped (WindowSerial:%d)"), Req.WindowSerial)
		return;
	}

	//	纯搬运：二次校验、技能定位、修改产出全在子系统内完成——PC 不认识窗口，也不该认识
	auto Subsystem = UBattleFunctionLibrary::GetBattleManager(this);
	if (!Subsystem)
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[BattlePlayerController][Server_SubmitWindowResponse] Game flow subsystem is missing, submission dropped"))
		return;
	}
	Subsystem->SubmitWindowResponse(Responder, Req);
}

void ABattlePlayerController::Server_DeclineWindowResponse_Implementation(int32 WindowSerial)
{
	auto Responder = GetPlayerState<APlayerState>();
	if (!Responder)
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[BattlePlayerController][Server_DeclineWindowResponse] PlayerState is missing, decline dropped (WindowSerial:%d)"), WindowSerial)
		return;
	}

	auto Subsystem = UBattleFunctionLibrary::GetBattleManager(this);
	if (!Subsystem)
	{
		UE_LOG(LogGamePlay, Warning, TEXT("[BattlePlayerController][Server_DeclineWindowResponse] Game flow subsystem is missing, decline dropped (WindowSerial:%d)"), WindowSerial)
		return;
	}
	Subsystem->DeclineWindowResponse(Responder, WindowSerial);
}

void ABattlePlayerController::Server_UseCard_Implementation(APlayerState* Source, const TArray<APlayerState*>& Targets, const FArkCard& Card)
{
	if (auto Comp = UCardComponentBase::Get(this, Card))
	{
		Comp->Use(Source, Targets, Card);
	}
}

void ABattlePlayerController::Server_MoveCard_Implementation(const TArray<FArkCard>& Cards,
	const TScriptInterface<ICardContainerInterface>& From, const TScriptInterface<ICardContainerInterface>& To, const FString& Msg)
{
}

void ABattlePlayerController::Server_Response_Implementation(APlayerState* Target, const FArkCard& Source, const FArkCard& Card)
{
}

void ABattlePlayerController::Server_ShowCard_Implementation(const TArray<FArkCard>& Cards)
{
}

void ABattlePlayerController::Server_StartComparison_Implementation(const TArray<APlayerState*>& Targets)
{
}

void ABattlePlayerController::Server_ResponseComparison_Implementation(APlayerState* Target)
{
}

void ABattlePlayerController::Server_ConfirmComparison_Implementation(const FArkCard& Card, bool bIsInitiator)
{
}
