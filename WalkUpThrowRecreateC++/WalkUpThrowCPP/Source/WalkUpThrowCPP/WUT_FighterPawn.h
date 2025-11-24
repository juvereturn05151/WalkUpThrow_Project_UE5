// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "WUT_FighterPawn.generated.h"

UCLASS()
class WALKUPTHROWCPP_API AWUT_FighterPawn : public APawn
{
	GENERATED_BODY()

public:
    AWUT_FighterPawn();

    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(BlueprintReadWrite)
    int32 PadIndex = -1;

protected:
    UPROPERTY(EditAnywhere)
    float MoveSpeed = 300.f;

    float GetHorizontalInput() const;

};
