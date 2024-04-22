// Copyright 2019-Present LexLiu. All Rights Reserved.

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
#include "PrefabAnimation/LGUIPrefabSequenceComponent.h"
#include "Selection.h"
#include "Core/Actor/UIBaseActor.h"
#include "LevelEditor.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabSequenceEditorWidget"

DECLARE_DELEGATE_OneParam(FPrefabAnimationOnComponentSelected, TSharedPtr<FSCSEditorTreeNode>);
DECLARE_DELEGATE_RetVal_OneParam(bool, FPrefabAnimationIsComponentValid, UActorComponent*);


#include "Core/ActorComponent/UIBatchMeshRenderable.h"
#include "PrefabAnimation/MovieSceneLGUIMaterialTrack.h"

class SLGUIPrefabSequenceEditorWidgetImpl : public SCompoundWidget, public FEditorUndoClient
{
public:

	bool bUpdatingSequencerSelection = false;

	SLATE_BEGIN_ARGS(SLGUIPrefabSequenceEditorWidgetImpl){}
	SLATE_END_ARGS();

	void Close()
	{
		if (Sequencer.IsValid())
		{
			Sequencer->SetShowCurveEditor(false);
			FLevelEditorSequencerIntegration::Get().RemoveSequencer(Sequencer.ToSharedRef());
			Sequencer->Close();
			Sequencer = nullptr;
		}

		GEditor->UnregisterForUndo(this);
	}

	~SLGUIPrefabSequenceEditorWidgetImpl()
	{
		Close();

		// Un-Register sequencer menu extenders.
		ISequencerModule& SequencerModule = FModuleManager::Get().LoadModuleChecked<ISequencerModule>("Sequencer");
		SequencerModule.GetAddTrackMenuExtensibilityManager()->GetExtenderDelegates().RemoveAll([this](const FAssetEditorExtender& Extender)
			{
				return SequencerAddTrackExtenderHandle == Extender.GetHandle();
			});
	}
	
	TSharedRef<SDockTab> SpawnCurveEditorTab(const FSpawnTabArgs&)
	{
		const FSlateIcon SequencerGraphIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "GenericCurveEditor.TabIcon");
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
			.TextStyle(FAppStyle::Get(), "UMGEditor.NoAnimationFont")
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
		ToolkitHost = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor").GetFirstLevelEditor();

