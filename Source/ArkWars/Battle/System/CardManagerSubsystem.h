// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ArkWars/Battle/Toolkits/ArkWarTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "CardManagerSubsystem.generated.h"

class UCardComponentBase;
struct FHandCardInfo;

USTRUCT(BlueprintType)
struct FCardInfo : public FInfoWithData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	FGameplayTagContainer CardTags;
	
	UPROPERTY(BlueprintReadOnly)
	FText CardName;
	
	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<UCardComponentBase> Class;
	
	FCardInfo() : Class(nullptr) {};
	
	FCardInfo& operator=(const FCardInfo& Other);
	FCardInfo& operator=(const FHandCardInfo& Info);
	
	bool IsValid();
};

/**
 *	用于管理游戏整体所有的卡牌类型
 *	无关花色点数等具体信息
 *	只储存卡牌的通用信息
 *	允许通过Tag获取到特定的卡牌种类与相关信息
 */
UCLASS()
class ARKWARS_API UCardManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override {return true;};
	virtual void Deinitialize() override;
	
	TMap<FGameplayTag, TSharedPtr<FCardInfo>> InfoMapping;
	
public:
	bool GetCardInfoByTag(const FGameplayTag& Tag, TSharedPtr<FCardInfo>& OutInfo);
	
	FGameplayTag GetCardType(const FGameplayTag& Tag);
};
