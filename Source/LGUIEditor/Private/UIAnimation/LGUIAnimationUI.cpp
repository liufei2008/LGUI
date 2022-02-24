// Copyright Epic Games, Inc. All Rights Reserved.

#include "LGUIAnimationUI.h"
#include "MovieSceneBinding.h"
#include "MovieSceneFolder.h"
#include "MovieScene.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Engine/SimpleConstructionScript.h"

#include "Tracks/MovieScenePropertyTrack.h"
#include "ISequencerModule.h"
#include "SequencerSettings.h"
#include "ObjectEditorUtils.h"

#include "Animation/MovieSceneWidgetMaterialTrack.h"
#include "Animation/WidgetMaterialTrackUtilities.h"
#include "ScopedTransaction.h"
#include "Widgets/SWidget.h"
#include "LGUI.h"
//#include "Widgets/Docking/SDockTabStack.h"
#include "EngineUtils.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Widgets/Text/STextBlock.h"
#include "Core/Actor/UIBaseActor.h"
#include "Core/ActorComponent/UIAnimationComp.h"
#include "PrefabEditor/LGUIPrefabEditor.h"
#include "Editor/UnrealEdEngine.h"
#include "UnrealEdGlobals.h"

//extern UUnrealEdEngine* GUnrealEd;

PRAGMA_DISABLE_OPTIMIZATION

class FObjectAndDisplayName
{
public:
	FObjectAndDisplayName(FText InDisplayName, UObject* InObject)
	{
		DisplayName = InDisplayName;
		Object = InObject;
	}

	bool operator<(FObjectAndDisplayName const& Other) const
	{
		return DisplayName.CompareTo(Other.DisplayName) < 0;
	}

	FText DisplayName;
	UObject* Object;

};

class FFakeToolkitHost : public IToolkitHost
{

public:

	FFakeToolkitHost()
	{
		 SAssignNew(ParentWiget, STextBlock);
		 //SAssignNew(DockTabStack, SDockTabStack);
	}
	virtual ~FFakeToolkitHost()
	{
	}

	/** Gets a widget that can be used to parent a modal window or pop-up to.  You shouldn't be using this widget for
	    anything other than parenting, as the type of widget and behavior/lifespan is completely up to the host. */
	virtual TSharedRef< class SWidget > GetParentWidget() override
	{
		return ParentWiget.ToSharedRef();
	}

	/** Brings this toolkit host's window (and tab, if it has one), to the front */
	virtual void BringToFront()
	{
	}

	/** Gets a tab stack to place a new tab for the specified toolkit area */
	virtual TSharedRef< class SDockTabStack > GetTabSpot(const EToolkitTabSpot::Type TabSpot)
	{
		return TSharedPtr<SDockTabStack>().ToSharedRef();
	}

	/** Access the toolkit host's tab manager */
	virtual TSharedPtr< class FTabManager > GetTabManager() const
	{
		return FGlobalTabmanager::Get();
	}

	/** Called when a toolkit is opened within this host */
	virtual void OnToolkitHostingStarted(const TSharedRef< class IToolkit >& Toolkit)
	{

	}

	/** Called when a toolkit is no longer being hosted within this host */
	virtual void OnToolkitHostingFinished(const TSharedRef< class IToolkit >& Toolkit)
	{
	}

	/** @return For world-centric toolkit hosts, gets the UWorld associated with this host */
	virtual class UWorld* GetWorld() const
	{
		return nullptr;
	}

	//todo:zackma
	virtual FEditorModeTools& GetEditorModeManager() const
	{
		return *EditorModeManager.Get();
	}

	/** Returns the common actions implementation for this toolkit host */
	virtual UTypedElementCommonActions* GetCommonActions() const
	{
		return nullptr;
	}

	/** Gets a multicast delegate which is executed whenever the toolkit host's active viewport changes. */
	virtual FOnActiveViewportChanged& OnActiveViewportChanged()
	{
		return OnActiveViewportChangedDelegate;
	}

protected:
	 TSharedPtr< class SWidget > ParentWiget;
	 //TSharedPtr< class SDockTabStack > DockTabStack;
private:
	/** The editor mode manager */
	TSharedPtr<FEditorModeTools> EditorModeManager;

	/** A delegate which is called any time the LevelEditor's active viewport changes. */
	FOnActiveViewportChanged OnActiveViewportChangedDelegate;
};


#define LOCTEXT_NAMESPACE "LGUIEditor"

void SLGUIAnimationUI::Construct(const FArguments& Args, FLGUIPrefabEditor* InMainUI)
{
	MainUI = InMainUI;

	InitToolKit();
	TSharedRef<SWidget> Widget = CreateSequencerWidget();

	ChildSlot
		[
			Widget
		];

	FCoreDelegates::OnActorLabelChanged.AddSP(this, &SLGUIAnimationUI::OnActorLabelChanged);
};

SLGUIAnimationUI::~SLGUIAnimationUI()
{
	FCoreDelegates::OnActorLabelChanged.RemoveAll(this);
}

void SLGUIAnimationUI::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	// 更新选中目标的位置, zachma todo: 每帧Tick并不是好的实现，需要寻找更优的解决办法。监听OnComponentLocationChanged或许是一个办法。
	// 参考：GEngine->BroadcastOnComponentTransformChanged
	if (GetSelectedActor())
	{
		GUnrealEd->UpdatePivotLocationForSelection();
	}
}

