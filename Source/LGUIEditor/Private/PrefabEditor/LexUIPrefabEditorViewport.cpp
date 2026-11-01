// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LexUIPrefabEditorViewport.h"
#include "LexUIPrefabEditorViewportClient.h"
#include "LexUIPrefabEditor.h"
#include "LexUIPrefabEditorViewportToolbar.h"
#include "EditorViewportCommands.h"
#include "EditorModeManager.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabEditorViewport"

void SLexUIPrefabEditorViewport::Construct(const FArguments& InArgs, TSharedPtr<FLexUIPrefabEditor> InPrefabEditor, EViewModeIndex InViewMode)
{
	this->PrefabEditorPtr = InPrefabEditor;
	this->ViewMode = InViewMode;
	SEditorViewport::Construct(SEditorViewport::FArguments());
}
void SLexUIPrefabEditorViewport::BindCommands()
{
	SEditorViewport::BindCommands();

	// The engine cycle also walks through parent/explicit space when the experimental gizmo is enabled, but this
	// viewport's gizmo only implements world and local space, so make Ctrl+` a strict two-state toggle.
	const FEditorViewportCommands& ViewportCommands = FEditorViewportCommands::Get();
	TWeakPtr<SEditorViewport> WeakViewport = SharedThis(this);
	CommandList->MapAction(
		ViewportCommands.CycleTransformGizmoCoordSystem,
		FExecuteAction::CreateLambda([WeakViewport]()
			{
				if (TSharedPtr<SEditorViewport> Viewport = WeakViewport.Pin())
				{
					if (TSharedPtr<FEditorViewportClient> ViewportClient = Viewport->GetViewportClient())
					{
						const bool bIsLocal = ViewportClient->GetModeTools()->GetCoordSystem(true) == COORD_Local;
						ViewportClient->SetWidgetCoordSystemSpace(bIsLocal ? COORD_World : COORD_Local);
					}
				}
			}),
		FCanExecuteAction(),
		FIsActionChecked::CreateLambda([WeakViewport]()
			{
				if (TSharedPtr<SEditorViewport> Viewport = WeakViewport.Pin())
				{
					return Viewport->IsCoordSystemActive(COORD_Local);
				}
				return false;
			})
	);
}
TSharedRef<FEditorViewportClient> SLexUIPrefabEditorViewport::MakeEditorViewportClient()
{
	EditorViewportClient = MakeShareable(new FLexUIPrefabEditorViewportClient(this->PrefabEditorPtr, SharedThis(this)));
	EditorViewportClient->bSetListenerPosition = false;
	EditorViewportClient->SetRealtime(true);
	EditorViewportClient->SetShowStats(true);
	EditorViewportClient->VisibilityDelegate.BindLambda([]() {return true; });
	EditorViewportClient->SetViewMode(ViewMode);
	return EditorViewportClient.ToSharedRef();
}

TSharedPtr<SWidget> SLexUIPrefabEditorViewport::BuildViewportToolbar()
{
	return SNew(SLexUIPrefabEditorViewportToolbar, SharedThis(this));
}
EVisibility SLexUIPrefabEditorViewport::GetTransformToolbarVisibility() const
{
	return EVisibility::Hidden;
}
void SLexUIPrefabEditorViewport::OnFocusViewportToSelection()
{
	EditorViewportClient->FocusViewportToTargets();
}

TSharedRef<SEditorViewport> SLexUIPrefabEditorViewport::GetViewportWidget()
{
	return SharedThis(this);
}
TSharedPtr<FExtender> SLexUIPrefabEditorViewport::GetExtenders() const
{
	TSharedPtr<FExtender> Result(MakeShareable(new FExtender));
	return Result;
}
void SLexUIPrefabEditorViewport::OnFloatingButtonClicked()
{

}

#undef LOCTEXT_NAMESPACE