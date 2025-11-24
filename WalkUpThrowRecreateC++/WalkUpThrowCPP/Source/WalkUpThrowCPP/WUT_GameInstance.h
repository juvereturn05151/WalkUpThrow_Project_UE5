// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "WUT_GameInstance.generated.h"

/**
 * 
 */
UCLASS()
class WALKUPTHROWCPP_API UWUT_GameInstance : public UGameInstance
{
	GENERATED_BODY()
public:

    UPROPERTY(BlueprintReadWrite)
    int32 Player1PadIndex = -1;

    UPROPERTY(BlueprintReadWrite)
    int32 Player2PadIndex = -1;
};