bool SLGUIAnimationUI::OnRequestClose() 
{ 
	return true; 
}


//  代码来自 FAssetEditorToolkit::InitAssetEditor
void SLGUIAnimationUI::InitToolKit()
{
	//TSharedPtr<SDockTab> NewMajorTab = SNew(SDockTab);
	//TSharedPtr<SStandaloneAssetEditorToolkitHost> NewStandaloneHost;
	//const TSharedRef<FTabManager> NewTabManager = FGlobalTabmanager::Get()->NewTabManager( NewMajorTab.ToSharedRef() );		
	//NewMajorTab->SetContent
	//(
	//	SAssignNew(NewStandaloneHost, SStandaloneAssetEditorToolkitHost, NewTabManager, TEXT("LGUIAnimationEditor"))
	//		.OnRequestClose(this, &FAssetEditorToolkit::OnRequestClose)
	//);

	//// Assign our toolkit host before we setup initial content.  (Important: We must cache this pointer here as SetupInitialContent
	//// will callback into the toolkit host.)
	//ToolkitHost = NewStandaloneHost;

	//FToolkitManager& ToolkitManager = FToolkitManager::Get();

	//check(ToolkitHost.IsValid());
	//// 实现IToolKit接口?
	////ToolkitManager.RegisterNewToolkit(SharedThis(this));

	ToolkitHost = MakeShared<FFakeToolkitHost>();

}

TSharedRef<SWidget> SLGUIAnimationUI::CreateSequencerWidget()
{
	TSharedRef<SOverlay> SequencerOverlayRef =
		SNew(SOverlay)
		.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("Sequencer")));
	SequencerOverlay = SequencerOverlayRef;

	TSharedRef<STextBlock> NoAnimationTextBlockRef = 
		SNew(STextBlock)
		.TextStyle(FEditorStyle::Get(), "UMGEditor.NoAnimationFont")
		.Text(LOCTEXT("NoAnimationSelected", "No Animation Selected"));
	NoAnimationTextBlock = NoAnimationTextBlockRef;

	SequencerOverlayRef->AddSlot(0)
	[
		GetSequencer()->GetSequencerWidget()
	];

	SequencerOverlayRef->AddSlot(1)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
	[
		NoAnimationTextBlockRef
	];

	return SequencerOverlayRef;
}

TSharedPtr<ISequencer>& SLGUIAnimationUI::GetSequencer()
{
	if(!Sequencer.IsValid())
	{
		const float InTime  = 0.f;
		const float OutTime = 5.0f;

		FSequencerViewParams ViewParams(TEXT("UMGSequencerSettings"));
		{
			ViewParams.OnGetAddMenuContent = FOnGetAddMenuContent::CreateSP(this, &SLGUIAnimationUI::OnGetAnimationAddMenuContent);
		    ViewParams.OnBuildCustomContextMenuForGuid = FOnBuildCustomContextMenuForGuid::CreateSP(this, &SLGUIAnimationUI::OnBuildCustomContextMenuForGuid);
		}

		FSequencerInitParams SequencerInitParams;
		{
			ULGUIAnimation* NullAnimation = ULGUIAnimation::GetNullAnimation();
			FFrameRate TickResolution = NullAnimation->MovieScene->GetTickResolution();
			FFrameNumber StartFrame = (InTime  * TickResolution).FloorToFrame();
			FFrameNumber EndFrame   = (OutTime * TickResolution).CeilToFrame();
			NullAnimation->MovieScene->SetPlaybackRange(StartFrame, (EndFrame-StartFrame).Value);
			FMovieSceneEditorData& EditorData = NullAnimation->MovieScene->GetEditorData();
			EditorData.WorkStart = InTime;
			EditorData.WorkEnd   = OutTime;

			SequencerInitParams.ViewParams = ViewParams;
			SequencerInitParams.RootSequence = NullAnimation;
			SequencerInitParams.bEditWithinLevelEditor = false;
			SequencerInitParams.ToolkitHost = ToolkitHost;
			SequencerInitParams.PlaybackContext = TAttribute<UObject*>(this, &SLGUIAnimationUI::GetAnimationPlaybackContext);
			SequencerInitParams.EventContexts = TAttribute<TArray<UObject*>>(this, &SLGUIAnimationUI::GetAnimationEventContexts);

			SequencerInitParams.HostCapabilities.bSupportsCurveEditor = true;
		};

		Sequencer = FModuleManager::LoadModuleChecked<ISequencerModule>("Sequencer").CreateSequencer(SequencerInitParams);
		// Never recompile the blueprint on evaluate as this can create an insidious loop
		Sequencer->GetSequencerSettings()->SetCompileDirectorOnEvaluate(false);
		Sequencer->OnMovieSceneDataChanged().AddSP( this, &SLGUIAnimationUI::OnMovieSceneDataChanged );
		Sequencer->OnMovieSceneBindingsPasted().AddSP( this, &SLGUIAnimationUI::OnMovieSceneBindingsPasted );
		// Change selected widgets in the sequencer tree view
		Sequencer->GetSelectionChangedObjectGuids().AddSP(this, &SLGUIAnimationUI::SyncSelectedWidgetsWithSequencerSelection);
		//OnSelectedWidgetsChanged.AddSP(this, &SUIAnimationUI::SyncSequencerSelectionToSelectedWidgets);
		
		// Allow sequencer to test which bindings are selected
		Sequencer->OnGetIsBindingVisible().BindRaw(this, &SLGUIAnimationUI::IsBindingSelected);

		Sequencer->OnPlayEvent().AddSP(this, &SLGUIAnimationUI::OnPlayEvent);
		Sequencer->OnStopEvent().AddSP(this, &SLGUIAnimationUI::OnStopEvent);

		ChangeViewedAnimation(*ULGUIAnimation::GetNullAnimation());
	}

	return Sequencer;
}

