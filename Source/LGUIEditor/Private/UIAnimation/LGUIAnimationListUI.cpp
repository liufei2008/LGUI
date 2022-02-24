/**
  * LGUI动画列表
  */
#include "LGUIAnimationListUI.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "MovieScene.h"
#include "Animation/LGUIAnimation.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SScrollBorder.h"
#include "Widgets/Views/STableViewBase.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/SListView.h"

#if WITH_EDITOR
	#include "EditorStyleSet.h"
#endif // WITH_EDITOR

#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "Framework/Commands/GenericCommands.h"
#include "ScopedTransaction.h"
#include "Misc/TextFilter.h"
#include "Kismet2/Kismet2NameValidators.h"
#include "SEditorHeaderButton.h"
#include "Core/Actor/UIBaseActor.h"
#include "LGUI.h"
#include "PrefabEditor/LGUIPrefabEditor.h"
#include "Core/ActorComponent/UIAnimationComp.h"
#include "UIAnimation/LGUIAnimationUI.h"

#define LOCTEXT_NAMESPACE "LGUI"

PRAGMA_DISABLE_OPTIMIZATION

bool VerifyAnimationRename(AUIBaseActor* UIRootActor, ULGUIAnimation* Animation, FString NewAnimationName, FText& OutErrorMessage )
{
	if (NewAnimationName != SlugStringForValidName(NewAnimationName))
	{
		FString InvalidCharacters = INVALID_OBJECTNAME_CHARACTERS;
		FString CurrentInvalidCharacter;
		FString FoundInvalidCharacters;

		// Create a string with all invalid characters found to output with the error message.
		for (int32 StringIndex = 0; StringIndex < InvalidCharacters.Len(); ++StringIndex)
		{
			CurrentInvalidCharacter = InvalidCharacters.Mid(StringIndex, 1);

			if (NewAnimationName.Contains(CurrentInvalidCharacter))
			{
				FoundInvalidCharacters += CurrentInvalidCharacter;
			}
		}
		
		OutErrorMessage = FText::Format(LOCTEXT("NameContainsInvalidCharacters", "The object name may not contain the following characters:  {0}"), FText::FromString(FoundInvalidCharacters));
		return false;
	}

	if (UIRootActor && FindObject<ULGUIAnimation>(UIRootActor, *NewAnimationName, true ) )
	{
		OutErrorMessage = LOCTEXT( "NameInUseByAnimation", "An animation with this name already exists" );
		return false;
	}

	FName NewAnimationNameAsName( *NewAnimationName );
	if (UIRootActor)
	{
		TArray<UUIAnimationComp*> Components;
		UIRootActor->GetComponents<UUIAnimationComp>(Components);

		for (UActorComponent* ActorComponent : Components)
		{
			if (ActorComponent->GetFName() == NewAnimationNameAsName)
			{
				OutErrorMessage = LOCTEXT("NameInUseByWidget", "A widget with this name already exists");
				return false;
			}
		}
	}

	//FKismetNameValidator Validator( Blueprint );
	//EValidatorResult ValidationResult = Validator.IsValid( NewAnimationName );
	//if ( ValidationResult != EValidatorResult::Ok )
	//{
	//	FString ErrorString = FKismetNameValidator::GetErrorString( NewAnimationName, ValidationResult );
	//	OutErrorMessage = FText::FromString( ErrorString );
	//	return false;
	//}

	return true;
}


struct FLGUIAnimationListItem
{
	FLGUIAnimationListItem(ULGUIAnimation* InAnimation, bool bInRenameRequestPending = false, bool bInNewAnimation = false )
		: Animation(InAnimation)
		, bRenameRequestPending(bInRenameRequestPending)
		, bNewAnimation( bInNewAnimation )
	{}

	ULGUIAnimation* Animation;
	bool bRenameRequestPending;
	bool bNewAnimation;
};


class SLGUIAnimationListItem : public STableRow<TSharedPtr<FLGUIAnimationListItem> >
{
public:
	SLATE_BEGIN_ARGS( SLGUIAnimationListItem ){}
	SLATE_END_ARGS()

