// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WUT_MenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class WALKUPTHROWCPP_API UWUT_MenuWidget : public UUserWidget
{
	GENERATED_BODY()
public:

    /** Called when a player joins using a specific pad index.
     *  Implement this in Blueprint to update the UI.
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "Menu")
    void OnPlayerJoined(int32 PlayerIndex, int32 PadIndex);

    /** Optional: called whenever join state changes (e.g. both joined).
     *  Implement if you want fancier UI updates.
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "Menu")
    void OnJoinStateChanged(
        bool bPlayer1Joined,
        bool bPlayer2Joined,
        int32 Player1PadIndex,
        int32 Player2PadIndex
    );
};
