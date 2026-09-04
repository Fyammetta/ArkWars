#include "GameMessage.h"

#include "ArkWarTags.h"
#include "ArkWars/ArkWars.h"

namespace
{
	int32 TagToPoint(const FGameplayTag& Tag)
	{
		// 取标签名末位字符（如 "Card.Point.9" 的 '9'）
		const FString Name = Tag.ToString();
		if (Name.IsEmpty()) return INDEX_NONE;

		switch (const TCHAR Num = Name[Name.Len() - 1])
		{
			case 'A':			return 1;
			case 'X':			return 10;
			case 'J':			return 11;
			case 'Q':			return 12;
			case 'K':			return 13;
			default:
				{
					int32 Value = static_cast<int32>(Num) - static_cast<int32>('0');
					return  Value >=2 && Value < 10 ? Value : INDEX_NONE;
				}	 /* return N; */
		}
	}

	struct FindKeyHelper
	{
		FString Msg;
		FString* Out;
		
		FindKeyHelper(FString InMsg, FString* InOut)
		{
			Msg = InMsg;
			Out = InOut;
		}
		
		bool operator()(GameMessage::Key K) const
		{
			if (Msg.Find(K) == INDEX_NONE) return false;
		
			int32 Begin = Msg.Find(K) + FCString::Strlen(K) + 1;
			int32 End = Begin;
			while (Msg[End] != ' ' && Msg[End] != '\0')
			{
				End++;
			}
		
			*Out = Msg.Mid(Begin, End - Begin);
			return true;
		}
	};
}



const TMap<FString, FGameplayTag>& GameMessage::FMoveMessage::GetDefaultAreaMap()
{
	// 函数内静态：首次调用时构造，此时原生标签必然已注册完毕
	static const TMap<FString, FGameplayTag> DefaultArea = {
		{Hand,						CardTags::Hand		},
		{Discard,					CardTags::Discard	},
		{Equipment,					CardTags::Equipment	},
		{Pile,						CardTags::Pile		},
		{Judgement,					CardTags::Judgement	},
		{Cache,						CardTags::Cache		},
		{Used,						CardTags::Used		},
	};
	return DefaultArea;
}

const TMap<FString, FGameplayTag>& GameMessage::FMoveMessage::GetSuitMap()
{
	static const TMap<FString, FGameplayTag> SuitMap = {
		{Diamond,					CardTags::Diamond	},
		{Club,						CardTags::Club		},
		{Heart,						CardTags::Heart		},
		{Spade,						CardTags::Spade	}
	};
	return SuitMap;
}

const TMap<FString, EGamePhase>& GameMessage::FPhaseMessage::GetPhaseMap()
{
	// 函数内静态：首次调用时构造
	// 注意："-J" 在 OP= 的语境里是 Jump，在 PHASE= 的语境里是判定阶段，两处分开解析，互不冲突
	static const TMap<FString, EGamePhase> PhaseMap = {
		{Begin,						EGamePhase::Begin			},
		{Prepare,					EGamePhase::Preparation		},
		{Judgement,					EGamePhase::Judgment		},
		{Draw,						EGamePhase::Draw			},
		{Action,					EGamePhase::Action			},
		{Discard,					EGamePhase::Discard			},
		{Finish,					EGamePhase::Finish			},
	};
	return PhaseMap;
}

