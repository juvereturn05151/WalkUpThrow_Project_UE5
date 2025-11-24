// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AWUT_MenuGameMode.generated.h"

class APlayerController;

/**
 * 
 */
UCLASS()
class WALKUPTHROWCPP_API AWUT_MenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
    AWUT_MenuGameMode();

    // Called by the Menu PlayerController when Start is pressed
    UFUNCTION(BlueprintCallable)
    void HandleMenuStartPressed(APlayerController* PressingController);

    // Whether each slot has been filled
    UPROPERTY(BlueprintReadOnly)
    bool bPlayer1Joined = false;

    UPROPERTY(BlueprintReadOnly)
    bool bPlayer2Joined = false;

    // Blueprints can react when someone joins
    UFUNCTION(BlueprintImplementableEvent)
    void OnPlayerJoined(int32 PlayerIndex, APlayerController* Controller);

protected:
    // For now we just remember *which* controller joined each slot (optional)
    UPROPERTY()
    TWeakObjectPtr<APlayerController> Player1Controller;

    UPROPERTY()
    TWeakObjectPtr<APlayerController> Player2Controller;
	
};
