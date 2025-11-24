// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AWUT_GameplayGameMode.generated.h"

class AWUT_FighterPawn;

/**
 * 
 */
UCLASS()
class WALKUPTHROWCPP_API AAWUT_GameplayGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
    virtual void BeginPlay() override;

protected:
    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<AWUT_FighterPawn> FighterClass;

    void SpawnFighters();
};
