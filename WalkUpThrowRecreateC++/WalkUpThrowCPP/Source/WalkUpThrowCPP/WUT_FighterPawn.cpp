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
    if (CurrentState != EFighterState::Neutral &&
        CurrentState != EFighterState::Walking &&
        CurrentState != EFighterState::Blocking)
        return;

    FVector Loc = GetActorLocation();
    Loc.Y = 0.f;
    Loc.Z = FloorZ;

    if (FMath::Abs(InputX) > 0.2f && CurrentState != EFighterState::Blocking)
    {
        CurrentState = EFighterState::Walking;
        Loc.X += InputX * MoveSpeed * DeltaSeconds;
    }
    else if (CurrentState == EFighterState::Walking)
    {
        CurrentState = EFighterState::Neutral;
    }

    SetActorLocation(Loc);
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
    if (CurrentState == EFighterState::Airborne ||
        CurrentState == EFighterState::Thrown)
    {
        // No ground control in air
        return;
    }

    // Attacking: Tick move
    if (CurrentState == EFighterState::Attacking)
    {
        TickMove();
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

        if (bHadokenPressed)
        {
            TryStartHadoken();
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

    VerticalVelocity += Gravity * DeltaSeconds;

    FVector Loc = GetActorLocation();
    Loc.Z += VerticalVelocity * DeltaSeconds;

    if (Loc.Z <= FloorZ)
    {
        Loc.Z = FloorZ;
        SetActorLocation(Loc);

        bUseGravity = false;
        VerticalVelocity = 0.f;

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

    // Simple cancel window: allow cancel during active/recovery if flagged by cancel logic
    if (CurrentMoveFrame >= TotalFrames)
    {
        CurrentMove = nullptr;
        CurrentMoveFrame = 0;
        ReturnToNeutral();
    }
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
        return;

    // Special cancel only: must be in cancel window & target move allowed by cancel data
    if (!bInCancelWindow || !CurrentMove)
        return;

    const FName TargetName = HadokenMove->MoveName;

    // Decide if we can cancel based on last hit/block
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
        return;

    // Do the cancel
    StartMove(HadokenMove);
}

// Throw like Footsies: if in range, victim is KO with arc
void AWUT_FighterPawn::TryStartThrow()
{
    if (!Opponent)
        return;

    if (CurrentState != EFighterState::Neutral &&
        CurrentState != EFighterState::Walking &&
        CurrentState != EFighterState::Blocking)
        return;

    float DistX = FMath::Abs(Opponent->GetActorLocation().X - GetActorLocation().X);
    if (DistX <= ThrowRange)
    {
        // Attacker enters Throw state (for animation)
        CurrentState = EFighterState::Thrown; // or a separate Throwing state if you want
        // Victim KO arc
        Opponent->EnterThrownKO(800.f);
    }
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
        EnterAirborneKO(MoveData->HitProps.KOInitialVelocityZ);
    }
    else
    {
        // Normal hitstun (CrMK)
        EnterHitstun(MoveData->HitProps.HitstunFrames);
    }
}

void AWUT_FighterPawn::EnterHitstun(int32 Frames)
{
    CurrentState = EFighterState::Hitstun;
    HitstunFramesRemaining = FMath::Max(Frames, 0);
}

// KO via air (Hadoken)
void AWUT_FighterPawn::EnterAirborneKO(float InitialVelocityZ)
{
    CurrentState = EFighterState::Airborne;
    bUseGravity = true;
    VerticalVelocity = InitialVelocityZ;
    bKOOnLanding = true;
}

// KO via throw
void AWUT_FighterPawn::EnterThrownKO(float InitialVelocityZ)
{
    CurrentState = EFighterState::Thrown;
    bUseGravity = true;
    VerticalVelocity = InitialVelocityZ;
    bKOOnLanding = true;
}

void AWUT_FighterPawn::FinishKO()
{
    CurrentState = EFighterState::KO;
    bUseGravity = false;
    VerticalVelocity = 0.f;

    UE_LOG(LogTemp, Log, TEXT("%s KO'd"), *GetName());
}

void AWUT_FighterPawn::ReturnToNeutral()
{
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
    float CenterX = Loc.X + HurtboxOffset.X;
    float CenterZ = FloorZ + HurtboxOffset.Y;

    OutBox.MinX = CenterX - HurtboxHalfSize.X;
    OutBox.MaxX = CenterX + HurtboxHalfSize.X;
    OutBox.MinZ = CenterZ - HurtboxHalfSize.Y;
    OutBox.MaxZ = CenterZ + HurtboxHalfSize.Y;
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
    case EFighterState::Thrown:
        Desired = ThrowFlipbook;
        break;
    case EFighterState::KO:
        Desired = KOFlipbook;
        break;
    default:
        Desired = IdleFlipbook;
        break;
    }

    if (Desired && Sprite->GetFlipbook() != Desired)
    {
        Sprite->SetFlipbook(Desired);
        Sprite->Play();
    }
}
