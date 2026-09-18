// Fill out your copyright notice in the Description page of Project Settings.


#include "BattlePlayerState.h"
#include "AbilitySystemComponent.h"
#include "ArkWars/ArkWars.h"
#include "ArkWars/Battle/Component/Card/CardManagementBusComponent.h"
#include "ArkWars/Battle/Component/GameMode/GameModeComponentBase.h"
#include "ArkWars/Battle/Component/Skill/SkillComponentBase.h"
#include "ArkWars/Battle/GAS/Attributes/OperatorBasicAttributes.h"
#include "ArkWars/Battle/Toolkits/ArkWarOperatorTypes.h"
#include "ArkWars/Settings/OperatorSettings.h"
#include "GameFramework/GameModeBase.h"
#include "Net/UnrealNetwork.h"

ABattlePlayerState::ABattlePlayerState()
{
	Asc = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	Asc->SetIsReplicated(true);
	//Asc->AddAttributeSetSubobject(UOperatorBasicAttributes::StaticClass()->GetDefaultObject());
}

TArray<FArkCard> ABattlePlayerState::GetCardsByKey(const FGameplayTag& Area) const
{
	ICardContainerInterface* Container = nullptr;
	for (UActorComponent* Component : GetComponentsByInterface(UCardContainerInterface::StaticClass()))
	{
		Container = Cast<ICardContainerInterface>(Component);
		if (Container && Container->GetAreaKeys().Contains(Area))
			return Container->GetCardsByKey(Area);
	}
	
	return {};
}

const FArkCard* ABattlePlayerState::GetCardById(int32 CardId) const
{
	ICardContainerInterface* Container = nullptr;
	for (UActorComponent* Component : GetComponentsByInterface(UCardContainerInterface::StaticClass()))
	{
		Container = Cast<ICardContainerInterface>(Component);
		if (Container && Container->GetCardById(CardId))
			return Container->GetCardById(CardId);
	}
	
	return nullptr;
}

TArray<int32> ABattlePlayerState::Select(const FGameplayTag& Area, const FMessageType& Msg) const
{
	ICardContainerInterface* Container = nullptr;
	for (UActorComponent* Component : GetComponentsByInterface(UCardContainerInterface::StaticClass()))
	{
		Container = Cast<ICardContainerInterface>(Component);
		if (Container && Container->GetAreaKeys().Contains(Area))
			return Container->Select(Area, Msg);
	}
	
	return {};
}

TArray<FArkCard> ABattlePlayerState::Consume(const FGameplayTag& Area, const TArray<int32>& CardIds)
{
	ICardContainerInterface* Container = nullptr;
	for (UActorComponent* Component : GetComponentsByInterface(UCardContainerInterface::StaticClass()))
	{
		Container = Cast<ICardContainerInterface>(Component);
		if (Container && Container->GetAreaKeys().Contains(Area))
			return Container->Consume(Area, CardIds);
	}
	
	return {};
}

EAreaWriteResult ABattlePlayerState::Add(const FGameplayTag& AreaKey, TArray<FArkCard>& Cards, const FMessageType& Msg)
{
	ICardContainerInterface* Container = nullptr;
	for (UActorComponent* Component : GetComponentsByInterface(UCardContainerInterface::StaticClass()))
	{
		Container = Cast<ICardContainerInterface>(Component);
		if (Container && Container->GetAreaKeys().Contains(AreaKey))
			return Container->Add(AreaKey, Cards, Msg);
	}
	
	return EAreaWriteResult::Mismatch;
}

TArray<FGameplayTag> ABattlePlayerState::GetAreaKeys() const
{
	TArray<FGameplayTag> AreaKeys{};
	ICardContainerInterface* Container = nullptr;
	for (UActorComponent* Component : GetComponentsByInterface(UCardContainerInterface::StaticClass()))
	{
		Container = Cast<ICardContainerInterface>(Component);
		if (Container)
			AreaKeys.Append(Container->GetAreaKeys());
	}
	
	return AreaKeys;
}

void ABattlePlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ABattlePlayerState, Operator);
}

void ABattlePlayerState::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		auto Comp = NewObject<UCardManagementBusComponent>(this, UCardManagementBusComponent::StaticClass(), TEXT("CardManagementBus"));
		Comp->RegisterComponent();
	}
}

