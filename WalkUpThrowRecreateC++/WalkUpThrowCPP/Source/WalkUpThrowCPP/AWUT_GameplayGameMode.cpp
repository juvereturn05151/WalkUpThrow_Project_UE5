// WUT_GameplayGameMode.cpp

#include "AWUT_GameplayGameMode.h"
#include "WUT_FighterPawn.h"
#include "WUT_GameInstance.h"
#include "WUT_MoveData.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

AWUT_GameplayGameMode::AWUT_GameplayGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AWUT_GameplayGameMode::BeginPlay()
{
    Super::BeginPlay();
    SpawnFighters();
}

void AWUT_GameplayGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    CheckCollisions();
}

void AWUT_GameplayGameMode::SpawnFighters()
{
    UWorld* World = GetWorld();
    if (!World || !FighterClass)
        return;

    FVector P1Loc(-200.f, 0.f, 0.f);
    FVector P2Loc(+160.f, 0.f, 0.f);
    FRotator Rot = FRotator::ZeroRotator;

    AWUT_FighterPawn* P1 = World->SpawnActor<AWUT_FighterPawn>(FighterClass, P1Loc, Rot);
    AWUT_FighterPawn* P2 = World->SpawnActor<AWUT_FighterPawn>(FighterClass, P2Loc, Rot);

    if (P1 && P2)
    {
        Fighters.Empty();
        Fighters.Add(P1);
        Fighters.Add(P2);

        P1->Opponent = P2;
        P2->Opponent = P1;

        if (UWUT_GameInstance* GI = GetGameInstance<UWUT_GameInstance>())
        {
            P1->PadIndex = GI->Player1PadIndex;
            P2->PadIndex = GI->Player2PadIndex;
        }
        else
        {
            P1->PadIndex = 0;
            P2->PadIndex = 1;
        }
    }
}

void AWUT_GameplayGameMode::CheckCollisions()
{
    if (Fighters.Num() < 2)
        return;

    CheckHitPair(Fighters[0], Fighters[1]);
    CheckHitPair(Fighters[1], Fighters[0]);
}

void AWUT_GameplayGameMode::CheckHitPair(AWUT_FighterPawn* Attacker, AWUT_FighterPawn* Defender)
{
    if (!Attacker || !Defender)
        return;

    if (Attacker->IsKO() || Defender->IsKO())
        return;

    TArray<FActiveHitbox> Hitboxes;
    const UWUT_MoveData* Move = nullptr;

    if (!Attacker->GetActiveHitboxes(Hitboxes, Move) || !Move)
        return;

    FActiveHitbox Hurt;
    Defender->GetHurtbox(Hurt);

    auto Overlaps = [](const FActiveHitbox& A, const FActiveHitbox& B)
        {
            return !(A.MaxX < B.MinX || A.MinX > B.MaxX ||
                A.MaxZ < B.MinZ || A.MinZ > B.MaxZ);
        };

    bool bAnyHit = false;
    for (const FActiveHitbox& HB : Hitboxes)
    {
        if (Overlaps(HB, Hurt))
        {
            bAnyHit = true;
            break;
        }
    }

    if (!bAnyHit)
        return;

    // Determine block
    bool bBlocked = Defender->IsBlocking() || Defender->IsHoldingBack();

    // Apply hit/block effects
    Defender->OnHitByMove(Attacker, Move, bBlocked);

    // Special-cancel: CrMK -> Hadoken on hit OR block
    if (Move->MoveName == TEXT("CrMK"))
    {
        // Let attacker know they can special cancel to Hadoken; we don't track full
        // cancel state here to keep this version smaller; instead, we rely on:
        // attacker’s MoveData.CancelData having "Hadoken" in CancelOnHit/CancelOnBlock
        // and our TryStartHadoken() checking bCanCancelOnHit / bCanCancelOnBlock.
        //
        // To finish wiring this fully, you'd add friends/accessors in AWUT_FighterPawn
        // and set bInCancelWindow + bCanCancelOnHit / bCanCancelOnBlock here.
        //
        // For now, you can make cancel always available during CrMK recovery and
        // gate it simply by MoveData.CancelData lists in TryStartHadoken().
        //
        // (If you want, we can refine this part next.)
    }

    UE_LOG(LogTemp, Log, TEXT("%s %s %s with %s"),
        *Attacker->GetName(),
        bBlocked ? TEXT("hit-blocked") : TEXT("hit"),
        *Defender->GetName(),
        *Move->MoveName.ToString());
}