void SLGUIAnimationUI::ChangeViewedAnimation( ULGUIAnimation& InAnimationToView )
{
	CurrentAnimation = &InAnimationToView;

	if (Sequencer.IsValid())
	{
		Sequencer->ResetToNewRootSequence(InAnimationToView);
	}

	TSharedPtr<SOverlay> SequencerOverlayPin = SequencerOverlay.Pin();
	if (SequencerOverlayPin.IsValid())
	{
		TSharedPtr<STextBlock> NoAnimationTextBlockPin = NoAnimationTextBlock.Pin();
		if( &InAnimationToView == ULGUIAnimation::GetNullAnimation())
		{
			const FName CurveEditorTabName = FName(TEXT("SequencerGraphEditor"));
			//TSharedPtr<SDockTab> ExistingTab = GetToolkitHost()->GetTabManager()->FindExistingLiveTab(CurveEditorTabName);
			//if (ExistingTab)
			//{
			//	ExistingTab->RequestCloseTab();
			//}

			// Disable sequencer from interaction
			Sequencer->GetSequencerWidget()->SetEnabled(false);
			Sequencer->SetAutoChangeMode(EAutoChangeMode::None);
			NoAnimationTextBlockPin->SetVisibility(EVisibility::Visible);
			SequencerOverlayPin->SetVisibility( EVisibility::HitTestInvisible );
		}
		else
		{
			// Allow sequencer to be interacted with
			Sequencer->GetSequencerWidget()->SetEnabled(true);
			NoAnimationTextBlockPin->SetVisibility(EVisibility::Collapsed);
			SequencerOverlayPin->SetVisibility( EVisibility::SelfHitTestInvisible );
		}
	}
}


const ULGUIAnimation* SLGUIAnimationUI::RefreshCurrentAnimation()
{
	return CurrentAnimation.Get();
}

void SLGUIAnimationUI::SelectLGUIBaseActor(AUIBaseActor* Actor, bool bClearPreviosSelection /*= true*/)
{
	if (bClearPreviosSelection)
	{
		SelectedWidgets.Empty();
	}
	SelectedWidgets.Add(Actor);
}

// 当前预览的Animation数据
UUIAnimationComp* SLGUIAnimationUI::GetPreviewAnimationComp() const
{
	if (MainUI == nullptr || MainUI->GetUIMainContainer() == nullptr)
	{
		UE_LOG(LGUI, Error, TEXT("GetPreviewUIActor failed! MainUI is nullptr!"));
		return nullptr;
	}

	return MainUI->GetUIMainContainer()->FindComponentByClass<UUIAnimationComp>();
}

class AUIBaseActor* SLGUIAnimationUI::GetSelectedActor()
{
	if (MainUI == nullptr)
	{
		UE_LOG(LGUI, Error, TEXT("GetSelectedActor failed! MainUI is nullptr!"));
		return nullptr;
	}

	return Cast<AUIBaseActor>(MainUI->GetSelectedActor());
}

bool SLGUIAnimationUI::IsBindingSelected(const FMovieSceneBinding& InBinding)
{
	// zachma todo:
	return false;
}

// 添加菜单
void SLGUIAnimationUI::OnGetAnimationAddMenuContent(FMenuBuilder& MenuBuilder, TSharedRef<ISequencer> InSequencer)
{
	if (CurrentAnimation.IsValid())
	{
		const TSet<AUIBaseActor*>& Selection = GetSelectedWidgets();
		for (AUIBaseActor* SelectedWidget : Selection)
		{
			if (SelectedWidget)
			{
				FUIAction AddWidgetTrackAction(FExecuteAction::CreateSP(this, &SLGUIAnimationUI::AddObjectToAnimation, (UObject*)SelectedWidget));
				MenuBuilder.AddMenuEntry(FText::FromString(SelectedWidget->GetActorLabel()), FText(), FSlateIcon(), AddWidgetTrackAction);
			}
		}

		MenuBuilder.AddSubMenu(
			LOCTEXT("AllNamedWidgets", "All Named Widgets"),
			LOCTEXT("AllNamedWidgetsTooltip", "Select a widget or slot to create an animation track for"),
			FNewMenuDelegate::CreateRaw(this, &SLGUIAnimationUI::OnGetAnimationAddMenuContentAllWidgets),
			false,
			FSlateIcon()
		);
	}
}

