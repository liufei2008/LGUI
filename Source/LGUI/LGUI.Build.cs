// Copyright 2019 LexLiu. All Rights Reserved.

using UnrealBuildTool;

public class LGUI : ModuleRules
{
	public LGUI(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        string EnginSourceFolder = EngineDirectory + "/Source/";
        PrivateIncludePaths.AddRange(
                new string[] {
                    EnginSourceFolder + "/Runtime/Renderer/Private",
                    EnginSourceFolder + "/Runtime/RHI/Private",
                });

        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "RHI","RenderCore","Renderer",
                "LTween",
                "InputCore",//UITextInput
                "FreeType2",
                "ApplicationCore",//UITextInput/RequiresVirtualKeyboard, debug
                "UtilityShaders",
                "Projects",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

        if (Target.Type == TargetType.Editor)
        {
            PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "UnrealEd",
                "EditorStyle",
            }
            );
        }
    }
}
