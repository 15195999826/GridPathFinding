﻿// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GridPathFinding : ModuleRules
{
	public GridPathFinding(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);


		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"Engine",
				"AIModule",
				"FastNoise", 
				"FastNoiseGenerator", 
				"GeometryScriptingCore",
				"NavigationSystem", 
				"GeometryFramework", 
				"DeveloperSettings", 
				"LomoLib",
				"UMG",
				"Json",
				"JsonUtilities",
				"InputCore",
				"ApplicationCore",
				"UnrealEd",
				// ... add other public dependencies that you statically link with here ...
			}
		);


		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"CoreUObject",
			
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
