#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "ArkWarSkillTypes.generated.h"

class USkillComponentBase;
class APlayerState;

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

	/// 技能文案，其中所有数据部分以{0}、{1}、{2}....方式标记，包括纯数字与游戏内变化的数据(X、Y、Z等)
	UPROPERTY(EditAnywhere)
	FText Description = FText::GetEmpty();

	/// 文案中涉及的数据
	UPROPERTY(EditAnywhere)
	TArray<FString> Data = {};
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

struct FSkillListenerEntry
{
	TWeakObjectPtr<USkillComponentBase> Skill;

	TWeakObjectPtr<APlayerState> Owner;

	///	响应优先级：开窗名单排序主键（降序，卷 12 §4.1 ③），登记时由技能侧带入
	int32 Priority = 0;

	bool operator==(const FSkillListenerEntry& Other) const
	{
		return Skill == Other.Skill;
	}
	
	bool IsValid() const
	{
		return Skill.IsValid() && Owner.IsValid();
	}
};