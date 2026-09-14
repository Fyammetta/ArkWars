#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "GameplayTagContainer.h"
#include "Sound/SoundBase.h"
#include "ArkWarOperatorTypes.generated.h"

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

USTRUCT(BlueprintType)
struct FOperatorCardInfo : public FTableRowBase
{
	GENERATED_BODY()

	///干员名
	UPROPERTY(EditAnywhere)
	FText OperatorName = FText::GetEmpty();

	/// 角色卡面贴图（软引用，按需加载）
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UTexture2D> CardTexture = {};

	/// 角色语音：事件标签 → 语音资产（软引用，按需加载）
	/// 键的约定：通用时机用 Voice.Event.* 标签（如选中、受击、阵亡），技能语音直接用对应的技能标签
	UPROPERTY(EditAnywhere)
	TMap<FGameplayTag, TSoftObjectPtr<USoundBase>> VoiceLines = {};

	/// 用于标记初始化赋予干员相对应的技能
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer SkillComps = {};

	/// 标记干员的种族
	UPROPERTY(EditAnywhere)
	FGameplayTag Race = {};

	/// 标记干员所属的势力，一般只取首位
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer Faction = {};

	/// 干员的初始血量
	UPROPERTY(EditAnywhere)
	int32 Health = 6;
	
	/// 干员的初始血量上限
	UPROPERTY(EditAnywhere)
	int32 MaxHealth = 6;
	
	/// 干员的初始护盾
	UPROPERTY(EditAnywhere)
	int32 Shield = 0;
};