// todo: 遍历所有的LevelActor，过滤多所有的UIBaseActor（排除掉UIRooot）
void SLGUIAnimationUI::OnGetAnimationAddMenuContentAllWidgets(FMenuBuilder& MenuBuilder)
{
	TArray<FObjectAndDisplayName> BindableObjects;
	{
		for (FActorIterator ActorItr(MainUI->GetWorld()); ActorItr; ++ActorItr)
		{
			AUIBaseActor* Actor = Cast<AUIBaseActor>(*ActorItr);
			if (Actor)
			{
				BindableObjects.Add(FObjectAndDisplayName(FText::FromString(Actor->GetActorLabel()), Actor));
			}
		}

		BindableObjects.Sort();
	}

	for (FObjectAndDisplayName& BindableObject : BindableObjects)
	{
		FGuid BoundObjectGuid = Sequencer->FindObjectId(*BindableObject.Object, Sequencer->GetFocusedTemplateID());
		if (BoundObjectGuid.IsValid() == false)
		{
			FUIAction AddMenuAction(FExecuteAction::CreateSP(this, &SLGUIAnimationUI::AddObjectToAnimation, BindableObject.Object));
			MenuBuilder.AddMenuEntry(BindableObject.DisplayName, FText(), FSlateIcon(), AddMenuAction);
		}
	}
}

void SLGUIAnimationUI::AddObjectToAnimation(UObject* ObjectToAnimate)
{
	UMovieScene* MovieScene = Sequencer->GetFocusedMovieSceneSequence()->GetMovieScene();
	if (MovieScene->IsReadOnly())
	{
		return;
	}

	const FScopedTransaction Transaction( LOCTEXT( "AddUIActorToAnimation", "Add UIBaseActor to animation" ) );
	Sequencer->GetFocusedMovieSceneSequence()->Modify();

	// @todo Sequencer - Make this kind of adding more explicit, this current setup seem a bit brittle.
	FGuid NewGuid = Sequencer->GetHandleToObject(ObjectToAnimate);

	TArray<UMovieSceneFolder*> SelectedParentFolders;
	Sequencer->GetSelectedFolders(SelectedParentFolders);

	if (SelectedParentFolders.Num() > 0)
	{
		SelectedParentFolders[0]->AddChildObjectBinding(NewGuid);
	}
}

TSharedRef<FExtender> SLGUIAnimationUI::GetAddTrackSequencerExtender( const TSharedRef<FUICommandList> CommandList, const TArray<UObject*> ContextSensitiveObjects )
{
	TSharedRef<FExtender> AddTrackMenuExtender( new FExtender() );
	AddTrackMenuExtender->AddMenuExtension(
		SequencerMenuExtensionPoints::AddTrackMenu_PropertiesSection,
		EExtensionHook::Before,
		CommandList,
		FMenuExtensionDelegate::CreateRaw( this, &SLGUIAnimationUI::ExtendSequencerAddTrackMenu, ContextSensitiveObjects ) );
	return AddTrackMenuExtender;
}

void SLGUIAnimationUI::OnBuildCustomContextMenuForGuid(FMenuBuilder& MenuBuilder, FGuid ObjectBinding)
{
	if (CurrentAnimation.IsValid())
	{
		TArray<AUIBaseActor*> ValidSelectedWidgets;
		for (AUIBaseActor* SelectedWidget : SelectedWidgets)
		{
			if (SelectedWidget)
			{
				//need to make sure it's a widget, if not bound assume it is.
				AUIBaseActor* BoundWidget = nullptr;
				bool bNotBound = true;
				for (TWeakObjectPtr<> WeakObjectPtr : GetSequencer()->FindObjectsInCurrentSequence(ObjectBinding))
				{
					BoundWidget = Cast<AUIBaseActor>(WeakObjectPtr.Get());
					bNotBound = false;
					break;
				}

				if (bNotBound)
				{
					ValidSelectedWidgets.Add(SelectedWidget);
				}
			}
		}
		
		if(ValidSelectedWidgets.Num() > 0)
		{
			MenuBuilder.AddMenuSeparator();
			
			MenuBuilder.AddMenuEntry(
				LOCTEXT("AddSelectedToBinding", "Add Selected"),
				LOCTEXT("AddSelectedToBindingToolTip", "Add selected objects to this track"),
				FSlateIcon(),
				FExecuteAction::CreateRaw(this, &SLGUIAnimationUI::AddWidgetsToTrack, ValidSelectedWidgets, ObjectBinding)
			);
			
			if (ValidSelectedWidgets.Num() > 1)
			{
				MenuBuilder.AddMenuEntry(
					LOCTEXT("ReplaceBindingWithSelected", "Replace with Selected"),
					LOCTEXT("ReplaceBindingWithSelectedToolTip", "Replace the object binding with selected objects"),
					FSlateIcon(),
					FExecuteAction::CreateRaw(this, &SLGUIAnimationUI::ReplaceTrackWithWidgets, ValidSelectedWidgets, ObjectBinding)
				);
			}
			else
			{
				MenuBuilder.AddMenuEntry(
					FText::Format(LOCTEXT("ReplaceObject", "Replace with {0}"), FText::FromString(ValidSelectedWidgets[0]->GetName())),
					FText::Format(LOCTEXT("ReplaceObjectToolTip", "Replace the bound widget in this animation with {0}"), FText::FromString(ValidSelectedWidgets[0]->GetName())),
					FSlateIcon(),
					FExecuteAction::CreateRaw(this, &SLGUIAnimationUI::ReplaceTrackWithWidgets, ValidSelectedWidgets, ObjectBinding)
				);
			}
			
			MenuBuilder.AddMenuEntry(
				LOCTEXT("RemoveSelectedFromBinding", "Remove Selected"),
				LOCTEXT("RemoveSelectedFromBindingToolTip", "Remove selected objects from this track"),
				FSlateIcon(),
				FExecuteAction::CreateRaw(this, &SLGUIAnimationUI::RemoveWidgetsFromTrack, ValidSelectedWidgets, ObjectBinding)
			);

			MenuBuilder.AddMenuEntry(
				LOCTEXT("RemoveAllBindings", "Remove All"),
				LOCTEXT("RemoveAllBindingsToolTip", "Remove all bound objects from this track"),
				FSlateIcon(),
				FExecuteAction::CreateRaw(this, &SLGUIAnimationUI::RemoveAllWidgetsFromTrack, ObjectBinding)
			);
			
			MenuBuilder.AddMenuEntry(
				LOCTEXT("RemoveMissing", "Remove Missing"),
				LOCTEXT("RemoveMissingToolTip", "Remove missing objects bound to this track"),
				FSlateIcon(),
				FExecuteAction::CreateRaw(this, &SLGUIAnimationUI::RemoveMissingWidgetsFromTrack, ObjectBinding)
			);
		}
	}
}

