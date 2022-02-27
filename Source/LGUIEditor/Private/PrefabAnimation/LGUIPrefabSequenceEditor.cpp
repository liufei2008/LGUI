// Copyright Epic Games, Inc. All Rights Reserved.

#include "LGUIPrefabSequenceEditor.h"

#include "PrefabAnimation/LGUIPrefabSequence.h"
#include "PrefabAnimation/LGUIPrefabSequenceComponent.h"
#include "EditorStyleSet.h"
#include "GameFramework/Actor.h"
#include "IDetailsView.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Docking/SDockTab.h"
#include "SSCSEditor.h"
#include "BlueprintEditorTabs.h"
#include "ScopedTransaction.h"
#include "ISequencerModule.h"
#include "Editor.h"
#include "LGUIPrefabSequenceEditorTabSummoner.h"
#include "IPropertyUtilities.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Layout/SScrollBorder.h"
#include "Widgets/Input/SSearchBox.h"
#include "LGUIEditorModule.h"
#include "Framework/Commands/GenericCommands.h"

#define LOCTEXT_NAMESPACE "SLGUIPrefabSequenceEditor"

SLGUIPrefabSequenceEditor::~SLGUIPrefabSequenceEditor()
{
	GEditor->OnObjectsReplaced().Remove(OnObjectsReplacedHandle);
}

void SLGUIPrefabSequenceEditor::Construct(const FArguments& InArgs)
{
	SAssignNew(AnimationListView, SListView<ULGUIPrefabSequence*>)
		.SelectionMode(ESelectionMode::Single)
		.ListItemsSource(&EmptyListItemSource)
		.OnGenerateRow(this, &SLGUIPrefabSequenceEditor::OnGenerateRowForAnimationListView)
		.OnSelectionChanged(this, &SLGUIPrefabSequenceEditor::OnAnimationListViewSelectionChanged)
		.OnContextMenuOpening(this, &SLGUIPrefabSequenceEditor::OnContextMenuOpening)
		;

	ChildSlot
		[
			SNew(SSplitter)
			+SSplitter::Slot()
			.Value(0.2f)
			[
				SNew(SBorder)
				.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.Padding( 2 )
					.AutoHeight()
					[
						SNew( SHorizontalBox )
						+ SHorizontalBox::Slot()
						.Padding(0)
						.VAlign( VAlign_Center )
						.AutoWidth()
						[
		#if ENGINE_MAJOR_VERSION >= 5
							SNew(SEditorHeaderButton)
		#else
							SNew(SButton)
		#endif
							.OnClicked(this, &SLGUIPrefabSequenceEditor::OnNewAnimationClicked)
							.Text(LOCTEXT("NewAnimationButtonText", "+ Animation"))
						]
						+ SHorizontalBox::Slot()
						.Padding(2.0f, 0.0f)
						.VAlign( VAlign_Center )
						[
							SAssignNew(SearchBoxPtr, SSearchBox)
							.HintText(LOCTEXT("Search Animations", "Search Animations"))
							.OnTextChanged(this, &SLGUIPrefabSequenceEditor::OnAnimationListViewSearchChanged)
						]
					]
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					[
						SNew(SScrollBorder, AnimationListView.ToSharedRef())
						[
							AnimationListView.ToSharedRef()
						]
					]
				]
			]
			+SSplitter::Slot()
			.Value(0.8f)
			[
				SAssignNew(PrefabSequenceEditor, SLGUIPrefabSequenceEditorWidget, nullptr)
			]
		];

	PrefabSequenceEditor->AssignSequence(GetLGUIPrefabSequence());
	CreateCommandList();

	OnObjectsReplacedHandle = GEditor->OnObjectsReplaced().AddSP(this, &SLGUIPrefabSequenceEditor::OnObjectsReplaced);
}

void SLGUIPrefabSequenceEditor::AssignLGUIPrefabSequenceComponent(TWeakObjectPtr<ULGUIPrefabSequenceComponent> InSequenceComponent)
{
	WeakSequenceComponent = InSequenceComponent;
	AnimationListView->SetListItemsSource(WeakSequenceComponent->GetSequenceArray());
}

ULGUIPrefabSequence* SLGUIPrefabSequenceEditor::GetLGUIPrefabSequence() const
{
	ULGUIPrefabSequenceComponent* SequenceComponent = WeakSequenceComponent.Get();
	return SequenceComponent ? SequenceComponent->GetSequenceByIndex(CurrentSelectedAnimationIndex) : nullptr;
}

