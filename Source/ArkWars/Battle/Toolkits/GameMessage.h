#pragma once
#include "ArkWarFlowTypes.h"
#include "GameplayTagContainer.h"
#include "Containers/Map.h"
#include "Containers/UnrealString.h"

#ifndef KEY
#define KEY static constexpr Key 
#endif


struct FArkCard;

namespace GameMessage
{
	using Key = const TCHAR*;
	
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
	struct FMoveMessage
	{
		enum class EOrder
		{
			Top,
			Bottom,
			Random
		};
#pragma region Keys
		KEY	Num			= TEXT("NUM");			//用于非指定移动时，声明移动的数量，通常用于抽牌,不存在则视为1
		KEY	From		= TEXT("FROM");			//用于指定实际的来源容器，如手牌、抽牌堆等,不存在则视为-P
		KEY	To			= TEXT("TO");			//用于指定实际的卡牌去向，如弃牌堆、装备区等,不存在则视为-H
		KEY IsDiscard	= TEXT("DISCARD");		//若存在且值不为0，则被视为弃牌
		KEY Hand		= TEXT("-AH");			//手牌区
		KEY	Used		= TEXT("-AU");			//缓冲堆
		KEY	Discard		= TEXT("-AD");			//弃牌堆
		KEY	Pile		= TEXT("-AP");			//抽牌堆
		KEY	Equipment	= TEXT("-AE");			//装备区
		KEY	Judgement	= TEXT("-AJ");			//判定区
		KEY	Cache		= TEXT("-AC");			//暂存区
		KEY Special		= TEXT("-AS");			//特殊区
		KEY Area		= TEXT("AREA");			//若FROM/TO的参数为-S,需要声明特定区域类型
		KEY Condition	= TEXT("CONDITION:");	//后续KEY为判定性
		KEY End			= TEXT("*");			//存在判定性KEY时，宣言Msg结束
		KEY Class		= TEXT("CLASS");		//用于非指定移动时，用于声明移动牌的类型
		KEY Suit		= TEXT("SUIT");			//用于非指定移动时，用于声明移动牌的花色
		KEY Max			= TEXT("MAX");			//用于非指定移动时，用于声明移动牌的最大点数，只允许使用数字
		KEY Min			= TEXT("MIN");			//用于非指定移动时，用于声明移动牌的最小点数，只允许使用数字
		KEY Spade		= TEXT("-SS");			//黑桃
		KEY Heart		= TEXT("-SH");			//红心
		KEY Diamond		= TEXT("-SD");			//方片
		KEY Club		= TEXT("-SC");			//草花
		KEY Order		= TEXT("ORDER");		//获取顺序，若不存在则默认为Top
		KEY Top			= TEXT("-OT");			//从容器栈顶开始
		KEY Random		= TEXT("-OR");			//每次随机从容器中获得一张
		KEY Bottom		= TEXT("-OB");			//从容器栈底开始
		KEY Meta		= TEXT("META");			//后续的部分不会被解码，直到出现' '或'\0'，可用于各自约定的Message定义
#pragma endregion
	private:
		//	静态表延迟到首次调用时构造，避免在静态初始化期触碰标签
		static const TMap<FString, FGameplayTag>& GetDefaultAreaMap();
		static const TMap<FString, FGameplayTag>& GetSuitMap();
	public:
		FString _Meta;
		int32 _Count;
		bool _bDiscard;
		EOrder _Order;
		FGameplayTag _From;
		FGameplayTag _To;
		
		TFunction<bool(const FArkCard&)> Predicate;

		FMoveMessage(const FString& Msg);
		
		bool operator()(const FArkCard& Card) const;
		
		TArray<FArkCard> operator()(const TArray<FArkCard>& Cards) const;
		
		bool IsValid() const;
	};
	
	///
	///	规则：
	///	Msg 格式为："KEY1=VAL1 KEY2=VAL2 KEY3=VAL3"
	///	无判定性KEY，当操作需要额外参数时，必须填入，否则无视
	///	存在PHASE=/PLAYER=时，只在特定阶段/特定玩家处触发
	///
	struct FPhaseMessage
	{
		enum class EOperation
		{
			Continue,
			Skip,
			Extra,
			Jump
		};
		
		static constexpr EGamePhase AnyPhase = EGamePhase::GameStart;
#pragma region Keys
		KEY Phase		= TEXT("PHASE");		//允许执行的阶段
		KEY Player		= TEXT("PLAYER");		//允许执行的玩家
		KEY Operation	= TEXT("OP");			//需要执行的操作，默认为-C
		KEY Continue	= TEXT("-C");			//继续
		KEY Skip		= TEXT("-S");			//跳过特定阶段，直接进入下一个阶段
		KEY Extra		= TEXT("-E");			//获得额外阶段
		KEY Jump		= TEXT("-J");			//跳至特定阶段
		KEY Target		= TEXT("TARGET");		//若为Jump/Extra,需要写入相应的阶段
		KEY Begin		= TEXT("-B");			//开始阶段
		KEY Prepare		= TEXT("-P");			//准备阶段
		KEY Judgement	= TEXT("-J");			//判定阶段
		KEY Draw		= TEXT("-D");			//摸牌阶段
		KEY Action		= TEXT("-A");			//行动阶段
		KEY Discard		= TEXT("-L");			//弃牌阶段
		KEY Finish		= TEXT("-F");			//结束阶段
		
#pragma endregion
		EOperation _Op;
		EGamePhase _Target;
		EGamePhase _Phase;		// GameStart 表示不设阶段限制
		int32 _Player;			// INDEX_NONE 表示不设玩家限制
		FPhaseMessage() : _Op(EOperation::Continue), _Target(EGamePhase::GameStart), _Phase(EGamePhase::GameStart), _Player(INDEX_NONE) {};

		FPhaseMessage(const FString& Msg);

		/**
		 *	判定本指令此刻是否允许执行
		 *	阶段判定：_Phase 为 GameStart 则不限；否则仅当 Current 的下一阶段恰为 _Phase 对应的 Pre 阶段时通过
		 *	玩家判定：_Player 为 INDEX_NONE 则不限；否则须与当前活跃玩家一致
		 *	@param Current		当前所处阶段
		 *	@param ActivePlayer	当前活跃玩家的座次
		 *	@return				两者皆通过时返回真
		 */
		bool CanExecute(EGamePhase Current, int32 ActivePlayer) const;
	private:
		//	静态表延迟到首次调用时构造，仿照 FMoveMessage 的形态
		static const TMap<FString, EGamePhase>& GetPhaseMap();
	};
}

#ifdef KEY
	#undef KEY
#endif