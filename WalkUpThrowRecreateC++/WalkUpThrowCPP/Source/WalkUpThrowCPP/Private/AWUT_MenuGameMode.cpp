// (c) 2025 MyLoyalFans. All rights reserved.
#include "AWUT_MenuGameMode.h"
#include "WUT_InputManager.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

AWUT_MenuGameMode::AWUT_MenuGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AWUT_MenuGameMode::BeginPlay()
{
    Super::BeginPlay();

    //AudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComp"));
    //AudioComp->SetupAttachment(RootComponent);
    //AudioComp->bAutoActivate = false;

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
        return;

    // We loop over ALL input channels:
    // - Gamepads: 0..MaxPads-1
    // - Keyboard A: KeyboardAIndex
    // - Keyboard B: KeyboardBIndex

    TArray<int32> InputIndices;

    // Gamepads 0..3
    for (int32 i = 0; i < AWUT_InputManager::MaxPads; ++i)
        InputIndices.Add(i);

    // Keyboards
    InputIndices.Add(AWUT_InputManager::KeyboardAIndex);
    InputIndices.Add(AWUT_InputManager::KeyboardBIndex);

    for (int32 PadIndex : InputIndices)
    {
        // ----- Skip unconnected gamepads -----
        if (PadIndex < AWUT_InputManager::MaxPads)
        {
            if (!InputManager->IsPadConnected(PadIndex))
                continue;
        }

        // ----- Check Start button input -----
        if (!InputManager->WasStartJustPressed(PadIndex))
            continue;

        // ----- Skip already assigned -----
        if (PadIndex == Player1PadIndex || PadIndex == Player2PadIndex)
            continue;

        // ----- Assign Player 1 -----
        if (!bPlayer1Joined)
        {
            bPlayer1Joined = true;
            Player1PadIndex = PadIndex;

            UE_LOG(LogTemp, Log, TEXT("Player 1 joined using input %d"), PadIndex);
            OnPlayerJoined(1, PadIndex);
        }
        // ----- Assign Player 2 -----
        else if (!bPlayer2Joined)
        {
            bPlayer2Joined = true;
            Player2PadIndex = PadIndex;

            UE_LOG(LogTemp, Log, TEXT("Player 2 joined using input %d"), PadIndex);
            OnPlayerJoined(2, PadIndex);
        }
    }
}

