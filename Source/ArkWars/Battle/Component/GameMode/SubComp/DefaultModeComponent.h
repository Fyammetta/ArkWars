// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArkWars/Battle/Component/GameMode/GameModeComponentBase.h"
#include "DefaultModeComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARKWARS_API UDefaultModeComponent : public UGameModeComponentBase
{
	GENERATED_BODY()
	

public:
	virtual void InitCardDeck() override;
	virtual int32 AllocateIdentity() override;
	virtual int32 GetStartCardNum(UAbilitySystemComponent* Asc = nullptr) override;
	virtual void SentSelectOperatorNotify() override;
	virtual void CheckOperatorSelection(APlayerState* Player, const FName& Operator) override;
	

};
