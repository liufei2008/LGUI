// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "SlateBasics.h"
#include "ISceneOutliner.h"
#include "ISceneOutlinerColumn.h"

namespace LGUISceneOutliner
{
	class FLGUISceneOutlinerInfoColumn : public ISceneOutlinerColumn
	{
	public:
		static TSharedRef<ISceneOutlinerColumn> MakeInstance(ISceneOutliner& SceneOutliner);

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

		FText GetTextForItem(const TWeakPtr<SceneOutliner::ITreeItem> TreeItem) const;
		bool CanShowPrefabIcon(AActor* InActor) const;
		AActor* GetActorFromTreeItem(const TWeakPtr<SceneOutliner::ITreeItem> TreeItem) const;
		bool IsInsidePrefabActor(AActor* InActor)const;

		class ALGUIPrefabActor* GetPrefabActor_WhichManageThisActor(AActor* InActor)const;

		void GotoPrefabActor(AActor* InActor);

		/** Weak reference to the outliner widget that owns our list */
		TWeakPtr< ISceneOutliner > WeakSceneOutliner;
		TWeakObjectPtr<UWorld> CurrentWorld;
	};
}