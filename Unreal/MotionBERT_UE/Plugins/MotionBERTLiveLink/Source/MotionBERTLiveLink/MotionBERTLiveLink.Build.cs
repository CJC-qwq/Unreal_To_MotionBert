using UnrealBuildTool;

public class MotionBERTLiveLink : ModuleRules
{
	public MotionBERTLiveLink(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"AnimationCore",
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"Json",
				"JsonUtilities",
				"LiveLinkInterface",
				"LiveLink",
				"LiveLinkAnimationCore",
				"Networking",
				"Projects",
				"Sockets",
				"Slate",
				"SlateCore"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"ApplicationCore"
			}
		);
	}
}
