// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArkWars/Battle/Toolkits/ArkWarDelegates.h"
#include "ArkWars/Battle/Toolkits/ArkWarCardTypes.h"
#include "ArkWars/Battle/Toolkits/CardContainerInterface.h"
#include "Components/ActorComponent.h"
#include "CardManagementBusComponent.generated.h"

/**
 * 	设计附加到PlayerState上的组件，用于储存该玩家具有的所有卡牌及区域
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARKWARS_API UCardManagementBusComponent : public UActorComponent, public ICardContainerInterface
{
	GENERATED_BODY()
	///选中仅在客户端中暂存
	TArray<FArkCard> SelectedCards;
	///本地暂存的选中目标（弱引用，取走前须有效性过滤）
	TArray<TWeakObjectPtr<APlayerState>> SelectedPlayers;
	
	///装备区：按槽位下标存放（空槽以 Identity == INDEX_NONE 的占位牌填充）
	UPROPERTY(Replicated)
	TArray<FArkCard> EquipmentArea;
	
	//约定：奇数位槽置放效果判定，偶数位槽置放伤害判定
	UPROPERTY(Replicated)
	TArray<FArkCard> JudgementArea;
	
	///手牌区：纯顺序数组，尾元素为牌顶
	UPROPERTY(Replicated)
	TArray<FArkCard> HandCards;
public:

	///构造：开启组件复制（复制字段见 GetLifetimeReplicatedProps）
	UCardManagementBusComponent();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	///玩家侧本地选择目标的变更广播（供 UI 或结算方订阅）
	FPlayerSelectionChangeDelegate OnPlayerSelectionChanged;
	///玩家侧本地选择卡牌的变更广播（供 UI 或结算方订阅）
	FCardSelectionChangeDelegate OnCardSelectionChanged;


	/**
	 *	在手牌中查找指定卡的下标
	 *	@param Card	要查找的卡（按 FArkCard 相等比较）
	 *	@return			命中下标；未找到返回 INDEX_NONE
	 */
	int32 GetIndexOfCard(const FArkCard& Card) const;

	/**
	 *	将一张卡加入本地待选集合（仅客户端暂存，需用时经 ConsumeCards 取走）
	 *	@param Card	被选中的卡
	 */
	void SelectCard(const FArkCard& Card);

	/**
	 *	从本地待选集合移除一张卡（反选）
	 *	@param Card	要取消选中的卡
	 */
	void ClearCardSelection(const FArkCard& Card);

	/**
	 *	获取本地待选卡集合（只读引用）
	 *	@return	当前选中的卡数组
	 */
	const TArray<FArkCard>& GetSelectedCards() { return SelectedCards; };

	/**
	 *	取走并清空本地待选卡集合，并广播一次选择变更
	 *	@return	本次取走的卡（以移动语义返回）
	 */
	TArray<FArkCard> ConsumeCards();

	/**
	 *	将一位玩家加入本地待选目标集合
	 *	@param Target	被选中的目标玩家
	 */
	void SelectTarget(APlayerState* Target);

	/**
	 *	从本地待选目标集合移除一位玩家
	 *	@param Target	要取消选中的玩家
	 */
	void ClearTargetsSelection(APlayerState* Target);

	/**
	 *	获取本地待选目标集合（只读引用）
	 *	@return	当前选中的玩家弱引用数组
	 */
	const TArray<TWeakObjectPtr<APlayerState>>& GetSelectedTargets() { return SelectedPlayers;};

	/**
	 *	取走并清空本地待选目标集合：过滤失效弱引用后返回，并广播一次选择变更
	 *	@return	本次取走的有效玩家指针数组
	 */
	TArray<APlayerState*> ConsumeTargets();

	
	/**
	 *	询问指定判定牌能否放入判定区（同种唯一/槽位占用的前置判定）
	 *	@param Card	要放入判定区的卡
	 *	@return			可放返回 true；牌类不符 / 槽下标解析失败 / 槽已占用返回 false
	 */
	bool CanPutInJudgement(const FArkCard& Card) const;

	/**
	 *	询问指定牌能否装备到装备区（槽位占用/废除标记的前置判定）
	 *	@param Card	要装备的卡
	 *	@return			可装备返回 true；牌类不符 / 槽下标解析失败 / 槽已占用返回 false
	 */
	bool CanEquipCard(const FArkCard& Card) const;

	/**
	 *	按卡的 Tag 定位装备槽并将牌落入装备区
	 *	槽位不足时以空占位牌扩容到目标下标；槽空则置入，槽已占用则以新牌顶替并
	 *	对被顶下的旧牌发起一次移动交易（Equipment → Used）
	 *	仅服务器权威执行（无权限直接返回）
	 *	@param Card	要装备的卡
	 */
	void Equip(const FArkCard& Card);

	/**
	 *	按卡的 Tag 定位判定槽并将判定牌置入判定区
	 *	槽位不足时以空占位牌扩容到目标下标；槽空则置入，槽已占用则静默忽略
	 *	仅服务器权威执行（无权限直接返回）
	 *	@param Card	要放入判定区的卡
	 */
	void PutIntoJudgement(const FArkCard& Card);

	/**
	 *	结算本玩家判定区中的判定牌
	 *	TODO: 当前为空实现，判定结算待阶段体接入（卷 08 §8）
	 */
	void HandleJudgement();


	/**
	 *	返回指定玩家区域内的卡牌副本
	 *	仅支持本组件名下的 Hand / Equipment / Judgement 三区，其余区域返回空数组
	 *	@param Area	区域标签（CardTags::Hand / Equipment / Judgement）
	 *	@return			对应区域的卡数组副本；区域不识别时为空数组
	 */
	virtual TArray<FArkCard> GetCardsByKey(const FGameplayTag& Area) const override;

	/**
	 *	在本组件三区（手牌 / 判定区 / 装备区）内按实体 Id 反查卡
	 *	@param CardId	卡牌实体 Id（FArkCard::Identity）
	 *	@return			命中的卡指针；未找到返回 nullptr
	 */
	virtual const FArkCard* GetCardById(int32 CardId) const override;

	/**
	 *	按移动消息的取序与谓词，从指定区域筛选卡并返回其源区下标
	 *	只读：不修改区域内容；区域为空或不可识别时返回空数组
	 *	@param Area	区域标签（仅 Hand / Equipment / Judgement）
	 *	@param Msg	移动意图消息：_Order 决定扫描方向（Top 尾→头 / Bottom 头→尾 / Random），
	 *	             _Count 决定目标数量，Msg(Card) 谓词决定单卡是否命中
	 *	@return			命中卡在源区中的下标数组（可直接作为 Consume 的输入）
	 */
	virtual TArray<int32> Select(const FGameplayTag& Area, const FMessageType& Msg) const override;

	/**
	 *	把指定下标对应的卡从源区取出并整壳返回（源侧"移出"半跳）
	 *	仅服务器权威执行（无权限返回空数组）；返回值必须被接手（喂给目标 Add 或进入结算去向）
	 *	@param Area	        区域标签（仅 Hand / Equipment / Judgement）
	 *	@param CardIndexes	源区下标数组（一般来自 Select 返回值）
	 *	@return			        取出的卡数组；无权限或区域不识别时为空数组
	 */
	[[nodiscard]] virtual TArray<FArkCard> Consume(const FGameplayTag& Area, const TArray<int32>& CardIndexes) override;

	/**
	 *	把待入的卡落入目标区域（目标侧"移入"半跳）
	 *	仅服务器权威执行（无权限返回 Mismatch）；成功路径调用后 Cards 被清空
	 *	Equipment / Judgement 分支：数量不为 1 → Mismatch；槽位校验不过 → Full；
	 *	通过才真落区（Equip / PutIntoJudgement）并清空入参 Cards
	 *	@param AreaKey	区域标签（Hand / Equipment / Judgement）
	 *	@param Cards	待入的卡（成功落位后为空）
	 *	@param Msg	        移动意图消息（当前 Hand 直接入尾；Equipment / Judgement 走槽位逻辑）
	 *	@return			写入结果；区域不识别返回 Mismatch
	 */
	virtual EAreaWriteResult Add(const FGameplayTag& AreaKey, TArray<FArkCard>& Cards, const FMessageType& Msg) override;

	/**
	 *	返回本组件名下的区域标签集合
	 *	@return	区域标签集合（Hand / Equipment / Judgement）
	 */
	virtual TArray<FGameplayTag> GetAreaKeys() const override;

	/**
	 *	返回承载本组件的 Actor，供交易定位容器宿主
	 *	@return	持有本组件的 Owner（APlayerState）
	 */
	virtual AActor* GetContainerActor() override {return GetOwner();};

};