// zachma todo: 拓展Sequencer AdddMenuTrack菜单，添加材质等选项
void SLGUIAnimationUI::ExtendSequencerAddTrackMenu( FMenuBuilder& AddTrackMenuBuilder, const TArray<UObject*> ContextObjects )
{
	//if ( ContextObjects.Num() == 1 )
	//{
	//	AUIBaseActor* Widget = Cast<AUIBaseActor>( ContextObjects[0] );

	//	if ( Widget != nullptr && Widget->GetTypedOuter<UUserWidget>() == GetPreview() )
	//	{
	//		if( Widget->GetParent() != nullptr && Widget->Slot != nullptr )
	//		{
	//			AddTrackMenuBuilder.BeginSection( "Slot", LOCTEXT( "SlotSection", "Slot" ) );
	//			{
	//				FUIAction AddSlotAction( FExecuteAction::CreateRaw( this, &SUIAnimationUI::AddSlotTrack, Widget->Slot ) );
	//				FText AddSlotLabel = FText::Format(LOCTEXT("SlotLabelFormat", "{0} Slot"), FText::FromString(Widget->GetParent()->GetName()));
	//				FText AddSlotToolTip = FText::Format(LOCTEXT("SlotToolTipFormat", "Add {0} slot"), FText::FromString( Widget->GetParent()->GetName()));
	//				AddTrackMenuBuilder.AddMenuEntry(AddSlotLabel, AddSlotToolTip, FSlateIcon(), AddSlotAction);
	//			}
	//			AddTrackMenuBuilder.EndSection();
	//		}

	//		TArray<FWidgetMaterialPropertyPath> MaterialBrushPropertyPaths;
	//		WidgetMaterialTrackUtilities::GetMaterialBrushPropertyPaths( Widget, MaterialBrushPropertyPaths );
	//		if ( MaterialBrushPropertyPaths.Num() > 0 )
	//		{
	//			AddTrackMenuBuilder.BeginSection( "Materials", LOCTEXT( "MaterialsSection", "Materials" ) );
	//			{
	//				for (FWidgetMaterialPropertyPath& MaterialBrushPropertyPath : MaterialBrushPropertyPaths )
	//				{
	//					FString DisplayName = MaterialBrushPropertyPath.PropertyPath[0]->GetDisplayNameText().ToString();
	//					for ( int32 i = 1; i < MaterialBrushPropertyPath.PropertyPath.Num(); i++)
	//					{
	//						DisplayName.AppendChar( '.' );
	//						DisplayName.Append( MaterialBrushPropertyPath.PropertyPath[i]->GetDisplayNameText().ToString() );
	//					}
	//					DisplayName.AppendChar('.');
	//					DisplayName.Append(MaterialBrushPropertyPath.DisplayName);

	//					FText DisplayNameText = FText::FromString( DisplayName );
	//					FUIAction AddMaterialAction( FExecuteAction::CreateRaw( this, &SUIAnimationUI::AddMaterialTrack, Widget, MaterialBrushPropertyPath.PropertyPath, DisplayNameText ) );
	//					FText AddMaterialLabel = DisplayNameText;
	//					FText AddMaterialToolTip = FText::Format( LOCTEXT( "MaterialToolTipFormat", "Add a material track for the {0} property." ), DisplayNameText );
	//					AddTrackMenuBuilder.AddMenuEntry( AddMaterialLabel, AddMaterialToolTip, FSlateIcon(), AddMaterialAction );
	//				}
	//			}
	//			AddTrackMenuBuilder.EndSection();
	//		}
	//	}
	//}
}

