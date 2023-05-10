// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LGUIEditorCommands.h"
#include "LGUIEditorModule.h"
#include "LGUIEditorStyle.h"

#define LOCTEXT_NAMESPACE "FLGUIEditorCommands"

FLGUIEditorCommands::FLGUIEditorCommands()
	: TCommands<FLGUIEditorCommands>(TEXT("LGUIEditor"), NSLOCTEXT("Contexts", "LGUIEditor", "LGUIEditor Plugin"), NAME_None, FLGUIEditorStyle::GetStyleSetName())
{
}
void FLGUIEditorCommands::RegisterCommands()
{
	UI_COMMAND(CopyActor, "Copy Actors", "Copy selected actors with hierarchy", EUserInterfaceActionType::Button, FInputChord(EKeys::C, EModifierKey::Shift | EModifierKey::Alt));
	UI_COMMAND(PasteActor, "Paste Actors", "Paste actors with hierarchy", EUserInterfaceActionType::Button, FInputChord(EKeys::V, EModifierKey::Shift | EModifierKey::Alt));
	UI_COMMAND(CutActor, "Cut Actors", "Cut actors with hierarchy", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(DuplicateActor, "Duplicate Actors", "Duplicate selected actors with hierarchy", EUserInterfaceActionType::Button, FInputChord(EKeys::D, EModifierKey::Shift | EModifierKey::Alt));
	UI_COMMAND(DestroyActor, "Destroy Actors", "Destroy selected actors with hierarchy", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ToggleSpatiallyLoaded, "Toggle Actors IsSpatiallyLoaded", "Toggle selected actor's IsSpatiallyLoaded property for WorldPartition", EUserInterfaceActionType::ToggleButton, FInputChord());
	
	UI_COMMAND(CopyComponentValues, "Copy Component Values", "Copy selected component values", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(PasteComponentValues, "Paste Component Values", "Paste values to selected component", EUserInterfaceActionType::Button, FInputChord());

	UI_COMMAND(FocusToScreenSpaceUI, "Focus to Screen-Space-UI", "Make the editor viewport's camera face to Screen-Space-UI (If there is a screen space UI)", EUserInterfaceActionType::Button, FInputChord(EKeys::F, EModifierKey::Alt | EModifierKey::Shift));
	UI_COMMAND(FocusToSelectedUI, "Focus to Selected UI", "Make the editor viewport's camera face to selected UI", EUserInterfaceActionType::Button, FInputChord());

	UI_COMMAND(ActiveViewportAsLGUIPreview, "Active Viewport as ScreenSpaceUI Preview", "Use current selected active editor viewport for Screen-Space-UI preview", EUserInterfaceActionType::ToggleButton, FInputChord());
	UI_COMMAND(ToggleLGUIInfoColume, "Show LGUI column in SceneOutliner", "Show LGUI column in SceneOutliner", EUserInterfaceActionType::ToggleButton, FInputChord());
	UI_COMMAND(ToggleDrawHelperFrame, "Draw helper box on selected UI element", "Draw helper box on selected UI element", EUserInterfaceActionType::ToggleButton, FInputChord());
	UI_COMMAND(ToggleAnchorTool, "Show Anchor Tool on selected UI element", "Show Anchor Tool on selected UI element", EUserInterfaceActionType::ToggleButton, FInputChord());
	UI_COMMAND(ForceGC, "ForceGC", "Force garbage collection immediately, this is useful in some test work", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
