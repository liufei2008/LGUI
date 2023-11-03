// Copyright 2019-Present LexLiu. All Rights Reserved.

using UnrealBuildTool;

public class LGUI : ModuleRules
{
	public LGUI(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnableUndefinedIdentifierWarnings = false;

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
                //"FreeType2",
                "UElibPNG",
                "zlib",
                "ApplicationCore",//UITextInput/RequiresVirtualKeyboard, debug
                "Projects",
                "MovieScene",
                "MovieSceneTracks",
                "UMG",
				// ... add other public dependencies that you statically link with here ...
            }
            );
		if(Target.Type != TargetType.Server)
        {
            if (Target.bCompileFreeType)
            {
                PublicDependencyModuleNames.Add("FreeType2");
                //AddEngineThirdPartyPrivateStaticDependencies(Target, "FreeType2");
                PublicDefinitions.Add("WITH_FREETYPE=1");
            }
            else
            {
                PublicDefinitions.Add("WITH_FREETYPE=0");
            }
        }
		
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
                "TargetPlatform",
            }
            );
        }

        //PublicDefinitions.Add("LGUI_USE_32BIT_INDEXBUFFER");//uncommet this line to use 32-bit index buffer
    }
}
