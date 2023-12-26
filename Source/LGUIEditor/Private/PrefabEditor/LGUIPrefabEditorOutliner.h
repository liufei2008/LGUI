// Copyright 2019-Present LexLiu. All Rights Reserved.

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

	void InitOutliner(UWorld* World, TSharedPtr<FLGUIPrefabEditor> PrefabEditorPtr, const TSet<AActor*>& InUnexpendActorSet);

	TSharedPtr<SBox> GetOutlinerWidget() { return OutlinerWidget; }

	// DECLARE_DELEGATE_RetVal_OneParam(bool, FOnShouldFilterActor, const AActor*);
	FOnShouldFilterActor ActorFilter;

	void Refresh();
	void FullRefresh();

	FOnActorPicked OnActorPickedDelegate;
	FOnActorPicked OnActorDoubleClickDelegate;

	AActor* GetSelectedActor() const { return SelectedActor.Get(); }
	void ClearSelectedActor();
	void GetUnexpendActor(TArray<AActor*>& InOutAllActors)const;
	void UnexpandActorForDragDroppedPrefab(AActor* InActor);
private:
	void OnSceneOutlinerDoubleClick(FSceneOutlinerTreeItemPtr ItemPtr);
	void OnEditorSelectionChanged(UObject* Object);
	void OnDelete(const TArray<TWeakPtr<ISceneOutlinerTreeItem>>& InSelectedTreeItemArray);
	AActor* GetActorFromTreeItem(FSceneOutlinerTreeItemPtr TreeItem)const;
private:
	TSharedPtr<SBox> OutlinerWidget;
	TSharedPtr<SSceneOutliner> SceneOutlinerPtr;
	TWeakObjectPtr<AActor> SelectedActor;
	TWeakPtr<FLGUIPrefabEditor> PrefabEditorPtr;

	TWeakObjectPtr<UWorld> CurrentWorld;

	TSharedPtr<ISceneOutlinerTreeItem> CachedItemPtr;

	float OutlinerWidth = 100;
	float OutlinerHeight = 300;
};