void ABattlePlayerState::InitOperator(const FName& SelectedOperator)
{
	if (!HasAuthority()) return;

	if (!Asc)
	{
		UE_LOG(LogOperator, Warning, TEXT("[ABattlePlayerState][InitOperator] AbilitySystemComponent is missing on %s, initializing aborted"), *GetName())
		return;
	}

	auto Settings = UOperatorSettings::Get();
	if (!Settings)
	{
		UE_LOG(LogOperator, Warning, TEXT("[ABattlePlayerState][InitOperator] OperatorSettings is unavailable, initializing %s aborted"), *SelectedOperator.ToString())
		return;
	}

	//	档案键 = 干员名：与 UDefaultModeComponent 抽取候选时写入 TotalOperators 的键同源
	const FOperatorCardInfo* Info = Settings->GetOperatorDataTable()->FindRow<FOperatorCardInfo>(SelectedOperator, TEXT("[ABattlePlayerState][InitOperator]"));
	if (!Info)
	{
		UE_LOG(LogOperator, Warning, TEXT("[ABattlePlayerState][InitOperator] Operator %s has no row in the operator data table, initializing aborted"), *SelectedOperator.ToString())
		return;
	}

	//	属性集与 GE 都要 ActorInfo 就位后才认账：先立身份，再取集、灌值
	Asc->InitAbilityActorInfo(this, this);

	//	属性集由挂载侧负责（本函数只读取，不创建）：取不到即告警退出——
	//	半初始化（属性空着却继续建技能）比不初始化更难查
	const UOperatorBasicAttributes* MountedSet = Asc->GetSet<UOperatorBasicAttributes>();
	if (!MountedSet)
	{
		UE_LOG(LogOperator, Warning, TEXT("[ABattlePlayerState][InitOperator] No attribute set of %s is mounted on %s, initializing aborted"), *UOperatorBasicAttributes::StaticClass()->GetName(), *GetName())
		return;
	}

	//	GetSet 声明返回 const（引擎口径：属性应经 GE 修改）；此处与引擎 InitStats 同款——
	//	初始化阶段尚无 GE 介入，直改是安全的
	UOperatorBasicAttributes* Attributes = const_cast<UOperatorBasicAttributes*>(MountedSet);

	//	顺序不可换：上限先落，否则 Health 撞上默认上限 0 会被钳成 0
	Attributes->SetMaxHealth(Info->MaxHealth);
	Attributes->SetHealth(Info->Health);
	Attributes->SetShield(Info->Shield);

	//	干员表声明的技能逐条落地。CreateSkill 走技能索引，索引未预载时会按需补载
	//	（见 GetCurrentSkillByTag），故此处不依赖"全部选完"才发生的批量预载
	for (const FGameplayTag& SkillTag : Info->SkillComps)
	{
		USkillComponentBase::CreateSkill(this, SkillTag);
	}

	UE_LOG(LogOperator, Log, TEXT("[ABattlePlayerState][InitOperator] Operator %s initialized on %s (MaxHealth:%d Health:%d Shield:%d Skills:%d)"),
		*SelectedOperator.ToString(), *GetName(), Info->MaxHealth, Info->Health, Info->Shield, Info->SkillComps.Num())
}

void ABattlePlayerState::OnRep_OperatorSelected()
{
}

void ABattlePlayerState::ApplyOperatorSelection(const FName& SelectedOperator)
{
	//	选择先落字段（已成事实），再去裁决：旧写法把这行锁在组件判空的内层，
	//	组件一缺，这次选择就无声蒸发——连复制面也拿不到数据
	Operator = SelectedOperator;

	auto GM = GetWorld() ? GetWorld()->GetAuthGameMode() : nullptr;
	auto Comp = GM ? GM->FindComponentByClass<UGameModeComponentBase>() : nullptr;
	if (!Comp)
	{
		UE_LOG(LogOperator, Warning, TEXT("[ABattlePlayerState][ApplyOperatorSelection] Mode component is missing, selection of %s was recorded but not arbitrated on %s"), *SelectedOperator.ToString(), *GetName())
		return;
	}

	Comp->CheckOperatorSelection(this, SelectedOperator);
	InitOperator(SelectedOperator);
}


