// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LexWidgetEditorHierarchyView.h"

#include "LGUIEditorModule.h"
#include "LexUIPrefabEditor.h"
#include "LexWidgetEditorHierarchyViewItem.h"
#include "Core/LexUIManager.h"
#include "Widgets/Layout/SScrollBorder.h"
#include "Widgets/Input/SSearchBox.h"
#include "Framework/Commands/GenericCommands.h"
#include "PrefabSystem/LexUIPrefab.h"
#include "PrefabSystem/LexUIPrefabHelperObject.h"

#define LOCTEXT_NAMESPACE "LexWidgetEditorHierarchyView"

UE_DISABLE_OPTIMIZATION

void SLexWidgetEditorHierarchyView::Construct(const FArguments& InArgs, UWorld* InWorld)
{
	World = InWorld;
	Manager = FLexUIPrefabEditor::GetEditorByWorld(World.Get());
	bRebuildTreeRequested = false;
	bIsUpdatingSelection = false;

	// register for any objects replaced
	FCoreUObjectDelegates::OnObjectsReplaced.AddRaw(this, &SLexWidgetEditorHierarchyView::OnObjectsReplaced);
	
	SearchBoxWidgetFilter = MakeShareable(new WidgetTextFilter(WidgetTextFilter::FItemToStringArray::CreateSP(this, &SLexWidgetEditorHierarchyView::GetWidgetFilterStrings)));

	FilterHandler = MakeShareable(new TreeFilterHandler<TWeakObjectPtr<ULexWidget>>());
	FilterHandler->SetFilter(SearchBoxWidgetFilter.Get());
	FilterHandler->SetRootItems(&RootWidgets, &TreeRootWidgets);
	FilterHandler->SetGetChildrenDelegate(TreeFilterHandler< TWeakObjectPtr<ULexWidget> >::FOnGetChildren::CreateRaw(this, &SLexWidgetEditorHierarchyView::OnGetChildren));

	CommandList = MakeShareable(new FUICommandList);
	CommandList->MapAction(
		FGenericCommands::Get().Rename,
		FExecuteAction::CreateSP(this, &SLexWidgetEditorHierarchyView::BeginRename),
		FCanExecuteAction::CreateSP(this, &SLexWidgetEditorHierarchyView::CanRename)
	);

	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.Padding(4)
		.AutoHeight()
		[
			SAssignNew(SearchBoxPtr, SSearchBox)
			.HintText(LOCTEXT("SearchWidgets", "Search Widgets"))
			.OnTextChanged(this, &SLexWidgetEditorHierarchyView::OnSearchChanged)
		]

		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SAssignNew(TreeViewArea, SBorder)
			.Padding(0)
			.BorderImage( FAppStyle::GetBrush( "NoBrush" ) )
		]
	];

	RebuildTreeView();

	bRefreshRequested = true;

	if (Manager.IsValid())
	{
		Manager.Pin()->OnSelectionChanged.AddRaw(this, &SLexWidgetEditorHierarchyView::OnEditorSelectionChanged);

		auto PrefabHelperObject = Manager.Pin()->GetPrefabHelperObject();
		auto UnexpandWidgetGuidSet = Manager.Pin()->GetPrefabBeingEdited()->PrefabDataForPrefabEditor.UnexpandedWidgetSet;
		TSet<ULexWidget*> UnexpendWidgetSet;
		for (auto& ItemActorGuid : UnexpandWidgetGuidSet)
		{
			if (auto ObjectPtr = PrefabHelperObject->MapGuidToObject.Find(ItemActorGuid))
			{
				if (auto Widget = Cast<ULexWidget>(*ObjectPtr))
				{
					UnexpendWidgetSet.Add(Widget);
				}
			}
		}

		ULexUIManagerObject::AddOneShotTickFunction([=, this]()
		{
			TSet<TWeakObjectPtr<ULexWidget>> VisitingItems;
			WidgetTreeView->GetExpandedItems(VisitingItems);
			for (auto& Item : VisitingItems)
			{
				if (UnexpendWidgetSet.Contains(Item.Get()))
				{
					WidgetTreeView->SetItemExpansion(Item, false);
				}
			}
		},1);
	}
}
SLexWidgetEditorHierarchyView::~SLexWidgetEditorHierarchyView()
{
	if (Manager.IsValid())
	{
		Manager.Pin()->OnSelectionChanged.RemoveAll(this);
	}

	FCoreUObjectDelegates::OnObjectsReplaced.RemoveAll(this);
}
void SLexWidgetEditorHierarchyView::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (bRebuildTreeRequested || bRefreshRequested)
	{
		if (bRebuildTreeRequested)
		{
			RebuildTreeView();
		}

		RefreshTree();

		UpdateItemsExpansionFromModel();

		OnEditorSelectionChanged();

		bRefreshRequested = false;
		bRebuildTreeRequested = false;
	}
}
void SLexWidgetEditorHierarchyView::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{

}
void SLexWidgetEditorHierarchyView::OnMouseLeave(const FPointerEvent& MouseEvent)
{

}
FReply SLexWidgetEditorHierarchyView::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (Manager.IsValid() && Manager.Pin()->GetToolkitCommands()->ProcessCommandBindings(InKeyEvent))
	{
		return FReply::Handled();
	}
	if (CommandList->ProcessCommandBindings(InKeyEvent))
	{
		return FReply::Handled();
	}
	return FReply::Unhandled();
}
bool SLexWidgetEditorHierarchyView::CanRename() const
{
	auto SelectedItems = WidgetTreeView->GetSelectedItems();
	if (SelectedItems.Num() == 1)
	{
		auto ItemWidget = StaticCastSharedPtr<SLexWidgetEditorHierarchyViewItem>(WidgetTreeView->WidgetFromItem(SelectedItems[0]));
		return ItemWidget->CanRename();
	}
	return false;
}
void SLexWidgetEditorHierarchyView::BeginRename()
{
	auto SelectedItems = WidgetTreeView->GetSelectedItems();
	if (SelectedItems.Num() == 1)
	{
		auto ItemWidget = StaticCastSharedPtr<SLexWidgetEditorHierarchyViewItem>(WidgetTreeView->WidgetFromItem(SelectedItems[0]));
		ItemWidget->RequestEditName();
	}
}
TWeakObjectPtr<ULexWidget> SLexWidgetEditorHierarchyView::SetSelectionByNodeObject(ULexWidget* Element)
{
	WidgetTreeView->ClearSelection();
	if (!Element)return nullptr;
	struct LOCAL
	{
		static ULexWidget* RecursiveSearch(ULexWidget* Element, ULexWidget* Root)
		{
			auto& Children = Root->GetChildren();
			for (auto& Item: Children)
			{
				if (Item == Element)
				{
					return Item;
				}
				if (auto Result = RecursiveSearch(Element, Item))
				{
					return Result;
				}
			}
			return nullptr;
		}
	};
	
	if (auto FoundItem = LOCAL::RecursiveSearch(Element, RootWidgets[0].Get()))
	{
		WidgetTreeView->SetSelection(FoundItem);
		return FoundItem;
	}
	return nullptr;
}

