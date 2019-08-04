// Copyright 2019 LexLiu. All Rights Reserved.

#include "LGUIEditorCommands.h"
#include "LGUIEditorModule.h"

#define LOCTEXT_NAMESPACE "FLGUIEditorModule"

void FLGUIEditorCommands::RegisterCommands()
{
	UI_COMMAND(OpenEditorToolsWindow, "LGUIEditorTools", "Open these tools as a window", EUserInterfaceActionType::Button, FInputGesture());

	UI_COMMAND(CopyActor, "Copy Actors", "Copy selected actors with hierarchy", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(PasteActor, "Paste Actors", "Paste actors with hierarchy", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(DuplicateActor, "Duplicate Actors", "Duplicate selected actors with hierarchy", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(DeleteActor, "Delete Actors", "Delete selected actors with hierarchy", EUserInterfaceActionType::Button, FInputGesture());
	
	UI_COMMAND(CopyComponentValues, "Copy Component Values", "Copy selected component values", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(PasteComponentValues, "Paste Component Values", "Paste values to selected component", EUserInterfaceActionType::Button, FInputGesture());

	UI_COMMAND(ShowSelectionFrameInEditMode, "Enable EditMode SelectionFrame", "Show or hide selection frame in edit mode", EUserInterfaceActionType::ToggleButton, FInputChord());
	UI_COMMAND(ShowSelectionFrameInPlayMode, "Enable PlayMode SelectionFrame", "Show or hide selection frame in play mode", EUserInterfaceActionType::ToggleButton, FInputChord());

	UI_COMMAND(OpenAtlasViewer, "OpenAtlasViewer", "Open LGUI atlas viewer", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
