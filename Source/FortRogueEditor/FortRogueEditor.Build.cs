// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FortRogueEditor : ModuleRules
{
	public FortRogueEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"FortRogue",
			"InputCore",
			"Slate",
			"SlateCore",
			"ToolMenus",
			"UnrealEd",
			"PropertyEditor"
		});
	}
}
