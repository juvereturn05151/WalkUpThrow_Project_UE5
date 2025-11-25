// WUT_GameplayGameMode.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AWUT_GameplayGameMode.generated.h"

class AWUT_FighterPawn;
class UWUT_HealthUI;

UCLASS()
class WALKUPTHROWCPP_API AWUT_GameplayGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AWUT_GameplayGameMode();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UFUNCTION(BlueprintCallable)
    void HandleGameOver(AWUT_FighterPawn* Loser);

    void UpdateHealthUI();

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Fighter")
    TSubclassOf<AWUT_FighterPawn> FighterClass;

    UPROPERTY()
    TArray<AWUT_FighterPawn*> Fighters;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TSubclassOf<UWUT_HealthUI> HealthUIClass;

    UWUT_HealthUI* HealthUI = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TSubclassOf<UUserWidget> GameOverWidgetClass;



    UUserWidget* GameOverWidget = nullptr;

    bool bGameOver = false;

    void SpawnFighters();
    void CheckCollisions();
    void CheckHitPair(AWUT_FighterPawn* Attacker, AWUT_FighterPawn* Defender);


};