void SLexWidgetEditorHierarchyView::SetSelectionsByNodeObjects(const TArray<TWeakObjectPtr<ULexWidget>>& ElementArray)
{
	WidgetTreeView->ClearSelection();
	WidgetTreeView->SetItemSelection(ElementArray, true);
}

void SLexWidgetEditorHierarchyView::ClearSelection()
{
	WidgetTreeView->ClearSelection();
	RequestRefresh();
}

void SLexWidgetEditorHierarchyView::RequestRefresh()
{
	bRefreshRequested = true;
}
void SLexWidgetEditorHierarchyView::RefreshImmediately()
{
	RebuildTreeView();
	RefreshTree();
	UpdateItemsExpansionFromModel();
}
void SLexWidgetEditorHierarchyView::RefreshTree()
{
	RootWidgets.Empty();
	if (auto LexUIManager = ULexUIManagerWorldSubsystem::GetInstance(World.Get()))
	{
		for (auto& Widget : LexUIManager->GetAllWidgetArray())
		{
			if (Widget->IsRootWidgetInHierarchy())
			{
				RootWidgets.Add(Widget);
			}
		}
	}

	FilterHandler->RefreshAndFilterTree();
}
void SLexWidgetEditorHierarchyView::RebuildTreeView()
{
	float OldScrollOffset = 0;
	if (WidgetTreeView.IsValid())
	{
		OldScrollOffset = WidgetTreeView->GetScrollOffset();
	}

	SAssignNew(WidgetTreeView, STreeView<TWeakObjectPtr<ULexWidget>>)
		.SelectionMode(ESelectionMode::Multi)
		.OnGetChildren(FilterHandler.ToSharedRef(), &TreeFilterHandler< TWeakObjectPtr<ULexWidget>>::OnGetFilteredChildren)
		.OnGenerateRow(this, &SLexWidgetEditorHierarchyView::OnGenerateRow)
		.OnSelectionChanged(this, &SLexWidgetEditorHierarchyView::OnSelectionChanged)
		.OnExpansionChanged(this, &SLexWidgetEditorHierarchyView::OnExpansionChanged)
		.OnContextMenuOpening(this, &SLexWidgetEditorHierarchyView::OnContextMenuOpening)
		.OnSetExpansionRecursive(this, &SLexWidgetEditorHierarchyView::SetItemExpansionRecursive)
		.TreeItemsSource(&TreeRootWidgets)
		//.OnMouseButtonClick(this, &SLexWidgetHierarchyView::WidgetHierarchy_OnMouseClick)
		;

	FilterHandler->SetTreeView(WidgetTreeView.Get());

	TreeViewArea->SetContent(
		SNew(SScrollBorder, WidgetTreeView.ToSharedRef())
		[
			WidgetTreeView.ToSharedRef()
		]
	);

	// Restore the previous scroll offset
	WidgetTreeView->SetScrollOffset(OldScrollOffset);
}

