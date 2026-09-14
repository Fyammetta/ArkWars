// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"
#include "ArkWarBattleSettings.generated.h"

class IBattleModeInterface;
/**
 * 
 */
UCLASS(Config = Game, DisplayName = "Ark War Battle")
class ARKWARS_API UArkWarBattleSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	static UArkWarBattleSettings* Get();
	
	UPROPERTY(EditAnywhere, Category="Data|GameMode")
	TMap<FGameplayTag, TSubclassOf<UActorComponent>> GameModes;

	/**
	 *	响应窗口的默认时间预算（秒），随候选下发给 UI（FResponseWindow.Timeout）。
	 *	关闭时机统一只通过 UI 决定：倒计时到点 UI 直接请求关闭（= 放弃，走 DeclineWindowResponse
	 *	同一入口）；引擎侧不持有任何定时器（P3 §3.5）
	 */
	UPROPERTY(EditAnywhere, Config, Category="Window", meta = (ClampMin = "1.0", UIMin = "10.0", UIMax = "120.0"))
	float DefaultWindowTimeout = 10.f;
};
