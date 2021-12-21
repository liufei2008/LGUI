// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "LGUIEditorCommands.h"
#include "LGUIEditorModule.h"
#include "LGUIEditorStyle.h"

#define LOCTEXT_NAMESPACE "FLGUIEditorModule"

FLGUIEditorCommands::FLGUIEditorCommands()
	: TCommands<FLGUIEditorCommands>(TEXT("LGUIEditor"), NSLOCTEXT("Contexts", "LGUIEditor", "LGUIEditor Plugin"), NAME_None, FLGUIEditorStyle::GetStyleSetName())
{
}
void FLGUIEditorCommands::RegisterCommands()
{
	//UI_COMMAND(OpenEditorToolsWindow, "LGUIEditorTools", "Open these tools as a window", EUserInterfaceActionType::Button, FInputGesture());

	UI_COMMAND(CopyActor, "Copy Actors", "Copy selected actors with hierarchy", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(PasteActor, "Paste Actors", "Paste actors with hierarchy", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(DuplicateActor, "Duplicate Actors", "Duplicate selected actors with hierarchy", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(DestroyActor, "Destroy Actors", "Destroy selected actors with hierarchy", EUserInterfaceActionType::Button, FInputGesture());
	
	UI_COMMAND(CopyComponentValues, "Copy Component Values", "Copy selected component values", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(PasteComponentValues, "Paste Component Values", "Paste values to selected component", EUserInterfaceActionType::Button, FInputGesture());

	UI_COMMAND(FocusToScreenSpaceUI, "Focus to Screen-Space-UI", "Make the editor viewport's camera face to Screen-Space-UI (If there is a screen space UI)", EUserInterfaceActionType::Button, FInputChord(EKeys::F, EModifierKey::Alt | EModifierKey::Shift));
	UI_COMMAND(FocusToSelectedUI, "Focus to Selected UI", "Make the editor viewport's camera face to selected UI", EUserInterfaceActionType::Button, FInputChord());

	UI_COMMAND(ActiveViewportAsLGUIPreview, "Active Viewport as LGUI Preview", "Use current selected active editor viewport for Screen-Space-UI preview", EUserInterfaceActionType::ToggleButton, FInputChord(EKeys::V, EModifierKey::Alt | EModifierKey::Shift));
	UI_COMMAND(ToggleLGUIInfoColume, "Show LGUI column in SceneOutliner", "Show LGUI column in SceneOutliner", EUserInterfaceActionType::ToggleButton, FInputGesture());
	UI_COMMAND(ForceGC, "ForceGC", "Force garbage collection immediately, this is useful in some test work", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
