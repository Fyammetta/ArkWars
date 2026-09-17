#pragma once
#include "GameplayTagContainer.h"
#include "EventTypes.generated.h"

struct FResponseWindow;
class UBattleTransaction;



//	窗口策略（权威定义；卷 10 §2 的表述与此逐字一致）——三策略回答的是"名单上的人【怎么被问、何时收敛】"，
//	不是"谁有资格被问"；粒度是【技能】而非【玩家】（同一人的多个技能各占一条，同开类下各自独立决定）。
//	发问侧只有两种形态：InOrder 问一个就停，All / FirstOnly 一次问全——文档：All/FirstOnly 无序，
//	现实层面同时发生（排序照跑，只是策略不消费次序）；三者的差别全在【收敛条件】，
//	实现见 BattleGameFlowSubsystem 的待回账本 ResponsePending（关窗判据是"账平"而非"发完"）。
//	名单序 = 技能级优先级（降序主键）× 座次（次序），见卷 10 §2 / CollectResponders
enum class EWindowPolicy
{
	All,          // 名单上所有合法技能同开，各自独立决定（多伤害减免可同时响应）
	InOrder,      // 按次序逐个询问，前一个不响/响完再到下一个（"逐人询问是否出回应牌"）
	FirstOnly     // 谁先响应生效，其余作废（首响抢占类）
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
	
	//	默认构造必须显式保留：RPC 序列化要它，删了 Client_OpenResponseWindow 就编不过
	FClientResponseWindow() = default;

	//	服务器窗口 → 客户端视图（P3 §3.3 候选下发的翻译层）。
	//	定义落在 EventTypes.cpp 而非头内联：读 FResponseWindow 的成员要它完整，而它定义在
	//	BattleTransaction.h（本头反向 include 会成环）——头里只留声明，完整类型的账由那个 TU 付。
	//	带 Entry 而非只带 Window：下发的候选就是【本次询问的那一条】，理由见 .cpp
	FClientResponseWindow(const FResponseWindow& Window, const FResponseEntry& Entry);
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