void SLGUIAnimationUI::AddWidgetsToTrack(const TArray<AUIBaseActor*> Widgets, FGuid ObjectId)
{
	const FScopedTransaction Transaction(LOCTEXT("AddSelectedWidgetsToTrack", "Add Widgets to Track"));

	ULGUIAnimation* LGUIAnimation = Cast<ULGUIAnimation>(Sequencer->GetFocusedMovieSceneSequence());
	UMovieScene* MovieScene = LGUIAnimation->GetMovieScene();

	FText ExistingBindingName;
	TArray<AUIBaseActor*> WidgetsToAdd;
	for (AUIBaseActor* Widget : Widgets)
	{
		AUIBaseActor* PreviewWidget = Widget;

		// If this widget is already bound to the animation we cannot add it to 2 separate bindings
		FGuid SelectedWidgetId = Sequencer->FindObjectId(*PreviewWidget, MovieSceneSequenceID::Root);
		if (!SelectedWidgetId.IsValid())
		{
			WidgetsToAdd.Add(Widget);
		}
		else if (ExistingBindingName.IsEmpty())
		{
			ExistingBindingName = MovieScene->GetObjectDisplayName(SelectedWidgetId);
		}
	}

	if (WidgetsToAdd.Num() == 0)
	{
		FNotificationInfo Info(FText::Format(LOCTEXT("WidgetAlreadyBound", "Widget already bound to {0}"), ExistingBindingName));
		Info.FadeInDuration = 0.1f;
		Info.FadeOutDuration = 0.5f;
		Info.ExpireDuration = 2.5f;
		auto NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);

		NotificationItem->SetCompletionState(SNotificationItem::CS_Success);
		NotificationItem->ExpireAndFadeout();
	}
	else
	{
		MovieScene->Modify();
		LGUIAnimation->Modify();

		for (AUIBaseActor* Widget : WidgetsToAdd)
		{
			AUIBaseActor* PreviewWidget = Widget;
			LGUIAnimation->BindPossessableObject(ObjectId, *PreviewWidget, GetAnimationPlaybackContext());
		}

		UpdateTrackName(ObjectId);

		Sequencer->NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::MovieSceneStructureItemsChanged);
	}
}

void SLGUIAnimationUI::RemoveWidgetsFromTrack(const TArray<AUIBaseActor*> Widgets, FGuid ObjectId)
{
	const FScopedTransaction Transaction(LOCTEXT("RemoveWidgetsFromTrack", "Remove Widgets from Track"));

	ULGUIAnimation* WidgetAnimation = Cast<ULGUIAnimation>(Sequencer->GetFocusedMovieSceneSequence());
	UMovieScene* MovieScene = WidgetAnimation->GetMovieScene();

	TArray<AUIBaseActor*> WidgetsToRemove;

	for (AUIBaseActor* Widget : Widgets)
	{
		AUIBaseActor* PreviewWidget = Widget;
		FGuid WidgetId = Sequencer->FindObjectId(*PreviewWidget, MovieSceneSequenceID::Root);
		if (WidgetId.IsValid() && WidgetId == ObjectId)
		{
			WidgetsToRemove.Add(Widget);
		}
	}

	if (WidgetsToRemove.Num() == 0)
	{
		FNotificationInfo Info(LOCTEXT("SelectedWidgetNotBound", "Selected Widget not Bound to Track"));
		Info.FadeInDuration = 0.1f;
		Info.FadeOutDuration = 0.5f;
		Info.ExpireDuration = 2.5f;
		auto NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);

		NotificationItem->SetCompletionState(SNotificationItem::CS_Success);
		NotificationItem->ExpireAndFadeout();
	}
	else
	{
		MovieScene->Modify();
		WidgetAnimation->Modify();

		for (AUIBaseActor* Widget : WidgetsToRemove)
		{
			AUIBaseActor* PreviewWidget = Widget;
			WidgetAnimation->RemoveBinding(*PreviewWidget);

			Sequencer->RestorePreAnimatedState(*PreviewWidget);
		}

		UpdateTrackName(ObjectId);

		Sequencer->NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::MovieSceneStructureItemsChanged);
	}
}

void SLGUIAnimationUI::RemoveAllWidgetsFromTrack(FGuid ObjectId)
{
	const FScopedTransaction Transaction(LOCTEXT("RemoveAllWidgetsFromTrack", "Remove All Widgets from Track"));

	ULGUIAnimation* LGUIAnimation = Cast<ULGUIAnimation>(Sequencer->GetFocusedMovieSceneSequence());
	UMovieScene* MovieScene = LGUIAnimation->GetMovieScene();

	UUIAnimationComp* PreviewAnimationComp = GetPreviewAnimationComp();
	check(PreviewAnimationComp);

	LGUIAnimation->Modify();
	MovieScene->Modify();

	// Restore object animation state
	for (TWeakObjectPtr<> WeakObject : Sequencer->FindBoundObjects(ObjectId, MovieSceneSequenceID::Root))
	{
		if (UObject* Obj = WeakObject.Get())
		{
			Sequencer->RestorePreAnimatedState(*Obj);
		}
	}

	// Remove bindings
	for (int32 Index = LGUIAnimation->AnimationBindings.Num() - 1; Index >= 0; --Index)
	{
		if (LGUIAnimation->AnimationBindings[Index].ActorGuid == ObjectId)
		{
			LGUIAnimation->AnimationBindings.RemoveAt(Index, 1, false);
		}
	}

	Sequencer->NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::MovieSceneStructureItemsChanged);
}

