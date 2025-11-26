// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class WalkUpThrowCPPEditorTarget : TargetRules
{
	public WalkUpThrowCPPEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
		ExtraModuleNames.Add("WalkUpThrowCPP");
        ExtraModuleNames.Add("WalkUpThrowCPPEditor"); // editor module
    }
}
