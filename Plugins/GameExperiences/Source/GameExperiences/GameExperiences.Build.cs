// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GameExperiences : ModuleRules
{
	public GameExperiences(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"CommonUser",
			"Core",
			"GameFeatures",
			"GameplayTags",
			"ModularGameplay",
			"ModularGameplayActors",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
		});
	}
}