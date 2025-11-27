// (c) 2025 MyLoyalFans. All rights reserved.

#include "WUT_MoveDataImporter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorAssetLibrary.h"
#include "WalkUpThrowCPP/Public/WUT_MoveData.h"

void UWUT_MoveDataImporter::ImportCSV()
{
    UE_LOG(LogTemp, Warning, TEXT("=== Import MoveData from CSV ==="));

    // ---------- 1. Load CSV file ----------
    FString AbsolutePath = FPaths::ConvertRelativePathToFull(
        FPaths::ProjectContentDir() + CSVRelativePath.Replace(TEXT("/Game/"), TEXT(""))
    );

    FString FileContent;
    if (!FFileHelper::LoadFileToString(FileContent, *AbsolutePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load CSV file: %s"), *AbsolutePath);
        return;
    }

    // Split CSV into lines
    TArray<FString> Lines;
    FileContent.ParseIntoArrayLines(Lines);

    if (Lines.Num() <= 1)
    {
        UE_LOG(LogTemp, Error, TEXT("CSV is empty or missing header."));
        return;
    }

    // ---------- 2. Load MoveData assets ----------
    FARFilter Filter;
    Filter.PackagePaths.Add(*MoveDataFolder);   // ex: "/Game/MoveData_Asset"
    Filter.bRecursivePaths = true;

    TArray<FAssetData> FoundAssets;
    FAssetRegistryModule::GetRegistry().GetAssets(Filter, FoundAssets);

    // Create a lookup map: MoveName → Object
    TMap<FName, UWUT_MoveData*> MoveDataMap;

    for (FAssetData& Asset : FoundAssets)
    {
        if (UWUT_MoveData* MoveAsset = Cast<UWUT_MoveData>(Asset.GetAsset()))
        {
            MoveDataMap.Add(MoveAsset->MoveName, MoveAsset);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Found %d MoveData assets"), MoveDataMap.Num());


    // ---------- 3. Parse each CSV line ----------
    for (int32 i = 1; i < Lines.Num(); ++i)  // skip header
    {
        if (Lines[i].TrimStartAndEnd().IsEmpty())
            continue;

        TArray<FString> Columns;
        Lines[i].ParseIntoArray(Columns, TEXT(","), true);

        if (Columns.Num() < 7)
        {
            UE_LOG(LogTemp, Warning, TEXT("Row %d has insufficient columns"), i);
            continue;
        }

        // CSV columns:
        // 0 = Name (key)
        // 1 = MoveName (display)
        // 2 = Startup
        // 3 = Active
        // 4 = Recovery
        // 5 = Hitstun
        // 6 = Blockstun

        FString KeyName = Columns[0].TrimStartAndEnd();
        FString DisplayName = Columns[1].TrimStartAndEnd();

        int32 Startup = FCString::Atoi(*Columns[2]);
        int32 Active = FCString::Atoi(*Columns[3]);
        int32 Recovery = FCString::Atoi(*Columns[4]);
        int32 Hitstun = FCString::Atoi(*Columns[5]);
        int32 Blockstun = FCString::Atoi(*Columns[6]);

        FName Key(*KeyName);

        if (!MoveDataMap.Contains(Key))
        {
            UE_LOG(LogTemp, Warning, TEXT("No MoveData asset found for: %s"), *KeyName);
            continue;
        }

        // Found asset
        UWUT_MoveData* MoveAsset = MoveDataMap[Key];
        MoveAsset->Modify();

        // Update data
        MoveAsset->MoveName = FName(*DisplayName);
        MoveAsset->StartupFrames = Startup;
        MoveAsset->ActiveFrames = Active;
        MoveAsset->RecoveryFrames = Recovery;
        MoveAsset->HitProps.HitstunFrames = Hitstun;
        MoveAsset->HitProps.BlockstunFrames = Blockstun;

        // Save
        UEditorAssetLibrary::SaveLoadedAsset(MoveAsset);

        UE_LOG(LogTemp, Warning,
            TEXT("Updated %s (%s) → Startup=%d Active=%d Recovery=%d Hitstun=%d Blockstun=%d"),
            *KeyName, *DisplayName,
            Startup, Active, Recovery, Hitstun, Blockstun
        );
    }

    UE_LOG(LogTemp, Warning, TEXT("=== Import Completed ==="));
}
