// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WUT_HealthUI.generated.h"

UCLASS()
class WALKUPTHROWCPP_API UWUT_HealthUI : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintImplementableEvent)
    void SetPlayerHealth(int32 PlayerIndex, int32 Health);
};