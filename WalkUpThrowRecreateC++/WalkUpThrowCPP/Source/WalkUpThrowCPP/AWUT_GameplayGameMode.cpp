// WUT_GameplayGameMode.cpp

#include "AWUT_GameplayGameMode.h"
#include "WUT_FighterPawn.h"
#include "WUT_GameInstance.h"
#include "WUT_MoveData.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Blueprint/UserWidget.h" 
#include "Kismet/GameplayStatics.h"
#include "WUT_HealthUI.h"
#include "WUT_RoundUI.h"

AWUT_GameplayGameMode::AWUT_GameplayGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AWUT_GameplayGameMode::BeginPlay()
{
    Super::BeginPlay();
    SpawnFighters();

    if (HealthUIClass)
    {
        HealthUI = CreateWidget<UWUT_HealthUI>(GetWorld(), HealthUIClass);
        if (HealthUI)
        {
            HealthUI->AddToViewport();
        }
    }


    if (RoundUIClass)
    {
        RoundUI = CreateWidget<UWUT_RoundUI>(GetWorld(), RoundUIClass);
        if (RoundUI)
        {
            RoundUI->AddToViewport();
        }
    }

    ShowReadyFight();
}

void AWUT_GameplayGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    CheckCollisions();

    if (!bGameOver)
        return;

    // Check if either player presses Start
    for (AWUT_FighterPawn* Fighter : Fighters)
    {
        if (Fighter && Fighter->IsStartPressed())
        {
            UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()));
        }
    }
}

void AWUT_GameplayGameMode::HandleGameOver(AWUT_FighterPawn* Loser)
{
    if (bGameOver)
        return;

    bGameOver = true;

    // Show GameOver UI
    if (GameOverWidgetClass)
    {
        GameOverWidget = CreateWidget<UUserWidget>(GetWorld(), GameOverWidgetClass);
        if (GameOverWidget)
        {
            GameOverWidget->AddToViewport();
        }
    }

    // Freeze fighters
    for (AWUT_FighterPawn* Fighter : Fighters)
    {
        if (Fighter)
            Fighter->DisableInput(nullptr);
    }
}

void AWUT_GameplayGameMode::ShowReadyFight()
{
    // Freeze both fighters
    for (AWUT_FighterPawn* Fighter : Fighters)
        if (Fighter) Fighter->bCanControl = false;


    // UI: READY
    if (RoundUI)
        RoundUI->ShowReady();

    // After 0.6 sec -> FIGHT
    FTimerHandle Timer1;
    GetWorld()->GetTimerManager().SetTimer(
        Timer1,
        [this]()
        {
            if (RoundUI)
                RoundUI->ShowFight();

            // After 0.35 sec -> hide and resume gameplay
            FTimerHandle Timer2;
            GetWorld()->GetTimerManager().SetTimer(
                Timer2,
                [this]()
                {
                    if (RoundUI)
                        RoundUI->HideAll();

                    // Re-enable fighters
                    for (AWUT_FighterPawn* Fighter : Fighters)
                        if (Fighter) Fighter->bCanControl = true;
                },
                0.35f,
                false
            );
        },
        0.6f,
        false
    );
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
            if (GI->Player1PadIndex == 100)
            {
                P1->InputDevice = EInputDeviceType::KeyboardA;
            }
            else if(GI->Player1PadIndex == 101)
            {
                P1->InputDevice = EInputDeviceType::KeyboardB;
            }
            else 
            {
                P1->InputDevice = EInputDeviceType::Gamepad;
                P1->GamepadIndex = GI->Player1PadIndex;
            }

            if (GI->Player2PadIndex == 100)
            {
                P2->InputDevice = EInputDeviceType::KeyboardA;
            }
            else if (GI->Player2PadIndex == 101)
            {
                P2->InputDevice = EInputDeviceType::KeyboardB;
            }
            else
            {
                P2->InputDevice = EInputDeviceType::Gamepad;
                P2->GamepadIndex = GI->Player2PadIndex;
            }

            P1->PadIndex = GI->Player1PadIndex;
            P2->PadIndex = GI->Player2PadIndex;
        }
        else
        {
            P1->InputDevice = EInputDeviceType::Gamepad;
            P2->InputDevice = EInputDeviceType::Gamepad;

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

void AWUT_GameplayGameMode::UpdateHealthUI()
{
    if (!HealthUI) return;

    if (Fighters.Num() == 2)
    {
        HealthUI->SetPlayerHealth(0, Fighters[0]->Health);
        HealthUI->SetPlayerHealth(1, Fighters[1]->Health);
    }
}

void AWUT_GameplayGameMode::HandleRoundReset()
{
    // Disable input during reset
    for (AWUT_FighterPawn* Fighter : Fighters)
    {
        if (Fighter)
            Fighter->DisableInput(nullptr);
    }

    // Respawn location like Footsies
    FVector P1Loc(-200.f, 0.f, 0.f);
    FVector P2Loc(+160.f, 0.f, 0.f);

    ResetFighter(Fighters[0], P1Loc);
    ResetFighter(Fighters[1], P2Loc);

    // Optional: small pause (Footsies freeze)
    FTimerHandle Timer;
    GetWorldTimerManager().SetTimer(
        Timer,
        [this]()
        {
            // Re-enable inputs
            for (AWUT_FighterPawn* Fighter : Fighters)
                Fighter->EnableInput(nullptr);
        },
        0.5f,   // freeze duration
        false
    );

    ShowReadyFight();
}

void AWUT_GameplayGameMode::ResetFighter(AWUT_FighterPawn* Fighter, const FVector& NewLocation)
{
    if (!Fighter) return;

    // Reset position instantly
    Fighter->SetActorLocation(NewLocation);
    Fighter->SetActorRotation(FRotator::ZeroRotator);

    // Reset movement state / custom data
    Fighter->OnRoundReset(); // call into pawn for custom reset
}

