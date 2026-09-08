#include "EventTags.h"

namespace Timing
{
	namespace Phase
	{
		UE_DEFINE_GAMEPLAY_TAG(GameStart,				"Event.Timing.Phase.GameStart.Start")
		UE_DEFINE_GAMEPLAY_TAG(Begin,					"Event.Timing.Phase.Begin.Start")
		UE_DEFINE_GAMEPLAY_TAG(Preparation_Pre,			"Event.Timing.Phase.Preparation.Pre")
		UE_DEFINE_GAMEPLAY_TAG(Preparation,				"Event.Timing.Phase.Preparation.Start")
		UE_DEFINE_GAMEPLAY_TAG(Preparation_Post,		"Event.Timing.Phase.Preparation.Post")
		UE_DEFINE_GAMEPLAY_TAG(Judgment_Pre,			"Event.Timing.Phase.Judgment.Pre")
		UE_DEFINE_GAMEPLAY_TAG(Judgment,				"Event.Timing.Phase.Judgment.Start")
		UE_DEFINE_GAMEPLAY_TAG(Judgment_Post,			"Event.Timing.Phase.Judgment.Post")
		UE_DEFINE_GAMEPLAY_TAG(Draw_Pre,				"Event.Timing.Phase.Draw.Pre")
		UE_DEFINE_GAMEPLAY_TAG(Draw,					"Event.Timing.Phase.Draw.Start")
		UE_DEFINE_GAMEPLAY_TAG(Draw_Post,				"Event.Timing.Phase.Draw.Post")
		UE_DEFINE_GAMEPLAY_TAG(Action_Pre,				"Event.Timing.Phase.Action.Pre")
		UE_DEFINE_GAMEPLAY_TAG(Action,					"Event.Timing.Phase.Action.Start")
		UE_DEFINE_GAMEPLAY_TAG(Action_Post,				"Event.Timing.Phase.Action.Post")
		UE_DEFINE_GAMEPLAY_TAG(Discard_Pre,				"Event.Timing.Phase.Discard.Pre")
		UE_DEFINE_GAMEPLAY_TAG(Discard,					"Event.Timing.Phase.Discard.Start")
		UE_DEFINE_GAMEPLAY_TAG(Discard_Post,			"Event.Timing.Phase.Discard.Post")
		UE_DEFINE_GAMEPLAY_TAG(Finish,					"Event.Timing.Phase.Finish.Start")
	}
	namespace Card
	{
		UE_DEFINE_GAMEPLAY_TAG(PreMove,					"Event.Timing.Card.PreMove")
		UE_DEFINE_GAMEPLAY_TAG(MoveIn,					"Event.Timing.Card.MoveIn")
		UE_DEFINE_GAMEPLAY_TAG(MoveOut,					"Event.Timing.Card.MoveOut")
		
		UE_DEFINE_GAMEPLAY_TAG(Play,					"Event.Timing.PlayCard.Play")
		UE_DEFINE_GAMEPLAY_TAG(Use,						"Event.Timing.PlayCard.Use")
	}
	namespace Damage
	{
		UE_DEFINE_GAMEPLAY_TAG(PreHurt,					"Event.Timing.Damage.PreHurt")
		UE_DEFINE_GAMEPLAY_TAG(Dealt,					"Event.Timing.Damage.Dealt")
		UE_DEFINE_GAMEPLAY_TAG(Dying,					"Event.Timing.Dying")
	}
}

namespace Notify
{
	namespace Phase
	{
		UE_DEFINE_GAMEPLAY_TAG(Entered,					"Event.Notify.Phase.Entered")
		UE_DEFINE_GAMEPLAY_TAG(Exited,					"Event.Notify.Phase.Exited")
	}
	namespace Card
	{
		UE_DEFINE_GAMEPLAY_TAG(Moved,					"Event.Notify.Card.Moved")
		UE_DEFINE_GAMEPLAY_TAG(MovedIn,					"Event.Notify.Card.MovedIn")
		UE_DEFINE_GAMEPLAY_TAG(MovedOut,				"Event.Notify.Card.MovedOut")
		
		UE_DEFINE_GAMEPLAY_TAG(Played,					"Event.Notify.PlayCard.Played")
		UE_DEFINE_GAMEPLAY_TAG(Used,					"Event.Notify.PlayCard.Used")
		UE_DEFINE_GAMEPLAY_TAG(Nullified,				"Event.Notify.PlayCard.Nullified")
	}
	namespace Damage
	{
		UE_DEFINE_GAMEPLAY_TAG(Applied,					"Event.Notify.Damage.Applied")
		UE_DEFINE_GAMEPLAY_TAG(Prevented,				"Event.Notify.Damage.Prevented")
		UE_DEFINE_GAMEPLAY_TAG(Dead,					"Event.Notify.Dying.Dead")
		UE_DEFINE_GAMEPLAY_TAG(Saved,					"Event.Notify.Dying.Saved")
	}
}