// WUT_MoveData.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WUT_MoveTypes.h"
#include "WUT_MoveData.generated.h"

UCLASS(BlueprintType)
class WALKUPTHROWCPP_API UWUT_MoveData : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName MoveName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EMoveType MoveType = EMoveType::Normal;

    // Frame data
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 StartupFrames = 5;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 ActiveFrames = 3;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 RecoveryFrames = 10;

    // Hit/cancel behavior
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FMoveHitProperties HitProps;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FMoveCancelData CancelData;

    // Hitboxes relative to the fighter, multiple per move
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FMoveHitbox> Hitboxes;
};
