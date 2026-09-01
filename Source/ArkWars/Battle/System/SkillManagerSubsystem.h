// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArkWars/Battle/Toolkits/ArkWarTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "SkillManagerSubsystem.generated.h"

struct FGameplayAbilitySpec;
struct FGameplayTag;
class UGameplayAbility;

USTRUCT(BlueprintType)
struct FSkillInfo : public FInfoWithData
{
	GENERATED_BODY()	
	
	UPROPERTY(EditAnywhere)
	FGameplayTag SkillTag = {};
	
	UPROPERTY(EditAnywhere)
	TEnumAsByte<ESkillActivateType> SkillType;
	
	UPROPERTY(EditAnywhere)
	FText SkillName = FText::GetEmpty();
	
	TUniquePtr<FGameplayAbilitySpec> Spec;
	
	FSkillInfo();
	
	FSkillInfo(const TSubclassOf<UGameplayAbility>&  Ability);
	
	FSkillInfo& operator=(const FOperatorSkillInfo& Info);

	FSkillInfo& operator=(const FSkillInfo& Info);

	virtual ~FSkillInfo() override;
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
	
public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const { return true; }
	virtual void Initialize(FSubsystemCollectionBase& Collection);
	virtual void Deinitialize();
	
	TSharedPtr<FSkillInfo> GetCurrentSkillByTag(const FGameplayTag& Tag);
	
private:
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


