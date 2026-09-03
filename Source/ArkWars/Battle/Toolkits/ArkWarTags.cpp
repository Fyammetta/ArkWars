// Fill out your copyright notice in the Description page of Project Settings.


#include "ArkWarTags.h"

namespace SkillTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, "Skill");
}

namespace CardTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, "Card");
	UE_DEFINE_GAMEPLAY_TAG(Class, "Card.Class");
	UE_DEFINE_GAMEPLAY_TAG(Type, "Card.Type");
	UE_DEFINE_GAMEPLAY_TAG(Suit, "Card.Suit");
	UE_DEFINE_GAMEPLAY_TAG(Diamond, "Card.Suit.Diamond");
	UE_DEFINE_GAMEPLAY_TAG(Heart, "Card.Suit.Heart");
	UE_DEFINE_GAMEPLAY_TAG(Club, "Card.Suit.Club");
	UE_DEFINE_GAMEPLAY_TAG(Spade, "Card.Suit.Spade");
	UE_DEFINE_GAMEPLAY_TAG(Point, "Card.Point");
	UE_DEFINE_GAMEPLAY_TAG(PointA, "Card.Point.A");
	UE_DEFINE_GAMEPLAY_TAG(Point2, "Card.Point.2");
	UE_DEFINE_GAMEPLAY_TAG(Point3, "Card.Point.3");
	UE_DEFINE_GAMEPLAY_TAG(Point4, "Card.Point.4");
	UE_DEFINE_GAMEPLAY_TAG(Point5, "Card.Point.5");
	UE_DEFINE_GAMEPLAY_TAG(Point6, "Card.Point.6");
	UE_DEFINE_GAMEPLAY_TAG(Point7, "Card.Point.7");
	UE_DEFINE_GAMEPLAY_TAG(Point8, "Card.Point.8");
	UE_DEFINE_GAMEPLAY_TAG(Point9, "Card.Point.9");
	UE_DEFINE_GAMEPLAY_TAG(PointX, "Card.Point.X");
	UE_DEFINE_GAMEPLAY_TAG(PointJ, "Card.Point.J");
	UE_DEFINE_GAMEPLAY_TAG(PointQ, "Card.Point.Q");
	UE_DEFINE_GAMEPLAY_TAG(PointK, "Card.Point.K");
	UE_DEFINE_GAMEPLAY_TAG(Pile, "Card.Area.Pile");
	UE_DEFINE_GAMEPLAY_TAG(Discard, "Card.Area.Discard");
	UE_DEFINE_GAMEPLAY_TAG(Hand, "Card.Area.Hand");
	UE_DEFINE_GAMEPLAY_TAG(Judgement, "Card.Area.Judgement");
	UE_DEFINE_GAMEPLAY_TAG(Equipment, "Card.Area.Equipment");
	UE_DEFINE_GAMEPLAY_TAG(Cache, "Card.Area.Cache");
	UE_DEFINE_GAMEPLAY_TAG(Used, "Card.Area.Used");

	FGameplayTag GetPointTag(TCHAR PointChar)
	{
		switch (PointChar)
		{
			case 'A':	return PointA;
			case '2':	return Point2;
			case '3':	return Point3;
			case '4':	return Point4;
			case '5':	return Point5;
			case '6':	return Point6;
			case '7':	return Point7;
			case '8':	return Point8;
			case '9':	return Point9;
			case 'X':	return PointX;
			case 'J':	return PointJ;
			case 'Q':	return PointQ;
			case 'K':	return PointK;
			default:	return FGameplayTag();
		}
	}

	FGameplayTag GetPointTag(int32 PointValue)
	{
		switch (PointValue)
		{
			case 1:		return PointA;
			case 2:		return Point2;
			case 3:		return Point3;
			case 4:		return Point4;
			case 5:		return Point5;
			case 6:		return Point6;
			case 7:		return Point7;
			case 8:		return Point8;
			case 9:		return Point9;
			case 10:	return PointX;
			case 11:	return PointJ;
			case 12:	return PointQ;
			case 13:	return PointK;
			default:	return FGameplayTag();
		}
	}
}

namespace GameModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Default, "Gamemode.Default");
}

namespace VoiceTags
{
	UE_DEFINE_GAMEPLAY_TAG(Root, "Voice.Event");
	UE_DEFINE_GAMEPLAY_TAG(Select, "Voice.Event.Select");
	UE_DEFINE_GAMEPLAY_TAG(CardPlay, "Voice.Event.CardPlay");
	UE_DEFINE_GAMEPLAY_TAG(Damage, "Voice.Event.Damage");
	UE_DEFINE_GAMEPLAY_TAG(Dying, "Voice.Event.Dying");
	UE_DEFINE_GAMEPLAY_TAG(Death, "Voice.Event.Death");
	UE_DEFINE_GAMEPLAY_TAG(Victory, "Voice.Event.Victory");
}

namespace IdentityTags
{
	UE_DEFINE_GAMEPLAY_TAG(Commander, "Identity.Commander");
	UE_DEFINE_GAMEPLAY_TAG(Operator, "Identity.Operator");
	UE_DEFINE_GAMEPLAY_TAG(Spy, "Identity.Spy");
	UE_DEFINE_GAMEPLAY_TAG(Raider, "Identity.Raider");
}
