// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MotionBERT_UE : ModuleRules
{
	public MotionBERT_UE(ReadOnlyTargetRules Target) : base(Target)
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
			"LiveLink",
			"LiveLinkAnimationCore",
			"LiveLinkInterface",
			"MotionBERTLiveLink",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"MotionBERT_UE",
			"MotionBERT_UE/Variant_Platforming",
			"MotionBERT_UE/Variant_Platforming/Animation",
			"MotionBERT_UE/Variant_Combat",
			"MotionBERT_UE/Variant_Combat/AI",
			"MotionBERT_UE/Variant_Combat/Animation",
			"MotionBERT_UE/Variant_Combat/Gameplay",
			"MotionBERT_UE/Variant_Combat/Interfaces",
			"MotionBERT_UE/Variant_Combat/UI",
			"MotionBERT_UE/Variant_SideScrolling",
			"MotionBERT_UE/Variant_SideScrolling/AI",
			"MotionBERT_UE/Variant_SideScrolling/Gameplay",
			"MotionBERT_UE/Variant_SideScrolling/Interfaces",
			"MotionBERT_UE/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
