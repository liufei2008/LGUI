// Copyright 2019-2022 LexLiu. All Rights Reserved.

using UnrealBuildTool;

public class LGUIEditor : ModuleRules
{
	public LGUIEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        string EnginSourceFolder = EngineDirectory + "/Source/";
        PrivateIncludePaths.AddRange(
                new string[] {
                    EnginSourceFolder + "/Editor/DetailCustomizations/Private",
                });

        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "CoreUObject",
                "Slate",
                "SlateCore",
                "Engine",
                "UnrealEd",
                "PropertyEditor",
                "RenderCore",
                "RHI",
                "LGUI",
                "LevelEditor",
                "Projects",
                "EditorWidgets",
                "DesktopPlatform",//file system
                "ImageWrapper",//texture load
                "InputCore",//STableRow
                "AssetTools",//Asset editor
                "ContentBrowser",//LGUI editor
                "SceneOutliner",//LGUIPrefab editor, extend SceneOutliner
                "ApplicationCore",//ClipboardCopy
                "KismetCompiler",
                "AppFramework",
                //"AssetRegistry",
                //"InputCore",
				// ... add other public dependencies that you statically link with here ...
                
                "Kismet",
                "ToolMenus",//PrefabEditor
                "Sequencer",
				"MovieScene",
				"MovieSceneTracks",
				"MovieSceneTools",
            }
            );
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "EditorStyle",
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
