// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "ICustomSceneOutliner.h"
#include "ActorPickerMode.h"

class UWorld;
class AActor;
class SBox;
class UActorComponent;

class FLGUIPrefabEditorOutliner
{
public:
	~FLGUIPrefabEditorOutliner();

	void InitOutliner(UWorld* World);

	TSharedPtr<SBox> GetOutlinerWidget() { return OutlinerWidget; }
	
	// DECLARE_DELEGATE_RetVal_OneParam(bool, FOnShouldFilterActor, const AActor*);
	FOnShouldFilterActor ActorFilter;

	void Refresh();
	void FullRefresh();

	FOnActorPicked OnActorPickedDelegate;
	FOnActorPicked OnActorDoubleClickDelegate;

	AActor* GetSelectedActor() const {return SelectedActor.Get();}
	void ClearSelectedActor();
	void RenameSelectedActor();
protected:
	void OnSceneOutlinerItemPicked(TSharedRef<SceneOutliner::ITreeItem> ItemPtr);
	void OnSceneOutlinerSelectionChanged(SceneOutliner::FTreeItemPtr ItemPtr, ESelectInfo::Type SelectionMode);
	void OnSceneOutlinerDoubleClick(SceneOutliner::FTreeItemPtr ItemPtr);
	void OnEditorSelectionChanged(UObject* Object);

protected:
	TSharedPtr<SBox> OutlinerWidget;
	TSharedPtr<ICustomSceneOutliner> SceneOutlinerPtr;
	TWeakObjectPtr<AActor> SelectedActor;

	TWeakObjectPtr<UWorld> CurrentWorld;

	TSharedPtr<SceneOutliner::ITreeItem> CachedItemPtr;

	float OutlinerWidth = 100;
	float OutlinerHeight = 300;
};


