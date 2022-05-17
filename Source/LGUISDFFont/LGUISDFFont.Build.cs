// Copyright 2019-2022 LexLiu. All Rights Reserved.

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
                "LGUI",
				"FreeType2",
				"zlib",
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
