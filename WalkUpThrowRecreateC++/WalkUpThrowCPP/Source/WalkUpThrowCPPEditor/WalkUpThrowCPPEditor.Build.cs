using UnrealBuildTool;

public class WalkUpThrowCPPEditor : ModuleRules
{
    public WalkUpThrowCPPEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "UnrealEd",               // required
            "AssetRegistry",          // required
            "EditorScriptingUtilities",
                "ToolMenus",
            "WalkUpThrowCPP"          // your runtime module
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Slate",
            "SlateCore",
            "InputCore"
        });
    }
}
