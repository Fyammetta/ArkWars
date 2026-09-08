#pragma once
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"

namespace Timing
{
	namespace Phase
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
	namespace Card
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(PreMove)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(MoveIn)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(MoveOut)
		
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Play)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Use)
	}
	namespace Damage
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(PreHurt)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dealt)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dying)
	}
}

namespace Notify
{
	namespace Phase
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Entered)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Exited)
	}
	namespace Card
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Moved)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(MovedIn)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(MovedOut)
		
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Played)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Used)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Nullified)
	}
	namespace Damage
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Applied)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Prevented)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dead)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Saved)
	}
}