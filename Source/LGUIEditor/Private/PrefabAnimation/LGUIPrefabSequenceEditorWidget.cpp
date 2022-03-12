// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "LGUIPrefabSequenceEditorWidget.h"

#include "PrefabAnimation/LGUIPrefabSequence.h"
#include "ISequencer.h"
#include "ISequencerModule.h"
#include "LevelEditorSequencerIntegration.h"
#include "SSCSEditor.h"
#include "Styling/SlateIconFinder.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EditorStyleSet.h"
#include "EditorUndoClient.h"
#include "Widgets/Images/SImage.h"
#include "Editor.h"
#include "ScopedTransaction.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Application/SlateApplication.h"
#include "EngineUtils.h"
#include "Utils/LGUIUtils.h"
#include "LGUIEditorTools.h"
#include "PrefabSystem/LGUIPrefabHelperObject.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabSequenceEditorWidget"

DECLARE_DELEGATE_OneParam(FPrefabAnimationOnComponentSelected, TSharedPtr<FSCSEditorTreeNode>);
DECLARE_DELEGATE_RetVal_OneParam(bool, FPrefabAnimationIsComponentValid, UActorComponent*);


class SLGUIPrefabSequenceEditorWidgetImpl : public SCompoundWidget, public FEditorUndoClient
{
public:

	SLATE_BEGIN_ARGS(SLGUIPrefabSequenceEditorWidgetImpl){}
	SLATE_END_ARGS();

	void Close()
	{
		if (Sequencer.IsValid())
		{
			FLevelEditorSequencerIntegration::Get().RemoveSequencer(Sequencer.ToSharedRef());
			Sequencer->Close();
			Sequencer = nullptr;
		}

		GEditor->UnregisterForUndo(this);
	}

	~SLGUIPrefabSequenceEditorWidgetImpl()
	{
		Close();
	}
	
	TSharedRef<SDockTab> SpawnCurveEditorTab(const FSpawnTabArgs&)
	{
		const FSlateIcon SequencerGraphIcon = FSlateIcon(FEditorStyle::GetStyleSetName(), "GenericCurveEditor.TabIcon");
		auto Tab = SNew(SDockTab)
			.Label(NSLOCTEXT("Sequencer", "SequencerMainGraphEditorTitle", "Sequencer Curves"))
			[
				SNullWidget::NullWidget
			];
		Tab->SetTabIcon(SequencerGraphIcon.GetIcon());
		return Tab;
	}

