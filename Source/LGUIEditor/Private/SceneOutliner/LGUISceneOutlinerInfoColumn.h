// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "ISceneOutliner.h"
#include "ISceneOutlinerColumn.h"
#if ENGINE_MAJOR_VERSION >= 5
#include "SceneOutlinerFwd.h"
#endif

namespace LGUISceneOutliner
{
	class LGUIEDITOR_API FLGUISceneOutlinerInfoColumn : public ISceneOutlinerColumn
	{
	public:
		static TSharedRef<ISceneOutlinerColumn> MakeInstance(ISceneOutliner& SceneOutliner);

#if ENGINE_MAJOR_VERSION >= 5
		FLGUISceneOutlinerInfoColumn(ISceneOutliner& InSceneOutliner);

		virtual ~FLGUISceneOutlinerInfoColumn();

		static FName GetID();

		// Begin ISceneOutlinerColumn Implementation
		virtual FName GetColumnID() override;

		virtual SHeaderRow::FColumn::FArguments ConstructHeaderRowColumn() override;

		virtual const TSharedRef< SWidget > ConstructRowWidget(FSceneOutlinerTreeItemRef TreeItem, const STableRow<FSceneOutlinerTreeItemPtr>& Row) override;

		virtual void PopulateSearchStrings(const ISceneOutlinerTreeItem& Item, TArray< FString >& OutSearchStrings) const override;

		virtual bool SupportsSorting() const override { return true; }

		virtual void SortItems(TArray<FSceneOutlinerTreeItemPtr>& OutItems, const EColumnSortMode::Type SortMode) const override;

		// End ISceneOutlinerColumn Implementation
	private:

		AActor* GetActorFromTreeItem(FSceneOutlinerTreeItemRef TreeItem) const;

		EVisibility GetPrefabIconVisibility(FSceneOutlinerTreeItemRef TreeItem)const;
		EVisibility GetDownArrowVisibility(FSceneOutlinerTreeItemRef TreeItem)const;
		EVisibility GetCanvasIconVisibility(FSceneOutlinerTreeItemRef TreeItem)const;
		EVisibility GetDrawcallCountVisibility(FSceneOutlinerTreeItemRef TreeItem)const;
		FSlateColor GetPrefabIconColor(FSceneOutlinerTreeItemRef TreeItem)const;
		FSlateColor GetDrawcallIconColor(FSceneOutlinerTreeItemRef TreeItem)const;
		FText GetDrawcallInfo(FSceneOutlinerTreeItemRef TreeItem)const;

		/** Weak reference to the outliner AnchorData that owns our list */
		TWeakPtr< ISceneOutliner > WeakSceneOutliner;
		TWeakObjectPtr<UWorld> CurrentWorld;
#else
		FLGUISceneOutlinerInfoColumn(ISceneOutliner& InSceneOutliner);

		virtual ~FLGUISceneOutlinerInfoColumn();

		static FName GetID();

		// Begin ISceneOutlinerColumn Implementation
		virtual FName GetColumnID() override;

		virtual SHeaderRow::FColumn::FArguments ConstructHeaderRowColumn() override;

		virtual const TSharedRef< SWidget > ConstructRowWidget(SceneOutliner::FTreeItemRef TreeItem, const STableRow<SceneOutliner::FTreeItemPtr>& Row) override;

		virtual void PopulateSearchStrings(const SceneOutliner::ITreeItem& Item, TArray< FString >& OutSearchStrings) const override;

		virtual bool SupportsSorting() const override { return true; }

		virtual void SortItems(TArray<SceneOutliner::FTreeItemPtr>& OutItems, const EColumnSortMode::Type SortMode) const override;

		// End ISceneOutlinerColumn Implementation
		FString GetTextForActor(AActor* InActor) const;
	private:

		AActor* GetActorFromTreeItem(const TWeakPtr<SceneOutliner::ITreeItem> TreeItem) const;

		EVisibility GetPrefabIconVisibility(const TWeakPtr<SceneOutliner::ITreeItem> TreeItem)const;
		EVisibility GetDownArrowVisibility(const TWeakPtr<SceneOutliner::ITreeItem> TreeItem)const;
		EVisibility GetCanvasIconVisibility(const TWeakPtr<SceneOutliner::ITreeItem> TreeItem)const;
		EVisibility GetDrawcallCountVisibility(const TWeakPtr<SceneOutliner::ITreeItem> TreeItem)const;
		FSlateColor GetPrefabIconColor(const TWeakPtr<SceneOutliner::ITreeItem> TreeItem)const;
		FSlateColor GetDrawcallIconColor(const TWeakPtr<SceneOutliner::ITreeItem> TreeItem)const;
		FText GetDrawcallInfo(const TWeakPtr<SceneOutliner::ITreeItem> TreeItem)const;

		/** Weak reference to the outliner AnchorData that owns our list */
		TWeakPtr< ISceneOutliner > WeakSceneOutliner;
		TWeakObjectPtr<UWorld> CurrentWorld;
#endif
	};
}