void SLGUIAnimationUI::RemoveMissingWidgetsFromTrack(FGuid ObjectId)
{
	const FScopedTransaction Transaction(LOCTEXT("RemoveMissingWidgetsFromTrack", "Remove Missing Widgets from Track"));

	ULGUIAnimation* LGUIAnimation = Cast<ULGUIAnimation>(Sequencer->GetFocusedMovieSceneSequence());
	UMovieScene* MovieScene = LGUIAnimation->GetMovieScene();

	UUIAnimationComp* PreviewAnimationComp = GetPreviewAnimationComp();
	if (PreviewAnimationComp == nullptr)
	{
		UE_LOG(LGUI, Warning, TEXT("RemoveMissingWidgetsFromTrack failed, PreviewRoot is nullptr!"));
		return;
	}

	LGUIAnimation->Modify();
	MovieScene->Modify();
	MainUI->SetPrefabModify();

	for (int32 Index = LGUIAnimation->AnimationBindings.Num() - 1; Index >= 0; --Index)
	{
		const FLGUIAnimationBinding& Binding = LGUIAnimation->AnimationBindings[Index];
		if (Binding.ActorGuid == ObjectId && Binding.FindRuntimeObject(*PreviewAnimationComp, LGUIAnimation) == nullptr)
		{
			LGUIAnimation->AnimationBindings.RemoveAt(Index, 1, false);
		}
	}

	UpdateTrackName(ObjectId);
}


void SLGUIAnimationUI::OnActorLabelChanged(AActor* Actor)
{
	AUIBaseActor* UIActor = Cast<AUIBaseActor>(Actor);
	if (CurrentAnimation == nullptr || UIActor == nullptr)
	{
		return;
	}

	UE_LOG(LGUI, Log, TEXT("OnActorLabelChanged, Actor=%s, NewLabel=%s"), *GetNameSafe(UIActor), *UIActor->GetActorLabel());

	UUIAnimationComp* PreviewRoot = GetPreviewAnimationComp();
	if (PreviewRoot == nullptr)
	{
		return;
	}
	for (ULGUIAnimation* LGUIAnimation : PreviewRoot->Animations)
	{
		const FLGUIAnimationBinding* FoundBind = LGUIAnimation->GetBindings().FindByPredicate([&](const FLGUIAnimationBinding& Binding) {return Binding.UIActorID == UIActor->UIActorID; });
		if (FoundBind)
		{
			UpdateTrackName(FoundBind->ActorGuid);
		}
	}
}


void SLGUIAnimationUI::ReplaceTrackWithWidgets(TArray<AUIBaseActor*> Widgets, FGuid ObjectId)
{
	ULGUIAnimation* LGUIAnimation = Cast<ULGUIAnimation>(Sequencer->GetFocusedMovieSceneSequence());
	UMovieScene* MovieScene = LGUIAnimation->GetMovieScene();

	// Filter out anything in the input array that is currently bound to another object in the animation
	FText ExistingBindingName;
	for (int32 Index = Widgets.Num()-1; Index >= 0; --Index)
	{
		AUIBaseActor* PreviewWidget = Widgets[Index];
		FGuid WidgetId = Sequencer->FindObjectId(*PreviewWidget, MovieSceneSequenceID::Root);
		if (WidgetId.IsValid() && WidgetId != ObjectId)
		{
			Widgets.RemoveAt(Index, 1, false);

			if (ExistingBindingName.IsEmpty())
			{
				ExistingBindingName = MovieScene->GetObjectDisplayName(WidgetId);
			}
		}
	}

	if (Widgets.Num() == 0)
	{
		FNotificationInfo Info(FText::Format(LOCTEXT("WidgetAlreadyBound", "Widget already bound to {0}"), ExistingBindingName));
		Info.FadeInDuration = 0.1f;
		Info.FadeOutDuration = 0.5f;
		Info.ExpireDuration = 2.5f;
		auto NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);

		NotificationItem->SetCompletionState(SNotificationItem::CS_Success);
		NotificationItem->ExpireAndFadeout();
		return;
	}

	const FScopedTransaction Transaction( LOCTEXT( "ReplaceTrackWithSelectedWidgets", "Replace Track with Selected Widgets" ) );


	LGUIAnimation->Modify();
	MovieScene->Modify();

	// Remove everything from the track
	RemoveAllWidgetsFromTrack(ObjectId);

	AddWidgetsToTrack(Widgets, ObjectId);

	UpdateTrackName(ObjectId);

	Sequencer->NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::MovieSceneStructureItemsChanged);
}

void SLGUIAnimationUI::AddSlotTrack( AUIBaseActor* Slot )
{
	GetSequencer()->GetHandleToObject( Slot );
}

void SLGUIAnimationUI::OnMovieSceneDataChanged(EMovieSceneDataChangeType DataChangeType)
{
	UE_LOG(LGUI, Log, TEXT("LGUIAnimationUI::OnMovieSceneDataChanged, DataChangeType=%d"), (int32)DataChangeType);
	bRefreshGeneratedClassAnimations = true;
}