GameMessage::FMoveMessage::FMoveMessage(const FString& Msg)
{
	_Count = 1;
	_From = CardTags::Pile;
	_To = CardTags::Hand;
	_Order = EOrder::Top;
	if (Msg.IsEmpty()) return;
	
	FString Temp {};

	FindKeyHelper Find = {Msg, &Temp};
	
	if (Find(Num))
		_Count = FCString::Atoi(*Temp);
	
	auto FindA = [&Msg, &Temp](Key K)->bool
	{
		if (Msg.Find(K) == INDEX_NONE) return false;

		int32 Begin = Msg.Find(K) + FCString::Strlen(K) + 1;

		FString Right = Msg.Right(Begin);

		if (Right.Find(Area) != INDEX_NONE) return false;
		Begin = Right.Find(K) + FCString::Strlen(K) + 1;
		int32 End = Begin;
		while (Right[End] != ' ' && Right[End] != '\0')
		{
			End++;
		}
		Temp = Right.Mid(Begin, End - Begin);
		return true;
	};
	if (Find(From))
	{
		if (Temp == Special && FindA(Area))
		{
			
			_From = FGameplayTag::RequestGameplayTag(FName(*Temp));
		}
		else
		{
			_From = GetDefaultAreaMap()[*Temp];
		}
	}
	if (Find(To))
	{
		if (Temp == Special && FindA(Area))
		{
			
			_To = FGameplayTag::RequestGameplayTag(FName(*Temp));
		}
		else
		{
			_To = GetDefaultAreaMap()[*Temp];
		}
	}

	_bDiscard = Msg.Find(IsDiscard) && FCString::Atoi(*Temp);

	if (Find(Order))
	{
		if (Temp == Random) _Order = EOrder::Random;
		if (Temp == Botton) _Order = EOrder::Botton;
	}
	if (Find(Meta))
		_Meta = Temp;
	
	FString CondMsg {};
	if (Msg.Find(Condition) != INDEX_NONE)
		Msg.Split(Condition,nullptr,&CondMsg);
	else
		return;
	
	int32 BeginIndex = 0;
	int32 Length = 0;
	enum ELogicOperation
	{
		And,
		Or,
		Not
	};
	FString CondKey {};
	FString CondValue {};
	struct FPredicate
	{
		ELogicOperation Operation;
		TArray<TFunction<bool(const FGameplayTagContainer&)>> Complex;
		
		FPredicate(ELogicOperation Op) : Operation(Op) , Complex({}) {}
	};
	
	TArray<FPredicate> Stack = {And};
	
	while (BeginIndex + Length < CondMsg.Len())
	{		
		auto C = CondMsg[BeginIndex + Length];
		switch (C)
		{
			case '{' : Stack.Add(Or); BeginIndex += 1; break ;
			case '[' : Stack.Add(Not); BeginIndex += 1; break ;
			case '}' :
			case ']' :
				{
					FPredicate TempP = Stack.Pop(); 
					Stack.Last().Complex.Add([P = MoveTemp(TempP)](const FGameplayTagContainer& Card)->bool
					{
						switch (P.Operation)
						{
							case And:
								{
									UE_LOG(LogCard, Log, TEXT("[Message][Move] Operation <AND>"))

									for (auto& Single : P.Complex)
									{
										if (!Single(Card))
											return false;
									}
									return true;
								}
							case Or:
								{
									UE_LOG(LogCard, Log, TEXT("[Message][Move] Operation <OR>"))

									for (auto& Single : P.Complex)
									{
										if (Single(Card))
											return true;
									}
									return false;
								}
							case Not:
								{
									UE_LOG(LogCard, Log, TEXT("[Message][Move] Operation <NOT>"))

									for (auto& Single : P.Complex)
									{
										if (!Single(Card))
											return true;
									}
									return false;
								}
							default: return false;
						}
					});
					BeginIndex += Length + 1;
					Length = 0;
					break;
				}
			case '=':
				{
					CondKey = CondMsg.Mid(BeginIndex, Length);
					BeginIndex += Length + 1;
					Length = 0;
					break;
				}
			case ' ' :
			case '*' :
				{
					CondValue = CondMsg.Mid(BeginIndex, Length);
					BeginIndex += Length + 1;
					Length = 0;
					Stack.Last().Complex.Add([CK = MoveTemp(CondKey), CV = MoveTemp(CondValue)]
						(const FGameplayTagContainer& Card)->bool
					{
						if (CK == Class)
						{
							FGameplayTag ClassValue = FGameplayTag::RequestGameplayTag(FName(CV));
							UE_LOG(LogCard, Log, TEXT("[Message][Move][Class] Current Value is %s "),*ClassValue.ToString())

							if (Card.HasTag(ClassValue))
							{
								return true;
							}
						}
						else if (CK == Suit)
						{
							FGameplayTag SuitValue = GetSuitMap()[CV];
							UE_LOG(LogCard, Log, TEXT("[Message][Move][Suit] Current Value is %s "),*SuitValue.ToString())

							if (Card.HasTag(SuitValue))
							{
								return true;
							}
						}
						else if (CK == Max)
						{
							int32 MaxValue = FCString::Atoi(*CV);
							UE_LOG(LogCard, Log, TEXT("[Message][Move][Point] Max Value is %d "),MaxValue)

							for (const FGameplayTag& Tag : Card.GetGameplayTagArray())
							{
								if (Tag.MatchesTag(CardTags::Point))
									if (TagToPoint(Tag) < MaxValue) return true;
							}
						}
						else if (CK == Min)
						{
							int32 MinValue = FCString::Atoi(*CV);
							UE_LOG(LogCard, Log, TEXT("[Message][Move][Point] Min Value is %d "),MinValue)

							for (const FGameplayTag& Tag : Card.GetGameplayTagArray())
							{
								if (Tag.MatchesTag(CardTags::Point))
									if (TagToPoint(Tag) > MinValue) return true;
							}
						}
						UE_LOG(LogCard, Log, TEXT("[Message][Move] Condition Key = %s Value = %s, return false "),*CK, *CV)
						return false;
					});
					break;
				}
			default:  Length += 1;
		}
	}

	
	Predicate = [List = Stack.Pop().Complex](const FGameplayTagContainer& Card)->bool
	{
		for (const auto& Function : List)
		{
			if (!Function(Card))	return false;
		}
		return true;
	};
	
	UE_LOG(LogCard, Log, TEXT("[Message][Move] Constructed new message info: Count = %d, From %s, To %s%s"),
		_Count, *_From.ToString(), *_To.ToString(), _bDiscard ? TEXT(", this is a discard") : TEXT(""))
}