void SLexWidgetEditorHierarchyView::OnEditorSelectionChanged()
{
	if (!bIsUpdatingSelection)
	{
		WidgetTreeView->ClearSelection();

		auto Selection = ULexUISelection::GetInstance(World.Get());
		auto SelectedWidgets = Selection ? Selection->GetSelectedWidgets() : TArray<TWeakObjectPtr<ULexWidget>>();
		if (SelectedWidgets.Num() == 0)
		{
			ClearSelection();
		}
		else
		{
			TArray<TWeakObjectPtr<ULexWidget>> SelectedItems;
			for (auto& Item: SelectedWidgets)
			{
				if (Item.IsValid())
				{
					SelectedItems.Add(Item.Get());
				}
			}
			if (SelectedItems.Num() == 0)
			{
				ClearSelection();
			}
			else
			{
				SetSelectionsByNodeObjects(SelectedItems);

				//expand
				if (SelectedItems.Num() == 1)
				{
					auto Widget = SelectedItems[0]->GetParent();
					while (Widget != nullptr)
					{
						WidgetTreeView->SetItemExpansion(Widget, true);
						Widget = Widget->GetParent();
					}
				}
			}
		}
	}
}

void SLexWidgetEditorHierarchyView::OnWidgetHierarchyChanged()
{
	bRefreshRequested = true;
}

void SLexWidgetEditorHierarchyView::OnObjectsReplaced(const TMap<UObject*, UObject*>& ReplacementMap)
{
	if ( !bRebuildTreeRequested )
	{
		bRefreshRequested = true;
		bRebuildTreeRequested = true;
	}
}

