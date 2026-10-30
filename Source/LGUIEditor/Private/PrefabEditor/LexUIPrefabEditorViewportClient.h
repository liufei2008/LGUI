// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorViewportClient.h"

class FLexUIPrefabEditor;
class ULexUIPrefab;
class FLexUITransformWidget;

/** Viewport client for the prefab editor viewport. */
class FLexUIPrefabEditorViewportClient : public FEditorViewportClient
{
public:
	FLexUIPrefabEditorViewportClient(TWeakPtr<FLexUIPrefabEditor> InPrefabEditorPtr, const TSharedRef<class SLexUIPrefabEditorViewport>& InEditorViewportPtr);
	virtual ~FLexUIPrefabEditorViewportClient()override;

	// FEditorViewportClient interface
	virtual void ProcessClick(FSceneView& View, HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY) override;
	virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;
	virtual void CapturedMouseMove(FViewport* InViewport, int32 InMouseX, int32 InMouseY) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual bool InputWidgetDelta(FViewport* InViewport, EAxisList::Type InCurrentAxis, FVector& Drag, FRotator& Rot, FVector& Scale) override;
	virtual void SetViewportType(ELevelViewportType InViewportType) override;

	// Begin override because PreviewScene is nullptr
	virtual UWorld* GetWorld()const override;
	// End

	bool FocusViewportToTargets();
private:
	int PrevMouseX = 0, PrevMouseY = 0;
	int IndexOfClickSelectUI = INDEX_NONE;
	TUniquePtr<FLexUITransformWidget> TransformWidget = nullptr;
	FDelegateHandle OnSelectionChangedDelegateHandle;

	TWeakPtr<FLexUIPrefabEditor> PrefabEditorPtr;

	ULexUIPrefab* GetPrefabBeingEdited()const;
};