	void Construct(const FArguments&, TWeakPtr<FBlueprintEditor> InBlueprintEditor)
	{
		NoAnimationTextBlock =
			SNew(STextBlock)
			.TextStyle(FEditorStyle::Get(), "UMGEditor.NoAnimationFont")
			.Text(LOCTEXT("NoAnimationSelected", "No Animation Selected"));

		ChildSlot
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SAssignNew(Content, SBox)
				.MinDesiredHeight(200)
			]
			+SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				NoAnimationTextBlock.ToSharedRef()
			]
		];

		GEditor->RegisterForUndo(this);
	}


	virtual void PostUndo(bool bSuccess) override
	{
		if (!GetLGUIPrefabSequence())
		{
			Close();
		}
	}

	FText GetDisplayLabel() const
	{
		ULGUIPrefabSequence* Sequence = WeakSequence.Get();
		return Sequence ? Sequence->GetDisplayName() : LOCTEXT("DefaultSequencerLabel", "Sequencer");
	}

	ULGUIPrefabSequence* GetLGUIPrefabSequence() const
	{
		return WeakSequence.Get();
	}

	UObject* GetPlaybackContext() const
	{
		ULGUIPrefabSequence* LocalLGUIPrefabSequence = GetLGUIPrefabSequence();
		if (LocalLGUIPrefabSequence)
		{
			if (AActor* Actor = LocalLGUIPrefabSequence->GetTypedOuter<AActor>())
			{
				return Actor;
			}
			else if (UBlueprintGeneratedClass* GeneratedClass = LocalLGUIPrefabSequence->GetTypedOuter<UBlueprintGeneratedClass>())
			{
				if (GeneratedClass->SimpleConstructionScript)
				{
					return GeneratedClass->SimpleConstructionScript->GetComponentEditorActorInstance();
				}
			}
		}
		
		return nullptr;
	}

	TArray<UObject*> GetEventContexts() const
	{
		TArray<UObject*> Contexts;
		if (auto* Context = GetPlaybackContext())
		{
			Contexts.Add(Context);
		}
		return Contexts;
	}

	auto GetNullSequence()
	{
		static ULGUIPrefabSequence* NullSequence = nullptr;
		if (!NullSequence)
		{
			NullSequence = NewObject<ULGUIPrefabSequence>(GetTransientPackage(), NAME_None);
			NullSequence->AddToRoot();
			NullSequence->GetMovieScene()->SetDisplayRate(FFrameRate(30, 1));
		}
		return NullSequence;
	}

	void SetLGUIPrefabSequence(ULGUIPrefabSequence* NewSequence)
	{
		if (ULGUIPrefabSequence* OldSequence = WeakSequence.Get())
		{
			if (OnSequenceChangedHandle.IsValid())
			{
				OldSequence->OnSignatureChanged().Remove(OnSequenceChangedHandle);
			}
		}

		WeakSequence = NewSequence;

		if (NewSequence)
		{
			OnSequenceChangedHandle = NewSequence->OnSignatureChanged().AddSP(this, &SLGUIPrefabSequenceEditorWidgetImpl::OnSequenceChanged);
		}

		if (NewSequence == nullptr)
		{
			Content->SetEnabled(false);
			NoAnimationTextBlock->SetVisibility(EVisibility::Visible);
		}
		else
		{
			Content->SetEnabled(true);
			NoAnimationTextBlock->SetVisibility(EVisibility::Collapsed);
		}

		// If we already have a sequencer open, just assign the sequence
		if (Sequencer.IsValid() && NewSequence)
		{
			if (Sequencer->GetRootMovieSceneSequence() != NewSequence)
			{
				Sequencer->ResetToNewRootSequence(*NewSequence);
			}
			return;
		}

		// If we're setting the sequence to none, destroy sequencer
		//if (!NewSequence)
		//{
		//	if (Sequencer.IsValid())
		//	{
		//		FLevelEditorSequencerIntegration::Get().RemoveSequencer(Sequencer.ToSharedRef());
		//		Sequencer->Close();
		//		Sequencer = nullptr;
		//	}

		//	Content->SetContent(SNew(STextBlock).Text(LOCTEXT("NothingSelected", "Select a sequence")));
		//	return;
		//}

		// We need to initialize a new sequencer instance
		FSequencerInitParams SequencerInitParams;
		{
			TWeakObjectPtr<ULGUIPrefabSequence> LocalWeakSequence = NewSequence;

			SequencerInitParams.RootSequence = NewSequence ? NewSequence : GetNullSequence();
			SequencerInitParams.EventContexts = TAttribute<TArray<UObject*>>(this, &SLGUIPrefabSequenceEditorWidgetImpl::GetEventContexts);
			SequencerInitParams.PlaybackContext = TAttribute<UObject*>(this, &SLGUIPrefabSequenceEditorWidgetImpl::GetPlaybackContext);

			TSharedRef<FExtender> AddMenuExtender = MakeShareable(new FExtender);

			AddMenuExtender->AddMenuExtension("AddTracks", EExtensionHook::Before, nullptr,
				FMenuExtensionDelegate::CreateRaw(this, &SLGUIPrefabSequenceEditorWidgetImpl::AddPossessMenuExtensions)
			);

			SequencerInitParams.ViewParams.bReadOnly = !NewSequence->IsEditable();
			SequencerInitParams.bEditWithinLevelEditor = false;
			SequencerInitParams.ViewParams.AddMenuExtender = AddMenuExtender;
			SequencerInitParams.ViewParams.UniqueName = "EmbeddedLGUIPrefabSequenceEditor";
			SequencerInitParams.ViewParams.ScrubberStyle = ESequencerScrubberStyle::FrameBlock;
			SequencerInitParams.ViewParams.OnReceivedFocus.BindRaw(this, &SLGUIPrefabSequenceEditorWidgetImpl::OnSequencerReceivedFocus);
		}

		Sequencer = FModuleManager::LoadModuleChecked<ISequencerModule>("Sequencer").CreateSequencer(SequencerInitParams);
		Content->SetContent(Sequencer->GetSequencerWidget());

		FLevelEditorSequencerIntegrationOptions Options;
		Options.bRequiresLevelEvents = true;
		Options.bRequiresActorEvents = false;
		Options.bForceRefreshDetails = false;

		FLevelEditorSequencerIntegration::Get().AddSequencer(Sequencer.ToSharedRef(), Options);
	}

	void OnSequencerReceivedFocus()
	{
		if (Sequencer.IsValid())
		{
			FLevelEditorSequencerIntegration::Get().OnSequencerReceivedFocus(Sequencer.ToSharedRef());
		}
	}

	void OnSelectionUpdated(TSharedPtr<FSCSEditorTreeNode> SelectedNode)
	{
		if (SelectedNode->GetNodeType() != FSCSEditorTreeNode::ComponentNode)
		{
			return;
		}

		UActorComponent* EditingComponent = nullptr;

		if (AActor* Actor = GetSelectedActor())
		{
			EditingComponent = SelectedNode->FindComponentInstanceInActor(Actor);
		}

		if (EditingComponent)
		{
			const FScopedTransaction Transaction(LOCTEXT("AddComponentToSequencer", "Add component to Sequencer"));
			Sequencer->GetHandleToObject(EditingComponent, true);
		}

		FSlateApplication::Get().DismissAllMenus();
	}

	void AddPossessMenuExtensions(FMenuBuilder& MenuBuilder)
	{
		if (!WeakSequence.IsValid())return;

		Sequencer->State.ClearObjectCaches(*Sequencer);
		TSet<UObject*> AllBoundObjects;
		UMovieScene* MovieScene = Sequencer->GetFocusedMovieSceneSequence()->GetMovieScene();
		for (int32 Index = 0; Index < MovieScene->GetPossessableCount(); ++Index)
		{
			FMovieScenePossessable& Possessable = MovieScene->GetPossessable(Index);
			for (TWeakObjectPtr<> WeakObject : Sequencer->FindBoundObjects(Possessable.GetGuid(), Sequencer->GetFocusedTemplateID()))
			{
				if (UObject* Object = WeakObject.Get())
				{
					AllBoundObjects.Add(Object);
				}
			}
		}

		//actor menu
		{
			TArray<AActor*> ValidActorArray;
			if (auto Actor = WeakSequence->GetTypedOuter<AActor>())
			{
				TArray<AActor*> AllChildrenActors;
				LGUIUtils::CollectChildrenActors(Actor, AllChildrenActors);
				for (auto ActorItem : AllChildrenActors)
				{
					if (!AllBoundObjects.Contains(ActorItem))
					{
						ValidActorArray.Add(ActorItem);
					}
				}
			}

			MenuBuilder.AddSubMenu(
				LOCTEXT("AddActor_Label", "Actor"),
				LOCTEXT("AddActor_Tooltip", "Add a binding to one of actor and allow it to be animated by Sequencer"),
				FNewMenuDelegate::CreateRaw(this, &SLGUIPrefabSequenceEditorWidgetImpl::AddPossessActorMenuExtensions, ValidActorArray),
				false, FSlateIcon()
						);
		}
		//component menu
		if (auto Actor = GetSelectedActor())
		{
			TArray<UActorComponent*> AllCompArray;
			Actor->GetComponents(AllCompArray);
			TArray<UActorComponent*> ValidCompArray;
			for (auto Comp : AllCompArray)
			{
				if (Comp->HasAnyFlags(EObjectFlags::RF_Transient))continue;
				if (!AllBoundObjects.Contains(Comp))//already bounded
				{
					ValidCompArray.Add(Comp);
				}
			}

			MenuBuilder.AddSubMenu(
				LOCTEXT("AddComponent_Label", "Component"),
				LOCTEXT("AddComponent_ToolTip", "Add a binding to one of this actor's components and allow it to be animated by Sequencer"),
				FNewMenuDelegate::CreateRaw(this, &SLGUIPrefabSequenceEditorWidgetImpl::AddPossessComponentMenuExtensions, ValidCompArray),
				false,
				FSlateIcon()
			);
		}
	}
	void AddPossessActorMenuExtensions(FMenuBuilder& MenuBuilder, TArray<AActor*> InActorArray)
	{
		for (auto Actor : InActorArray)
		{
			if (IsValid(Actor))
			{
				MenuBuilder.AddMenuEntry(
					FText::Format(LOCTEXT("ActorLabelFormat", "{0} ({1})"), FText::FromString(Actor->GetActorLabel()), FText::FromString(Actor->GetClass()->GetName())),
					FText::FromString(Actor->GetName()), FSlateIcon(),
					FUIAction(FExecuteAction::CreateLambda([=]() {
						const FScopedTransaction Transaction(LOCTEXT("AddActorToSequencer", "Add actor to Sequencer"));
						Sequencer->GetHandleToObject(Actor, true);
						}))
				);
			}
		}
	}
	void AddPossessComponentMenuExtensions(FMenuBuilder& MenuBuilder, TArray<UActorComponent*> InCompArray)
	{
		for (auto Comp : InCompArray)
		{
			if (IsValid(Comp))
			{
				MenuBuilder.AddMenuEntry(
					FText::Format(LOCTEXT("ComponentLabelFormat", "{0} ({1})"), FText::FromString(Comp->GetName()), FText::FromString(Comp->GetClass()->GetName())),
					FText::FromString(Comp->GetName()), FSlateIcon(),
					FUIAction(FExecuteAction::CreateLambda([=]() {
						const FScopedTransaction Transaction(LOCTEXT("AddComponentToSequencer", "Add component to Sequencer"));
						Sequencer->GetHandleToObject(Comp, true);
						}))
				);
			}
		}
	}

	AActor* GetSelectedActor()const
	{
		TArray<AActor*, TInlineAllocator<1>> SelectedActorArray;
		if (WeakSequence.IsValid())
		{
			TArray<FGuid> SelectedObjects;
			Sequencer->GetSelectedObjects(SelectedObjects);
			for (auto GuidItem : SelectedObjects)
			{
				TArray<UObject*, TInlineAllocator<1>> BoundObjects;
				WeakSequence->LocateBoundObjects(GuidItem, nullptr, BoundObjects);
				for (auto Obj : BoundObjects)
				{
					if (auto Actor = Cast<AActor>(Obj))
					{
						SelectedActorArray.Add(Actor);
					}
				}
			}
		}
		return SelectedActorArray.Num() == 1 ? SelectedActorArray[0] : nullptr;
	}

	void OnSequenceChanged()
	{
		ULGUIPrefabSequence* LGUIPrefabSequence = WeakSequence.Get();
		auto Actor = WeakSequence.IsValid() ? WeakSequence->GetTypedOuter<AActor>() : nullptr;
		if (Actor)
		{
			if (auto PrefabHelperObject = LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(Actor))
			{
				PrefabHelperObject->SetAnythingDirty();
			}
		}
	}

private:
	TWeakObjectPtr<ULGUIPrefabSequence> WeakSequence;

	TSharedPtr<SBox> Content;
	TSharedPtr<ISequencer> Sequencer;

	FDelegateHandle OnSequenceChangedHandle;

	TSharedPtr<STextBlock> NoAnimationTextBlock;
};

void SLGUIPrefabSequenceEditorWidget::Construct(const FArguments&, TWeakPtr<FBlueprintEditor> InBlueprintEditor)
{
	ChildSlot
	[
		SAssignNew(Impl, SLGUIPrefabSequenceEditorWidgetImpl, InBlueprintEditor)
	];
}

FText SLGUIPrefabSequenceEditorWidget::GetDisplayLabel() const
{
	return Impl.Pin()->GetDisplayLabel();
}

void SLGUIPrefabSequenceEditorWidget::AssignSequence(ULGUIPrefabSequence* NewLGUIPrefabSequence)
{
	Impl.Pin()->SetLGUIPrefabSequence(NewLGUIPrefabSequence);
}

ULGUIPrefabSequence* SLGUIPrefabSequenceEditorWidget::GetSequence() const
{
	return Impl.Pin()->GetLGUIPrefabSequence();
}

#undef LOCTEXT_NAMESPACE
