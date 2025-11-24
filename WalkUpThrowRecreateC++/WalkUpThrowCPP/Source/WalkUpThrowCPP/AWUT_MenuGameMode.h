// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AWUT_MenuGameMode.generated.h"

class APlayerController;
class AWUT_InputManager;

/**
 * 
 */
UCLASS()
class WALKUPTHROWCPP_API AWUT_MenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
    AWUT_MenuGameMode();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(BlueprintReadOnly)
    bool bPlayer1Joined = false;

    UPROPERTY(BlueprintReadOnly)
    bool bPlayer2Joined = false;

    UFUNCTION(BlueprintImplementableEvent, Category = "Menu")
    void OnPlayerJoined(int32 PlayerIndex, int32 PadIndex);

protected:
    UPROPERTY()
    AWUT_InputManager* InputManager;

    UPROPERTY()
    int32 Player1PadIndex = -1;

    UPROPERTY()
    int32 Player2PadIndex = -1;

    void CheckPadJoins();
	
};
