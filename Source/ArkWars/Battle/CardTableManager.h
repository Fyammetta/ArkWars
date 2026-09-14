// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Toolkits/CardContainerInterface.h"
#include "CardTableManager.generated.h"

///	管理当局游戏的非角色卡牌区域，如抽牌堆、弃牌堆等
///	服务器权威，并复制到每个客户端



UCLASS()
class ARKWARS_API ACardTableManager : public AActor , public ICardContainerInterface
{
	GENERATED_BODY()
	
	///全局唯一管理器实例（TStrongObjectPtr 持有，见 Get / EndPlay）
	static TStrongObjectPtr<ACardTableManager> Instance;
	
	//展示、暂存等卡牌的临时区，
	UPROPERTY(ReplicatedUsing=OnRep_CachedAreaChanged)
	TArray<FArkCard> CachedArea;
	
	//被打出的牌会进入此区域，弃牌阶段开始前以至弃牌区并不触发弃牌事件,UI设计上只显示最新的一张，可以手动展开显示全部
	UPROPERTY(ReplicatedUsing=OnRep_PlayedAreaChanged)
	TArray<FArkCard> PlayedArea;
	
	//抽牌堆，默认以数组尾端作为牌堆顶
	UPROPERTY(ReplicatedUsing=OnRep_PileAreaChanged)
	TArray<FArkCard> PileArea;
	
	//弃牌堆，在抽牌堆空时洗入抽牌堆
	UPROPERTY(ReplicatedUsing=OnRep_DiscardAreaChanged)
	TArray<FArkCard> DiscardArea;
	
	//判定用的牌最终流向此处，结束阶段前置入弃牌堆
	UPROPERTY(ReplicatedUsing=OnRep_JudgementAreaChanged)
	TArray<FArkCard> JudgementArea;
	
public:
	/**
	 *	默认构造：管理器随世界复制（服务器权威）
	 */
	ACardTableManager();

	/**
	 *	注册 CachedArea / PlayedArea / PileArea / DiscardArea / JudgementArea 五个
	 *	区域字段的复制，并在客户端触发对应 OnRep
	 *	@param OutLifetimeProps	UE 复制系统收集的属性表
	 */
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;


	/**
	 *	获取当前世界的管理器单例
	 *	@param World			需要获取管理器的世界
	 *	@return					管理器实例；世界中已存在则复用；客户端（非 Listen / Dedicated Server）
	 *	                        未找到时返回 nullptr，服务器侧才会新建
	 */
	static ACardTableManager* Get(UWorld* World);

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 *	复制通知：CachedArea（缓存/临时区）变化
	 *	TODO: 当前为空实现，表现更新待接
	 */
	UFUNCTION()
	virtual void OnRep_CachedAreaChanged();
	
	/**
	 *	复制通知：PlayedArea（已出牌缓冲）变化
	 *	TODO: 当前为空实现，表现更新待接
	 */
	UFUNCTION()
	virtual void OnRep_PlayedAreaChanged();
	
	/**
	 *	复制通知：PileArea（抽牌堆）变化
	 *	TODO: 当前为空实现，表现更新待接
	 */
	UFUNCTION()
	virtual void OnRep_PileAreaChanged();
	
	/**
	 *	复制通知：DiscardArea（弃牌堆）变化
	 *	TODO: 当前为空实现，表现更新待接
	 */
	UFUNCTION()
	virtual void OnRep_DiscardAreaChanged();
	
	/**
	 *	复制通知：JudgementArea（桌面判定流向区）变化
	 *	TODO: 当前为空实现，表现更新待接
	 */
	UFUNCTION()
	virtual void OnRep_JudgementAreaChanged();

public:
	/**
	 *	在桌面五区（缓存 / 判定流向 / 已出 / 牌堆 / 弃牌）内按实体 Id 反查卡
	 *	@param CardId	卡牌实体 Id（FArkCard::Identity）
	 *	@return			命中的卡指针；未找到返回 nullptr
	 */
	virtual const FArkCard* GetCardById(int32 CardId) const override;

	/**
	 *	按移动消息的取序与谓词，从桌面区域筛选卡并返回其源区下标
	 *	只读：不修改区域内容；区域为空或不可识别时返回空数组
	 *	@param Area	区域标签（Pile / Discard / Judgement / Used / Cache）
	 *	@param Msg	移动意图消息：_Order 决定扫描方向（Top 尾→头 / Bottom 头→尾 / Random），
	 *	             _Count 决定目标数量，Msg(Card) 谓词决定单卡是否命中
	 *	@return			命中卡在源区中的下标数组（可直接作为 Consume 的输入）
	 */
	virtual TArray<int32> Select(const FGameplayTag& Area, const FMessageType& Msg) const override;

	/**
	 *	把指定下标对应的卡从源区取出并整壳返回（源侧"移出"半跳）
	 *	@param Area	        区域标签（Pile / Discard / Judgement / Used / Cache）
	 *	@param CardIndexes	源区下标数组（一般来自 Select 返回值）
	 *	@return			取出的卡数组；区域不识别时为空数组
	 */
	[[nodiscard]] virtual TArray<FArkCard> Consume(const FGameplayTag& Area, const TArray<int32>& CardIndexes) override;

	/**
	 *	把待入的卡落入目标区域（目标侧"移入"半跳）
	 *	Pile 依 _Order 落位（Top = 入数组尾即牌堆顶、Bottom = 入数组头、Random = 随机插入）；
	 *	Discard / Judgement / Used / Cache 追加数组尾部；成功路径调用后 Cards 被清空
	 *	@param AreaKey	区域标签（Pile / Discard / Judgement / Used / Cache）
	 *	@param Cards	待入的卡（成功落位后为空）
	 *	@param Msg	        移动意图消息（Pile 使用 _Order；其余区域忽略）
	 *	@return			落位成功返回 Accepted；未识别区域键返回 Mismatch
	 */
	virtual EAreaWriteResult Add(const FGameplayTag& AreaKey, TArray<FArkCard>& Cards, const FMessageType& Msg) override;

	/**
	 *	返回管理器自身（Actor 即容器宿主）
	 *	@return	this
	 */
	virtual AActor* GetContainerActor() override { return this;};

protected:
	/**
	 *	返回指定桌面区域的卡牌副本
	 *	支持 Pile / Discard / Judgement / Used(PlayedArea) / Cache 五区（09-10 已补 Judgement）
	 *	@param Key	区域标签
	 *	@return		对应区域的卡数组副本；区域不识别时为空数组
	 */
	virtual TArray<FArkCard> GetCardsByKey(const FGameplayTag& Key) const override;

	/**
	 *	返回桌面已注册的区域标签集合
	 *	@return	区域标签集合（Pile / Discard / Cache / Used / Judgement，与 GetCardsByKey 一致）
	 */
	virtual TArray<FGameplayTag> GetAreaKeys() const override;
};
