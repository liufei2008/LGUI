// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PreviewScene.h"
#include "EditorViewportClient.h"
#include "Animation/AnimInstance.h"

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
	// End of FEditorViewportClient interface

	void BeginTransaction(const FText& SessionName);
	void MarkTransactionAsDirty();
	void EndTransaction();

	virtual bool InputWidgetDelta(FViewport* InViewport, EAxisList::Type InCurrentAxis, FVector& Drag, FRotator& Rot, FVector& Scale) override;

	void ApplyDeltaToActors(const FVector& InDrag, const FRotator& InRot, const FVector& InScale);
	void ApplyDeltaToActor(AActor* InActor, const FVector& InDeltaDrag, const FRotator& InDeltaRot, const FVector& InDeltaScale);

	void TickWorld(float DeltaSeconds);

	virtual void SetWidgetMode(FWidget::EWidgetMode NewMode) override;

	bool FocusViewportToTargets();

private:

	// The current transaction for undo/redo
	class FScopedTransaction* ScopedTransaction = nullptr;
	TWeakPtr<FLGUIPrefabEditor> PrefabEditorPtr;
	// Are we currently manipulating something?
	bool bManipulating = false;

	ULGUIPrefab* GetPrefabBeingEdited()const;

	bool LastCameraAutoFocus;
	bool LastAutoFocusRes;

	TWeakObjectPtr<UUIBaseRenderable> LastSelectTarget;
	TWeakObjectPtr<AActor> LastSelectedActor;

};