TSharedRef< ITableRow > SLexWidgetEditorHierarchyView::OnGenerateRow(TWeakObjectPtr<ULexWidget> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SLexWidgetEditorHierarchyViewItem, OwnerTable, InItem, SharedThis(this), Manager.Pin())
	.MouseEnter_Lambda([=, this] {
		})
	.MouseExit_Lambda([=, this] {
		})
	;
}

void SLexWidgetEditorHierarchyView::OnSelectionChanged(TWeakObjectPtr<ULexWidget> SelectedItem, ESelectInfo::Type SelectInfo)
{
	if (SelectInfo != ESelectInfo::Direct)
	{
		bIsUpdatingSelection = true;

		auto SelectedItems = WidgetTreeView->GetSelectedItems();

		TSet<ULexWidget*> NewSelectedItems;
		for (auto& Item : SelectedItems)
		{
			NewSelectedItems.Add(Item.Get());
		}
		if (Manager.IsValid())
		{
			Manager.Pin()->SelectWidgets(NewSelectedItems, false);
		}
		else
		{
			if (auto Selection = ULexUISelection::GetInstance(World.Get()))
			{
				const FScopedTransaction Transaction(LOCTEXT("SelectionChanged_Transaction", "Select Widgets"));
				Selection->Modify();
				Selection->SelectNone();
				for ( const auto& Widget : NewSelectedItems )
				{
					Selection->SelectWidget(Widget);
				}
			}
		}

		bIsUpdatingSelection = false;
	}
}
void SLexWidgetEditorHierarchyView::OnGetChildren(TWeakObjectPtr<ULexWidget> InParent, TArray< TWeakObjectPtr<ULexWidget> >& OutChildren)
{
	if (!InParent.IsValid())return;
	auto& Children = InParent->GetChildren();
	OutChildren.Append(Children);
}
void SLexWidgetEditorHierarchyView::GetWidgetFilterStrings(TWeakObjectPtr<ULexWidget> Item, TArray<FString>& OutStrings)
{
	OutStrings.Add(Item->GetDisplayName());
}
void SLexWidgetEditorHierarchyView::OnSearchChanged(const FText& InFilterText)
{
	bRefreshRequested = true;
	const bool bFilteringEnabled = !InFilterText.IsEmpty();
	if (bFilteringEnabled != FilterHandler->GetIsEnabled())
	{
		FilterHandler->SetIsEnabled(bFilteringEnabled);
		if (bFilteringEnabled)
		{
			SaveItemsExpansion();
		}
		else
		{
			RestoreItemsExpansion();
		}
	}
	SearchBoxWidgetFilter->SetRawFilterText(InFilterText);
	SearchBoxPtr->SetError(SearchBoxWidgetFilter->GetFilterErrorText());
}

void SLexWidgetEditorHierarchyView::OnExpansionChanged(TWeakObjectPtr<ULexWidget> Item, bool bExpanded)
{
	ExpansionMap.FindOrAdd(Item.Get()) = bExpanded;
}
TSharedPtr<SWidget> SLexWidgetEditorHierarchyView::OnContextMenuOpening()
{
	if (!Manager.IsValid())
	{
		return nullptr;
	}
	TFunction<ULexWidget*()> GetSelectedWidgetFunction = [this]()
	{
		if (Manager.IsValid())
		{
			if (Manager.Pin()->GetSelectedWidgets().Num() == 1)
			{
				auto Widget = Manager.Pin()->GetSelectedWidgets()[0];
				if (Widget.IsValid())
				{
					return Widget.Get();
				}
			}
		}
		return (ULexWidget*)nullptr;
	};
	TFunction<TArray<ULexWidget*>()> GetSelectedWidgetsFunction = [this]()
	{
		TArray<ULexWidget*> Widgets;
		if (Manager.IsValid())
		{
			for (auto Widget : Manager.Pin()->GetSelectedWidgets())
			{
				Widgets.Add(Widget.Get());
			}
		}
		return Widgets;
	};
	return FLGUIEditorModule::Get().MakeEditorToolsMenu( GetSelectedWidgetFunction, [=, this](FMenuBuilder& MenuBuilder)
	{
		MenuBuilder.BeginSection("Edit", LOCTEXT("Edit", "Edit"));
		{
			MenuBuilder.PushCommandList(Manager.Pin()->GetToolkitCommands());
			{
				MenuBuilder.AddMenuEntry(FGenericCommands::Get().Copy);
				MenuBuilder.AddMenuEntry(FGenericCommands::Get().Paste);
				MenuBuilder.AddMenuEntry(FGenericCommands::Get().Cut);
				MenuBuilder.AddMenuEntry(FGenericCommands::Get().Duplicate);
				MenuBuilder.AddMenuEntry(FGenericCommands::Get().Delete);
			}
			MenuBuilder.PopCommandList();
			MenuBuilder.PushCommandList(CommandList.ToSharedRef());
			{
				MenuBuilder.AddMenuEntry(FGenericCommands::Get().Rename);
			}
			MenuBuilder.PopCommandList();
		}
		MenuBuilder.EndSection();
	});
}

