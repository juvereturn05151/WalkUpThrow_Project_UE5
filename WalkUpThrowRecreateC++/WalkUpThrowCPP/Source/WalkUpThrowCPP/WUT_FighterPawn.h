// WUT_FighterPawn.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "WUT_FighterState.h"
#include "WUT_MoveTypes.h"
#include "WUT_FighterPawn.generated.h"

class UWUT_MoveData;
class UPaperFlipbook;
class UPaperFlipbookComponent;
class AWUT_FighterPawn;
class DrawDebugHelpers;

USTRUCT()
struct FActiveHitbox
{
    GENERATED_BODY()

    // World-space AABB on X/Z
    float MinX, MaxX;
    float MinZ, MaxZ;
};


UENUM(BlueprintType)
enum class EInputDeviceType : uint8
{
    None,
    KeyboardA,
    KeyboardB,
    Gamepad
};

UCLASS()
class WALKUPTHROWCPP_API AWUT_FighterPawn : public APawn
{
    GENERATED_BODY()

public:
    AWUT_FighterPawn();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    // Assigned externally (Menu/Gameplay) to know which pad drives this fighter
    UPROPERTY(BlueprintReadWrite, Category = "Fighter")
    int32 PadIndex = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EInputDeviceType InputDevice = EInputDeviceType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GamepadIndex = -1; // Only used for Gamepads

    UPROPERTY(BlueprintReadWrite, Category = "Fighter")
    AWUT_FighterPawn* Opponent = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Fighter")
    EFighterState CurrentState = EFighterState::Neutral;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 Health = 3;

    UFUNCTION(BlueprintCallable)
    void LoseHealth();

    void CheckWinAnimationFinished();

    UFUNCTION(BlueprintCallable)
    void OnRoundReset();

    UFUNCTION(BlueprintCallable)
    bool IsStartPressed() const;

    // 2D movement config
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fighter|Config")
    float MoveSpeed = 350.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fighter|Config")
    float FloorZ = 0.f;

    // Hurtbox: local center/size on X/Z
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fighter|Hurtbox")
    FVector2D HurtboxOffset = FVector2D(0.f, 50.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fighter|Hurtbox")
    FVector2D HurtboxHalfSize = FVector2D(40.f, 50.f);

    // Throw range
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fighter|Throw")
    float ThrowRange = 80.f;
    // Throw timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fighter|Throw")
    int32 ThrowFreezeFrames = 20;     // Opponent locked in place

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fighter|Throw")
    int32 ThrowReleaseFrame = 15;     // At this frame, opponent is launched

    // Proximity guard
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fighter|Defense")
    float ProximityGuardDistance = 200.f;

    // Gravity for KO arcs
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fighter|Physics")
    float Gravity = -2000.f;

    // Move data references
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fighter|Moves")
    UWUT_MoveData* CrMKMove;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fighter|Moves")
    UWUT_MoveData* HadokenMove;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fighter|Moves")
    UWUT_MoveData* ThrowMove; // optional, for future use

    // Pushbox (body collision)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fighter|Pushbox")
    FVector2D PushboxOffset = FVector2D(0.f, 50.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fighter|Pushbox")
    FVector2D PushboxHalfSize = FVector2D(40.f, 50.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage")
    float StageLeftX = -800.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage")
    float StageRightX = 800.f;

    // Paper2D sprite + flipbooks
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
    UPaperFlipbookComponent* Sprite;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
    UPaperFlipbook* IdleFlipbook;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
    UPaperFlipbook* WalkForwardFlipbook;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
    UPaperFlipbook* WalkBackwardFlipbook;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
    UPaperFlipbook* CrMKFlipbook;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
    UPaperFlipbook* HadokenFlipbook;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
    UPaperFlipbook* BlockFlipbook;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
    UPaperFlipbook* HitstunFlipbook;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
    UPaperFlipbook* AirborneFlipbook;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
    UPaperFlipbook* TryGrabFlipbook;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
    UPaperFlipbook* ThrowingFlipbook;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
    UPaperFlipbook* BeingThrownFlipbook;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
    UPaperFlipbook* KOFlipbook;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
    UPaperFlipbook* WinFlipbook;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
    USceneComponent* VisualRoot;

    // === Interface for GameMode / collision system ===

    // Returns all active hitboxes this frame in world space
    bool GetActiveHitboxes(TArray<FActiveHitbox>& OutHitboxes, const UWUT_MoveData*& OutMove) const;

    // Compute world-space hurtbox for collision
    void GetHurtbox(FActiveHitbox& OutBox) const;

    void GetPushbox(FActiveHitbox& OutBox) const;

    bool IsKO() const { return CurrentState == EFighterState::KO; }

    bool IsHoldingBack() const;

    bool IsBlocking() const { return CurrentState == EFighterState::Blocking; }

    // Called by collision resolution when this fighter is hit
    void OnHitByMove(AWUT_FighterPawn* Attacker, const UWUT_MoveData* MoveData, bool bWasBlocked);

    void ClearMoveState();

protected:
    // Facing: +1 = right, -1 = left
    int32 FacingDir = 1;

    // Input state
    float InputX = 0.f;
    bool bCrMKPressed = false;
    bool bHadokenPressed = false;
    bool bThrowPressed = false;

    bool bCrMKDownPrev = false;
    bool bHadokenDownPrev = false;
    bool bThrowDownPrev = false;
    bool bStartPressed = false;
    bool bStartDownPrev = false;
    

    // Vertical physics (for KO arcs)
    bool bUseGravity = false;
    float VerticalVelocity = 0.f;
    float HorizontalVelocityX = 0.f;
    bool bKOOnLanding = false;

    // Hitstun
    int32 HitstunFramesRemaining = 0;

    // Move executor state
    UPROPERTY()
    UWUT_MoveData* CurrentMove = nullptr;

    int32 CurrentMoveFrame = 0;
    bool bHasConnectedThisMove = false;  // to avoid multi-hits if you want

    // Cancel state
    bool bInCancelWindow = false;
    bool bCanCancelOnHit = false;
    bool bCanCancelOnBlock = false;

    // --- Internal helpers ---
    void ReadPadInput();
    bool IsKeyDown(FKey Key) const;
    void UpdateFacing();
    void HandleGroundMovement(float DeltaSeconds);
    void HandleState(float DeltaSeconds);
    void ApplyVerticalMovement(float DeltaSeconds);
    void UpdateAnimation();

    void TryStartCrMK();
    void TryStartHadoken();
    void TryStartThrow();
    void EnterBeingThrown(AWUT_FighterPawn* Thrower);
    void LaunchFromThrow(AWUT_FighterPawn* Thrower);
    void EnterBeingThrownState(AWUT_FighterPawn* Thrower);
    void EnterThrowingState();
    void TryGrabAttempt();
    void EnterWinState();

    void StartMove(UWUT_MoveData* MoveData);
    void TickMove();

    void EnterHitstun(int32 Frames);
    void EnterAirborneKO(const FMoveHitProperties& HitProps);
    void EnterThrownKO(float InitialVelocityZ);
    void FinishKO();
    void ReturnToNeutral();

    bool IsMoveActiveFrame(const UWUT_MoveData* MoveData, int32 Frame) const;

	void DrawHitboxes() const;
	void DrawHurtbox() const;
	void DrawWorkingDirection() const;
    void DrawPushbox() const;
};
