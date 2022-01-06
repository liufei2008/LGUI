// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#pragma once

class FLGUIPrefabEditor;
class AActor;

/**
 * 
 */
class SLGUIPrefabEditorDetails : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLGUIPrefabEditorDetails)
    {
    }

    SLATE_END_ARGS()

    /** Widget constructor */
    void Construct(const FArguments& Args, TSharedPtr<FLGUIPrefabEditor> InPrefabEditor);

	virtual ~SLGUIPrefabEditorDetails();
public:
	void RefreshOverrideParameter();
private:
	AActor* GetActorContext() const;
	void OnEditorSelectionChanged(UObject* Object);
	void OnSCSEditorTreeViewSelectionChanged(const TArray<TSharedPtr<class FSCSEditorTreeNode> >& SelectedNodes);
	void OnSCSEditorTreeViewItemDoubleClicked(const TSharedPtr<class FSCSEditorTreeNode> ClickedNode);

	TWeakPtr<FLGUIPrefabEditor> PrefabEditorPtr;

	bool IsPropertyReadOnly(const FPropertyAndParent& InPropertyAndParent);
	bool IsPrefabButtonEnable()const;
	FOptionalSize GetPrefabButtonHeight()const;
	EVisibility GetPrefabButtonVisibility()const;
	bool IsSSCSEditorAllowEditing()const;

	TSharedPtr<class SLGUIPrefabOverrideDataViewer> OverrideParameterEditor;
	TSharedPtr<class IDetailsView> DetailsView;
	TSharedPtr<class SBox> ComponentsBox;
	TSharedPtr<class SSCSEditor> SCSEditor;
	TWeakObjectPtr<AActor> CachedActor;
};
