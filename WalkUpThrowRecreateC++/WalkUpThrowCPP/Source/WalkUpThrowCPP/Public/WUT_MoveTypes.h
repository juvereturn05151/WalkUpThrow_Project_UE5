// WUT_MoveTypes.h

#pragma once

#include "CoreMinimal.h"
#include "WUT_MoveTypes.generated.h"

UENUM(BlueprintType)
enum class EMoveType : uint8
{
    Normal   UMETA(DisplayName = "Normal"),
    Special  UMETA(DisplayName = "Special"),
    Throw    UMETA(DisplayName = "Throw")
};

// Local-space hitbox, 2D on X and Z
USTRUCT(BlueprintType)
struct FMoveHitbox
{
    GENERATED_BODY()

    // Frame range [StartFrame, EndFrame)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 StartFrame = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 EndFrame = 0;

    // Local-space center offset (X/Z), Y ignored
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D Offset = FVector2D(50.f, 50.f);

    // Half size (X/Z)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D HalfSize = FVector2D(25.f, 25.f);
};

USTRUCT(BlueprintType)
struct FMoveCancelData
{
    GENERATED_BODY()

    // Moves this move can special cancel into ON HIT
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> CancelOnHit;

    // Moves this move can special cancel into ON BLOCK
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> CancelOnBlock;

    // For now we don't support whiff cancel
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanCancelOnWhiff = false;
};

USTRUCT(BlueprintType)
struct FMoveHitProperties
{
    GENERATED_BODY()

    // Frames of hitstun on hit
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 HitstunFrames = 10;

    // Frames of blockstun on block
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BlockstunFrames = 8;

    // If true, this move causes an airborne KO on hit (like Hadoken)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAirborneKOOnHit = false;

    // Initial upward velocity for KO arc
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float KOInitialVelocityZ = 800.f;

    // Initial horizontal velocity for KO arc
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float KOInitialVelocityX = 300.f;

    // If true, X velocity gets flipped to knock opponent away
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bKOReverseFacing = true;

};

USTRUCT(BlueprintType)
struct FMoveHurtbox
{
    GENERATED_BODY()

    // Frame range [StartFrame, EndFrame)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 StartFrame = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 EndFrame = 0;

    // Local-space center offset (X/Z)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D Offset = FVector2D(0.f, 50.f);

    // Half size of hurtbox
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D HalfSize = FVector2D(40.f, 60.f);
};

