// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WUT_MenuPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

/**
 * 
 */
UCLASS()
class WALKUPTHROWCPP_API AWUT_MenuPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AWUT_MenuPlayerController();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

    // Enhanced Input assets (assign these in Blueprint or defaults)
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> MenuInputMappingContext;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> MenuStartAction;

    // Callback for Start button
    UFUNCTION()
    void OnMenuStartTriggered(const FInputActionValue& Value);

    // Internal helper to add mapping context
    void AddMenuInputMapping();
};