// zachma todo: 暂不支持复制粘贴
void SLGUIAnimationUI::OnMovieSceneBindingsPasted(const TArray<FMovieSceneBinding>& BindingsPasted)
{
	TArray<FObjectAndDisplayName> BindableObjects;
	{
		for (FActorIterator ActorItr(MainUI->GetWorld()); ActorItr; ++ActorItr)
		{
			AUIBaseActor* Actor = Cast<AUIBaseActor>(*ActorItr);
			if (Actor)
			{
				BindableObjects.Add(FObjectAndDisplayName(FText::FromString(Actor->GetActorLabel()), Actor));
			}
		}

		BindableObjects.Sort();
	}

	UMovieSceneSequence* AnimationSequence = GetSequencer().Get()->GetFocusedMovieSceneSequence();
	UObject* BindingContext = GetAnimationPlaybackContext();

	// First, rebind top level possessables (without parents) - match binding pasted's name with the bindable object name
	for (const FMovieSceneBinding& BindingPasted : BindingsPasted)
	{
		FMovieScenePossessable* Possessable = AnimationSequence->GetMovieScene()->FindPossessable(BindingPasted.GetObjectGuid());
		if (Possessable && !Possessable->GetParent().IsValid())
		{
			for (FObjectAndDisplayName& BindableObject : BindableObjects)
			{
				if (BindableObject.DisplayName.ToString() == BindingPasted.GetName())
				{
					AnimationSequence->BindPossessableObject(BindingPasted.GetObjectGuid(), *BindableObject.Object, BindingContext);			
					break;
				}
			}
		}
	}
}

void SLGUIAnimationUI::OnPlayEvent()
{
	UE_LOG(LGUI, Log ,TEXT("Start play LGUIAnimation!"));
	bPlayingAnimation = true;
}

void SLGUIAnimationUI::OnStopEvent()
{
	UE_LOG(LGUI, Log ,TEXT("Stop play LGUIAnimation!"));
	bPlayingAnimation = false;
}


// sequence 选择控件改变
void SLGUIAnimationUI::SyncSelectedWidgetsWithSequencerSelection(TArray<FGuid> ObjectGuids)
{
	if (bUpdatingSequencerSelection)
	{
		return;
	}

	UE_LOG(LGUI, Log, TEXT("SyncSelectedWidgetsWithSequencerSelection, ObjectGuids.Num()=%d"), ObjectGuids.Num());

	TGuardValue<bool> Guard(bUpdatingExternalSelection, true);

	UMovieSceneSequence* AnimationSequence = GetSequencer().Get()->GetFocusedMovieSceneSequence();
	UObject* BindingContext = GetAnimationPlaybackContext();
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

	// 同步选中效果
	if (SequencerSelectedWidgets.Num() != 0)
	{
		AUIBaseActor* SelectedActor = *SequencerSelectedWidgets.begin();

		UE_LOG(LGUI, Log, TEXT("SyncSelectedWidgetsWithSequencerSelection, SelectedActor=%s"), *GetNameSafe(SelectedActor));

		// 同步选中
		MainUI->OnOutlinerPickedChanged(SelectedActor);
		GEditor->SelectActor(SelectedActor, true, true, true);
	}
}

void SLGUIAnimationUI::SyncSequencerSelectionToSelectedWidgets()
{
	if (bUpdatingExternalSelection)
	{
		return;
	}

	TGuardValue<bool> Guard(bUpdatingSequencerSelection, true);

	if (GetSequencer()->GetSequencerSettings()->GetShowSelectedNodesOnly())
	{
		GetSequencer()->RefreshTree();
	}

	GetSequencer()->ExternalSelectionHasChanged();
}

UObject* SLGUIAnimationUI::GetAnimationPlaybackContext() const
{
	return GetPreviewAnimationComp();
}

TArray<UObject*> SLGUIAnimationUI::GetAnimationEventContexts() const
{
	TArray<UObject*> EventContexts; 
	EventContexts.Add(GetPreviewAnimationComp()); 
	return EventContexts;
}

void SLGUIAnimationUI::UpdateTrackName(FGuid ObjectId)
{
	UUIAnimationComp* PreviewRoot = GetPreviewAnimationComp();
	UObject* BindingContext = GetAnimationPlaybackContext();

	for (ULGUIAnimation* LGUIAnimation : PreviewRoot->Animations)
	{
		if (LGUIAnimation == nullptr)
		{
			continue;
		}
		UMovieScene* MovieScene = LGUIAnimation->GetMovieScene();
		const TArray<FLGUIAnimationBinding>& WidgetBindings = LGUIAnimation->GetBindings();

		for (FLGUIAnimationBinding& Binding : LGUIAnimation->AnimationBindings)
		{
			if (Binding.ActorGuid != ObjectId)
			{
				continue;
			}

			TArray<UObject*, TInlineAllocator<1>> BoundObjects;
			LGUIAnimation->LocateBoundObjects(ObjectId, BindingContext, BoundObjects);

			if (BoundObjects.Num() > 0)
			{
				AActor* Actor = Cast<AActor>(BoundObjects[0]);
				if (Actor)
				{
					FString NewLabel = Actor->GetActorLabel();
					if (BoundObjects.Num() > 1)
					{
						NewLabel.Append(FString::Printf(TEXT(" (%d)"), BoundObjects.Num()));
					}

					MovieScene->SetObjectDisplayName(ObjectId, FText::FromString(NewLabel));
				}
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE


PRAGMA_ENABLE_OPTIMIZATION