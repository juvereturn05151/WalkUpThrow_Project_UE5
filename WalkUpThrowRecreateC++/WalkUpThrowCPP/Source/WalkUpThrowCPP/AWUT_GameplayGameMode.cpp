// Fill out your copyright notice in the Description page of Project Settings.


#include "AWUT_GameplayGameMode.h"
#include "WUT_FighterPawn.h"
#include "WUT_GameInstance.h"
#include "Engine/World.h"

void AAWUT_GameplayGameMode::BeginPlay()
{
    Super::BeginPlay();
    SpawnFighters();
}

void AAWUT_GameplayGameMode::SpawnFighters()
{
    UWorld* World = GetWorld();
    if (!World) return;

    UWUT_GameInstance* GI = GetGameInstance<UWUT_GameInstance>();
    if (!GI) return;

    FVector P1Loc(-200.f, 0.f, 100.f);
    FVector P2Loc(+200.f, 0.f, 100.f);
    FRotator Rot = FRotator::ZeroRotator;

    // Spawn Player 1
    AWUT_FighterPawn* P1 = World->SpawnActor<AWUT_FighterPawn>(FighterClass, P1Loc, Rot);
    if (P1)
    {
        P1->PadIndex = GI->Player1PadIndex;
    }

    // Spawn Player 2
    AWUT_FighterPawn* P2 = World->SpawnActor<AWUT_FighterPawn>(FighterClass, P2Loc, Rot);
    if (P2)
    {
        P2->PadIndex = GI->Player2PadIndex;
    }
}
