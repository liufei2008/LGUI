// Copyright 2019 LexLiu. All Rights Reserved.

using UnrealBuildTool;

public class LGUIEditor : ModuleRules
{
	public LGUIEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
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
                "Json",//json
                "JsonUtilities",//json
                "AssetTools",//Asset editor
                "KismetWidgets",
                "ContentBrowser",//LGUI editor
                "SceneOutliner",//LGUIPrefab editor, extend SceneOutliner
                "ApplicationCore",//ClipboardCopy
                //"AssetRegistry",
                //"LShapePlugin",
                //"InputCore",
				// ... add other public dependencies that you statically link with here ...
                
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
