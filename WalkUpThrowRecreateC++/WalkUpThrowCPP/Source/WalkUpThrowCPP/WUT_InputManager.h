// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WUT_InputManager.generated.h"

struct FWUTPadState;

USTRUCT()
struct FPadButtonState
{
    GENERATED_BODY()

    bool bPressedLastFrame = false;
    bool bPressedThisFrame = false;

    void Update(bool bIsPressedNow)
    {
        bPressedThisFrame = (bIsPressedNow && !bPressedLastFrame);
        bPressedLastFrame = bIsPressedNow;
    }
};



UCLASS()
class WALKUPTHROWCPP_API AWUT_InputManager : public AActor
{
	GENERATED_BODY()
	
public:
    AWUT_InputManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    // Max number of pads we support (XInput = 4)
    static const int32 MaxPads = 2;
    static constexpr int32 KeyboardAIndex = 100;
    static constexpr int32 KeyboardBIndex = 101;

    FPadButtonState PadStartButtons[MaxPads];

    // Is pad i connected?
    UFUNCTION(BlueprintCallable, Category = "Input|WUT")
    bool IsPadConnected(int32 PadIndex) const;

    // Did pad i press START this frame? (rising edge)
    UFUNCTION(BlueprintCallable, Category = "Input|WUT")
    bool WasStartJustPressed(int32 PadIndex) const;

protected:
    // Internal per–pad state
    UPROPERTY(VisibleAnywhere, Category = "Input|WUT")
    TArray<bool> PadConnected;

    UPROPERTY(VisibleAnywhere, Category = "Input|WUT")
    TArray<bool> StartDown;

    UPROPERTY(VisibleAnywhere, Category = "Input|WUT")
    TArray<bool> PrevStartDown;

    UPROPERTY(VisibleAnywhere, Category = "Input|WUT")
    TArray<bool> StartJustPressed;

    // Poll XInput
    void PollPads();

};
