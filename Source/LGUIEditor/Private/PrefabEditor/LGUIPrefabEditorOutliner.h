// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#if ENGINE_MAJOR_VERSION >= 5
#include "SceneOutlinerModule.h"
#include "SSceneOutliner.h"
#else
#include "ICustomSceneOutliner.h"
#endif
#include "ActorPickerMode.h"

class UWorld;
class AActor;
class SBox;
class UActorComponent;
class FLGUIPrefabEditor;

class FLGUIPrefabEditorOutliner
{
public:
	~FLGUIPrefabEditorOutliner();

	friend class FLGUIPrefabEditorOutlinerMode;
	void InitOutliner(UWorld* World, TSharedPtr<FLGUIPrefabEditor> PrefabEditorPtr);

	TSharedPtr<SBox> GetOutlinerWidget() { return OutlinerWidget; }

	// DECLARE_DELEGATE_RetVal_OneParam(bool, FOnShouldFilterActor, const AActor*);
	FOnShouldFilterActor ActorFilter;

	void Refresh();
	void FullRefresh();

	FOnActorPicked OnActorPickedDelegate;
	FOnActorPicked OnActorDoubleClickDelegate;

	AActor* GetSelectedActor() const { return SelectedActor.Get(); }
	void ClearSelectedActor();
	void RenameSelectedActor();
private:
	void OnSceneOutlinerSelectionChanged(FSceneOutlinerTreeItemPtr ItemPtr, ESelectInfo::Type SelectionMode);
	void OnSceneOutlinerDoubleClick(FSceneOutlinerTreeItemPtr ItemPtr);
	void OnEditorSelectionChanged(UObject* Object);
	void OnDelete(const TArray<TWeakPtr<ISceneOutlinerTreeItem>>& InSelectedTreeItemArray);
	AActor* GetActorFromTreeItem(FSceneOutlinerTreeItemPtr TreeItem)const;
private:
	TSharedPtr<SBox> OutlinerWidget;
	TSharedPtr<SSceneOutliner> SceneOutlinerPtr;
	TWeakObjectPtr<AActor> SelectedActor;
	TWeakPtr<FLGUIPrefabEditor> PrefabEditorPtr;
	/** Reentrancy guard */
	bool bIsReentrant = false;

	TWeakObjectPtr<UWorld> CurrentWorld;

	TSharedPtr<ISceneOutlinerTreeItem> CachedItemPtr;

	float OutlinerWidth = 100;
	float OutlinerHeight = 300;
};


