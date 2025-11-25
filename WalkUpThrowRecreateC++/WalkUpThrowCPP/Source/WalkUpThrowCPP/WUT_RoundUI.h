// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WUT_RoundUI.generated.h"

/**
 * 
 */
UCLASS()
class WALKUPTHROWCPP_API UWUT_RoundUI : public UUserWidget
{
	GENERATED_BODY()
public:

    UFUNCTION(BlueprintImplementableEvent)
    void ShowReady();

    UFUNCTION(BlueprintImplementableEvent)
    void ShowFight();

    UFUNCTION(BlueprintImplementableEvent)
    void HideAll();
};
