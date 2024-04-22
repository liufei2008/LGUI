// Copyright 2019-Present LexLiu. All Rights Reserved.

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
#include "LGUIPrefabSequenceEditorWidget.h"
#include "IPropertyUtilities.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Layout/SScrollBorder.h"
#include "Widgets/Input/SSearchBox.h"
#include "LGUIEditorModule.h"
#include "Framework/Commands/GenericCommands.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "Misc/TextFilter.h"
#include "PropertyCustomizationHelpers.h"
#include "PrefabSystem/LGUIPrefabHelperObject.h"
#include "LGUIEditorTools.h"

#define LOCTEXT_NAMESPACE "SLGUIPrefabSequenceEditor"


struct FWidgetAnimationListItem
{
	FWidgetAnimationListItem(ULGUIPrefabSequence* InAnimation, bool bInRenameRequestPending = false, bool bInNewAnimation = false)
		: Animation(InAnimation)
		, bRenameRequestPending(bInRenameRequestPending)
		, bNewAnimation(bInNewAnimation)
	{}

	ULGUIPrefabSequence* Animation;
	bool bRenameRequestPending;
	bool bNewAnimation;
};


typedef SListView<TSharedPtr<FWidgetAnimationListItem> > SWidgetAnimationListView;

class SWidgetAnimationListItem : public STableRow<TSharedPtr<FWidgetAnimationListItem> >
{
public:
	SLATE_BEGIN_ARGS(SWidgetAnimationListItem) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, SLGUIPrefabSequenceEditor* InEditor, TSharedPtr<FWidgetAnimationListItem> InListItem)
	{
		ListItem = InListItem;
		Editor = InEditor;

		STableRow<TSharedPtr<FWidgetAnimationListItem>>::Construct(
			STableRow<TSharedPtr<FWidgetAnimationListItem>>::FArguments()
			.Padding(FMargin(3.0f, 2.0f))
			.Content()
			[
				SAssignNew(InlineTextBlock, SInlineEditableTextBlock)
				.Font(FCoreStyle::Get().GetFontStyle("NormalFont"))
				.Text(this, &SWidgetAnimationListItem::GetMovieSceneText)
				//.HighlightText(InArgs._HighlightText)
				.OnVerifyTextChanged(this, &SWidgetAnimationListItem::OnVerifyNameTextChanged)
				.OnTextCommitted(this, &SWidgetAnimationListItem::OnNameTextCommited)
				.IsSelected(this, &SWidgetAnimationListItem::IsSelectedExclusively)
			],
			InOwnerTableView);
	}

	void BeginRename()
	{
		InlineTextBlock->EnterEditingMode();
	}

private:
	FText GetMovieSceneText() const
	{
		if (ListItem.IsValid())
		{
			return ListItem.Pin()->Animation->GetDisplayName();
		}

		return FText::GetEmpty();
	}

	bool OnVerifyNameTextChanged(const FText& InText, FText& OutErrorMessage)
	{
		auto Animation = ListItem.Pin()->Animation;

		auto SequenceComp = Editor->GetSequenceComponent();
		if (SequenceComp)
		{
			auto& SequenceArray = SequenceComp->GetSequenceArray();
			auto ExistIndex = SequenceArray.IndexOfByPredicate([InText, this](const ULGUIPrefabSequence* Item) {
				if (ListItem.Pin()->Animation == Item)
				{
					return false;
				}
				return Item->GetDisplayName().EqualTo(InText);
				});
			if (ExistIndex != INDEX_NONE)
			{
				OutErrorMessage = LOCTEXT("NameInUseByAnimation", "An animation with this name already exists");
				return false;
			}
		}
		return true;
	}

	void OnNameTextCommited(const FText& InText, ETextCommit::Type CommitInfo)
	{
		auto Animation = ListItem.Pin()->Animation;

		// Name has already been checked in VerifyAnimationRename
		auto NewName = InText.ToString();
		auto OldName = Animation->GetDisplayName().ToString();

		//FObjectPropertyBase* ExistingProperty = CastField<FObjectPropertyBase>(Blueprint->ParentClass->FindPropertyByName(NewFName));
		//const bool bBindWidgetAnim = ExistingProperty && FWidgetBlueprintEditorUtils::IsBindWidgetAnimProperty(ExistingProperty) && ExistingProperty->PropertyClass->IsChildOf(UWidgetAnimation::StaticClass());

		const bool bValidName = !OldName.Equals(NewName) && !InText.IsEmpty();
		const bool bCanRename = (bValidName/* || bBindWidgetAnim*/);

		const bool bNewAnimation = ListItem.Pin()->bNewAnimation;
		if (bCanRename)
		{
			FText TransactionName = bNewAnimation ? LOCTEXT("NewAnimation", "New Animation") : LOCTEXT("RenameAnimation", "Rename Animation");
			{
				const FScopedTransaction Transaction(TransactionName);
				Animation->Modify();

				Animation->SetDisplayName(NewName);

				if (bNewAnimation)
				{
					Editor->RefreshAnimationList();
					ListItem.Pin()->bNewAnimation = false;
				}
			}
		}
		else if (bNewAnimation)
		{
			const FScopedTransaction Transaction(LOCTEXT("NewAnimation", "New Animation"));
			Editor->RefreshAnimationList();
			ListItem.Pin()->bNewAnimation = false;
		}
	}
