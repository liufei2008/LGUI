// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "ISceneOutliner.h"
#include "ISceneOutlinerColumn.h"
#include "SceneOutlinerFwd.h"


class ULGUIPrefab;

namespace LGUISceneOutliner
{
	class FLGUISceneOutlinerActorLabelColumn : public ISceneOutlinerColumn
	{
	public:
		static TSharedRef<ISceneOutlinerColumn> MakeInstance(ISceneOutliner& SceneOutliner);

		FLGUISceneOutlinerActorLabelColumn(ISceneOutliner& InSceneOutliner);

		virtual ~FLGUISceneOutlinerActorLabelColumn();

		static FName GetID();

		// Begin ISceneOutlinerColumn Implementation
		virtual FName GetColumnID() override;

		virtual SHeaderRow::FColumn::FArguments ConstructHeaderRowColumn() override;

		FText GetTextForItem(TWeakPtr<ISceneOutlinerTreeItem> TreeItem) const;
		EVisibility GetColumnDataVisibility(bool bIsClassHyperlink) const;
		virtual const TSharedRef< SWidget > ConstructRowWidget(FSceneOutlinerTreeItemRef TreeItem, const STableRow<FSceneOutlinerTreeItemPtr>& Row) override;

		// End ISceneOutlinerColumn Implementation
	private:
		TSharedPtr<SWidget> ConstructClassHyperlink(ISceneOutlinerTreeItem& TreeItem);
		TSharedRef<SWidget> GetSourceLink(ULGUIPrefab* Prefab, const TWeakObjectPtr<UObject> ObjectWeakPtr);
		TSharedRef<SWidget> GetSourceLinkFormatted(ULGUIPrefab* Class, const TWeakObjectPtr<UObject> ObjectWeakPtr, const FText& BlueprintFormat, const FText& CodeFormat);
		/** Weak reference to the outliner AnchorData that owns our list */
		TWeakPtr< ISceneOutliner > WeakSceneOutliner;
		TWeakObjectPtr<UWorld> CurrentWorld;
	};
}