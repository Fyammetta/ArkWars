#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "GameplayTagContainer.h"
#include "ArkWarCardTypes.generated.h"

class UCardComponentBase;

/// 卡牌在规则层的表示：一张牌 = 一组标签（类别 / 花色 / 点数……）
using FCard = TSharedPtr<FGameplayTagContainer>;
using FConvertedCard = TWeakPtr<FGameplayTagContainer>;

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

USTRUCT(BlueprintType)
struct FCardAreaSlot
{
	GENERATED_BODY()

	friend struct FCardAreaSlotFactory;
protected:
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Area;

	UPROPERTY(BlueprintReadOnly)
	TArray<FGameplayTagContainer> Cards;

	UPROPERTY(BlueprintReadOnly)
	int32 Capacity;

	FCardAreaSlot(const FGameplayTag& Key) : Area(Key), Cards({}), Capacity(1) {};

public:
	FCardAreaSlot() : Area({}), Cards({}), Capacity(0) {};



	int32 Num() const {return Cards.Num();}

	bool IsFull() const { return Capacity <= Num(); }

	bool IsEmpty() const { return Num() == 0; }

	bool Add(const FGameplayTagContainer& Card) { if (IsFull()) return false; return Cards.Add(Card) != INDEX_NONE; };

	bool Remove(const FGameplayTagContainer& Card) { return Cards.Remove(Card) != INDEX_NONE; };

	const TArray<FGameplayTagContainer>& GetAll() const { return Cards; }

	const FGameplayTag& GetAreaKey() const { return Area; }
};

struct FCardAreaSlotFactory
{
	static FCardAreaSlot CreateEquipmentSlot(const FGameplayTag& Key);

	static FCardAreaSlot CreateJudgementSlot(const FGameplayTagContainer& Card);

	static FCardAreaSlot CreateSpecialSlot(const FGameplayTag& Key, int32 Capacity = 1);
};
