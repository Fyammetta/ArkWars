#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"
#include "ArkWarFlowTypes.generated.h"

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

namespace GamePhase
{
	/// 回合循环内的阶段总数（不含 GameStart），供轮转取模使用
	constexpr int32 PhaseCount = static_cast<int32>(EGamePhase::Finish);

	/// 由阶段枚举取对应的原生标签，非法输入返回无效标签
	ARKWARS_API FGameplayTag GetPhaseTag(EGamePhase Phase);
}

/// 阶段标签（原生），字符串与旧版 "Phase.*" 保持一致
namespace PhaseTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameStart)			// Phase.GameStart
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Begin)				// Phase.Begin
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Preparation_Pre)		// Phase.Preparation_Pre
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Preparation)			// Phase.Preparation
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Preparation_Post)	// Phase.Preparation_Post
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Judgment_Pre)		// Phase.Judgment_Pre
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Judgment)			// Phase.Judgment
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Judgment_Post)		// Phase.Judgment_Post
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Draw_Pre)			// Phase.Draw_Pre
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Draw)				// Phase.Draw
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Draw_Post)			// Phase.Draw_Post
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Pre)			// Phase.Action_Pre
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action)				// Phase.Action
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Post)			// Phase.Action_Post
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Discard_Pre)			// Phase.Discard_Pre
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Discard)				// Phase.Discard
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Discard_Post)		// Phase.Discard_Post
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Finish)				// Phase.Finish
}
