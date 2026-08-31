#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ArkwarTypes.generated.h"

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
	FText OperatorName;
	
	/// 用于标记初始化赋予干员相对应的技能
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer SkillComps;
	
	/// 标记干员的种族
	UPROPERTY(EditAnywhere)
	FGameplayTag Race;

	/// 标记干员所属的势力，一般只取首位
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer Faction;

	/// 干员的初始血量，护盾不在此处记录，计划使用SkillComps的被动效果赋予
	UPROPERTY(EditAnywhere)
	int32 Health;
};

USTRUCT(BlueprintType)
struct FHandCardInfo : public FTableRowBase
{
	GENERATED_BODY()
	
	///手牌的类型标记
	UPROPERTY(EditAnywhere)
	FGameplayTag CardType;
	
	/// 手牌的具体种类标记，与牌名一一对应
	UPROPERTY(EditAnywhere)
	FGameplayTag CardTag;
	
	///手牌的命名
	UPROPERTY(EditAnywhere)
	FText CardName;
	
	/// 卡牌文案（若需要），其中所有数据部分以{0}、{1}、{2}....方式标记，包括纯数字与游戏内变化的数字(X、Y、Z等)
	UPROPERTY(EditAnywhere)
	FText Description;
	
	/// 文案中涉及的数据（若需要）
	UPROPERTY(EditAnywhere)
	TArray<FString> Data;
};

USTRUCT(BlueprintType)
struct FOperatorSkillComponentInfo : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FGameplayTag SkillTag;
	
	/// 技能的触发类型
	UPROPERTY(EditAnywhere)
	TEnumAsByte<ESkillActivateType> SkillType;
	
	/// 技能名
	UPROPERTY(EditAnywhere)
	FText SkillName;
	
	/// 技能文案，其中所有数据部分以{0}、{1}、{2}....方式标记，包括纯数字与游戏内变化的数字(X、Y、Z等)
	UPROPERTY(EditAnywhere)
	FText Description;
	
	/// 文案中涉及的数据
	UPROPERTY(EditAnywhere)
	TArray<FString> Digital;
};


USTRUCT(BlueprintType)
struct FComponentMapping : public FTableRowBase
{
	GENERATED_BODY()

	/// 技能Tag
	UPROPERTY(EditAnywhere)
	FGameplayTag Tag;
	
	/// 技能类
	UPROPERTY(EditAnywhere)
	TSubclassOf<UActorComponent> Comp;
};

using FCardCompMapping = FComponentMapping;
using FSkillCompMapping = FComponentMapping;

namespace EGamePhase
{
	enum Type
	{
		Judgment,			// 判定阶段
		Preparation,		// 准备阶段
		Action,				// 行动阶段
		Finish				// 结束阶段
	};
	
	inline void Roll(Type& Phase)
	{
		Phase = static_cast<Type>((Phase + 1) % 4);
	}
}
