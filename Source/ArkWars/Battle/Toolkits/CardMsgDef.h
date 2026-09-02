#pragma once
#include "GameplayTagContainer.h"
#include "Containers/Map.h"
#include "Containers/UnrealString.h"

#ifndef KEY
#define KEY static constexpr Key 
#endif


namespace CardMessage
{
	///
	///	规则：
	///	Msg 格式为："KEY1=VAL1 KEY2=VAL2 CONDITION:KEY3=VAL3 {KEY4=VAL4 KEY5=VAL5} [{KEY6=VAL6,KEY7=VAL7}]*"
	///	解码时，非判定性KEY用于索引和补填信息，如 "NUM=2 FROM=-P TO=-H" 表示从牌堆摸两张牌至手牌
	///	使用 "CONDITION:" 表示后续KEY为判定性KEY
	///	判定性KEY用于限定非指定移动的卡牌的条件，如 "NUM=1 FROM=-P TO=-H CONDITION:SUIT=/C"表示从牌堆摸一张方片至手牌
	///	组合判定性KEY默认视为且运算，使用 {KEY1=VAL1 KEY2=VAL2} 表示KEY1与KEY2进行或运算
	///	组合判定性KEY使用 [KEY1=VAL1] 标识非运算
	///	e.g. "NUM=5 FROM=-P TO=-S AREA=Skill.Foresee" 表示场上不少于5人时的观星技能
	///	e.g. "NUM=3 CONDITION:CLASS=Card.Class.Kill [SUIT=/H] {MIN=8 MAX=4}*" 表示从牌堆抽取3张花色不为红桃的任意杀，点数在[A,4] ∪ [8,K]范围
	///
	
	using Key = const TCHAR*;
	
	struct FMoveMessage
	{
		KEY	Num			= TEXT("NUM");			//用于非指定移动时，声明移动的数量，通常用于抽牌,不存在则视为1
		KEY	From		= TEXT("FROM");			//用于指定实际的来源容器，如手牌、抽牌堆等,不存在则视为-P
		KEY	To			= TEXT("TO");			//用于指定实际的卡牌去向，如弃牌堆、装备区等,不存在则视为-H
		KEY Hand		= TEXT("-H");			//手牌区
		KEY	Used		= TEXT("-U");			//缓冲堆，使用后的卡牌进入其中，在出牌阶段结束后移动至弃牌堆并不触发弃牌事件
		KEY	Discard		= TEXT("-D");			//弃牌堆
		KEY	Pile		= TEXT("-P");			//抽牌堆
		KEY	Equipment	= TEXT("-E");			//装备区
		KEY	Judgement	= TEXT("-J");			//判定区
		KEY	Cache		= TEXT("-C");			//暂存区
		KEY Special		= TEXT("-S");			//特殊区
		KEY Area		= TEXT("AREA");			//若FROM/TO的参数为-S,需要声明特定区域类型
		KEY Condition	= TEXT("CONDITION:");	//后续KEY为判定性
		KEY End			= TEXT("*");			//存在判定性KEY时，宣言Msg结束
		KEY Class		= TEXT("CLASS");		//用于非指定移动时，用于声明移动牌的类型
		KEY Suit		= TEXT("SUIT");			//用于非指定移动时，用于声明移动牌的花色
		KEY Max			= TEXT("MAX");			//用于非指定移动时，用于声明移动牌的最大点数，只允许使用数字
		KEY Min			= TEXT("MIN");			//用于非指定移动时，用于声明移动牌的最小点数，只允许使用数字
		KEY Spade		= TEXT("/S");			//黑桃
		KEY Heart		= TEXT("/H");			//红心
		KEY Diamond		= TEXT("/D");			//方片
		KEY Club		= TEXT("/C");			//草花
		
	private:
		static const TMap<FString, FGameplayTag> DefaultArea;
		static const TMap<FString, FGameplayTag> SuitMap;
	public:
		TFunction<bool(const FGameplayTagContainer&)> Predicate;
		FGameplayTag FromArea;
		FGameplayTag ToArea;
		int32 Count;
		bool bConsiderAsDiscard;
		
		FMoveMessage(const FString& Msg);
		
		bool operator()(const FGameplayTagContainer& Card) const;
		
		TArray<FGameplayTagContainer> operator()(const TArray<FGameplayTagContainer>& Cards) const;
	};

	struct Phase
	{
		
	};
}

#ifdef KEY
	#undef KEY
#endif