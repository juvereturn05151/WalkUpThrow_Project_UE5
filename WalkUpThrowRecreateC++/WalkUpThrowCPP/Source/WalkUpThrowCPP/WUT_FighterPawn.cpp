#include "WUT_FighterPawn.h"
#include "WUT_InputManager.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <XInput.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

AWUT_FighterPawn::AWUT_FighterPawn()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AWUT_FighterPawn::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    float X = GetHorizontalInput();
    if (FMath::Abs(X) > 0.1f)
    {
        FVector Loc = GetActorLocation();
        Loc.X += X * MoveSpeed * DeltaSeconds;
        SetActorLocation(Loc);
    }
}

float AWUT_FighterPawn::GetHorizontalInput() const
{
#if PLATFORM_WINDOWS
    if (PadIndex < 0) return 0.f;

    XINPUT_STATE State;
    ZeroMemory(&State, sizeof(State));

    if (XInputGetState(PadIndex, &State) == ERROR_SUCCESS)
    {
        // left stick on X
        return State.Gamepad.sThumbLX / 32767.f;
    }
#endif

    return 0.f;
}
