// (c) 2025 MyLoyalFans. All rights reserved.


#include "WUT_InputManager.h"
#include "Engine/Engine.h"

#if PLATFORM_WINDOWS
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
}

void AWUT_InputManager::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

#if PLATFORM_WINDOWS
    for (int32 PadIndex = 0; PadIndex < MaxPads; ++PadIndex)
    {
        XINPUT_STATE State;
        ZeroMemory(&State, sizeof(State));

        DWORD Result = XInputGetState(PadIndex, &State);
        bool bConnected = (Result == ERROR_SUCCESS);

        PadConnected[PadIndex] = bConnected;

        if (bConnected)
        {
            bool bStartPressed = (State.Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0;
            PadStartButtons[PadIndex].Update(bStartPressed);
        }
        else
        {
            PadStartButtons[PadIndex].Update(false);
        }
    }
#endif
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
    
#endif
}

bool AWUT_InputManager::IsPadConnected(int32 PadIndex) const
{
    // XINPUT pads:
    if (PadIndex < MaxPads)
        return PadConnected[PadIndex];

    // Keyboards always exist
    if (PadIndex == KeyboardAIndex) return true;
    if (PadIndex == KeyboardBIndex) return true;

    return false;
}

bool AWUT_InputManager::WasStartJustPressed(int32 PadIndex) const
{
    // ---------------- GAMEPAD ----------------
    if (PadIndex < MaxPads)
        return PadStartButtons[PadIndex].bPressedThisFrame;

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return false;

    // ---------------- KEYBOARD A ----------------
    if (PadIndex == KeyboardAIndex)
        return PC->WasInputKeyJustPressed(EKeys::Enter);

    // ---------------- KEYBOARD B ----------------
    if (PadIndex == KeyboardBIndex)
        return PC->WasInputKeyJustPressed(EKeys::Add);

    return false;
}



