// WUT_FighterState.h

#pragma once

#include "CoreMinimal.h"
#include "WUT_FighterState.generated.h"

UENUM(BlueprintType)
enum class EFighterState : uint8
{
    Neutral    UMETA(DisplayName = "Neutral"),
    Walking    UMETA(DisplayName = "Walking"),
    Blocking   UMETA(DisplayName = "Blocking"),
    Attacking  UMETA(DisplayName = "Attacking"),
    Hitstun    UMETA(DisplayName = "Hitstun"),
    Airborne   UMETA(DisplayName = "Airborne"),
    Grab            UMETA(DisplayName = "Grab"),          // NEW (attempting to grab)
    Throwing        UMETA(DisplayName = "Throwing"),      // NEW (performing the throw)
    BeingThrown     UMETA(DisplayName = "BeingThrown"),   // NEW (opponent locked in throw)
    KO         UMETA(DisplayName = "KO"),
    Win        UMETA(DisplayName = "Win")
};
