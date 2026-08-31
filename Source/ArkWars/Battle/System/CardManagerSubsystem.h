// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "CardManagerSubsystem.generated.h"

class UCardComponentBase;
struct FHandCardInfo;

USTRUCT(BlueprintType)
struct FCardInfo
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	FGameplayTagContainer CardTags;
	
	///手牌的命名
	UPROPERTY(BlueprintReadOnly)
	FText CardName;
	
	/// 卡牌文案（若需要），其中所有数据部分以{0}、{1}、{2}....方式标记，包括纯数字与游戏内变化的数字(X、Y、Z等)
	UPROPERTY(BlueprintReadOnly)
	FText Description;
	
	/// 文案中涉及的数据（若需要）
	UPROPERTY(BlueprintReadOnly)
	TArray<FString> Data;
	
	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<UCardComponentBase> Class;
	
	TDelegate<int32(const FString&)> DataGetter;
	
	TArray<FFormatArgumentData> GetNumericalDatas();
	
	FCardInfo() : Class(nullptr) {};
	
	FCardInfo& operator=(const FCardInfo& Other);
	FCardInfo& operator=(const FHandCardInfo& Info);
	
	bool IsValid();
	
	virtual ~FCardInfo() = default; 
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
	
	TMap<FGameplayTag, FCardInfo> InfoMapping;
	
public:
	UFUNCTION(BlueprintCallable)
	FCardInfo GetCardInfoByTag(const FGameplayTag& Tag);
	
	UFUNCTION(BlueprintCallable)
	FGameplayTag GetCardType(const FGameplayTag& Tag);
};
