// Copyright 2019-present LexLiu. All Rights Reserved.

using UnrealBuildTool;

public class LGUISDFFont : ModuleRules
{
	public LGUISDFFont(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
			new string[]
			{
                "Core",
                "CoreUObject",
                "Engine",
                "RHI",
				"LGUI",
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
