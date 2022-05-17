// Copyright 2021-present LexLiu. All Rights Reserved.

using UnrealBuildTool;

public class LGUISDFFontEditor : ModuleRules
{
	public LGUISDFFontEditor(ReadOnlyTargetRules Target) : base(Target)
    {
		
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
			new string[]
			{
				"Core",
                "CoreUObject",
                "Slate",
                "SlateCore",
                "Engine",
                "UnrealEd",
                "PropertyEditor",
                "LGUISDFFont",
                "LevelEditor",
                "Projects",
                "EditorWidgets",
                "DesktopPlatform",
                "ImageWrapper",
                "AssetTools",//Asset editor
				"SharedSettingsWidgets",
				// ... add other public dependencies that you statically link with here ...
                
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
