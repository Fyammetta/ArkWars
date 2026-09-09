#include "ArkWarCardTypes.h"

FCardAreaSlot FCardAreaSlotFactory::CreateEquipmentSlot(const FGameplayTag& Key)
{
	FCardAreaSlot Slot(Key);
	
	
	return Slot;
}

FCardAreaSlot FCardAreaSlotFactory::CreateJudgementSlot(const FArkCard& Card)
{
	FCardAreaSlot Slot(Card.Card.First());	
	Slot.Add(Card);
	return Slot;
}

FCardAreaSlot FCardAreaSlotFactory::CreateSpecialSlot(const FGameplayTag& Key, int32 Capacity)
{
	FCardAreaSlot Slot(Key);
	
	Slot.Capacity = Capacity;
	
	return Slot;
}
