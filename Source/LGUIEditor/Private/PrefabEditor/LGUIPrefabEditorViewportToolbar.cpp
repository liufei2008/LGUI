// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "LGUIPrefabEditorViewportToolbar.h"
#include "LGUIPrefabEditorViewport.h"

#define LOCTEXT_NAMESPACE "SLGUIPrefabEditorViewportToolbar"

///////////////////////////////////////////////////////////
// SSpriteEditorViewportToolbar

void SLGUIPrefabEditorViewportToolbar::Construct(const FArguments& InArgs, TSharedPtr<class ICommonEditorViewportToolbarInfoProvider> InInfoProvider)
{
	SCommonEditorViewportToolbarBase::Construct(SCommonEditorViewportToolbarBase::FArguments(), InInfoProvider);
}

TSharedRef<SWidget> SLGUIPrefabEditorViewportToolbar::GenerateShowMenu() const
{
	GetInfoProvider().OnFloatingButtonClicked();
	
	TSharedRef<SEditorViewport> ViewportRef = GetInfoProvider().GetViewportWidget();

	const bool bInShouldCloseWindowAfterMenuSelection = true;
	FMenuBuilder ShowMenuBuilder(bInShouldCloseWindowAfterMenuSelection, ViewportRef->GetCommandList());
	{
		//ShowMenuBuilder.AddMenuEntry(FSpriteEditorCommands::Get().SetShowSockets);
		//ShowMenuBuilder.AddMenuEntry(FSpriteEditorCommands::Get().SetShowPivot);

		//ShowMenuBuilder.AddMenuSeparator();

		//ShowMenuBuilder.AddMenuEntry(FSpriteEditorCommands::Get().SetShowGrid);
		//ShowMenuBuilder.AddMenuEntry(FSpriteEditorCommands::Get().SetShowBounds);
		//ShowMenuBuilder.AddMenuEntry(FSpriteGeometryEditCommands::Get().SetShowNormals);

		//ShowMenuBuilder.AddMenuSeparator();

		//ShowMenuBuilder.AddMenuEntry(FSpriteEditorCommands::Get().SetShowCollision);
		//ShowMenuBuilder.AddMenuEntry(FSpriteEditorCommands::Get().SetShowMeshEdges);
	}

	return ShowMenuBuilder.MakeWidget();
}

#undef LOCTEXT_NAMESPACE
