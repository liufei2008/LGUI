// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "LGUIPrefabEditorCommand.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabEditorCommand"

void FLGUIPrefabEditorCommand::RegisterCommands()
{
	UI_COMMAND(Apply, "Apply", "Apply changes to prefab.", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OverrideParameter, "OverrideParameter", "Open override parameter panel.", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(RawDataViewer, "RawDataViewer", "Open raw data viewer panel of this prefab.", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE