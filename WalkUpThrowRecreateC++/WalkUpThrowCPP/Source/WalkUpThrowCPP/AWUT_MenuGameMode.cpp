#include "AWUT_MenuGameMode.h"
#include "WUT_InputManager.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

AWUT_MenuGameMode::AWUT_MenuGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AWUT_MenuGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Find the InputManager in the world
    InputManager = nullptr;

    for (TActorIterator<AWUT_InputManager> It(GetWorld()); It; ++It)
    {
        InputManager = *It;
        break;
    }

    if (!InputManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("WUT_MenuGameMode: No WUT_InputManager found in level!"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("WUT_MenuGameMode: Found WUT_InputManager."));
    }
}

void AWUT_MenuGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    CheckPadJoins();
}

void AWUT_MenuGameMode::CheckPadJoins()
{
    if (!InputManager)
    {
        return;
    }

    for (int32 PadIndex = 0; PadIndex < AWUT_InputManager::MaxPads; ++PadIndex)
    {
        if (!InputManager->IsPadConnected(PadIndex))
        {
            continue;
        }

        if (!InputManager->WasStartJustPressed(PadIndex))
        {
            continue;
        }

        // Already assigned this pad?
        if (PadIndex == Player1PadIndex || PadIndex == Player2PadIndex)
        {
            continue;
        }

        if (!bPlayer1Joined)
        {
            bPlayer1Joined = true;
            Player1PadIndex = PadIndex;

            UE_LOG(LogTemp, Log, TEXT("Player 1 joined using pad %d"), PadIndex);
            OnPlayerJoined(1, PadIndex);
        }
        else if (!bPlayer2Joined)
        {
            bPlayer2Joined = true;
            Player2PadIndex = PadIndex;

            UE_LOG(LogTemp, Log, TEXT("Player 2 joined using pad %d"), PadIndex);
            OnPlayerJoined(2, PadIndex);
        }
        else
        {
            // both joined, ignore extra pads for now
        }
    }
}
