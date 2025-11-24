// Fill out your copyright notice in the Description page of Project Settings.


#include "WUT_InputManager.h"

#include "Engine/Engine.h"

#if PLATFORM_WINDOWS
// Allow including Windows headers in Unreal
#include "Windows/AllowWindowsPlatformTypes.h"
#include <Xinput.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

AWUT_InputManager::AWUT_InputManager()
{
    PrimaryActorTick.bCanEverTick = true;

    PadConnected.SetNum(MaxPads);
    StartDown.SetNum(MaxPads);
    PrevStartDown.SetNum(MaxPads);
    StartJustPressed.SetNum(MaxPads);

    for (int32 i = 0; i < MaxPads; ++i)
    {
        PadConnected[i] = false;
        StartDown[i] = false;
        PrevStartDown[i] = false;
        StartJustPressed[i] = false;
    }
}

void AWUT_InputManager::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("WUT_InputManager started. Polling XInput pads..."));
}

void AWUT_InputManager::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    PollPads();
}

void AWUT_InputManager::PollPads()
{
#if PLATFORM_WINDOWS
    for (DWORD i = 0; i < (DWORD)MaxPads; ++i)
    {
        PrevStartDown[i] = StartDown[i];
        StartDown[i] = false;
        StartJustPressed[i] = false;

        XINPUT_STATE State;
        ZeroMemory(&State, sizeof(XINPUT_STATE));

        DWORD Result = XInputGetState(i, &State);
        if (Result == ERROR_SUCCESS)
        {
            // Pad i is connected
            if (!PadConnected[i])
            {
                UE_LOG(LogTemp, Log, TEXT("Pad %d connected."), i);
            }

            PadConnected[i] = true;

            bool bNowStartDown = (State.Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0;
            StartDown[i] = bNowStartDown;

            if (bNowStartDown && !PrevStartDown[i])
            {
                StartJustPressed[i] = true;

                UE_LOG(LogTemp, Log, TEXT("Pad %d: START just pressed."), i);
            }
        }
        else
        {
            if (PadConnected[i])
            {
                UE_LOG(LogTemp, Log, TEXT("Pad %d disconnected."), i);
            }

            PadConnected[i] = false;
            StartDown[i] = false;
            PrevStartDown[i] = false;
            StartJustPressed[i] = false;
        }
    }
#else
    // Non-Windows: you can later implement platform-specific polling
#endif
}

bool AWUT_InputManager::IsPadConnected(int32 PadIndex) const
{
    if (PadIndex < 0 || PadIndex >= MaxPads)
    {
        return false;
    }

    return PadConnected[PadIndex];
}

bool AWUT_InputManager::WasStartJustPressed(int32 PadIndex) const
{
    if (PadIndex < 0 || PadIndex >= MaxPads)
    {
        return false;
    }

    return StartJustPressed[PadIndex];
}

