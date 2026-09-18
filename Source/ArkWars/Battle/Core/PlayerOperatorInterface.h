// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayerOperatorInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UPlayerOperatorInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARKWARS_API IPlayerOperatorInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	///	干员身份的查询面。选角的收发（上行提交 / 下行候选）已迁至 PC（卷 11 §5.2）：
	///	PlayerState 只留业务与字段，不留 RPC 口——两条消息函数若在此保留，即是第二个入口
	virtual const FName& GetOperator() const = 0;
};
