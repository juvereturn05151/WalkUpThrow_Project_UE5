// Copyright Epic Games, Inc. All Rights Reserved.

#include "WalkUpThrowCPPEditor.h"
#include "Modules/ModuleManager.h"
#include "LevelEditor.h"
#include "ToolMenus.h"
#include "WUT_MoveDataImporter.h"

class FWalkUpThrowCPPEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        UToolMenus::RegisterStartupCallback(
            FSimpleMulticastDelegate::FDelegate::CreateRaw(
                this, &FWalkUpThrowCPPEditorModule::RegisterMenus));
    }

    virtual void ShutdownModule() override
    {
        if (UObjectInitialized())
        {
            UToolMenus::UnRegisterStartupCallback(this);
            UToolMenus::UnregisterOwner(this);
        }
    }

    void RegisterMenus()
    {
        UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
        FToolMenuSection& Section = Menu->AddSection("WalkUpThrow Tools", FText::FromString("WalkUpThrow Tools"));

        Section.AddMenuEntry(
            "ImportMoveData",
            FText::FromString("Import Move Data"),
            FText::FromString("Updates all MoveData from DataTable (CSV)"),
            FSlateIcon(),
            FUIAction(FExecuteAction::CreateRaw(this, &FWalkUpThrowCPPEditorModule::RunImporter))
        );
    }

    void RunImporter()
    {
        UE_LOG(LogTemp, Warning, TEXT("=== Importer Button Clicked ==="));

        UWUT_MoveDataImporter* Importer = NewObject<UWUT_MoveDataImporter>();
        Importer->ImportCSV();

        UE_LOG(LogTemp, Warning, TEXT("=== Import Completed ==="));
    }
};

IMPLEMENT_MODULE(FWalkUpThrowCPPEditorModule, WalkUpThrowCPPEditor);
