// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PreviewScene.h"
#include "EditorViewportClient.h"
#include "Animation/AnimInstance.h"
#include "LevelEditorViewport.h"

class FLGUIPrefabPreviewManager;
class FLGUIPrefabPreviewScene;
class UUIBaseRenderable;
class FLGUIPrefabEditor;
class ULGUIPrefab;

/** Viewport client for editor viewports. Contains common functionality for camera movement, rendering debug information, etc. */
class FLGUIPrefabEditorViewportClient : public FEditorViewportClient
{
public:
	FLGUIPrefabEditorViewportClient(FLGUIPrefabPreviewScene& InPreviewScene, TWeakPtr<FLGUIPrefabEditor> InPrefabEditorPtr, const TSharedRef<class SLGUIPrefabEditorViewport>& InEditorViewportPtr);

	virtual ~FLGUIPrefabEditorViewportClient();

	// FViewportClient interface
	virtual void Draw(FViewport* InViewport, FCanvas* Canvas) override;
	virtual void Tick(float DeltaSeconds) override;
	// End of FViewportClient interface

	// FEditorViewportClient interface
	virtual void ProcessClick(FSceneView& View, HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY) override;
	virtual bool InputKey(FViewport* Viewport, int32 ControllerId, FKey Key, EInputEvent Event, float AmountDepressed = 1.f, bool bGamepad = false) override;
	virtual void TrackingStarted(const struct FInputEventState& InInputState, bool bIsDragging, bool bNudge) override;
	virtual void TrackingStopped() override;
	virtual void AbortTracking() override;

	virtual void CapturedMouseMove(FViewport* InViewport, int32 InMouseX, int32 InMouseY) override;
	virtual bool InputWidgetDelta(FViewport* InViewport, EAxisList::Type InCurrentAxis, FVector& Drag, FRotator& Rot, FVector& Scale) override;
	// End of FEditorViewportClient interface

	void ApplyDeltaToActors(const FVector& InDrag, const FRotator& InRot, const FVector& InScale);
	void ApplyDeltaToActor(AActor* InActor, const FVector& InDeltaDrag, const FRotator& InDeltaRot, const FVector& InDeltaScale);

	void TickWorld(float DeltaSeconds);

	bool FocusViewportToTargets();

private:
	int IndexOfClickSelectUI = -1;
	int MouseX = 0, MouseY = 0;

	TWeakPtr<FLGUIPrefabEditor> PrefabEditorPtr;
	// Are we currently manipulating something?
	bool bManipulating = false;
	FTrackingTransaction TrackingTransaction;
	/**
	 * true when a brush is being transformed by its Widget
	 */
	bool					bIsTrackingBrushModification;

	ULGUIPrefab* GetPrefabBeingEdited()const;
	/**
	 * Collects the set of components and actors on which to apply move operations during or after drag operations.
	 */
	void GetSelectedActorsAndComponentsForMove(TArray<AActor*>& OutActorsToMove, TArray<USceneComponent*>& OutComponentsToMove) const;
	/**
	 * Determines if it is valid to move an actor in this viewport.
	 *
	 * @param InActor - the actor that the viewport may be interested in moving.
	 * @returns true if it is valid for this viewport to update the given actor's transform.
	 */
	bool CanMoveActorInViewport(const AActor* InActor) const;

	/** @return	Returns true if the delta tracker was used to modify any selected actors or BSP.  Must be called before EndTracking(). */
	bool HaveSelectedObjectsBeenChanged() const;
};