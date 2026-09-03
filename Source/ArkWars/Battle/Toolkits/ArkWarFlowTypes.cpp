// Fill out your copyright notice in the Description page of Project Settings.


#include "ArkWarFlowTypes.h"

namespace PhaseTags
{
	UE_DEFINE_GAMEPLAY_TAG(GameStart,			"Phase.GameStart");
	UE_DEFINE_GAMEPLAY_TAG(Begin,				"Phase.Begin");
	UE_DEFINE_GAMEPLAY_TAG(Preparation_Pre,		"Phase.Preparation_Pre");
	UE_DEFINE_GAMEPLAY_TAG(Preparation,			"Phase.Preparation");
	UE_DEFINE_GAMEPLAY_TAG(Preparation_Post,	"Phase.Preparation_Post");
	UE_DEFINE_GAMEPLAY_TAG(Judgment_Pre,		"Phase.Judgment_Pre");
	UE_DEFINE_GAMEPLAY_TAG(Judgment,			"Phase.Judgment");
	UE_DEFINE_GAMEPLAY_TAG(Judgment_Post,		"Phase.Judgment_Post");
	UE_DEFINE_GAMEPLAY_TAG(Draw_Pre,			"Phase.Draw_Pre");
	UE_DEFINE_GAMEPLAY_TAG(Draw,				"Phase.Draw");
	UE_DEFINE_GAMEPLAY_TAG(Draw_Post,			"Phase.Draw_Post");
	UE_DEFINE_GAMEPLAY_TAG(Action_Pre,			"Phase.Action_Pre");
	UE_DEFINE_GAMEPLAY_TAG(Action,				"Phase.Action");
	UE_DEFINE_GAMEPLAY_TAG(Action_Post,			"Phase.Action_Post");
	UE_DEFINE_GAMEPLAY_TAG(Discard_Pre,			"Phase.Discard_Pre");
	UE_DEFINE_GAMEPLAY_TAG(Discard,				"Phase.Discard");
	UE_DEFINE_GAMEPLAY_TAG(Discard_Post,		"Phase.Discard_Post");
	UE_DEFINE_GAMEPLAY_TAG(Finish,				"Phase.Finish");
}

namespace GamePhase
{
	FGameplayTag GetPhaseTag(EGamePhase Phase)
	{
		switch (Phase)
		{
			case EGamePhase::GameStart:			return PhaseTags::GameStart;
			case EGamePhase::Begin:				return PhaseTags::Begin;
			case EGamePhase::Preparation_Pre:	return PhaseTags::Preparation_Pre;
			case EGamePhase::Preparation:		return PhaseTags::Preparation;
			case EGamePhase::Preparation_Post:	return PhaseTags::Preparation_Post;
			case EGamePhase::Judgment_Pre:		return PhaseTags::Judgment_Pre;
			case EGamePhase::Judgment:			return PhaseTags::Judgment;
			case EGamePhase::Judgment_Post:		return PhaseTags::Judgment_Post;
			case EGamePhase::Draw_Pre:			return PhaseTags::Draw_Pre;
			case EGamePhase::Draw:				return PhaseTags::Draw;
			case EGamePhase::Draw_Post:			return PhaseTags::Draw_Post;
			case EGamePhase::Action_Pre:		return PhaseTags::Action_Pre;
			case EGamePhase::Action:			return PhaseTags::Action;
			case EGamePhase::Action_Post:		return PhaseTags::Action_Post;
			case EGamePhase::Discard_Pre:		return PhaseTags::Discard_Pre;
			case EGamePhase::Discard:			return PhaseTags::Discard;
			case EGamePhase::Discard_Post:		return PhaseTags::Discard_Post;
			case EGamePhase::Finish:			return PhaseTags::Finish;
			default:							return FGameplayTag();
		}
	}
}
