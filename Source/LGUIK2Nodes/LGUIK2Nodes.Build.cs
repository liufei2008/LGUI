// Copyright 2019-2022 LexLiu. All Rights Reserved.

using UnrealBuildTool;

public class LGUIK2Nodes : ModuleRules
{
	public LGUIK2Nodes(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
			new string[]
			{
                "Core",
                "CoreUObject",
                "Engine",
                "UnrealEd",
                "LGUI",
                "KismetWidgets",
                "BlueprintGraph",
                "KismetCompiler",
                "Kismet",
            }
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{

            }
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				
			}
			);

    }
}
