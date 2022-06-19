// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "LGUIEditorStyle.h"

class ULGUIPrefab;

class FLGUIPrefabEditorCommand : public TCommands<FLGUIPrefabEditorCommand>
{
public:
	FLGUIPrefabEditorCommand()
		: TCommands<FLGUIPrefabEditorCommand>(
			TEXT("LGUIPrefabEditor"), // Context name for fast lookup
			NSLOCTEXT("Contexts", "LGUIPrefabEditor", "LGUI Prefab Editor"), // Localized context name for displaying
			NAME_None, // Parent
			FLGUIEditorStyle::Get().GetStyleSetName() // Icon Style Set
			)
	{
	}

	// TCommand<> interface
	virtual void RegisterCommands() override;
	// End of TCommand<> interface

public:
	TSharedPtr<FUICommandInfo> Apply;
	TSharedPtr<FUICommandInfo> RawDataViewer;
	TSharedPtr<FUICommandInfo> CopyActor;
	TSharedPtr<FUICommandInfo> PasteActor;
	TSharedPtr<FUICommandInfo> CutActor;
	TSharedPtr<FUICommandInfo> DuplicateActor;
	TSharedPtr<FUICommandInfo> DestroyActor;
};