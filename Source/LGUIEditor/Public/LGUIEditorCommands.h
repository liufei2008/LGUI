// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "SlateBasics.h"
#include "LGUIEditorStyle.h"

class FLGUIEditorCommands : public TCommands<FLGUIEditorCommands>
{
public:

	FLGUIEditorCommands()
		: TCommands<FLGUIEditorCommands>(TEXT("LGUIEditor"), NSLOCTEXT("Contexts", "LGUIEditor", "LGUIEditor Plugin"), NAME_None, FLGUIEditorStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> CopyActor;
	TSharedPtr<FUICommandInfo> PasteActor;
	TSharedPtr<FUICommandInfo> DuplicateActor;
	TSharedPtr<FUICommandInfo> DestroyActor;

	TSharedPtr<FUICommandInfo> CopyComponentValues;
	TSharedPtr<FUICommandInfo> PasteComponentValues;

	TSharedPtr<FUICommandInfo> PreserveSceneoutlinerHierarchy;

	TSharedPtr<FUICommandInfo> OpenAtlasViewer;
};