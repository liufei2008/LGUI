// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "Widgets/SWidget.h"
#include "SEditorViewport.h"
#include "SCommonEditorViewportToolbarBase.h"

class FLGUIPrefabEditor;
class FLGUIPrefabEditorViewportClient;

//Encapsulates a simple scene setup for preview or thumbnail rendering.
class SLGUIPrefabEditorViewport : public SEditorViewport, public ICommonEditorViewportToolbarInfoProvider
{
public:
	SLATE_BEGIN_ARGS(SLGUIPrefabEditorViewport) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedPtr<FLGUIPrefabEditor> InPrefabEditor, EViewModeIndex InViewMode);

	// SEditorViewport interface
	virtual void BindCommands() override;
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
	virtual TSharedPtr<SWidget> MakeViewportToolbar() override;
	virtual EVisibility GetTransformToolbarVisibility() const override;
	virtual void OnFocusViewportToSelection() override;
	// End of SEditorViewport interface

	// ICommonEditorViewportToolbarInfoProvider interface
	virtual TSharedRef<class SEditorViewport> GetViewportWidget() override;
	virtual TSharedPtr<FExtender> GetExtenders() const override;
	virtual void OnFloatingButtonClicked() override;
	// End of ICommonEditorViewportToolbarInfoProvider interface

	// SWidget interface
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	// End of SWidget interface

private:
	// Pointer back to owning sprite editor instance (the keeper of state)
	TWeakPtr<FLGUIPrefabEditor> PrefabEditorPtr;
	EViewModeIndex ViewMode = EViewModeIndex::VMI_Lit;

	// Viewport client
	TSharedPtr<FLGUIPrefabEditorViewportClient> EditorViewportClient;
};