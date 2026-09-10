#pragma once

#include "CoreMinimal.h"
#include "ArkWarTags.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "GameplayTagContainer.h"
#include "ArkWarCardTypes.generated.h"

class UCardComponentBase;

/// 卡牌在规则层的表示：一张牌 = 一组标签（类别 / 花色 / 点数……）
///	@deprecated	This is against to property replication and RPC, use FAckCard instead
using FCard = TSharedPtr<FGameplayTagContainer>;
///	@deprecated	This is against to property replication and RPC, use FAckCard instead
using FConvertedCard = TWeakPtr<FGameplayTagContainer>;

USTRUCT(BlueprintType)
struct FArkCard
{
	GENERATED_BODY()
	
	///卡牌实体 Id（全局唯一；跨区定位 / 去重 / 相等比较均以它为准）
	///默认 INDEX_NONE = 空槽哨兵（未分配实体的占位牌，如卡槽空位；实牌经 AllocateNewCard 分配）
	UPROPERTY(BlueprintReadOnly)
	int32 Identity = INDEX_NONE;
	
	///规则层标签集合（类别 / 花色 / 点数……）；定槽时常用 First() 作键
	UPROPERTY(BlueprintReadOnly)
	FGameplayTagContainer Card;
	
	///相等语义：只看实体 Id（不含标签），故跨区查找 / 去重都以 Id 为键
	bool operator==(const FArkCard& Other) const { return Identity == Other.Identity; }
	
	FGameplayTag GetClass() const { return Card.First(); }
	
	FGameplayTag GetSuit() const { return Card.GetByIndex(1); }
	
	FGameplayTag GetPoint() const { return Card.GetByIndex(2); }
};

UENUM(BlueprintType)
enum class ECardInstanceKind :uint8 { Direct , Converted, Virtual };

USTRUCT(BlueprintType)
struct FArkCardInstance
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	ECardInstanceKind Kind;
	
	UPROPERTY(BlueprintReadOnly)
	FArkCard Presented;
	FArkCardInstance() : Kind(ECardInstanceKind::Direct), Presented(FArkCard()) {}
	
	FArkCardInstance(const FArkCard& Card) : Kind(ECardInstanceKind::Direct), Presented(Card) {}
	
	FArkCardInstance(const FArkCard& Card, ECardInstanceKind InKind) : Kind(InKind), Presented(Card) {}
};

UENUM(BlueprintType)
enum ECardSuit : uint8
{
	Club		UMETA(DisplayName = "梅花"),
	Diamond		UMETA(DisplayName = "方片"),
	Heart		UMETA(DisplayName = "红桃"),
	Spade		UMETA(DisplayName = "黑桃")
};

USTRUCT(BlueprintType)
struct FHandCardInfo : public FTableRowBase
{
	GENERATED_BODY()

	///手牌的类型标记
	UPROPERTY(EditAnywhere)
	FGameplayTag CardType = {};

	/// 手牌的具体种类标记，与牌名一一对应
	UPROPERTY(EditAnywhere)
	FGameplayTag CardTag = {};

	///手牌的命名
	UPROPERTY(EditAnywhere)
	FText CardName = FText::GetEmpty();

	/// 卡面贴图（软引用，按需加载）
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UTexture2D> CardTexture = {};

	/// 卡牌文案（若需要），其中所有数据部分以{0}、{1}、{2}....方式标记，包括纯数字与游戏内变化的数据(X、Y、Z等)
	UPROPERTY(EditAnywhere)
	FText Description = FText::GetEmpty();

	/// 文案中涉及的数据（若需要）
	UPROPERTY(EditAnywhere)
	TArray<FString> Data = {};
};

USTRUCT(BlueprintType)
struct FCardComponentMapping : public FTableRowBase
{
	GENERATED_BODY()

	/// 卡牌Tag
	UPROPERTY(EditAnywhere)
	FGameplayTag Tag;

	/// 卡牌组件类
	UPROPERTY(EditAnywhere)
	TSubclassOf<UCardComponentBase> Comp;
};


UENUM(BlueprintType)
///区域写入结果：Accepted = 已写入；Full = 目标已满 / 槽位占用；Mismatch = 区域不识别或入参形状不符
//TODO: Accepted 是枚举首值，写接口的失败分支切勿 `return {}`（会默认命中 Accepted），须显式给 Full / Mismatch
enum class EAreaWriteResult : uint8 { Accepted , Full , Mismatch };

USTRUCT(BlueprintType)
///区域槽基元：区域键 + 容量 + 卡牌数组
//TODO: 全库暂无使用方，落位实现定稿后再定去留（P2 §1）
struct FCardAreaSlot
{
	GENERATED_BODY()

	friend struct FCardAreaSlotFactory;
protected:
	///槽所属的区域键
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Area;

	///槽内牌（数组顺序即槽内顺序）
	UPROPERTY(BlueprintReadOnly)
	TArray<FArkCard> Cards;

	///槽容量（默认 1，可经工厂改写；IsFull 判定以此为准）
	UPROPERTY(BlueprintReadOnly)
	int32 Capacity;

	FCardAreaSlot(const FGameplayTag& Key) : Area(Key), Cards({}), Capacity(1) {};

public:
	FCardAreaSlot() : Area({}), Cards({}), Capacity(0) {};



	int32 Num() const {return Cards.Num();}

	bool IsFull() const { return Capacity <= Num(); }

	bool IsEmpty() const { return Num() == 0; }

	bool Add(const FArkCard& Card) { if (IsFull()) return false; return Cards.Add(Card) != INDEX_NONE; };

	bool Remove(const FArkCard& Card) { return Cards.Remove(Card) != INDEX_NONE; };

	const TArray<FArkCard>& GetAll() const { return Cards; }

	const FGameplayTag& GetAreaKey() const { return Area; }
};

///槽工厂：按区域键 / 牌面构造容量与起始牌确定的槽
struct FCardAreaSlotFactory
{
	///装备槽：容量沿用默认 1，起始为空
	static FCardAreaSlot CreateEquipmentSlot(const FGameplayTag& Key);

	///判定槽：槽键取牌面首个标签，并把该判定牌直接放入槽中
	static FCardAreaSlot CreateJudgementSlot(const FArkCard& Card);

	///通用槽：容量可指定（默认 1）
	static FCardAreaSlot CreateSpecialSlot(const FGameplayTag& Key, int32 Capacity = 1);
};

///槽位映射：把"牌面标签键"翻译成区域内的槽下标
//TODO: 骨架，映射表待填（P2 §2-E 缺陷⑦）
namespace CardAreaSlot
{
	//TODO: 判定槽位映射恒返回 INDEX_NONE（REGISTER_CARD_SLOT 宏已备但未使用）
	ARKWARS_API int32 GetJudgementSlot(const FGameplayTag& Key);
	
	//TODO: 装备槽位映射同上，未填表，恒返回 INDEX_NONE
	ARKWARS_API int32 GetEquipmentSlot(const FGameplayTag& Key);
}
