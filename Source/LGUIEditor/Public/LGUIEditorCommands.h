// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "Framework/Commands/Commands.h"

class FLGUIEditorCommands : public TCommands<FLGUIEditorCommands>
{
public:

	FLGUIEditorCommands();
	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> CopyActor;
	TSharedPtr<FUICommandInfo> PasteActor;
	TSharedPtr<FUICommandInfo> DuplicateActor;
	TSharedPtr<FUICommandInfo> DestroyActor;

	TSharedPtr<FUICommandInfo> CopyComponentValues;
	TSharedPtr<FUICommandInfo> PasteComponentValues;

	TSharedPtr<FUICommandInfo> ActiveViewportAsLGUIPreview;
	TSharedPtr<FUICommandInfo> FocusToScreenSpaceUI;
};