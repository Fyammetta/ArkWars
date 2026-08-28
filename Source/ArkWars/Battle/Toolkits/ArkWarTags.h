#pragma once
#include "GameplayTagContainer.h"

#ifndef REGISTER_TAG
	#define REGISTER_TAG(Getter,Tag)											\
		inline const FGameplayTag& Getter() {									\
		static FGameplayTag T = FGameplayTag::RequestGameplayTag(#Tag);			\
		return T;}
#endif

#pragma region TagMacro
#ifndef RACE
	#define RACE(Tag)	FGameplayTag::RequestGameplayTag("Race."#Tag)
#endif

#ifndef SKILL
	#define OPERATOR(Operator,Index)	FGameplayTag::RequestGameplayTag("Race."#Operator"."#Index)
#endif

#ifndef Card
	#define OPERATOR(Card)	FGameplayTag::RequestGameplayTag("Card."#Card)
#endif
#pragma  endregion

namespace GameModeTags
{
	REGISTER_TAG(Default, Gamemode.Default)
}

namespace IdentityTags
{
	REGISTER_TAG(Commander, Identity.Commander)
	REGISTER_TAG(Operator, Identity.Operator)
	REGISTER_TAG(Spy, Identity.Spy)
	REGISTER_TAG(Raider, Identity.Raider)
}

#undef REGISTER_TAG