void SLexWidgetEditorHierarchyView::GetExpandWidgets(TSet<TWeakObjectPtr<ULexWidget>>& OutExpandWidgets)
{
	WidgetTreeView->GetExpandedItems(OutExpandWidgets);
}

void SLexWidgetEditorHierarchyView::UpdateItemsExpansionFromModel()
{
	for (auto Widget: RootWidgets)
	{
		RecursiveExpand(Widget.Get(), EExpandBehavior::FromModel);
	}
}

void SLexWidgetEditorHierarchyView::RestoreItemsExpansion()
{
	for (auto Widget : RootWidgets)
	{
		RecursiveExpand(Widget.Get(), EExpandBehavior::RestoreFromPrevious);
	}
}

void SLexWidgetEditorHierarchyView::SaveItemsExpansion()
{
	ExpandedItemNames.Empty();

	if (WidgetTreeView.IsValid())
	{
		TSet< TWeakObjectPtr<ULexWidget> > ExpandedItems;
		WidgetTreeView->GetExpandedItems(ExpandedItems);

		for (TWeakObjectPtr<ULexWidget> Item : ExpandedItems)
		{
			if (Item.IsValid())
			{
				ExpandedItemNames.Add(Item->GetName());
			}
		}
	}
}
void SLexWidgetEditorHierarchyView::RecursiveExpand(ULexWidget* Widget, EExpandBehavior ExpandBehavior)
{
	bool bShouldExpandItem = true;

	switch (ExpandBehavior)
	{
	case EExpandBehavior::NeverExpand:
		{
			bShouldExpandItem = false;
		}
		break;

	case EExpandBehavior::RestoreFromPrevious:
		{
			bShouldExpandItem = ExpandedItemNames.Contains(Widget->GetName());
		}
		break;

	case EExpandBehavior::AlwaysExpand:
		{
			bShouldExpandItem = true;
		}
		break;

	case EExpandBehavior::FromModel:
	default:
		{
			if (auto ValuePtr = ExpansionMap.Find(Widget))
			{
				bShouldExpandItem = *ValuePtr;
			}
			else
			{
				bShouldExpandItem = true;
			}
		}
		break;
	}

	WidgetTreeView->SetItemExpansion(Widget, bShouldExpandItem);

	auto& Children = Widget->GetChildren();
	for (auto Child: Children)
	{
		RecursiveExpand(Child, ExpandBehavior);
	}
}

void SLexWidgetEditorHierarchyView::SetItemExpansionRecursive(TWeakObjectPtr<ULexWidget> Model, bool bInExpansionState)
{
	if (Model.IsValid())
	{
		RecursiveExpand(Model.Get(), bInExpansionState ? EExpandBehavior::AlwaysExpand : EExpandBehavior::NeverExpand);
	}
}

UE_ENABLE_OPTIMIZATION

#undef LOCTEXT_NAMESPACE