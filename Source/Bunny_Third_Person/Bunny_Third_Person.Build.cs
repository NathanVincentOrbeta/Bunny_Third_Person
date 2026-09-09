// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Bunny_Third_Person : ModuleRules
{
	public Bunny_Third_Person(ReadOnlyTargetRules Target) : base(Target)
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
			"Bunny_Third_Person",
			"Bunny_Third_Person/Variant_Platforming",
			"Bunny_Third_Person/Variant_Platforming/Animation",
			"Bunny_Third_Person/Variant_Combat",
			"Bunny_Third_Person/Variant_Combat/AI",
			"Bunny_Third_Person/Variant_Combat/Animation",
			"Bunny_Third_Person/Variant_Combat/Gameplay",
			"Bunny_Third_Person/Variant_Combat/Interfaces",
			"Bunny_Third_Person/Variant_Combat/UI",
			"Bunny_Third_Person/Variant_SideScrolling",
			"Bunny_Third_Person/Variant_SideScrolling/AI",
			"Bunny_Third_Person/Variant_SideScrolling/Gameplay",
			"Bunny_Third_Person/Variant_SideScrolling/Interfaces",
			"Bunny_Third_Person/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
