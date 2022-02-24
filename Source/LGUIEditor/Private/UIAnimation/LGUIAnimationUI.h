// 

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "SComponentClassCombo.h"
#include "Animation/LGUIAnimation.h"
#include "ISequencer.h"

class AUIBaseActor;

/**
 * widget blueprint editor (extends Blueprint editor)
 */
class SLGUIAnimationUI : public SCompoundWidget
{
public:


	SLATE_BEGIN_ARGS(SLGUIAnimationUI)
	{
	}

	SLATE_END_ARGS()

	/** Widget constructor */
	void Construct(const FArguments& Args, class FLGUIPrefabEditor* InMainUI);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual ~SLGUIAnimationUI();

	virtual bool OnRequestClose();
	void InitToolKit();
	TSharedRef<SWidget> CreateSequencerWidget();
	/** @return The sequencer used to create widget animations */
	TSharedPtr<ISequencer>& GetSequencer();

	/** Changes the currently viewed animation in Sequencer to the new one*/
	void ChangeViewedAnimation( ULGUIAnimation& InAnimationToView );

	/** Get the current animation*/
	ULGUIAnimation* GetCurrentAnimation() { return CurrentAnimation.Get(); }

	/** Updates the current animation if it is invalid */
	const ULGUIAnimation* RefreshCurrentAnimation();

	const TSet<AUIBaseActor*>& GetSelectedWidgets() const
	{
		return SelectedWidgets;
	}

	void SelectLGUIBaseActor(AUIBaseActor* Actor, bool bClearPreviosSelection = true);

	// 当前打开prefab的跟节点
	class UUIAnimationComp* GetPreviewAnimationComp() const;

	// 当前选中的UIActor，用于编辑动画信息
	class AUIBaseActor* GetSelectedActor();

private:


	/** Called to determine whether a binding is selected in the tree view */
	bool IsBindingSelected(const FMovieSceneBinding& InBinding);

	/** Populates the sequencer add menu. */
	void OnGetAnimationAddMenuContent(FMenuBuilder& MenuBuilder, TSharedRef<ISequencer> Sequencer);

	/** Populates sequencer menu when added with objects. Used to handle case where widget is deleted so it's 
	    Object * is null.*/
	void OnBuildCustomContextMenuForGuid(FMenuBuilder& MenuBuilder,FGuid ObjectBinding);

	/** Populates the sequencer add submenu for the big list of widgets. */
	void OnGetAnimationAddMenuContentAllWidgets(FMenuBuilder& MenuBuilder);

	/** Adds the supplied UObject to the current animation. */
	void AddObjectToAnimation(UObject* ObjectToAnimate);

	/** Gets the extender to use for sequencers context sensitive menus and toolbars. */
	TSharedRef<FExtender> GetAddTrackSequencerExtender(const TSharedRef<FUICommandList> CommandList, const TArray<UObject*> ContextSensitiveObjects);

	/** Extends the sequencer add track menu. */
	void ExtendSequencerAddTrackMenu( FMenuBuilder& AddTrackMenuBuilder, const TArray<UObject*> ContextObjects );

	/** Binds additional widgets to a track of the same type */
	void AddWidgetsToTrack(const TArray<AUIBaseActor*> Widgets, FGuid ObjectId);

	/** Unbind widgets from a track*/
	void RemoveWidgetsFromTrack(const TArray<AUIBaseActor*> Widgets, FGuid ObjectId);

	/** Remove all bindings from a track */
	void RemoveAllWidgetsFromTrack(FGuid ObjectId);

	/** Remove any missing bindings from a track */
	void RemoveMissingWidgetsFromTrack(FGuid ObjectId);
	
	void OnActorLabelChanged(AActor* Actor);
	/** Replace current widget bindings on a track with new widget bindings */
	void ReplaceTrackWithWidgets(const TArray<AUIBaseActor*> Widgets, FGuid ObjectId);

	/** Add an animation track for the supplied slot to the current animation. */
	void AddSlotTrack( AUIBaseActor* Slot );

	/** Handler which is called whenever sequencer movie scene data changes. */
	void OnMovieSceneDataChanged(EMovieSceneDataChangeType DataChangeType);

	/** Handler which is called whenever sequencer binding is pasted. */
	void OnMovieSceneBindingsPasted(const TArray<FMovieSceneBinding>& BindingsPasted);

	void OnPlayEvent();
	void OnStopEvent();
	/** Fire off when sequencer selection changed */
	void SyncSelectedWidgetsWithSequencerSelection(TArray<FGuid> ObjectGuids);

	/** Tell sequencer the selected widgets changed */
	void SyncSequencerSelectionToSelectedWidgets();

	/** Get the animation playback context */
	UObject* GetAnimationPlaybackContext() const;

	/** Get the animation playback event contexts */
	TArray<UObject*> GetAnimationEventContexts() const;
	
	/** Update the name of a track to reflect changes in bindings */
	void UpdateTrackName(FGuid ObjectId);

private:

	FLGUIPrefabEditor* MainUI;

	/** The currently selected preview widgets in the preview GUI */
	TSet< AUIBaseActor* > SelectedWidgets;

	/** Sequencer for creating and previewing widget animations */
	TSharedPtr<ISequencer> Sequencer;

	/** Overlay used to display UI on top of sequencer */
	TWeakPtr<SOverlay> SequencerOverlay;

	/** A text block which is displayed in the overlay when no animation is selected. */
	TWeakPtr<STextBlock> NoAnimationTextBlock;

	/** The currently viewed animation, if any. */
	TWeakObjectPtr<ULGUIAnimation> CurrentAnimation;

	FDelegateHandle SequencerAddTrackExtenderHandle;

	/** When true the animation data in the generated class should be replaced with the current animation data. */
	bool bRefreshGeneratedClassAnimations = false;

	/** When true the sequencer selection is being updated from changes to the external selection. */
	bool bUpdatingSequencerSelection = false;

	/** When true the external selection is being updated from changes to the sequencer selection. */
	bool bUpdatingExternalSelection = false;

	bool bPlayingAnimation = false;

	/** The asset editor that created this Sequencer if any */
	TSharedPtr<IToolkitHost> ToolkitHost;
};
