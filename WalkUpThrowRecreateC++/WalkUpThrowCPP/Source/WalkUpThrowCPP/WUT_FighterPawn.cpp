// WUT_FighterPawn.cpp

#include "WUT_FighterPawn.h"
#include "WUT_MoveData.h"
#include "PaperFlipbookComponent.h"
#include "PaperFlipbook.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <Xinput.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

AWUT_FighterPawn::AWUT_FighterPawn()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create a proper root (SceneComponent)
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

    // Create a Visual parent so sprite can be offset and edited
    VisualRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VisualRoot"));
    VisualRoot->SetupAttachment(RootComponent);

    // Create sprite as a child of VisualRoot
    Sprite = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("Sprite"));
    Sprite->SetupAttachment(VisualRoot);

    // Default sprite offset — can be adjusted in Blueprint instead of C++
    Sprite->SetRelativeLocation(FVector::ZeroVector);

    FloorZ = 0.f;
}

void AWUT_FighterPawn::BeginPlay()
{
    Super::BeginPlay();

    // Snap to floor
    FVector Loc = GetActorLocation();
    Loc.Z = FloorZ;
    SetActorLocation(Loc);
}

void AWUT_FighterPawn::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (CurrentState == EFighterState::KO)
    {
        ApplyVerticalMovement(DeltaSeconds);
        UpdateAnimation();
        return;
    }

    ReadPadInput();
    UpdateFacing();
    HandleState(DeltaSeconds);
    ApplyVerticalMovement(DeltaSeconds);
    UpdateAnimation();

    DrawHitboxes();
    DrawHurtbox();
    DrawWorkingDirection();
    DrawPushbox();
}

// --- Input ---

