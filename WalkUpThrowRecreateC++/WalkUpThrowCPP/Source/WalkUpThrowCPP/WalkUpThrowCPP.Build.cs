// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class WalkUpThrowCPP : ModuleRules
{
    public WalkUpThrowCPP(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "UMG",
            "Paper2D",
            "Niagara",
            "NiagaraCore"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Slate",
            "SlateCore",
            "UMG",
            "Paper2D"
        });

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicSystemLibraries.Add("Xinput9_1_0.lib"); 
        }
    }
}
