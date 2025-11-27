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

/*
This Input Manager is independent from Unreal Input Manager(Or PlayerController)
*/

UCLASS()
class WALKUPTHROWCPP_API AWUT_InputManager : public AActor
{
	GENERATED_BODY()
	
public:
    AWUT_InputManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    static const int32 MaxPads = 2;
    static constexpr int32 KeyboardAIndex = 100;
    static constexpr int32 KeyboardBIndex = 101;

    FPadButtonState PadStartButtons[MaxPads];

    UFUNCTION(BlueprintCallable, Category = "Input|WUT")
    bool IsPadConnected(int32 PadIndex) const;

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
