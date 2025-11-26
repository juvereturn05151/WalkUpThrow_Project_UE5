#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "WUT_MoveDataImporter.generated.h"

class UWUT_MoveData;

UCLASS(BlueprintType, Blueprintable, EditInlineNew)
class WALKUPTHROWCPPEDITOR_API  UWUT_MoveDataImporter : public UObject
{
    GENERATED_BODY()

public:

    UFUNCTION(CallInEditor, Category = "Move Data")
    void ImportCSV();

    UPROPERTY(EditAnywhere, Category = "Move Data")
    FString CSVRelativePath = "/Game/CSV/WalkUpThrow_movedata.csv";

    UPROPERTY(EditAnywhere, Category = "Move Data")
    FString MoveDataFolder = "/Game/MoveData_Asset";
};
