#include "CardMsgDef.h"

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
}



const TMap<FString, FGameplayTag>& CardMessage::FMoveMessage::GetDefaultAreaMap()
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

const TMap<FString, FGameplayTag>& CardMessage::FMoveMessage::GetSuitMap()
{
	static const TMap<FString, FGameplayTag> SuitMap = {
		{Diamond,					CardTags::Diamond	},
		{Club,						CardTags::Club		},
		{Heart,						CardTags::Heart		},
		{Spade,						CardTags::Spade	}
	};
	return SuitMap;
}

CardMessage::FMoveMessage::FMoveMessage(const FString& Msg)
{
	Count = 1;
	FromArea = CardTags::Pile;
	ToArea = CardTags::Hand;
	bConsiderAsDiscard = Msg.Find(Discard) != INDEX_NONE;
	OutOrder = EOrder::Top;
	FString Temp {};

	auto Find = [&Msg, &Temp](Key K)->bool
	{
		if (Msg.Find(K) == INDEX_NONE) return false;
		
		int32 Begin = Msg.Find(K) + FCString::Strlen(K) + 1;
		int32 End = Begin;
		while (Msg[End] != ' ' && Msg[End] != '\0')
		{
			End++;
		}
		
		Temp = Msg.Mid(Begin, End - Begin);
		return true;
	};
	
	if (Find(Num))
		Count = FCString::Atoi(*Temp);
	
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
			
			FromArea = FGameplayTag::RequestGameplayTag(FName(*Temp));
		}
		else
		{
			FromArea = GetDefaultAreaMap()[*Temp];
		}
	}
	if (bConsiderAsDiscard)
		ToArea = CardTags::Discard;
	else if (Find(To))
	{
		if (Temp == Special && FindA(Area))
		{
			
			ToArea = FGameplayTag::RequestGameplayTag(FName(*Temp));
		}
		else
		{
			ToArea = GetDefaultAreaMap()[*Temp];
		}
	}
	if (Find(Order))
	{
		if (Temp == Random) OutOrder = EOrder::Random;
		if (Temp == Botton) OutOrder = EOrder::Botton;
	}
	if (Find(Meta))
		MetaInfo = Temp;
	
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
		Count, *FromArea.ToString(), *ToArea.ToString(), bConsiderAsDiscard ? TEXT(", this is a discard") : TEXT(""))
}

bool CardMessage::FMoveMessage::operator()(const FGameplayTagContainer& Card) const
{
	return Predicate ? Predicate(Card) : false;
};

TArray<FGameplayTagContainer> CardMessage::FMoveMessage::operator()(const TArray<FGameplayTagContainer>& Cards) const
{
	if (!Predicate) return {};
	TArray<FGameplayTagContainer> RetArr {};
	for (FGameplayTagContainer Card : Cards) 
	{
		if (Predicate(Card)) RetArr.Add(Card);
	}
		
	return RetArr;
};

