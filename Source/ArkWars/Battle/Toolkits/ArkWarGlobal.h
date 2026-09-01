#pragma once
#include "ArkWarTypes.h"


#ifndef PHASE_PAIR
#define PHASE_PAIR(Phase)	\
	{EGamePhase::Phase,  FGameplayTag::RequestGameplayTag("Phase."#Phase)}
#endif

namespace GamePhase
{
	const static TMap<EGamePhase, FGameplayTag> GamePhaseToTagMap = 
	{
		PHASE_PAIR(Begin),
		PHASE_PAIR(Preparation_Pre),
		PHASE_PAIR(Preparation),
		PHASE_PAIR(Preparation_Post),
		PHASE_PAIR(Judgment_Pre),
		PHASE_PAIR(Judgment),
		PHASE_PAIR(Judgment_Post),
		PHASE_PAIR(Draw_Pre),
		PHASE_PAIR(Draw),
		PHASE_PAIR(Draw_Post),
		PHASE_PAIR(Action_Pre),
		PHASE_PAIR(Action),
		PHASE_PAIR(Action_Post),
		PHASE_PAIR(Discard_Pre),
		PHASE_PAIR(Discard),
		PHASE_PAIR(Discard_Post),
		PHASE_PAIR(Finish)
	};
}

#undef PHASE_PAIR