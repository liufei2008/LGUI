// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "LGUIPrefabEditorViewport.h"
#include "LGUIPrefabEditorViewportClient.h"
#include "LGUIPrefabEditor.h"
#include "SLGUIPrefabEditorViewportToolbar.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabEditorViewport"

void SLGUIPrefabEditorViewport::Construct(const FArguments& InArgs, TSharedPtr<FLGUIPrefabEditor> InPrefabEditor)
{
	this->PrefabEditorPtr = InPrefabEditor;
	SEditorViewport::Construct(SEditorViewport::FArguments());
}
void SLGUIPrefabEditorViewport::BindCommands()
{
	SEditorViewport::BindCommands();
}
TSharedRef<FEditorViewportClient> SLGUIPrefabEditorViewport::MakeEditorViewportClient()
{
	EditorViewportClient = MakeShareable(new FLGUIPrefabEditorViewportClient(this->PrefabEditorPtr.Pin()->GetPreviewScene(), this->PrefabEditorPtr, SharedThis(this)));
	EditorViewportClient->ViewportType = LVT_Perspective;
	EditorViewportClient->bSetListenerPosition = false;
	EditorViewportClient->SetViewLocation(FVector(-1024.0f, 0.0f, 0.0f));
	EditorViewportClient->SetViewRotation(FRotator::ZeroRotator);
	EditorViewportClient->SetRealtime(true);
	EditorViewportClient->SetShowStats(true);
	EditorViewportClient->VisibilityDelegate.BindLambda([]() {return true; });
	return EditorViewportClient.ToSharedRef();
}

TSharedPtr<SWidget> SLGUIPrefabEditorViewport::MakeViewportToolbar()
{
	return SNew(SLGUIPrefabEditorViewportToolbar, SharedThis(this));
}
EVisibility SLGUIPrefabEditorViewport::GetTransformToolbarVisibility() const
{
	return EVisibility::Visible;
}
void SLGUIPrefabEditorViewport::OnFocusViewportToSelection()
{
	//EditorViewportClient->RequestFocusOnSelection(/*bInstant=*/ false);
}

TSharedRef<SEditorViewport> SLGUIPrefabEditorViewport::GetViewportWidget()
{
	return SharedThis(this);
}
TSharedPtr<FExtender> SLGUIPrefabEditorViewport::GetExtenders() const
{
	TSharedPtr<FExtender> Result(MakeShareable(new FExtender));
	return Result;
}
void SLGUIPrefabEditorViewport::OnFloatingButtonClicked()
{

}

FReply SLGUIPrefabEditorViewport::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	return PrefabEditorPtr.Pin()->TryHandleAssetDragDropOperation(DragDropEvent);
}

#undef LOCTEXT_NAMESPACE