	void Construct( const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, FLGUIPrefabEditor* InMainUI, TSharedPtr<FLGUIAnimationListItem> InListItem )
	{
		ListItem = InListItem;
		MainUI = InMainUI;

		STableRow<TSharedPtr<FLGUIAnimationListItem>>::Construct(
			STableRow<TSharedPtr<FLGUIAnimationListItem>>::FArguments()
			.Padding( FMargin( 3.0f, 2.0f) )
			.Content()
			[
				SAssignNew(InlineTextBlock, SInlineEditableTextBlock)
				.Font(FCoreStyle::Get().GetFontStyle("NormalFont"))
				.Text(this, &SLGUIAnimationListItem::GetMovieSceneText)
				//.HighlightText(InArgs._HighlightText)
				.OnVerifyTextChanged(this, &SLGUIAnimationListItem::OnVerifyNameTextChanged)
				.OnTextCommitted(this, &SLGUIAnimationListItem::OnNameTextCommited)
				.IsSelected(this, &SLGUIAnimationListItem::IsSelectedExclusively)
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
		if( ListItem.IsValid() )
		{
			return ListItem.Pin()->Animation->GetDisplayName();
		}

		return FText::GetEmpty();
	}

	bool OnVerifyNameTextChanged(const FText& InText, FText& OutErrorMessage)
	{
		ULGUIAnimation* Animation = ListItem.Pin()->Animation;

		const FName NewName = *InText.ToString();

		if ( Animation->GetFName() != NewName )
		{
			return VerifyAnimationRename(MainUI->GetUIMainContainer(), Animation, NewName.ToString(), OutErrorMessage );
		}

		return true;
	}

	void OnNameTextCommited(const FText& InText, ETextCommit::Type CommitInfo)
	{
		ULGUIAnimation* LGUIAnimation = ListItem.Pin()->Animation;

		AUIBaseActor* UIActor = MainUI->GetUIMainContainer();

		// Name has already been checked in VerifyAnimationRename
		const FName NewFName = *InText.ToString();
		const FName OldFName = LGUIAnimation->GetFName();

		const bool bValidName = !OldFName.IsEqual(NewFName) && !InText.IsEmpty();
		const bool bCanRename = bValidName;

		const bool bNewAnimation = ListItem.Pin()->bNewAnimation;
		if (bCanRename)
		{
			const FString NewNameStr = NewFName.ToString();
			const FString OldNameStr = OldFName.ToString();

			FText TransactionName = bNewAnimation ? LOCTEXT("NewAnimation", "New Animation") : LOCTEXT("RenameAnimation", "Rename Animation");
			{
				const FScopedTransaction Transaction(TransactionName);
				LGUIAnimation->Modify();
				LGUIAnimation->GetMovieScene()->Modify();

				LGUIAnimation->SetDisplayLabel(InText.ToString());
				LGUIAnimation->Rename(*NewNameStr);
				LGUIAnimation->GetMovieScene()->Rename(*NewNameStr);

				if (bNewAnimation)
				{
					UIActor->Modify();
					UIActor->FindComponentByClass<UUIAnimationComp>()->Animations.Add(LGUIAnimation);
					ListItem.Pin()->bNewAnimation = false;
				}
			}
		}
		else if (bNewAnimation)
		{
			const FScopedTransaction Transaction(LOCTEXT("NewAnimation", "New Animation"));
			UIActor->Modify();
			UIActor->FindComponentByClass<UUIAnimationComp>()->Animations.Add(LGUIAnimation);
			ListItem.Pin()->bNewAnimation = false;
		}
	}
private:
	TWeakPtr<FLGUIAnimationListItem> ListItem;
	FLGUIPrefabEditor* MainUI;
	TSharedPtr<SInlineEditableTextBlock> InlineTextBlock;
};

void SLGUIAnimationListUI::Construct( const FArguments& InArgs, FLGUIPrefabEditor* InMaiUI )
{
	MainUI = InMaiUI;

	//InBlueprintEditor->GetOnWidgetBlueprintTransaction().AddSP( this, &SLGUIAnimationList::OnWidgetBlueprintTransaction );
	//InBlueprintEditor->OnEnterWidgetDesigner.AddSP(this, &SLGUIAnimationList::OnEnteringDesignerMode);

	SAssignNew(AnimationListView, SLGUIAnimationListView)
		.ItemHeight(20.0f)
		.SelectionMode(ESelectionMode::Single)
		.OnGenerateRow(this, &SLGUIAnimationListUI::OnGenerateWidgetForMovieScene)
		.OnItemScrolledIntoView(this, &SLGUIAnimationListUI::OnItemScrolledIntoView)
		.OnSelectionChanged(this, &SLGUIAnimationListUI::OnSelectionChanged)
		.OnContextMenuOpening(this, &SLGUIAnimationListUI::OnContextMenuOpening)
		.ListItemsSource(&Animations);

	ChildSlot
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
					SNew(SEditorHeaderButton)
					.OnClicked( this, &SLGUIAnimationListUI::OnNewAnimationClicked )
					.Text( LOCTEXT("NewAnimationButtonText", "Animation") )
				]
				+ SHorizontalBox::Slot()
				.Padding(2.0f, 0.0f)
				.VAlign( VAlign_Center )
				[
					SAssignNew(SearchBoxPtr, SSearchBox)
					.HintText(LOCTEXT("Search Animations", "Search Animations"))
					.OnTextChanged(this, &SLGUIAnimationListUI::OnSearchChanged)
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
	];

	CreateCommandList();
	NextUpdateUITime = FPlatformTime::Seconds() + 0.5f;
}

FReply SLGUIAnimationListUI::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	FReply Reply = FReply::Unhandled();
	if (CommandList->ProcessCommandBindings(InKeyEvent))
	{
		Reply = FReply::Handled();
	}

