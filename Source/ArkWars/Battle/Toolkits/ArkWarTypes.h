#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ArkwarTypes.generated.h"

class USkillComponentBase;
class UGameplayAbility;
class UCardComponentBase;
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

UENUM(BlueprintType)
enum EPlayerIdentity : uint8
{
	///	=============== 主公 ===============
	Commander	UMETA(DisplayName = "指挥"),	
	///	=============== 忠臣 ===============
	Operator	UMETA(DisplayName = "干员"),
	///	=============== 内奸 ===============
	Spy			UMETA(DisplayName = "间谍"),
	///	=============== 反贼 ===============
	Raider		UMETA(DisplayName = "敌人")
};

// UENUM(BlueprintType)
// enum EHandCardType : uint8
// {
// 	///	=============== 基础牌 ===============
// 	Basic		UMETA(DisplayName = "基本牌"),
// 	///	=============== 锦囊牌 ===============
// 	Effect		UMETA(DisplayName = "效果牌"),	
// 	///	=============== 装备牌 ===============
// 	Item		UMETA(DisplayName = "道具牌"),	
// };

UENUM(BlueprintType)
enum ESkillActivateType : uint8
{
	///	=============== 主动技 ===============
	Proactive	UMETA(DisplayName = "主动技"),
	///	=============== 被动技 ===============
	Passive		UMETA(DisplayName = "被动技"),
	///	=============== 锁定技 ===============
	Locking		UMETA(DisplayName = "锁定技"),
	///	=============== 限定技 ===============
	Limited		UMETA(DisplayName = "限定技"),
	///	=============== 转换技 ===============
	Strategy	UMETA(DisplayName = "策略技"),
	///	=============== 主公技 ===============
	Command		UMETA(DisplayName = "指挥技")
};

USTRUCT(BlueprintType)
struct FOperatorCardInfo : public FTableRowBase
{
	GENERATED_BODY()
	
	///干员名
	UPROPERTY(EditAnywhere)
	FText OperatorName = FText::GetEmpty();
	
	/// 用于标记初始化赋予干员相对应的技能
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer SkillComps = {};
	
	/// 标记干员的种族
	UPROPERTY(EditAnywhere)
	FGameplayTag Race = {};

	/// 标记干员所属的势力，一般只取首位
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer Faction = {};

	/// 干员的初始血量，护盾不在此处记录，计划使用SkillComps的被动效果赋予
	UPROPERTY(EditAnywhere)
	int32 Health = 6;
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
	
	/// 卡牌文案（若需要），其中所有数据部分以{0}、{1}、{2}....方式标记，包括纯数字与游戏内变化的数字(X、Y、Z等)
	UPROPERTY(EditAnywhere)
	FText Description = FText::GetEmpty();
	
	/// 文案中涉及的数据（若需要）
	UPROPERTY(EditAnywhere)
	TArray<FString> Data = {};
};

USTRUCT(BlueprintType)
struct FOperatorSkillInfo : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FGameplayTag SkillTag = {};
	
	/// 技能的触发类型
	UPROPERTY(EditAnywhere)
	TEnumAsByte<ESkillActivateType> SkillType = Proactive;
	
	/// 技能名
	UPROPERTY(EditAnywhere)
	FText SkillName = FText::GetEmpty();
	
	/// 技能文案，其中所有数据部分以{0}、{1}、{2}....方式标记，包括纯数字与游戏内变化的数字(X、Y、Z等)
	UPROPERTY(EditAnywhere)
	FText Description = FText::GetEmpty();
	
	/// 文案中涉及的数据
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
struct FSkillComponentMapping : public FTableRowBase
{
	GENERATED_BODY()

	/// 技能Tag
	UPROPERTY(EditAnywhere)
	FGameplayTag Tag;
	
	/// 技能组件类
	UPROPERTY(EditAnywhere)
	TSubclassOf<USkillComponentBase> Comp;
};

USTRUCT(BlueprintType)
struct FInfoWithData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	FText Description;
	
	UPROPERTY(BlueprintReadOnly)
	TArray<FString> Data;
		
	TDelegate<int32(const FString&)> DataGetter;
	
	TArray<FFormatArgumentData> GetNumericalDatas()
	{
		TArray<FFormatArgumentData> RetVal;
		FFormatArgumentData Arg;

		for (const FString& String : Data)
		{
			Arg.ArgumentName = FString::Printf(TEXT("{%d}"), Data.Find(String));
			Arg.ArgumentValueType = EFormatArgumentType::Int;
			if (String.IsNumeric())
			{
				Arg.ArgumentValueInt = FCString::Atoi64(*String);
			}
			else if (DataGetter.IsBound())
			{
				Arg.ArgumentValueInt = DataGetter.Execute(String);
			}
			else
			{
				Arg.ArgumentValueInt = 0;
			}
		
			RetVal.Add(MoveTemp(Arg));
		}
	
		return RetVal;
	};
	
	virtual ~FInfoWithData() {}
};

UENUM(BlueprintType)
enum class EGamePhase: uint8
{
		GameStart = 0,
		Begin,				// 回合开始阶段
		Preparation_Pre,	// 准备阶段前
		Preparation,		// 准备阶段开始
		Preparation_Post,	// 准备阶段结束
		Judgment_Pre,		// 判定阶段前
		Judgment,			// 判定阶段开始
		Judgment_Post,		// 判定阶段结束
		Draw_Pre,			// 摸牌阶段前
		Draw,				// 摸牌阶段开始
		Draw_Post,			// 摸牌阶段结束
		Action_Pre,			// 行动阶段前
		Action,				// 行动阶段开始
		Action_Post,		// 行动阶段结束
		Discard_Pre,		// 弃牌阶段前
		Discard,			// 弃牌阶段开始
		Discard_Post,		// 弃牌阶段结束
		Finish				// 结束阶段
};

USTRUCT(BlueprintType)
struct FCardAreaSlot
{
	GENERATED_BODY()
	
	friend class FCardAreaSlotFactory;
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

class FCardAreaSlotFactory
{
	static FCardAreaSlot CreateEquipmentSlot(const FGameplayTag& Key);
	
	static FCardAreaSlot CreateJudgementSlot(const FGameplayTagContainer& Card);
	
	static FCardAreaSlot CreateSpecialSlot(const FGameplayTag& Key, int32 Capacity);
};