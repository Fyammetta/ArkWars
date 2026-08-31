#pragma once
#include "GameplayTagContainer.h"
#include "Containers/UnrealString.h"

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
	#define OPERATOR(Operator,Index)	FGameplayTag::RequestGameplayTag("Skill."#Operator"."#Index)
#endif

#ifndef CARD
	#define  CARD(Card)	FGameplayTag::RequestGameplayTag("Card.Class"#Card)
#endif



#pragma  endregion

namespace CardTags
{
	REGISTER_TAG(Root, Card)
	REGISTER_TAG(Class, Card.Class)
	REGISTER_TAG(Type, Card.Type)
	REGISTER_TAG(Diamond, Card.Suit.Diamond)
	REGISTER_TAG(Heart, Card.Suit.Heart)
	REGISTER_TAG(Club, Card.Suit.Club)
	REGISTER_TAG(Spade, Card.Suit.Spade)
	REGISTER_TAG(Point, Card.Point)
	
	namespace CardPoint
	{
		template<typename FString::ElementType T, bool Cond>
		struct TIsLegalPoint
		{
			static_assert(Cond, "TIsLegalPoint: Point is illegal");
		};
		template<wchar_t T>
		struct TIsLegalPoint<T,true>
		{
			constexpr static typename FString::ElementType Value = T;
		};
	}
	
	template<typename FString::ElementType T>
	const FGameplayTag& Point()
	{
		static FGameplayTag Point_V = 
			FGameplayTag::RequestGameplayTag(FName(FString::Printf(TEXT("Card.Point.%c"),
				CardPoint::TIsLegalPoint<T, T == 'K' || T == 'Q' || T == 'J' || T == 'A' || (static_cast<int>(T) >=2 && static_cast<int>(T)<=10) >::Value)));
		return Point_V;
	}

	
}

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