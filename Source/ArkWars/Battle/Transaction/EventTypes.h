#pragma once
#include "GameplayTagContainer.h"
#include "EventTypes.generated.h"

class UBattleTransaction;



enum class EWindowPolicy
{
	All,
	InOrder,
	FirstOnly
};


struct FTransactionModRequest
{
	enum class EModOp : uint8 { Override, Add, Multiply, Clamp };

	EModOp Operation;

	float Magnitude;

	TWeakObjectPtr<APlayerState> Actor;

	int32 Priority;
};


//	窗口内一条可响应项：时机索引给出的合法监听者快照（技能标识 / 所属玩家 / 登记优先级）。
//	不携带交易引用——名单与负载无关，负载只挂在窗口上（单一事实来源；候选下发也只用到 Key/Owner）
struct FResponseEntry
{
	int32 Priority;

	TWeakObjectPtr<APlayerState> Owner;

	FGameplayTag Key;
};

//	窗口基座（纯 server 私产，无复制语义，故为普通结构体）
struct FResponseWindow
{
	FGameplayTag Timing;

	//	负载本体 + 悬停期强持有：交易出队后 PendingTransactions 不再持有它，
	//	这里是窗口悬停期唯一的 GC 锚（TStrongObjectPtr 强引用，不依赖反射追踪）；纯阶段时机为空
	TStrongObjectPtr<UBattleTransaction> Tx;

	TArray<FResponseEntry> Responders;

	float Timeout;
};

USTRUCT(BlueprintType)
struct FClientResponseWindow
{
	GENERATED_BODY()
	
	//窗口句柄，Server校验当前窗口
	UPROPERTY(BlueprintReadWrite)
	int32 WindowSerial = 0;			
	
	//窗口展开时机
	UPROPERTY(BlueprintReadWrite)
	FGameplayTag Timing;
	
	//Client计算窗口时间，超时自动取消并告知Server
	UPROPERTY(BlueprintReadWrite)
	float Timeout = 0.f;
	
	//可选技能的Tags
	UPROPERTY(BlueprintReadWrite)
	FGameplayTagContainer Candidates;
	
	//若false，则为纯阶段窗口
	UPROPERTY(BlueprintReadWrite)
	bool bHasTransaction = false;
};

USTRUCT(BlueprintType)
struct FWindowResponseRequest
{
	GENERATED_BODY()
	
	//校验等于当前窗口句柄时才使用，否则丢弃
	UPROPERTY(BlueprintReadWrite)
	int32 WindowSerial = 0;
	
	//Client上报使用的技能
	UPROPERTY(BlueprintReadWrite)
	FGameplayTag Skill;
};

struct FNestedTransactionFrame
{
	TStrongObjectPtr<UBattleTransaction> Parent;
	TStrongObjectPtr<UBattleTransaction> Child;
	
	FNestedTransactionFrame() : Parent(nullptr), Child(nullptr) {}
	FNestedTransactionFrame(UBattleTransaction* Parent, UBattleTransaction* Child) : Parent(TStrongObjectPtr(Parent)), Child(TStrongObjectPtr(Child)) {}
};