void AWUT_FighterPawn::ReadPadInput()
{
    InputX = 0.f;
    bCrMKPressed = bHadokenPressed = bThrowPressed = false;

#if PLATFORM_WINDOWS
    if (PadIndex < 0)
        return;

    XINPUT_STATE State;
    ZeroMemory(&State, sizeof(State));
    DWORD Result = XInputGetState(PadIndex, &State);
    if (Result != ERROR_SUCCESS)
        return;

    // Left stick X
    const float LX = State.Gamepad.sThumbLX / 32767.f;
    if (FMath::Abs(LX) > 0.2f)
        InputX = LX;

    bool bCrMKDownNow = (State.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0;
    bool bHadokenDownNow = (State.Gamepad.wButtons & XINPUT_GAMEPAD_X) != 0;
    bool bThrowDownNow = (State.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0;

    bCrMKPressed = bCrMKDownNow && !bCrMKDownPrev;
    bHadokenPressed = bHadokenDownNow && !bHadokenDownPrev;
    bThrowPressed = bThrowDownNow && !bThrowDownPrev;

    bCrMKDownPrev = bCrMKDownNow;
    bHadokenDownPrev = bHadokenDownNow;
    bThrowDownPrev = bThrowDownNow;
#endif
}

// --- Facing / movement ---

void AWUT_FighterPawn::UpdateFacing()
{
    if (!Opponent)
        return;

    float DiffX = Opponent->GetActorLocation().X - GetActorLocation().X;
    FacingDir = (DiffX >= 0.f) ? 1 : -1;

    FRotator Rot = GetActorRotation();
    Rot.Yaw = (FacingDir == 1) ? 0.f : 180.f;
    SetActorRotation(Rot);
}

void AWUT_FighterPawn::HandleGroundMovement(float DeltaSeconds)
{
    // ------------------ 1. Only movable in these states ------------------
    if (CurrentState != EFighterState::Neutral &&
        CurrentState != EFighterState::Walking &&
        CurrentState != EFighterState::Blocking)
        return;

    FVector Loc = GetActorLocation();
    Loc.Y = 0.f;
    Loc.Z = FloorZ;

    // ------------------ 2. Apply player movement ------------------
    if (FMath::Abs(InputX) > 0.2f && CurrentState != EFighterState::Blocking)
    {
        CurrentState = EFighterState::Walking;
        Loc.X += InputX * MoveSpeed * DeltaSeconds;
    }
    else if (CurrentState == EFighterState::Walking)
    {
        CurrentState = EFighterState::Neutral;
    }

    // ------------------ 3. Clamp to stage borders BEFORE resolving pushbox ------------------
    Loc.X = FMath::Clamp(Loc.X, StageLeftX, StageRightX);
    SetActorLocation(Loc);

    // ------------------ 4. PUSHBOX RESOLUTION ------------------
    if (Opponent &&
        CurrentState != EFighterState::Airborne &&
        CurrentState != EFighterState::BeingThrown &&
        CurrentState != EFighterState::KO)
    {
        FActiveHitbox A, B;
        GetPushbox(A);
        Opponent->GetPushbox(B);

        bool bOverlap =
            !(A.MaxX < B.MinX ||
                A.MinX > B.MaxX ||
                A.MaxZ < B.MinZ ||
                A.MinZ > B.MaxZ);

        if (bOverlap)
        {
            // --- get centers ---
            float CenterA = (A.MinX + A.MaxX) * 0.5f;
            float CenterB = (B.MinX + B.MaxX) * 0.5f;

            // --- half widths ---
            float HalfA = (A.MaxX - A.MinX) * 0.5f;
            float HalfB = (B.MaxX - B.MinX) * 0.5f;

            // --- penetration amount ---
            float Separation = HalfA + HalfB;
            float Actual = FMath::Abs(CenterA - CenterB);
            float Penetration = Separation - Actual;

            if (Penetration > 0.f)
            {
                float Direction = (CenterA < CenterB) ? -1.f : 1.f;

                // ------------------ NEW: wall-aware push distribution ------------------
                float ThisPush = Penetration * 0.5f * Direction;
                float OtherPush = -ThisPush;

                FVector SelfLoc = GetActorLocation();
                FVector OppLoc = Opponent->GetActorLocation();

                // If **this** fighter is cornered, push the opponent only
                bool bSelfCornered =
                    (SelfLoc.X <= StageLeftX + 1.f) ||
                    (SelfLoc.X >= StageRightX - 1.f);

                bool bOpponentCornered =
                    (OppLoc.X <= StageLeftX + 1.f) ||
                    (OppLoc.X >= StageRightX - 1.f);

                if (bSelfCornered && !bOpponentCornered)
                {
                    // Only push the opponent away
                    OppLoc.X += (Penetration * Direction * -1.f);
                }
                else if (bOpponentCornered && !bSelfCornered)
                {
                    // Only push this fighter away
                    SelfLoc.X += (Penetration * Direction);
                }
                else
                {
                    // Normal mutual pushback
                    SelfLoc.X += ThisPush;
                    OppLoc.X += OtherPush;
                }

                // ------------------ Clamp to stage borders ------------------
                SelfLoc.X = FMath::Clamp(SelfLoc.X, StageLeftX, StageRightX);
                OppLoc.X = FMath::Clamp(OppLoc.X, StageLeftX, StageRightX);

                // Apply results
                SetActorLocation(SelfLoc);
                Opponent->SetActorLocation(OppLoc);
            }
        }
    }
}


void AWUT_FighterPawn::HandleState(float DeltaSeconds)
{
    // Hitstun
    if (CurrentState == EFighterState::Hitstun)
    {
        if (HitstunFramesRemaining > 0)
        {
            HitstunFramesRemaining--;
            return;
        }
        ReturnToNeutral();
    }

    // Airborne / thrown physics handled by gravity
    if (CurrentState == EFighterState::Airborne )
    {
        // No ground control in air
        return;
    }

    // Attacking: Tick move
    if (CurrentState == EFighterState::Attacking|| CurrentState == EFighterState::Grab || CurrentState == EFighterState::Throwing)
    {
        TickMove();

        if (bCrMKPressed && CurrentMove == CrMKMove)
        {
            TryStartHadoken();
        }

        return;
    }

    // Blocking: stay blocking while holding back
    if (CurrentState == EFighterState::Blocking)
    {
        if (!IsHoldingBack())
        {
            ReturnToNeutral();
        }
    }

    // Grounded movement
    HandleGroundMovement(DeltaSeconds);

    // Try starting moves

    // Throw takes priority
    if (bThrowPressed)
    {
        TryStartThrow();
    }
    else
    {
        if (bCrMKPressed)
        {
            TryStartCrMK();
        }
    }

    // Proximity guard
    if (Opponent && CurrentState == EFighterState::Neutral)
    {
        float DistX = FMath::Abs(Opponent->GetActorLocation().X - GetActorLocation().X);
        if (DistX <= ProximityGuardDistance && IsHoldingBack())
        {
            CurrentState = EFighterState::Blocking;
        }
    }
}

void AWUT_FighterPawn::ApplyVerticalMovement(float DeltaSeconds)
{
    if (!bUseGravity)
        return;

    // Gravity affects vertical velocity
    VerticalVelocity += Gravity * DeltaSeconds;

    FVector Loc = GetActorLocation();

    // Apply horizontal KO / thrown movement along X
    if (CurrentState == EFighterState::Airborne ||
        CurrentState == EFighterState::BeingThrown)
    {
        Loc.X += HorizontalVelocityX * DeltaSeconds;
    }

    // Apply vertical movement
    Loc.Z += VerticalVelocity * DeltaSeconds;

    // Check landing
    if (Loc.Z <= FloorZ)
    {
        Loc.Z = FloorZ;
        SetActorLocation(Loc);

        bUseGravity = false;
        VerticalVelocity = 0.f;
        HorizontalVelocityX = 0.f; // stop sliding on ground

        if (bKOOnLanding)
        {
            FinishKO();
            bKOOnLanding = false;
        }
        else
        {
            CurrentState = EFighterState::Neutral;
        }
        return;
    }

    SetActorLocation(Loc);
}


// --- Move execution / cancel ---

void AWUT_FighterPawn::StartMove(UWUT_MoveData* MoveData)
{
    if (!MoveData)
        return;

    CurrentMove = MoveData;
    CurrentMoveFrame = 0;
    bHasConnectedThisMove = false;
    bInCancelWindow = false;
    bCanCancelOnHit = false;
    bCanCancelOnBlock = false;

    CurrentState = EFighterState::Attacking;
}

void AWUT_FighterPawn::TickMove()
{
    if (!CurrentMove)
        return;

    CurrentMoveFrame++;

    const int32 TotalFrames =
        CurrentMove->StartupFrames +
        CurrentMove->ActiveFrames +
        CurrentMove->RecoveryFrames;

    if (CurrentState == EFighterState::Throwing)
    {
        if (CurrentMoveFrame == ThrowReleaseFrame && Opponent)
        {
            Opponent->LaunchFromThrow(this);
        }

        if (CurrentMoveFrame >= Sprite->GetFlipbookLengthInFrames())
        {
            ReturnToNeutral();
        }

        return;
    }

    if (CurrentState == EFighterState::Grab)
    {
        if (CurrentMoveFrame == CurrentMove->StartupFrames)
        {
            TryGrabAttempt();
        }

        if (CurrentMoveFrame >= TotalFrames)
        {
            ReturnToNeutral();
        }

        return;
    }

    // Simple cancel window: allow cancel during active/recovery if flagged by cancel logic
    if (CurrentMoveFrame >= TotalFrames)
    {
        CurrentMove = nullptr;
        CurrentMoveFrame = 0;
        ReturnToNeutral();
    }
}

void AWUT_FighterPawn::TryGrabAttempt()
{
    float DistX = FMath::Abs(Opponent->GetActorLocation().X - GetActorLocation().X);

    if (DistX <= ThrowMove->ThrowRange)
    {
        // SUCCESS
        EnterThrowingState();
        Opponent->EnterBeingThrownState(this);
    }
    else
    {
        // FAIL -> Do nothing, the Grab move will finish & go to neutral
        UE_LOG(LogTemp, Log, TEXT("%s throw whiffed"), *GetName());
    }
}

void AWUT_FighterPawn::EnterBeingThrownState(AWUT_FighterPawn* Thrower)
{
    CurrentState = EFighterState::BeingThrown;
    CurrentMoveFrame = 0;

    bUseGravity = false;
    VerticalVelocity = 0;
    HorizontalVelocityX = 0;

    Sprite->SetFlipbook(BeingThrownFlipbook);
    Sprite->SetLooping(false);
    Sprite->Play();
}

void AWUT_FighterPawn::EnterThrowingState()
{
    CurrentState = EFighterState::Throwing;
    CurrentMoveFrame = 0;

    Sprite->SetFlipbook(ThrowingFlipbook);
    Sprite->SetLooping(false);
    Sprite->Play();
}

bool AWUT_FighterPawn::IsMoveActiveFrame(const UWUT_MoveData* MoveData, int32 Frame) const
{
    if (!MoveData)
        return false;

    int32 Start = MoveData->StartupFrames;
    int32 End = Start + MoveData->ActiveFrames;
    return (Frame >= Start && Frame < End);
}

void AWUT_FighterPawn::DrawHitboxes() const
{
    TArray<FActiveHitbox> Hitboxes;
    const UWUT_MoveData* Move = nullptr;

    if (GetActiveHitboxes(Hitboxes, Move))
    {
        for (const FActiveHitbox& HB : Hitboxes)
        {
            FVector Min(HB.MinX, 0.f, HB.MinZ);
            FVector Max(HB.MaxX, 0.f, HB.MaxZ);

            FVector Center = (Min + Max) * 0.5f;
            FVector Extent = (Max - Min) * 0.5f;

            DrawDebugBox(
                GetWorld(),
                Center,
                Extent,
                FColor::Red,
                false, 0.f, 0, 2.f
            );
        }
    }
}

void AWUT_FighterPawn::DrawHurtbox() const
{
    FActiveHitbox Hurt;
    GetHurtbox(Hurt);

    FVector Min(Hurt.MinX, 0.f, Hurt.MinZ);
    FVector Max(Hurt.MaxX, 0.f, Hurt.MaxZ);

    FVector Center = (Min + Max) * 0.5f;
    FVector Extent = (Max - Min) * 0.5f;

    DrawDebugBox(
        GetWorld(),
        Center,
        Extent,
        FColor::Blue,
        false, 0.f, 0, 2.f
    );
}

void AWUT_FighterPawn::DrawWorkingDirection() const
{
    FVector Start = GetActorLocation() + FVector(0, 0, 100);
    FVector End = Start + FVector(100 * FacingDir, 0, 0);

    DrawDebugLine(
        GetWorld(),
        Start,
        End,
        FColor::Green,
        false, 0.f, 0, 3.f
    );
}

void AWUT_FighterPawn::DrawPushbox() const
{
    FActiveHitbox PB;
    GetPushbox(PB);

    FVector Min(PB.MinX, 0.f, PB.MinZ);
    FVector Max(PB.MaxX, 0.f, PB.MaxZ);

    FVector Center = (Min + Max) * 0.5f;
    FVector Extent = (Max - Min) * 0.5f;

    DrawDebugBox(GetWorld(), Center, Extent, FColor::Yellow, false, 0.f, 0, 2.f);
}


// Start CrMK (Normal)
void AWUT_FighterPawn::TryStartCrMK()
{
    if (!CrMKMove)
        return;

    // Allowed if neutral, walking, or blocking (for blockstring special cancel style)
    if (CurrentState == EFighterState::Neutral ||
        CurrentState == EFighterState::Walking ||
        CurrentState == EFighterState::Blocking)
    {
        StartMove(CrMKMove);
    }
}

// Start Hadoken as special cancel
void AWUT_FighterPawn::TryStartHadoken()
{
    if (!HadokenMove)
    {
        return;
    }

    // Must currently be canceling from CrMK
    if (!bInCancelWindow)
    {
        return;
    }

    if (!CurrentMove)
    {
        return;
    }

    if (CurrentMove != CrMKMove)
    {
        return;
    }

    // Must press SAME button again
    if (!bCrMKPressed)
    {
        return;
    }

    // Check cancel rules
    const FName TargetName = HadokenMove->MoveName;
    bool bAllowed = false;

    if (bCanCancelOnHit)
    {
        bAllowed = CurrentMove->CancelData.CancelOnHit.Contains(TargetName);
    }
    else if (bCanCancelOnBlock)
    {
        bAllowed = CurrentMove->CancelData.CancelOnBlock.Contains(TargetName);
    }

    if (!bAllowed)
    {
        return;
    }

    StartMove(HadokenMove);

    // Clear cancel flags to avoid double cancellation
    bInCancelWindow = false;
    bCanCancelOnHit = false;
    bCanCancelOnBlock = false;
}


// Throw like Footsies: if in range, victim is KO with arc
void AWUT_FighterPawn::TryStartThrow()
{
    if (!ThrowMove) return;

    // Only allowed if neutral/walking/blocking
    if (CurrentState != EFighterState::Neutral &&
        CurrentState != EFighterState::Walking &&
        CurrentState != EFighterState::Blocking)
        return;

    // Make it behave like a move
    StartMove(ThrowMove);
    CurrentState = EFighterState::Grab;  // NEW
}


void AWUT_FighterPawn::EnterBeingThrown(AWUT_FighterPawn* Thrower)
{
    CurrentState = EFighterState::BeingThrown;
    CurrentMoveFrame = 0;

    // Freeze movement completely
    bUseGravity = false;
    VerticalVelocity = 0.f;
    HorizontalVelocityX = 0.f;

    // No player input
    InputX = 0.f;

    Sprite->SetFlipbook(BeingThrownFlipbook);
    Sprite->SetLooping(false);
    Sprite->Play();

    UE_LOG(LogTemp, Warning, TEXT("%s is being thrown by %s"),
        *GetName(), *Thrower->GetName());
}

void AWUT_FighterPawn::LaunchFromThrow(AWUT_FighterPawn* Thrower)
{
    CurrentState = EFighterState::Airborne;
    bUseGravity = true;

    VerticalVelocity = ThrowMove->ThrowLaunchVelocityZ;
    HorizontalVelocityX = ThrowMove->ThrowLaunchVelocityX * (Thrower->FacingDir);

    bKOOnLanding = true;

    Sprite->SetFlipbook(AirborneFlipbook);
    Sprite->SetLooping(false);
    Sprite->Play();
}

// --- Cancel/Hit receive ---

void AWUT_FighterPawn::OnHitByMove(AWUT_FighterPawn* Attacker, const UWUT_MoveData* MoveData, bool bWasBlocked)
{
    if (!MoveData || !Attacker)
        return;

    if (bWasBlocked)
    {
        // Enter simple blockstun
        int32 Frames = MoveData->HitProps.BlockstunFrames;
        if (Frames > 0)
            EnterHitstun(Frames);
        CurrentState = EFighterState::Blocking;

        // If this was CrMK and has CancelOnBlock, enable cancel window for attacker
        // (Attacker handles this; here we only manage this fighter's state.)
        return;
    }

    // On clean hit
    if (MoveData->HitProps.bAirborneKOOnHit)
    {
        // Hadoken or similar
        EnterAirborneKO(MoveData->HitProps);
    }
    else
    {
        // Normal hitstun (CrMK)
        EnterHitstun(MoveData->HitProps.HitstunFrames);
    }

    // --- Cancel flags for attacker ---
    if (MoveData->MoveName == "CrMK")
    {
        Attacker->bInCancelWindow = true;

        if (bWasBlocked)
            Attacker->bCanCancelOnBlock = true;
        else
            Attacker->bCanCancelOnHit = true;
    }
}

void AWUT_FighterPawn::ClearMoveState()
{
    CurrentMove = nullptr;
    CurrentMoveFrame = 0;
    bHasConnectedThisMove = false;
    bInCancelWindow = false;
    bCanCancelOnHit = false;
    bCanCancelOnBlock = false;
}

void AWUT_FighterPawn::EnterHitstun(int32 Frames)
{
    ClearMoveState();
    CurrentState = EFighterState::Hitstun;
    HitstunFramesRemaining = FMath::Max(Frames, 0);
}

// KO via air (Hadoken)
void AWUT_FighterPawn::EnterAirborneKO(const FMoveHitProperties& HitProps)
{
    CurrentState = EFighterState::Airborne;
    bUseGravity = true;

    // Vertical launch
    VerticalVelocity = HitProps.KOInitialVelocityZ;

    // Horizontal launch:
    // "back of itself" = opposite of victim's facing direction.
    // FacingDir: +1 = facing right, -1 = facing left.
    int32 BackDir = -FacingDir;

    HorizontalVelocityX = HitProps.KOInitialVelocityX * BackDir;

    bKOOnLanding = true;

    UE_LOG(LogTemp, Log, TEXT("%s EnterAirborneKO: VelX=%f VelZ=%f (FacingDir=%d)"),
        *GetName(), HorizontalVelocityX, VerticalVelocity, FacingDir);
}

// KO via throw
void AWUT_FighterPawn::EnterThrownKO(float InitialVelocityZ)
{
    /*CurrentState = EFighterState::Thrown;
    bUseGravity = true;
    VerticalVelocity = InitialVelocityZ;
    bKOOnLanding = true;*/
}

void AWUT_FighterPawn::FinishKO()
{
    CurrentState = EFighterState::KO;
    bUseGravity = false;
    VerticalVelocity = 0.f;

    UE_LOG(LogTemp, Log, TEXT("%s KO'd"), *GetName());

    // ---- NEW: Tell opponent to enter Win state ----
    if (Opponent && Opponent->CurrentState != EFighterState::KO)
    {
        Opponent->EnterWinState();
    }
}

void AWUT_FighterPawn::EnterWinState() 
{
    UE_LOG(LogTemp, Warning, TEXT("%s WON THE ROUND!"), *GetName());

    CurrentState = EFighterState::Win;
    bUseGravity = false;
    VerticalVelocity = 0.f;
    HorizontalVelocityX = 0.f;
}

void AWUT_FighterPawn::ReturnToNeutral()
{
    ClearMoveState();
    CurrentState = EFighterState::Neutral;
}

// --- Blocking / hurtbox / active hitboxes ---

bool AWUT_FighterPawn::IsHoldingBack() const
{
    if (!Opponent)
        return false;

    float DiffX = Opponent->GetActorLocation().X - GetActorLocation().X;
    int32 LocalFacing = (DiffX >= 0.f) ? 1 : -1;

    // If opponent to the right, back = negative input
    if (LocalFacing == 1)
        return InputX < -0.2f;
    else
        return InputX > 0.2f;
}

void AWUT_FighterPawn::GetHurtbox(FActiveHitbox& OutBox) const
{
    FVector Loc = GetActorLocation();

    // --- 1. If in attack, and move defines hurtboxes, use those ---
    if (CurrentMove)
    {
        for (const FMoveHurtbox& HB : CurrentMove->Hurtboxes)
        {
            if (CurrentMoveFrame >= HB.StartFrame &&
                CurrentMoveFrame < HB.EndFrame)
            {
                float LocalX = HB.Offset.X * FacingDir;
                float LocalZ = HB.Offset.Y;

                float CenterX = Loc.X + LocalX;
                float CenterZ = FloorZ + LocalZ;

                OutBox.MinX = CenterX - HB.HalfSize.X;
                OutBox.MaxX = CenterX + HB.HalfSize.X;
                OutBox.MinZ = CenterZ - HB.HalfSize.Y;
                OutBox.MaxZ = CenterZ + HB.HalfSize.Y;
                return;
            }
        }
    }

    // --- 2. Otherwise, use default idle/walk/block hurtbox ---
    float CenterX = Loc.X + HurtboxOffset.X;
    float CenterZ = FloorZ + HurtboxOffset.Y;

    OutBox.MinX = CenterX - HurtboxHalfSize.X;
    OutBox.MaxX = CenterX + HurtboxHalfSize.X;
    OutBox.MinZ = CenterZ - HurtboxHalfSize.Y;
    OutBox.MaxZ = CenterZ + HurtboxHalfSize.Y;
}


void AWUT_FighterPawn::GetPushbox(FActiveHitbox& OutBox) const
{
    FVector Loc = GetActorLocation();

    float CenterX = Loc.X + PushboxOffset.X;
    float CenterZ = FloorZ + PushboxOffset.Y;

    OutBox.MinX = CenterX - PushboxHalfSize.X;
    OutBox.MaxX = CenterX + PushboxHalfSize.X;
    OutBox.MinZ = CenterZ - PushboxHalfSize.Y;
    OutBox.MaxZ = CenterZ + PushboxHalfSize.Y;
}

bool AWUT_FighterPawn::GetActiveHitboxes(TArray<FActiveHitbox>& OutHitboxes, const UWUT_MoveData*& OutMove) const
{
    OutHitboxes.Reset();
    OutMove = nullptr;

    if (!CurrentMove)
        return false;

    if (!IsMoveActiveFrame(CurrentMove, CurrentMoveFrame))
        return false;

    // Build world-space AABBs for each local hitbox
    FVector Loc = GetActorLocation();

    for (const FMoveHitbox& HB : CurrentMove->Hitboxes)
    {
        if (CurrentMoveFrame < HB.StartFrame || CurrentMoveFrame >= HB.EndFrame)
            continue;

        // Local to world
        float LocalX = HB.Offset.X * FacingDir;
        float LocalZ = HB.Offset.Y;

        float CenterX = Loc.X + LocalX;
        float CenterZ = FloorZ + LocalZ;

        FActiveHitbox Box;
        Box.MinX = CenterX - HB.HalfSize.X;
        Box.MaxX = CenterX + HB.HalfSize.X;
        Box.MinZ = CenterZ - HB.HalfSize.Y;
        Box.MaxZ = CenterZ + HB.HalfSize.Y;

        OutHitboxes.Add(Box);
    }

    if (OutHitboxes.Num() > 0)
    {
        OutMove = CurrentMove;
        return true;
    }

    return false;
}

// --- Animation state machine (Paper2D flipbooks) ---

void AWUT_FighterPawn::UpdateAnimation()
{
    if (!Sprite)
        return;

    UPaperFlipbook* Desired = IdleFlipbook;

    switch (CurrentState)
    {
    case EFighterState::Neutral:
    case EFighterState::Walking:
    {
        // Decide walk vs idle
        if (FMath::Abs(InputX) > 0.2f)
        {
            // Walk forward/back relative to facing
            bool bForward = (InputX > 0.f && FacingDir > 0) || (InputX < 0.f && FacingDir < 0);
            Desired = bForward ? WalkForwardFlipbook : WalkBackwardFlipbook;
        }
        else
        {
            Desired = IdleFlipbook;
        }
    }
    break;
    case EFighterState::Blocking:
        Desired = BlockFlipbook;
        break;
    case EFighterState::Attacking:
        if (CurrentMove == CrMKMove)
            Desired = CrMKFlipbook;
        else if (CurrentMove == HadokenMove)
            Desired = HadokenFlipbook;
        else
            Desired = CrMKFlipbook;
        break;
    case EFighterState::Hitstun:
        Desired = HitstunFlipbook;
        break;
    case EFighterState::Airborne:
        Desired = AirborneFlipbook;
        break;
    case EFighterState::Grab:
        Desired = TryGrabFlipbook;
        break;

    case EFighterState::Throwing:
        Desired = ThrowingFlipbook;
        break;

    case EFighterState::BeingThrown:
        Desired = BeingThrownFlipbook;
        break;
    case EFighterState::KO:
        Desired = KOFlipbook;
        break;
    case EFighterState::Win:
        Desired = WinFlipbook;
        break;
    default:
        Desired = IdleFlipbook;
        break;
    }

    if (Desired && Sprite->GetFlipbook() != Desired)
    {
        Sprite->SetFlipbook(Desired);

        // Stop looping for specific animations
        if (Desired == CrMKFlipbook ||
            Desired == HadokenFlipbook ||
            Desired == AirborneFlipbook ||
            Desired == KOFlipbook ||
            Desired == WinFlipbook ||
            Desired == TryGrabFlipbook ||
            Desired == ThrowingFlipbook ||
            Desired == BeingThrownFlipbook)
        {
            Sprite->SetLooping(false);
        }
        else
        {
            Sprite->SetLooping(true);    // idle/walk/block loops
        }

        Sprite->Play();
    }
}