private:
	TWeakPtr<FWidgetAnimationListItem> ListItem;
	SLGUIPrefabSequenceEditor* Editor = nullptr;
	TSharedPtr<SInlineEditableTextBlock> InlineTextBlock;
};


SLGUIPrefabSequenceEditor::~SLGUIPrefabSequenceEditor()
{
	FCoreUObjectDelegates::OnObjectsReplaced.Remove(OnObjectsReplacedHandle);
	LGUIEditorTools::OnEditingPrefabChanged.Remove(EditingPrefabChangedHandle);
	LGUIEditorTools::OnBeforeApplyPrefab.Remove(OnBeforeApplyPrefabHandle);
}

void SLGUIPrefabSequenceEditor::Construct(const FArguments& InArgs)
{
	SAssignNew(AnimationListView, SWidgetAnimationListView)
		.SelectionMode(ESelectionMode::Single)
		.ListItemsSource(&Animations)
		.OnGenerateRow(this, &SLGUIPrefabSequenceEditor::OnGenerateRowForAnimationListView)
		.OnItemScrolledIntoView(this, &SLGUIPrefabSequenceEditor::OnItemScrolledIntoView)
		.OnSelectionChanged(this, &SLGUIPrefabSequenceEditor::OnAnimationListViewSelectionChanged)
		.OnContextMenuOpening(this, &SLGUIPrefabSequenceEditor::OnContextMenuOpening)
		;

	ChildSlot
		[
			SNew(SSplitter)
			+SSplitter::Slot()
			.Value(0.2f)
			[
				SNew(SBox)
				.IsEnabled_Lambda([=]() {
					return WeakSequenceComponent.IsValid();
				})
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.Padding( 2 )
						.AutoHeight()
						[
							SNew(SHorizontalBox)
							+SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SButton)
								.Text_Lambda([=](){
									if (WeakSequenceComponent.IsValid())
									{
										auto Actor = WeakSequenceComponent->GetOwner();
										if (Actor)
										{
											auto DisplayText = Actor->GetActorLabel() + TEXT(".") + WeakSequenceComponent->GetName();
											return FText::FromString(DisplayText);
										}
									}
									return LOCTEXT("NullSequenceComponent", "Null (LGUIPrefabSequence)");
								})
								.ToolTipText(LOCTEXT("ObjectButtonTooltipText", "Actor.Component, click to select target"))
								.IsEnabled_Lambda([=](){
									return WeakSequenceComponent.IsValid();
								})
								.ButtonStyle( FAppStyle::Get(), "PropertyEditor.AssetComboStyle" )
								.ForegroundColor(FAppStyle::GetColor("PropertyEditor.AssetName.ColorAndOpacity"))
								.OnClicked_Lambda([=](){
									if (WeakSequenceComponent.IsValid())
									{
										GEditor->SelectNone(true, true);
										GEditor->SelectActor(WeakSequenceComponent->GetOwner(), true, true);
										GEditor->SelectComponent(WeakSequenceComponent.Get(), true, true);
									}
									return FReply::Handled();
								})
							]
							+SHorizontalBox::Slot()
							.HAlign(HAlign_Right)
							.VAlign(VAlign_Center)
							[
								PropertyCustomizationHelpers::MakeResetButton(
									FSimpleDelegate::CreateLambda([=]() {
										AssignLGUIPrefabSequenceComponent(nullptr);
										})
									, LOCTEXT("ClearSequenceComponent", "Click to clear current selected LGUISequenceComponent, so we will not edit it here.")
											)
							]
						]
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
								SNew(SButton)
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
			]
			+SSplitter::Slot()
			.Value(0.8f)
			[
				SAssignNew(PrefabSequenceEditor, SLGUIPrefabSequenceEditorWidget, nullptr)
			]
		];

	CreateCommandList();

	OnObjectsReplacedHandle = FCoreUObjectDelegates::OnObjectsReplaced.AddSP(this, &SLGUIPrefabSequenceEditor::OnObjectsReplaced);

	PrefabSequenceEditor->AssignSequence(GetLGUIPrefabSequence());
	EditingPrefabChangedHandle = LGUIEditorTools::OnEditingPrefabChanged.AddRaw(this, &SLGUIPrefabSequenceEditor::OnEditingPrefabChanged);
	OnBeforeApplyPrefabHandle = LGUIEditorTools::OnBeforeApplyPrefab.AddRaw(this, &SLGUIPrefabSequenceEditor::OnBeforeApplyPrefab);
}