bool GameMessage::FMoveMessage::operator()(const FGameplayTagContainer& Card) const
{
	return Predicate ? Predicate(Card) : false;
};

TArray<FGameplayTagContainer> GameMessage::FMoveMessage::operator()(const TArray<FGameplayTagContainer>& Cards) const
{
	if (!Predicate) return {};
	TArray<FGameplayTagContainer> RetArr {};
	for (FGameplayTagContainer Card : Cards) 
	{
		if (Predicate(Card)) RetArr.Add(Card);
	}
		
	return RetArr;
}

GameMessage::FPhaseMessage::FPhaseMessage(const FString& Msg)
{
	_Op = EOperation::Continue;
	_Target = EGamePhase::GameStart;
	_Phase = EGamePhase::GameStart;
	_Player = INDEX_NONE;
	if (Msg.IsEmpty()) return;

	FString Temp {};
	FindKeyHelper Find = {Msg, &Temp};

	// PHASE=<阶段>：本指令只允许在该阶段触发；无法识别的值视为不设限制（保持 GameStart）
	if (Find(Phase))
	{
		if (const EGamePhase* Found = GetPhaseMap().Find(Temp))
		{
			_Phase = *Found;
		}
	}

	// PLAYER=<座次>：本指令只允许指定玩家执行；INDEX_NONE 表示不设限制
	if (Find(Player))
	{
		_Player = FCString::Atoi(*Temp);
	}

	if (Find(Operation))
	{
		if (Temp == Skip)			_Op = EOperation::Skip;
		else if (Temp == Extra)		_Op = EOperation::Extra;
		else if (Temp == Jump)		_Op = EOperation::Jump;
		// Continue 与无法识别的操作一律保持 Continue
	}

	// Extra/Jump 必须显式给出目标阶段；缺填或无法识别时，整条指令按规则无视，退回 Continue
	if (_Op == EOperation::Extra || _Op == EOperation::Jump)
	{
		bool bResolved = false;
		if (Find(Target))
		{
			if (const EGamePhase* Found = GetPhaseMap().Find(Temp))
			{
				_Target = *Found;
				bResolved = true;
			}
		}
		if (!bResolved)
		{
			_Op = EOperation::Continue;
			_Target = EGamePhase::GameStart;
		}
	}
}

namespace
{
	// 主阶段 → 其对应的 Pre 阶段；无 Pre 的阶段（Begin/Finish）返回 GameStart
	EGamePhase GetPrePhase(EGamePhase Phase)
	{
		switch (Phase)
		{
		case EGamePhase::Preparation:	return EGamePhase::Preparation_Pre;
		case EGamePhase::Judgment:		return EGamePhase::Judgment_Pre;
		case EGamePhase::Draw:			return EGamePhase::Draw_Pre;
		case EGamePhase::Action:		return EGamePhase::Action_Pre;
		case EGamePhase::Discard:		return EGamePhase::Discard_Pre;
		default:						return EGamePhase::GameStart;
		}
	}
}

bool GameMessage::FPhaseMessage::CanExecute(EGamePhase Current, int32 ActivePlayer) const
{
	// 阶段判定：仅当 Current 的下一阶段恰为 _Phase 的 Pre 阶段时通过
	if (_Phase != EGamePhase::GameStart)
	{
		const int32 Next = static_cast<int32>(Current) + 1;
		if (Next != static_cast<int32>(GetPrePhase(_Phase)))
		{
			return false;
		}
	}

	// 玩家判定：须与当前活跃玩家一致
	if (_Player != INDEX_NONE && _Player != ActivePlayer)
	{
		return false;
	}

	return true;
}
