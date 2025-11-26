#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "WUT_MoveDataRow.generated.h"

USTRUCT(BlueprintType)
struct FWUT_MoveDataRow : public FTableRowBase
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName MoveName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Startup = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Active = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Recovery = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Hitstun = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Blockstun = 0;
};
