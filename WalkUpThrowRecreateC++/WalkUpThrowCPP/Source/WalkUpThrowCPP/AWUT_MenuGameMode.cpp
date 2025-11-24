// Fill out your copyright notice in the Description page of Project Settings.


#include "AWUT_MenuGameMode.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

AWUT_MenuGameMode::AWUT_MenuGameMode()
{
    // We’ll assign custom PlayerController/HUD in Project Settings -> Maps & Modes
}

void AWUT_MenuGameMode::HandleMenuStartPressed(APlayerController* PressingController)
{
    if (!PressingController)
    {
        return;
    }

    // Already full? Ignore
    if (bPlayer1Joined && bPlayer2Joined)
    {
        UE_LOG(LogTemp, Log, TEXT("Both players already joined – ignoring extra Start press."));
        return;
    }

    // First join -> Player 1
    if (!bPlayer1Joined)
    {
        bPlayer1Joined = true;
        Player1Controller = PressingController;

        UE_LOG(LogTemp, Log, TEXT("Player 1 joined with controller: %s"),
            *PressingController->GetName());

        OnPlayerJoined(1, PressingController);
        return;
    }

    // Second join -> Player 2
    if (!bPlayer2Joined)
    {
        // (Optional) prevent same controller joining twice
        if (Player1Controller.Get() == PressingController)
        {
            UE_LOG(LogTemp, Warning, TEXT("Same controller tried to join as Player 2 – ignoring for now."));
            return;
        }

        bPlayer2Joined = true;
        Player2Controller = PressingController;

        UE_LOG(LogTemp, Log, TEXT("Player 2 joined with controller: %s"),
            *PressingController->GetName());

        OnPlayerJoined(2, PressingController);
        return;
    }
}