	return Reply;
}

void SLGUIAnimationListUI::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (NextUpdateUITime > 0 && FPlatformTime::Seconds() > NextUpdateUITime)
	{
		UpdateAnimationList();
		NextUpdateUITime = 0;
	}
}

void SLGUIAnimationListUI::UpdateAnimationList()
{
	Animations.Empty();

	if (MainUI && MainUI->GetAnimationComp())
	{
		const TArray<ULGUIAnimation*>& LGUIAnimations = MainUI->GetAnimationComp()->Animations;

		for (ULGUIAnimation* Animation : LGUIAnimations)
		{
			if (Animation)
			{
				//if (Animation->GetMovieScene()->GetNodeGroups() == nullptr)
				//{
				//	Animation->GetMovieScene()->set
				//}

				Animations.Add(MakeShareable(new FLGUIAnimationListItem(Animation)));
			}
		}
	}
	AnimationListView->RequestListRefresh();

	if (Animations.Num() > 0)
	{
		if (MainUI && MainUI->GetAnimationUI())
		{
			ULGUIAnimation* Animation = Animations[0]->Animation;
			MainUI->GetAnimationUI()->ChangeViewedAnimation(*Animation);
		}
	}
}

void SLGUIAnimationListUI::RefreshAnimationListUI()
{
	UpdateAnimationList();

	const ULGUIAnimation* ViewedAnim =  MainUI->GetAnimationUI()->RefreshCurrentAnimation();

	if (ViewedAnim)
	{
		const TSharedPtr<FLGUIAnimationListItem>* FoundListItemPtr = Animations.FindByPredicate([&](const TSharedPtr<FLGUIAnimationListItem>& ListItem) { return ListItem->Animation == ViewedAnim; });

		if (FoundListItemPtr != nullptr)
		{
			AnimationListView->SetSelection(*FoundListItemPtr);
		}
	}
}

void SLGUIAnimationListUI::OnItemScrolledIntoView(TSharedPtr<FLGUIAnimationListItem> InListItem, const TSharedPtr<ITableRow>& InWidget) const
{
	if (InListItem->bRenameRequestPending)
	{
		StaticCastSharedPtr<SLGUIAnimationListItem>(InWidget)->BeginRename();
		InListItem->bRenameRequestPending = false;
	}
}

