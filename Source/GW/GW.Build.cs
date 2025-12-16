// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GW : ModuleRules
{
	public GW(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"GW",
			"GW/Variant_Platforming",
			"GW/Variant_Platforming/Animation",
			"GW/Variant_Combat",
			"GW/Variant_Combat/AI",
			"GW/Variant_Combat/Animation",
			"GW/Variant_Combat/Gameplay",
			"GW/Variant_Combat/Interfaces",
			"GW/Variant_Combat/UI",
			"GW/Variant_SideScrolling",
			"GW/Variant_SideScrolling/AI",
			"GW/Variant_SideScrolling/Gameplay",
			"GW/Variant_SideScrolling/Interfaces",
			"GW/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
