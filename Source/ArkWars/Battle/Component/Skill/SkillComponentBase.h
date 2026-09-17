// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SkillComponentBase.generated.h"


class UBattleTransaction;
struct FGameplayTag;
struct FSkillInfo;
struct FWindowResponseRequest;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARKWARS_API USkillComponentBase : public UActorComponent
{
	GENERATED_BODY()
	
	TSharedPtr<FSkillInfo> SkillInfo;
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
public:
	UFUNCTION(BlueprintCallable,BlueprintPure, Category=Skill)
	static USkillComponentBase* CreateSkill(APlayerState* Owner, const FGameplayTag& Skill);
	
	template <typename T = USkillComponentBase>
	static T* CreateSkill(APlayerState* Owner, const FGameplayTag& Skill) { return Cast<T>(CreateSkill(Owner, Skill)); };
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category=Skill)
	FGameplayTag GetSkillTag() const;
	
	/**
	 *	被点名钩子（发问侧，卷 09 §10）：窗口侧认定"这件技能听到了本次时机"后、在候选下发【之前】调用，
	 *	一次询问调一次。职责 = 【就位】，只三件事：
	 *	  ① 接住接缝——收下本窗的 Timing 与负载 Tx（这是技能在窗内拿交易引用的唯一途径，卷 04 §4.1），
	 *	     需要时应缓存到成员，供 OnRespond 取用；
	 *	  ② 自判与准备——决定"这一窗我要不要响、响的话代价与目标是哪个"，只【决定】不【落修改】；
	 *	  ③ （服务器侧）就位后自持。表现不在此列：客户端面板由 UI 订阅 FClientResponseWindow 自理。
	 *
	 *	【不得在此提交/放弃】：提交（SubmitWindowResponse）是回传侧落点，在此同步调用会把发问与回传
	 *	两个阶段叠进同一帧、抢在客户端收到面板之前落地生效，并触发重入——内层递归可能关窗、甚至
	 *	立刻开新窗，本层余句随即作废（引擎侧由 AskNextResponder 的重入守卫兜住：越界写账本与错位
	 *	下发都在那里被拦，但那是护栏、不该被当路走）。产修改归 OnRespond；
	 *	"自动生效"的正当写法是【不进询问通道】（卷 09 §8 契约行），而不是进名单后靠本钩子提交模拟自动化。
	 */
	UFUNCTION(BlueprintCallable)
	virtual void OnCanActivate(const FGameplayTag& Timing, UBattleTransaction* Tx) PURE_VIRTUAL(OnCanActivate);

	/**
	 *	窗口内响应产出：二次校验通过后由 UBattleGameFlowSubsystem 调用（卷 11 §5.2 上行落点的下游）。
	 *	"本技能的响应等于什么修改"是技能自己的知识——在此决定 EModOp/Magnitude/Priority 后调
	 *	Tx->AppendModification(...)（交易侧唯一收口，State == Querying 才收，卷 04 §4.1）；
	 *	修改不直写结果位、不覆盖共享值，Execute 前由族 FoldMods 统一折叠。
	 *	默认空体（非 PURE_VIRTUAL）——行使了响应权但不产生修改是合法情形（bAnyResponded 照记，
	 *	行为语义而非效果语义，卷 10 §2），且调用点无条件调本钩子，未覆写的子类不该让响应链运行期崩。
	 *	@param Tx			窗口负载；纯阶段时机为 nullptr（阶段修正走 PushPhaseResolvation 回写，卷 10 §3）
	 *	@param Req			上行意图 DTO：目标/代价参数随访 P5/P6 扩展（卷 11 §5.2）
	 *	非 UFUNCTION：Req 用前置声明即可（引用参数不需要完整类型），刻意不把 EventTypes.h 拉进本组件——
	 *	它内含 TStrongObjectPtr<UBattleTransaction> 的构造，要求 UBattleTransaction 完整，而它反向
	 *	include 不了定义 UBattleTransaction 的 BattleTransaction.h（后者已 include 它）。谁要完整类型谁自己带
	 */
	virtual void OnRespond(UBattleTransaction* Tx, const FWindowResponseRequest& Req) {};


	
protected:

	///	订/退钩子（卷 09 §9 / P3 §3.3.1）：内容 = 经 GetSkillManager(Owner) 调时机索引的
	///	RegisterSkillTiming / UnregisterSkillTiming。听哪些时机、何时订【全归子类】——
	///	基类 BeginPlay 是空实现、不代调，子类自选挂载点（覆写 BeginPlay / 构造 / 首次使用 / 按需增删）；
	///	退订由基类 EndPlay 兜底回调，不依赖子类记性。二者均为纯虚：未覆写即在其调用点 LowLevelFatalError
	virtual void RegisterActivateTiming() PURE_VIRTUAL(RegisterActivateTiming);
	virtual void UnRegisterActivateTiming() PURE_VIRTUAL(UnRegisterActivateTiming);
};
