// Copyright 2019-2021 LexLiu. All Rights Reserved.

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

protected:
	AActor* GetActorContext() const;
	void OnEditorSelectionChanged(UObject* Object);
	void OnSCSEditorTreeViewSelectionChanged(const TArray<TSharedPtr<class FSCSEditorTreeNode> >& SelectedNodes);
	void OnSCSEditorTreeViewItemDoubleClicked(const TSharedPtr<class FSCSEditorTreeNode> ClickedNode);

public:
	TSharedPtr<FLGUIPrefabEditor> PrefabEditorPtr;

private:
	TSharedPtr<class IDetailsView> DetailsView;     // 属性列表              
	TSharedPtr<class SBox> ComponentsBox;
	TSharedPtr<class SSCSEditor> SCSEditor;
	TWeakObjectPtr<AActor> CachedActor;
};
