// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "Widgets/Layout/SBox.h"
#include "Input/Reply.h"
#include "UObject/WeakObjectPtr.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/SCompoundWidget.h"

class ULGUIPrefabSequenceComponent;
class ULGUIPrefabSequence;
class SLGUIPrefabSequenceEditorWidget;
struct FWidgetAnimationListItem;

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
	void RefreshAnimationList();
	void OnEditingPrefabChanged(AActor* RootActor);

private:
	TWeakObjectPtr<ULGUIPrefabSequenceComponent> WeakSequenceComponent;
	FDelegateHandle OnObjectsReplacedHandle;
	FDelegateHandle EditingPrefabChangedHandle;

	TSharedPtr<SLGUIPrefabSequenceEditorWidget> PrefabSequenceEditor;

	TSharedPtr<SListView<TSharedPtr<FWidgetAnimationListItem>>> AnimationListView;
	TArray< TSharedPtr<FWidgetAnimationListItem> > Animations;
	int32 CurrentSelectedAnimationIndex = 0;
	TSharedRef<ITableRow> OnGenerateRowForAnimationListView(TSharedPtr<FWidgetAnimationListItem> InListItem, const TSharedRef<STableViewBase>& InOwnerTableView);
	void OnAnimationListViewSelectionChanged(TSharedPtr<FWidgetAnimationListItem> InListItem, ESelectInfo::Type InSelectInfo);
	void OnItemScrolledIntoView(TSharedPtr<FWidgetAnimationListItem> InListItem, const TSharedPtr<ITableRow>& InWidget) const;
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