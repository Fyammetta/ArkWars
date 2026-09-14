#pragma once
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"

// ---------------------------------------------------------------------------
//	动态标签宏：仅允许在运行期、由数据驱动的场合调用（表行、Msg 解析等）。
//	凡是静态已知的标签，一律使用下方声明的原生标签（UE_DEFINE_GAMEPLAY_TAG），
//	避免在静态初始化期调用 RequestGameplayTag。
// ---------------------------------------------------------------------------
#ifndef RACE
	#define RACE(Tag)	FGameplayTag::RequestGameplayTag("Race."#Tag)
#endif

#ifndef SKILL
	#define SKILL(Operator,Index)	FGameplayTag::RequestGameplayTag("Skill."#Operator"."#Index)
#endif

#ifndef CARD
	#define  CARD(Card)	FGameplayTag::RequestGameplayTag("Card.Class."#Card)
#endif


namespace SkillTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Root)				// Skill
}


namespace CardTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Root)				// Card
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Class)				// Card.Class
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Type)				// Card.Type
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(JudgeType)			// Card.Type.Judge
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(EquipType)			// Card.Type.Equip
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Suit)				// Card.Suit
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Diamond)				// Card.Suit.Diamond
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Heart)				// Card.Suit.Heart
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Club)				// Card.Suit.Club
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Spade)				// Card.Suit.Spade
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Point)				// Card.Point
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(PointA)				// Card.Point.A
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Point2)				// Card.Point.2
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Point3)				// Card.Point.3
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Point4)				// Card.Point.4
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Point5)				// Card.Point.5
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Point6)				// Card.Point.6
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Point7)				// Card.Point.7
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Point8)				// Card.Point.8
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Point9)				// Card.Point.9
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Point10)				// Card.Point.X
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(PointJ)				// Card.Point.J
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(PointQ)				// Card.Point.Q
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(PointK)				// Card.Point.K
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pile)				// Card.Area.Pile
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Discard)				// Card.Area.Discard
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Hand)				// Card.Area.Hand
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Judgement)			// Card.Area.Judgement
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment)			// Card.Area.Equipment
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cache)				// Card.Area.Cache
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Used)				// Card.Area.Used
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Deprecated)			// Card.Area.Deprecated

	/// 运行期按字符取点数标签（'A','2'..'9','X','J','Q','K'），非法输入返回无效标签
	ARKWARS_API FGameplayTag GetPointTag(TCHAR PointChar);

	/// 运行期按数值取点数标签（1..13），非法输入返回无效标签
	ARKWARS_API FGameplayTag GetPointTag(int32 PointValue);
}

namespace GameModeTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Default)				// Gamemode.Default
}

/// 语音事件标签：作为干员表中 VoiceLines 的键；技能语音直接使用技能标签，不走此列
namespace VoiceTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Root)				// Voice.Event
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Select)				// Voice.Event.Select		选中/部署
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(CardPlay)			// Voice.Event.CardPlay		出牌
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage)				// Voice.Event.Damage		受到伤害
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dying)				// Voice.Event.Dying		濒死
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Death)				// Voice.Event.Death		阵亡
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Victory)				// Voice.Event.Victory		胜利
}

namespace IdentityTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Commander)			// Identity.Commander
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Operator)			// Identity.Operator
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Spy)					// Identity.Spy
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Raider)				// Identity.Raider
}
