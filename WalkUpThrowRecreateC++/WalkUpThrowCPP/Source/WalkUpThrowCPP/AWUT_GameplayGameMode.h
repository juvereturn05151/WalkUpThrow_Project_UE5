// WUT_GameplayGameMode.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AWUT_GameplayGameMode.generated.h"

class AWUT_FighterPawn;

UCLASS()
class WALKUPTHROWCPP_API AWUT_GameplayGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AWUT_GameplayGameMode();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Fighter")
    TSubclassOf<AWUT_FighterPawn> FighterClass;

    UPROPERTY()
    TArray<AWUT_FighterPawn*> Fighters;

    void SpawnFighters();
    void CheckCollisions();
    void CheckHitPair(AWUT_FighterPawn* Attacker, AWUT_FighterPawn* Defender);
};
