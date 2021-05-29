// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "ISceneOutliner.h"
#include "ISceneOutlinerColumn.h"
#include "SceneOutlinerFwd.h"

namespace LGUISceneOutliner
{
	class LGUIEDITOR_API FLGUISceneOutlinerInfoColumn : public ISceneOutlinerColumn
	{
	public:
		static TSharedRef<ISceneOutlinerColumn> MakeInstance(ISceneOutliner& SceneOutliner);

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

		FString GetTextForActor(AActor* InActor) const;
	private:

		bool CanShowPrefabIcon(FSceneOutlinerTreeItemRef TreeItem) const;
		AActor* GetActorFromTreeItem(FSceneOutlinerTreeItemRef TreeItem) const;

		EVisibility GetPrefabIconVisibility(FSceneOutlinerTreeItemRef TreeItem)const;
		EVisibility GetDownArrowVisibility(FSceneOutlinerTreeItemRef TreeItem)const;
		EVisibility GetCanvasIconVisibility(FSceneOutlinerTreeItemRef TreeItem)const;
		EVisibility GetDrawcallCountVisibility(FSceneOutlinerTreeItemRef TreeItem)const;
		FSlateColor GetPrefabIconColor(FSceneOutlinerTreeItemRef TreeItem)const;
		FText GetDrawcallInfo(FSceneOutlinerTreeItemRef TreeItem)const;

		/** Weak reference to the outliner widget that owns our list */
		TWeakPtr< ISceneOutliner > WeakSceneOutliner;
		TWeakObjectPtr<UWorld> CurrentWorld;
	};
}