// Create a NewAnimation
FReply SLGUIAnimationListUI::OnNewAnimationClicked()
{
	const float InTime = 0.f;
	const float OutTime = 5.0f;

	if (MainUI == nullptr || MainUI->GetAnimationComp() == nullptr)
	{
		if (MainUI && MainUI->GetUIMainContainer())
		{
			UActorComponent* NewInstanceComponent = MainUI->GetUIMainContainer()->AddComponentByClass(UUIAnimationComp::StaticClass(), false, FTransform::Identity, false);
			if (NewInstanceComponent)
			{
				MainUI->GetUIMainContainer()->AddInstanceComponent(NewInstanceComponent);
			}

			MainUI->SetPrefabModify();
		}

		if (MainUI == nullptr || MainUI->GetAnimationComp() == nullptr)
		{
			UE_LOG(LGUI, Error, TEXT("OnNewAnimationClicked failed, AnimationComp is nullptr!"));
			return FReply::Handled();
		}
	}

	UUIAnimationComp* UIAnimationComp = MainUI->GetAnimationComp();

	UIAnimationComp->Animations.RemoveAll([&](const ULGUIAnimation* Animation) 
		{ 
			return Animation == nullptr;
		}
	);

	SLGUIAnimationUI* AnimationUI = MainUI->GetAnimationUI();
	if (!AnimationUI)
	{
		return FReply::Handled();
	}

	FString BaseName = TEXT("NewAnimation");
	ULGUIAnimation* NewAnimation = NewObject<ULGUIAnimation>(UIAnimationComp, FName(), RF_Transactional);

	FString UniqueName = BaseName;
	int32 NameIndex = 1;
	FText Unused;
	while (VerifyAnimationRename(MainUI->GetUIMainContainer(), NewAnimation, UniqueName, Unused) == false)
	{
		UniqueName = FString::Printf(TEXT("%s_%i"), *BaseName, NameIndex);
		NameIndex++;
	}
	const FName NewFName = FName(*UniqueName);
	//NewAnimation->SetDisplayLabel(UniqueName);
	//NewAnimation->Rename(*UniqueName);

	NewAnimation->MovieScene = NewObject<ULGUIMovieScene>(NewAnimation, NewFName, RF_Transactional);

	// Default to 20 fps display rate (as was the previous default in USequencerSettings)
	NewAnimation->MovieScene->SetDisplayRate(FFrameRate(20, 1));

	const FFrameTime InFrame = InTime * NewAnimation->MovieScene->GetTickResolution();
	const FFrameTime OutFrame = OutTime * NewAnimation->MovieScene->GetTickResolution();
	NewAnimation->MovieScene->SetPlaybackRange(TRange<FFrameNumber>(InFrame.FrameNumber, OutFrame.FrameNumber + 1));
	NewAnimation->MovieScene->GetEditorData().WorkStart = InTime;
	NewAnimation->MovieScene->GetEditorData().WorkEnd = OutTime;

	bool bRequestRename = true;
	bool bNewAnimation = true;

	int32 NewIndex = Animations.Add(MakeShareable(new FLGUIAnimationListItem(NewAnimation, bRequestRename, bNewAnimation)));

	AnimationListView->RequestScrollIntoView(Animations[NewIndex]);

	return FReply::Handled();
}

void SLGUIAnimationListUI::OnSearchChanged(const FText& InSearchText)
{
	if (MainUI == nullptr || MainUI->GetAnimationComp() == nullptr)
	{
		UE_LOG(LGUI, Error, TEXT("OnSearchChanged failed, AnimationComp is nullptr!"));
		return;
	}

	const TArray<ULGUIAnimation*>& LGUIAnimations = MainUI->GetAnimationComp()->Animations;

	if (!InSearchText.IsEmpty())
	{
		struct Local
		{
			static void UpdateFilterStrings(ULGUIAnimation* InAnimation, OUT TArray< FString >& OutFilterStrings)
			{
				OutFilterStrings.Add(InAnimation->GetName());
			}
		};

		TTextFilter<ULGUIAnimation*> TextFilter(TTextFilter<ULGUIAnimation*>::FItemToStringArray::CreateStatic(&Local::UpdateFilterStrings));

		TextFilter.SetRawFilterText(InSearchText);
		SearchBoxPtr->SetError(TextFilter.GetFilterErrorText());

		Animations.Reset();

		for (ULGUIAnimation* Animation : LGUIAnimations)
		{
			if (TextFilter.PassesFilter(Animation))
			{
				Animations.Add(MakeShareable(new FLGUIAnimationListItem(Animation)));
			}
		}

		AnimationListView->RequestListRefresh();
	}
	else
	{
		SearchBoxPtr->SetError(FText::GetEmpty());

		// Just regenerate the whole list
		UpdateAnimationList();
	}
}

void SLGUIAnimationListUI::OnSelectionChanged(TSharedPtr<FLGUIAnimationListItem> InSelectedItem, ESelectInfo::Type SelectionInfo)
{
	ULGUIAnimation* LGUIAnimation = nullptr;
	if (InSelectedItem.IsValid())
	{
		LGUIAnimation = InSelectedItem->Animation;
	}
	else
	{
		LGUIAnimation = ULGUIAnimation::GetNullAnimation();
	}

	const ULGUIAnimation* CurrentWidgetAnimation = MainUI->GetAnimationUI()->RefreshCurrentAnimation();
	if (LGUIAnimation != CurrentWidgetAnimation)
	{
		MainUI->GetAnimationUI()->ChangeViewedAnimation(*LGUIAnimation);
	}
}

TSharedPtr<SWidget> SLGUIAnimationListUI::OnContextMenuOpening() const
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

TSharedRef<ITableRow> SLGUIAnimationListUI::OnGenerateWidgetForMovieScene(TSharedPtr<FLGUIAnimationListItem> InListItem, const TSharedRef< STableViewBase >& InOwnerTableView)
{
	return SNew(SLGUIAnimationListItem, InOwnerTableView, MainUI, InListItem);
}