		// Register sequencer menu extenders.
		ISequencerModule& SequencerModule = FModuleManager::Get().LoadModuleChecked<ISequencerModule>("Sequencer");
		{
			int32 NewIndex = SequencerModule.GetAddTrackMenuExtensibilityManager()->GetExtenderDelegates().Add(
				FAssetEditorExtender::CreateRaw(this, &SLGUIPrefabSequenceEditorWidgetImpl::GetAddTrackSequencerExtender));
			SequencerAddTrackExtenderHandle = SequencerModule.GetAddTrackMenuExtensibilityManager()->GetExtenderDelegates()[NewIndex].GetHandle();
		}
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
			auto Component = LocalLGUIPrefabSequence->GetTypedOuter<ULGUIPrefabSequenceComponent>();
			return Component->GetOwner();
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
		if (!NewSequence)
		{
			if (Sequencer.IsValid())
			{
				Sequencer->SetShowCurveEditor(false);
				FLevelEditorSequencerIntegration::Get().RemoveSequencer(Sequencer.ToSharedRef());
				Sequencer->Close();
				Sequencer = nullptr;
			}

			Content->SetContent(SNew(STextBlock).Text(LOCTEXT("NothingSelected", "Select a sequence")));
			return;
		}

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
			SequencerInitParams.ViewParams.AddMenuExtender = AddMenuExtender;
			SequencerInitParams.ViewParams.UniqueName = "EmbeddedLGUIPrefabSequenceEditor";
			SequencerInitParams.ViewParams.ScrubberStyle = ESequencerScrubberStyle::FrameBlock;
			SequencerInitParams.ViewParams.OnReceivedFocus.BindRaw(this, &SLGUIPrefabSequenceEditorWidgetImpl::OnSequencerReceivedFocus);
			SequencerInitParams.bEditWithinLevelEditor = false;
			SequencerInitParams.ToolkitHost = ToolkitHost;
			SequencerInitParams.HostCapabilities.bSupportsCurveEditor = true;
		}

		Sequencer = FModuleManager::LoadModuleChecked<ISequencerModule>("Sequencer").CreateSequencer(SequencerInitParams);
		Content->SetContent(Sequencer->GetSequencerWidget());
		Sequencer->GetSelectionChangedObjectGuids().AddSP(this, &SLGUIPrefabSequenceEditorWidgetImpl::SyncSelectedWidgetsWithSequencerSelection);
		Sequencer->OnMovieSceneBindingsChanged().AddLambda([=]() {
			if (!WeakSequence.IsValid())return;
			if (!IsValid(WeakSequence->GetMovieScene()))return;
			auto& Bindings = WeakSequence->GetMovieScene()->GetBindings();
			for (auto& BindingItem : Bindings)
			{
				auto ObjectArray = Sequencer->FindObjectsInCurrentSequence(BindingItem.GetObjectGuid());
				if (ObjectArray.Num() > 0)
				{
					if (auto Actor = Cast<AActor>(ObjectArray[0]))
					{
						WeakSequence->GetMovieScene()->SetObjectDisplayName(BindingItem.GetObjectGuid(), FText::FromString(Actor->GetActorLabel()));
					}
					else if (auto Comp = Cast<UActorComponent>(ObjectArray[0]))
					{
						if (auto CompActor = Comp->GetOwner())
						{
							WeakSequence->GetMovieScene()->SetObjectDisplayName(BindingItem.GetObjectGuid(), FText::FromString(CompActor->GetActorLabel()));
						}
					}
				}
			}
			});

		FLevelEditorSequencerIntegrationOptions Options;
		Options.bRequiresLevelEvents = false;
		Options.bRequiresActorEvents = true;
		Options.bForceRefreshDetails = false;

		FLevelEditorSequencerIntegration::Get().AddSequencer(Sequencer.ToSharedRef(), Options);
	}

	// sequence select widget handler
	void SyncSelectedWidgetsWithSequencerSelection(TArray<FGuid> ObjectGuids)
	{
		if (Sequencer == nullptr || bUpdatingSequencerSelection)
		{
			return;
		}

		//UE_LOG(LGUI, Log, TEXT("SyncSelectedWidgetsWithSequencerSelection, ObjectGuids.Num()=%d"), ObjectGuids.Num());

		TGuardValue<bool> Guard(bUpdatingSequencerSelection, true);

		UMovieSceneSequence* AnimationSequence = Sequencer->GetFocusedMovieSceneSequence();
		UObject* BindingContext = WeakSequence.Get();
		TSet<AUIBaseActor*> SequencerSelectedWidgets;
		for (FGuid Guid : ObjectGuids)
		{
			TArray<UObject*, TInlineAllocator<1>> BoundObjects = AnimationSequence->LocateBoundObjects(Guid, BindingContext);
			if (BoundObjects.Num() == 0)
			{
				continue;
			}
			else
			{
				AUIBaseActor* BoundWidget = Cast<AUIBaseActor>(BoundObjects[0]);
				if (BoundWidget)
				{
					SequencerSelectedWidgets.Add(BoundWidget);
				}
			}
		}

		if (SequencerSelectedWidgets.Num() != 0)
		{
			AUIBaseActor* SelectedActor = *SequencerSelectedWidgets.begin();

			// Sync Selection
			GEditor->SelectNone(false, true, false);
			GEditor->SelectActor(SelectedActor, true, true, true);
		}
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
	TSharedRef<FExtender> GetAddTrackSequencerExtender(const TSharedRef<FUICommandList> CommandList, const TArray<UObject*> ContextSensitiveObjects)
	{
		TSharedRef<FExtender> AddTrackMenuExtender(new FExtender());
		AddTrackMenuExtender->AddMenuExtension(
			SequencerMenuExtensionPoints::AddTrackMenu_PropertiesSection,
			EExtensionHook::Before,
			CommandList,
			FMenuExtensionDelegate::CreateRaw(this, &SLGUIPrefabSequenceEditorWidgetImpl::ExtendSequencerAddTrackMenu, ContextSensitiveObjects));
		return AddTrackMenuExtender;
	}

	void ExtendSequencerAddTrackMenu(FMenuBuilder& AddTrackMenuBuilder, const TArray<UObject*> ContextObjects)
	{
		if (ContextObjects.Num() == 1)
		{
			auto Renderable = Cast<UUIBatchMeshRenderable>(ContextObjects[0]);

			if (Renderable != nullptr)
			{
				if (Renderable != nullptr && Renderable->GetCustomUIMaterial() != nullptr)
				{
					auto MaterialProperty = Renderable->GetClass()->FindPropertyByName(UUIBatchMeshRenderable::GetCustomUIMaterialPropertyName());
					AddTrackMenuBuilder.BeginSection("Materials", LOCTEXT("MaterialsSection", "Materials"));
					{
						FText DisplayNameText = MaterialProperty->GetDisplayNameText();
						FUIAction AddMaterialAction(FExecuteAction::CreateRaw(this, &SLGUIPrefabSequenceEditorWidgetImpl::AddMaterialTrack, Renderable, MaterialProperty, DisplayNameText));
						FText AddMaterialLabel = DisplayNameText;
						FText AddMaterialToolTip = FText::Format(LOCTEXT("MaterialToolTipFormat", "Add a material track for the {0} property."), DisplayNameText);
						AddTrackMenuBuilder.AddMenuEntry(AddMaterialLabel, AddMaterialToolTip, FSlateIcon(), AddMaterialAction);
					}
					AddTrackMenuBuilder.EndSection();
				}
			}
		}
	}

	void AddMaterialTrack(UUIBatchMeshRenderable* Renderable, FProperty* MaterialProperty, FText MaterialPropertyDisplayName)
	{
		FGuid WidgetHandle = Sequencer->GetHandleToObject(Renderable);
		if (WidgetHandle.IsValid())
		{
			UMovieScene* MovieScene = Sequencer->GetFocusedMovieSceneSequence()->GetMovieScene();

			if (MovieScene->IsReadOnly())
			{
				return;
			}

			FName MaterialPropertyNamePath = MaterialProperty->GetFName();
			if (MovieScene->FindTrack(UMovieSceneLGUIMaterialTrack::StaticClass(), WidgetHandle, MaterialPropertyNamePath) == nullptr)
			{
				const FScopedTransaction Transaction(LOCTEXT("AddWidgetMaterialTrack", "Add widget material track"));

				MovieScene->Modify();

				auto NewTrack = Cast<UMovieSceneLGUIMaterialTrack>(MovieScene->AddTrack(UMovieSceneLGUIMaterialTrack::StaticClass(), WidgetHandle));
				NewTrack->Modify();
				NewTrack->SetPropertyName(MaterialPropertyNamePath);
				NewTrack->SetDisplayName(FText::Format(LOCTEXT("TrackDisplayNameFormat", "{0}"), MaterialPropertyDisplayName));

				Sequencer->NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::MovieSceneStructureItemAdded);
			}
		}
	}
private:
	TWeakObjectPtr<ULGUIPrefabSequence> WeakSequence;

	TSharedPtr<SBox> Content;
	TSharedPtr<ISequencer> Sequencer;

	FDelegateHandle OnSequenceChangedHandle;

	TSharedPtr<STextBlock> NoAnimationTextBlock;

	/** The asset editor that created this Sequencer if any */
	TSharedPtr<IToolkitHost> ToolkitHost;

	FDelegateHandle SequencerAddTrackExtenderHandle;
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
