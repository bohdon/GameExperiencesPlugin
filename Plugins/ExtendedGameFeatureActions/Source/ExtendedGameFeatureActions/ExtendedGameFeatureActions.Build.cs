// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ExtendedGameFeatureActions : ModuleRules
{
	public ExtendedGameFeatureActions(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"CommonGame",
			"CommonUI",
			"Core",
			"ExtendedGameplayAbilities",
			"GameFeatures",
			"GameplayAbilities",
			"GameplayTags",
			"ModularGameplay",
			"UIExtension",
			"UMG",
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