void SLGUIAnimationListUI::CreateCommandList()
{
	CommandList = MakeShareable(new FUICommandList);

	CommandList->MapAction(
		FGenericCommands::Get().Duplicate,
		FExecuteAction::CreateSP(this, &SLGUIAnimationListUI::OnDuplicateAnimation),
		FCanExecuteAction::CreateSP(this, &SLGUIAnimationListUI::CanExecuteContextMenuAction)
	);

	CommandList->MapAction(
		FGenericCommands::Get().Delete,
		FExecuteAction::CreateSP(this, &SLGUIAnimationListUI::OnDeleteAnimation),
		FCanExecuteAction::CreateSP(this, &SLGUIAnimationListUI::CanExecuteContextMenuAction)
	);

	CommandList->MapAction(
		FGenericCommands::Get().Rename,
		FExecuteAction::CreateSP(this, &SLGUIAnimationListUI::OnRenameAnimation),
		FCanExecuteAction::CreateSP(this, &SLGUIAnimationListUI::CanExecuteContextMenuAction)
	);
}

bool SLGUIAnimationListUI::CanExecuteContextMenuAction() const
{
	return AnimationListView->GetNumItemsSelected() == 1;
}

void SLGUIAnimationListUI::OnDuplicateAnimation()
{
	TArray< TSharedPtr<FLGUIAnimationListItem> > SelectedAnimations = AnimationListView->GetSelectedItems();
	check(SelectedAnimations.Num() == 1);

	TSharedPtr<FLGUIAnimationListItem> SelectedAnimation = SelectedAnimations[0];

	AUIBaseActor* MainContainerActor = MainUI->GetUIMainContainer();

	ULGUIAnimation* NewAnimation =
		DuplicateObject<ULGUIAnimation>
		(
			SelectedAnimation->Animation,
			MainContainerActor,
			MakeUniqueObjectName(MainContainerActor->FindComponentByClass<UUIAnimationComp>(), ULGUIAnimation::StaticClass(), SelectedAnimation->Animation->GetFName())
			);

	NewAnimation->MovieScene->Rename(*NewAnimation->GetName(), nullptr, REN_DontCreateRedirectors | REN_ForceNoResetLoaders);
	NewAnimation->SetDisplayLabel(NewAnimation->GetName());

	bool bRenameRequestPending = true;
	bool bNewAnimation = true;
	int32 NewIndex = Animations.Add(MakeShareable(new FLGUIAnimationListItem(NewAnimation, bRenameRequestPending, bNewAnimation)));

	AnimationListView->RequestScrollIntoView(Animations[NewIndex]);
}

void SLGUIAnimationListUI::OnDeleteAnimation()
{
	TArray< TSharedPtr<FLGUIAnimationListItem> > SelectedAnimations = AnimationListView->GetSelectedItems();
	check(SelectedAnimations.Num() == 1);

	TSharedPtr<FLGUIAnimationListItem> SelectedAnimation = SelectedAnimations[0];

	if (MainUI->GetAnimationComp() == nullptr)
	{
		UE_LOG(LGUI, Error, TEXT("OnDeleteAnimation failed, AnimationComp not exist!"));
		return;
	}

	TArray<ULGUIAnimation*>& WidgetAnimations =  MainUI->GetAnimationComp()->Animations;
	{
		const FScopedTransaction Transaction(LOCTEXT("DeleteAnimationTransaction", "Delete Animation"));
		
		MainUI->SetPrefabModify();

		// Rename the animation and move it to the transient package to avoid collisions.
		SelectedAnimation->Animation->Rename(NULL, GetTransientPackage());
		WidgetAnimations.Remove(SelectedAnimation->Animation);

		UpdateAnimationList();
	}

	MainUI->GetAnimationUI()->ChangeViewedAnimation(*ULGUIAnimation::GetNullAnimation());
}

void SLGUIAnimationListUI::OnRenameAnimation()
{
	TArray< TSharedPtr<FLGUIAnimationListItem> > SelectedAnimations = AnimationListView->GetSelectedItems();
	check(SelectedAnimations.Num() == 1);

	TSharedPtr<FLGUIAnimationListItem> SelectedAnimation = SelectedAnimations[0];
	SelectedAnimation->bRenameRequestPending = true;

	AnimationListView->RequestScrollIntoView(SelectedAnimation);
}

#undef LOCTEXT_NAMESPACE 


PRAGMA_ENABLE_OPTIMIZATION
