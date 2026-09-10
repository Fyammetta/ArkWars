#include "ArkWarCardTypes.h"

#include "ArkWars/Battle/Transaction/EventTags.h"

FCardAreaSlot FCardAreaSlotFactory::CreateEquipmentSlot(const FGameplayTag& Key)
{
	//装备槽：容量沿用默认 1，起始无牌
	FCardAreaSlot Slot(Key);
	
	
	return Slot;
}

FCardAreaSlot FCardAreaSlotFactory::CreateJudgementSlot(const FArkCard& Card)
{
	//判定槽：槽键取牌面首个标签，并把这张判定牌直接入槽
	FCardAreaSlot Slot(Card.GetClass());	
	Slot.Add(Card);
	return Slot;
}

FCardAreaSlot FCardAreaSlotFactory::CreateSpecialSlot(const FGameplayTag& Key, int32 Capacity)
{
	//通用槽：按调用方给定容量构造
	FCardAreaSlot Slot(Key);
	
	//Capacity 为 protected，仅工厂可写
	Slot.Capacity = Capacity;
	
	return Slot;
}


#ifndef REGISTER_CARD_SLOT
	#define REGISTER_CARD_SLOT(Tag, Slot)	if (Key == Tag) return Slot;
#endif

int32 CardAreaSlot::GetJudgementSlot(const FGameplayTag& Key)
{
	//TODO: 映射表待填，落表时按 REGISTER_CARD_SLOT(CardTags::Xxx, 槽下标) 逐条登记
	return INDEX_NONE;
}

int32 CardAreaSlot::GetEquipmentSlot(const FGameplayTag& Key)
{
	//TODO: 同 GetJudgementSlot，映射表待填
	
	return INDEX_NONE;
}

#undef REGISTER_CARD_SLOT