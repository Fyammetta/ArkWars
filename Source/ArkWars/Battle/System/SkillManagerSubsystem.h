// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArkWars/Battle/Toolkits/ArkWarTableTypes.h"
#include "ArkWars/Battle/Toolkits/ArkWarSkillTypes.h"
#include "ArkWars/Battle/Toolkits/ArkWarOperatorTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "SkillManagerSubsystem.generated.h"

struct FGameplayAbilitySpec;
struct FGameplayTag;
class UGameplayAbility;

USTRUCT(BlueprintType)
struct FSkillInfo : public FInfoWithData
{
	GENERATED_BODY()	
	
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag SkillTag = {};
	
	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<ESkillActivateType> SkillType;
	
	UPROPERTY(BlueprintReadOnly)
	FText SkillName = FText::GetEmpty();
	
	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<USkillComponentBase> SkillClass;
	
	FSkillInfo();
	
	FSkillInfo(const TSubclassOf<USkillComponentBase>& Comp);
	
	FSkillInfo& operator=(const FOperatorSkillInfo& Info);

	FSkillInfo& operator=(const FSkillInfo& Info);
};
/**
 *	用于管理当局内的所有技能
 *	便于通过技能Tag获取技能的具体信息
 */
UCLASS()
class ARKWARS_API USkillManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
	TMap<FGameplayTag, TSharedPtr<FSkillInfo>> SkillMapping;
	
	TMap<FGameplayTag, TArray<FSkillListenerEntry>> SkillTimings;
	
public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	TSharedPtr<FSkillInfo> GetCurrentSkillByTag(const FGameplayTag& Tag);
	
	 TArray<FSkillListenerEntry> GetSkillListeners(const FGameplayTag& Timing) const;
	
	void RegisterSkillTiming(const FGameplayTag& Timing, FSkillListenerEntry&& Entry);
	
	void UnregisterSkillTiming(const FGameplayTag& Timing, USkillComponentBase* Owner);
	
private:
	///	索引/登记的服务器门控判据（与 UBattleGameFlowSubsystem 同口径）：时机索引只服务开窗，开窗仅服务器
	bool IsRunningOnServer() const { return GetWorld() ? GetWorld()->GetNetMode() < NM_Client : false; }

	void OnAllOperatorSelected(const TArray<FOperatorCardInfo>& Operators);

	TSharedPtr<FSkillInfo> AppendSkill(const FGameplayTag& Tag);
	
	FString GetLogNetContext() const noexcept
	{
#if !UE_BUILD_SHIPPING
		if (GetWorld())
		{
			switch (GetWorld()->GetNetMode())
			{
				case NM_Standalone: return TEXT("[Standalone]");
				case NM_DedicatedServer: return TEXT("[DedicatedServer]");
				case NM_Client: return TEXT("[NM_Client]");
				case NM_ListenServer: return TEXT("[ListenServer]");
			}
		}
		
		return TEXT("[Unknown]");
#endif
	}

};