void SLGUIPrefabSequenceEditor::OnObjectsReplaced(const TMap<UObject*, UObject*>& ReplacementMap)
{
	ULGUIPrefabSequenceComponent* Component = WeakSequenceComponent.Get(true);

	ULGUIPrefabSequenceComponent* NewSequenceComponent = Component ? Cast<ULGUIPrefabSequenceComponent>(ReplacementMap.FindRef(Component)) : nullptr;
	if (NewSequenceComponent)
	{
		WeakSequenceComponent = NewSequenceComponent;
		PrefabSequenceEditor->AssignSequence(GetLGUIPrefabSequence());
	}
}

TSharedRef<ITableRow> SLGUIPrefabSequenceEditor::OnGenerateRowForAnimationListView(ULGUIPrefabSequence* InListItem, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	return SNew(STableRow<ULGUIPrefabSequence*>, InOwnerTableView)
		[
			SNew(STextBlock)
			.Text(FText::FromName(InListItem->GetFName()))
		]
	;
}

void SLGUIPrefabSequenceEditor::OnAnimationListViewSelectionChanged(ULGUIPrefabSequence* InListItem, ESelectInfo::Type InSelectInfo)
{
	auto& SequenceArray = WeakSequenceComponent->GetSequenceArray();
	for (int i = 0; i < SequenceArray.Num(); i++)
	{
		if (SequenceArray[i] == InListItem)
		{
			CurrentSelectedAnimationIndex = i;
			break;
		}
	}
	PrefabSequenceEditor->AssignSequence(GetLGUIPrefabSequence());
}

FReply SLGUIPrefabSequenceEditor::OnNewAnimationClicked()
{
	if (WeakSequenceComponent.IsValid())
	{
		WeakSequenceComponent->AddNewAnimation();
		AnimationListView->RebuildList();
	}
	return FReply::Handled();
}

void SLGUIPrefabSequenceEditor::OnAnimationListViewSearchChanged(const FText& InSearchText)
{

}

TSharedPtr<SWidget> SLGUIPrefabSequenceEditor::OnContextMenuOpening()const
{
	FMenuBuilder MenuBuilder(true, CommandList.ToSharedRef());

	MenuBuilder.BeginSection("Edit", LOCTEXT("Edit", "Edit"));
	{
		MenuBuilder.AddMenuEntry(FGenericCommands::Get().Rename);
		MenuBuilder.AddMenuEntry(FGenericCommands::Get().Duplicate);
		MenuBuilder.AddMenuSeparator();

		MenuBuilder.AddMenuEntry(FGenericCommands::Get().Delete);
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

void SLGUIPrefabSequenceEditor::CreateCommandList()
{
	CommandList = MakeShareable(new FUICommandList);

	CommandList->MapAction(
		FGenericCommands::Get().Duplicate,
		FExecuteAction::CreateSP(this, &SLGUIPrefabSequenceEditor::OnDuplicateAnimation)
	);

	CommandList->MapAction(
		FGenericCommands::Get().Delete,
		FExecuteAction::CreateSP(this, &SLGUIPrefabSequenceEditor::OnDeleteAnimation)
	);

	CommandList->MapAction(
		FGenericCommands::Get().Rename,
		FExecuteAction::CreateSP(this, &SLGUIPrefabSequenceEditor::OnRenameAnimation)
	);
}

void SLGUIPrefabSequenceEditor::OnDuplicateAnimation()
{
	GEditor->BeginTransaction(LOCTEXT("LGUISequence_DuplicateAnimation", "LGUISequence Duplicate Animation"));
	WeakSequenceComponent->Modify();
	WeakSequenceComponent->DuplicateAnimationByIndex(CurrentSelectedAnimationIndex);
	GEditor->EndTransaction();

	AnimationListView->RebuildList();
}
void SLGUIPrefabSequenceEditor::OnDeleteAnimation()
{
	GEditor->BeginTransaction(LOCTEXT("LGUISequence_DeleteAnimation", "LGUISequence Delete Animation"));
	WeakSequenceComponent->Modify();
	WeakSequenceComponent->DeleteAnimationByIndex(CurrentSelectedAnimationIndex);
	GEditor->EndTransaction();

	AnimationListView->RebuildList();
	CurrentSelectedAnimationIndex = -1;
	PrefabSequenceEditor->AssignSequence(nullptr);
}
void SLGUIPrefabSequenceEditor::OnRenameAnimation()
{

}

#undef LOCTEXT_NAMESPACE
