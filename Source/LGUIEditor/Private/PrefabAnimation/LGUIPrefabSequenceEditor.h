// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "Widgets/Layout/SBox.h"
#include "Input/Reply.h"
#include "UObject/WeakObjectPtr.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/SCompoundWidget.h"

class ULGUIPrefabSequence;

class SLGUIPrefabSequenceEditor : public SCompoundWidget
{
public:
	~SLGUIPrefabSequenceEditor();

	SLATE_BEGIN_ARGS(SLGUIPrefabSequenceEditor) {}
	SLATE_END_ARGS();
	void Construct(const FArguments& InArgs);

	void AssignLGUIPrefabSequenceComponent(TWeakObjectPtr<ULGUIPrefabSequenceComponent> InSequenceComponent);
	ULGUIPrefabSequence* GetLGUIPrefabSequence() const;
	ULGUIPrefabSequenceComponent* GetSequenceComponent()const { return WeakSequenceComponent.Get(); }
private:

	TWeakObjectPtr<ULGUIPrefabSequenceComponent> WeakSequenceComponent;
	FDelegateHandle OnObjectsReplacedHandle;

	TSharedPtr<SLGUIPrefabSequenceEditorWidget> PrefabSequenceEditor;

	TSharedPtr<SListView<ULGUIPrefabSequence*>> AnimationListView;
	TArray<ULGUIPrefabSequence*> EmptyListItemSource;
	int32 CurrentSelectedAnimationIndex = 0;
	TSharedRef<ITableRow> OnGenerateRowForAnimationListView(ULGUIPrefabSequence* InListItem, const TSharedRef<STableViewBase>& InOwnerTableView);
	void OnAnimationListViewSelectionChanged(ULGUIPrefabSequence* InListItem, ESelectInfo::Type InSelectInfo);
	FReply OnNewAnimationClicked();
	TSharedPtr<class SSearchBox> SearchBoxPtr;
	void OnAnimationListViewSearchChanged(const FText& InSearchText);
	TSharedPtr<SWidget> OnContextMenuOpening()const;
	TSharedPtr<FUICommandList> CommandList;
	void CreateCommandList();
	void OnDuplicateAnimation();
	void OnDeleteAnimation();
	void OnRenameAnimation();

	void OnObjectsReplaced(const TMap<UObject*, UObject*>& ReplacementMap);
};