void SLGUIPrefabSequenceEditor::AssignLGUIPrefabSequenceComponent(TWeakObjectPtr<ULGUIPrefabSequenceComponent> InSequenceComponent)
{
	WeakSequenceComponent = InSequenceComponent;
	RefreshAnimationList();
}

ULGUIPrefabSequence* SLGUIPrefabSequenceEditor::GetLGUIPrefabSequence() const
{
	if (CurrentSelectedAnimationIndex != INDEX_NONE)
	{
		ULGUIPrefabSequenceComponent* SequenceComponent = WeakSequenceComponent.Get();
		return SequenceComponent ? SequenceComponent->GetSequenceByIndex(CurrentSelectedAnimationIndex) : nullptr;
	}
	return nullptr;
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

TSharedRef<ITableRow> SLGUIPrefabSequenceEditor::OnGenerateRowForAnimationListView(TSharedPtr<FWidgetAnimationListItem> InListItem, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	return SNew(SWidgetAnimationListItem, InOwnerTableView, this, InListItem);
}

void SLGUIPrefabSequenceEditor::OnAnimationListViewSelectionChanged(TSharedPtr<FWidgetAnimationListItem> InListItem, ESelectInfo::Type InSelectInfo)
{
	CurrentSelectedAnimationIndex = INDEX_NONE;
	if (InListItem.IsValid())
	{
		auto& SequenceArray = WeakSequenceComponent->GetSequenceArray();
		for (int i = 0; i < SequenceArray.Num(); i++)
		{
			if (SequenceArray[i] == InListItem->Animation)
			{
				CurrentSelectedAnimationIndex = i;
				break;
			}
		}
	}
	PrefabSequenceEditor->AssignSequence(GetLGUIPrefabSequence());
}

void SLGUIPrefabSequenceEditor::RefreshAnimationList()
{
	if (WeakSequenceComponent.IsValid())
	{
		Animations.Reset();
		auto& SequenceArray = WeakSequenceComponent->GetSequenceArray();
		for (auto& Item : SequenceArray)
		{
			Animations.Add(MakeShareable(new FWidgetAnimationListItem(Item)));
		}
		AnimationListView->RequestListRefresh();
		if (Animations.Num() > 0)
		{
			AnimationListView->SetSelection(Animations[0]);
		}
	}
}

void SLGUIPrefabSequenceEditor::OnBeforeApplyPrefab(ULGUIPrefabHelperObject* InObject)
{
	if (WeakSequenceComponent.IsValid())
	{
		if (auto Actor = WeakSequenceComponent->GetOwner())
		{
			if (InObject->IsActorBelongsToThis(Actor))
			{
				this->AnimationListView->ClearSelection();
			}
		}
	}
}

// Trigger when opening a new prefab
void SLGUIPrefabSequenceEditor::OnEditingPrefabChanged(AActor* RootActor)
{
	if (RootActor)
	{
		TArray<AActor*> ChildrenActors;
		RootActor->GetAttachedActors(ChildrenActors, true, true);
		
		for (AActor* ChildActor : ChildrenActors)
		{
			ULGUIPrefabHelperObject* PrefabHelperObject = LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(RootActor);
			if (PrefabHelperObject)
			{
				//skip sub prefab's PrefabSequenceComponent
				if (PrefabHelperObject->IsActorBelongsToSubPrefab(ChildActor))
				{
					continue;
				}
			}

			ULGUIPrefabSequenceComponent* PrefabSequencerComponent = ChildActor->FindComponentByClass<ULGUIPrefabSequenceComponent>();
			if (PrefabSequencerComponent)
			{
				AssignLGUIPrefabSequenceComponent(PrefabSequencerComponent);
			}
		}
	}
}

void SLGUIPrefabSequenceEditor::OnAnimationListViewSearchChanged(const FText& InSearchText)
{
	if (WeakSequenceComponent.IsValid())
	{
		auto& SequenceArray = WeakSequenceComponent->GetSequenceArray();
		if (!InSearchText.IsEmpty())
		{
			struct Local
			{
				static void UpdateFilterStrings(ULGUIPrefabSequence* InAnimation, OUT TArray< FString >& OutFilterStrings)
				{
					OutFilterStrings.Add(InAnimation->GetName());
				}
			};

			TTextFilter<ULGUIPrefabSequence*> TextFilter(TTextFilter<ULGUIPrefabSequence*>::FItemToStringArray::CreateStatic(&Local::UpdateFilterStrings));

			TextFilter.SetRawFilterText(InSearchText);
			SearchBoxPtr->SetError(TextFilter.GetFilterErrorText());

			Animations.Reset();

			for (ULGUIPrefabSequence* Animation : SequenceArray)
			{
				if (TextFilter.PassesFilter(Animation))
				{
					Animations.Add(MakeShareable(new FWidgetAnimationListItem(Animation)));
				}
			}

			AnimationListView->RequestListRefresh();
		}
		else
		{
			SearchBoxPtr->SetError(FText::GetEmpty());
			RefreshAnimationList();
		}
	}
}

void SLGUIPrefabSequenceEditor::OnItemScrolledIntoView(TSharedPtr<FWidgetAnimationListItem> InListItem, const TSharedPtr<ITableRow>& InWidget) const
{
	if (InListItem->bRenameRequestPending)
	{
		StaticCastSharedPtr<SWidgetAnimationListItem>(InWidget)->BeginRename();
		InListItem->bRenameRequestPending = false;
	}
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
		//create fix button
		{
			auto SelectedItems = AnimationListView->GetSelectedItems();
			if (SelectedItems.Num() == 1)
			{
				auto SelectedItem = SelectedItems[0];
				if (!SelectedItem->Animation->IsObjectReferencesGood(WeakSequenceComponent->GetOwner()))
				{
					MenuBuilder.AddMenuSeparator();
					MenuBuilder.AddMenuEntry(
						LOCTEXT("TryFixObjectReference", "Try fix object reference"),
						LOCTEXT("TryFixObjectReference_Tooltip", "LGUI can search target object by actor's path relative to ContextActor (Owner actor of LGUIPrefabSequenceComponent), so if ActorLabel and Actor's hierarchy is same as before, it is possible to fix the bad tracks."),
						FSlateIcon(),
						FUIAction(FExecuteAction::CreateLambda([=]() {
							SelectedItem->Animation->FixObjectReferences(WeakSequenceComponent->GetOwner());
							}))
					);
				}
			}
		}
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

FReply SLGUIPrefabSequenceEditor::OnNewAnimationClicked()
{
	if (WeakSequenceComponent.IsValid())
	{
		auto Sequence = WeakSequenceComponent->AddNewAnimation();
		bool bRequestRename = true;
		bool bNewAnimation = true;
		int32 NewIndex = Animations.Add(MakeShareable(new FWidgetAnimationListItem(Sequence, bRequestRename, bNewAnimation)));
		AnimationListView->RequestScrollIntoView(Animations[NewIndex]);
	}
	return FReply::Handled();
}

void SLGUIPrefabSequenceEditor::OnDuplicateAnimation()
{
	if (WeakSequenceComponent.IsValid())
	{
		GEditor->BeginTransaction(LOCTEXT("DuplicateAnimation_Transaction", "LGUISequence Duplicate Animation"));
		WeakSequenceComponent->Modify();
		auto Sequence = WeakSequenceComponent->DuplicateAnimationByIndex(CurrentSelectedAnimationIndex);
		GEditor->EndTransaction();

		if (Sequence)
		{
			bool bRequestRename = true;
			bool bNewAnimation = true;
			int32 NewIndex = Animations.Insert(MakeShareable(new FWidgetAnimationListItem(Sequence, bRequestRename, bNewAnimation)), CurrentSelectedAnimationIndex + 1);
			AnimationListView->RequestScrollIntoView(Animations[NewIndex]);
		}
	}
}
void SLGUIPrefabSequenceEditor::OnDeleteAnimation()
{
	if (WeakSequenceComponent.IsValid())
	{
		GEditor->BeginTransaction(LOCTEXT("DeleteAnimation_Transaction", "LGUISequence Delete Animation"));
		WeakSequenceComponent->Modify();
		bool bDeleted = WeakSequenceComponent->DeleteAnimationByIndex(CurrentSelectedAnimationIndex);
		GEditor->EndTransaction();

		if (bDeleted)
		{
			Animations.RemoveAt(CurrentSelectedAnimationIndex);
			AnimationListView->RebuildList();
			CurrentSelectedAnimationIndex = INDEX_NONE;
			PrefabSequenceEditor->AssignSequence(nullptr);
		}
	}
}
void SLGUIPrefabSequenceEditor::OnRenameAnimation()
{
	TArray< TSharedPtr<FWidgetAnimationListItem> > SelectedAnimations = AnimationListView->GetSelectedItems();
	check(SelectedAnimations.Num() == 1);

	TSharedPtr<FWidgetAnimationListItem> SelectedAnimation = SelectedAnimations[0];
	SelectedAnimation->bRenameRequestPending = true;

	AnimationListView->RequestScrollIntoView(SelectedAnimation);
}

#undef LOCTEXT_NAMESPACE
