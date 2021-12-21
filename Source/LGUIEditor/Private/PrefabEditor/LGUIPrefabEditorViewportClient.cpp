// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "LGUIPrefabEditorViewportClient.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SphereReflectionCaptureComponent.h"
#include "SceneInterface.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/Material.h"
#include "Engine/StaticMesh.h"
#include "Animation/AnimationAsset.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Character.h"
#include "Math/Vector.h"
#include "Viewports.h"
#include "Components/ShapeComponent.h"
#include "Components/DrawFrustumComponent.h"
#include "AssetEditorModeManager.h"
#include "EngineUtils.h"
#include "EdMode.h"
#include "Engine/Selection.h"
#include "Misc/CoreDelegates.h"
#include "SceneView.h"
#include "Dialogs/Dialogs.h"
#include "Editor/UnrealEdEngine.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Editor.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "LGUIPrefabPreviewScene.h"
#include "LGUIHeaders.h"
#include "ScopedTransaction.h"
#include "LGUIPrefabEditor.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabEditorViewportClient"


FLGUIPrefabEditorViewportClient::FLGUIPrefabEditorViewportClient(FLGUIPrefabPreviewScene& InPreviewScene
	, TWeakPtr<FLGUIPrefabEditor> InPrefabEditorPtr
	, const TSharedRef<SLGUIPrefabEditorViewport>& InEditorViewportPtr)
	: FEditorViewportClient(&GLevelEditorModeTools(), &InPreviewScene, StaticCastSharedRef<SEditorViewport>(InEditorViewportPtr))
{
	this->PrefabEditorPtr = InPrefabEditorPtr;

	GEditor->SelectNone(true, true);

	// The level editor fully supports mode tools and isn't doing any incompatible stuff with the Widget
	ModeTools->SetWidgetMode(FWidget::WM_Translate);
	Widget->SetUsesEditorModeTools(ModeTools);
	((FAssetEditorModeManager*)ModeTools)->SetPreviewScene(PreviewScene);

	// GEditorModeTools serves as our draw helper
	bUsesDrawHelper = false;

	// DrawHelper set up

	//DrawHelper.PerspectiveGridSize = HALF_WORLD_MAX1;
	//DrawHelper.AxesLineThickness = 0.0f;
	//DrawHelper.bDrawGrid = true;

	EngineShowFlags.Game = 0;
	EngineShowFlags.ScreenSpaceReflections = 1;
	EngineShowFlags.AmbientOcclusion = 1;
	EngineShowFlags.SetSnap(0);

	SetRealtime(true);

	EngineShowFlags.DisableAdvancedFeatures();
	EngineShowFlags.SetSeparateTranslucency(true);
	EngineShowFlags.SetCompositeEditorPrimitives(true);
	EngineShowFlags.SetParticles(true);
	EngineShowFlags.SetSelection(true);
	EngineShowFlags.SetSelectionOutline(true);

	if (UWorld* PreviewWorld = this->GetWorld())
	{
		PreviewWorld->bAllowAudioPlayback = false;
	}

	SetViewLocation(FVector(-300, 0, 0));
	SetViewRotation(FRotator::ZeroRotator);
}

FLGUIPrefabEditorViewportClient::~FLGUIPrefabEditorViewportClient()
{
}

void FLGUIPrefabEditorViewportClient::Draw(FViewport* InViewport, FCanvas* Canvas)
{
	FEditorViewportClient::Draw(InViewport, Canvas);
}

void FLGUIPrefabEditorViewportClient::Tick(float DeltaSeconds)
{

	FEditorViewportClient::Tick(DeltaSeconds);

	float RequestDelta = DeltaSeconds;
	{
		TickWorld(RequestDelta);
	}
}


bool FLGUIPrefabEditorViewportClient::InputKey(FViewport* InViewport, int32 ControllerId, FKey Key, EInputEvent Event, float AmountDepressed, bool bGamepad)
{
	bool Res = FEditorViewportClient::InputKey(InViewport, ControllerId, Key, Event, AmountDepressed, bGamepad);

	if (Key == EKeys::F && Event == IE_Pressed)
	{
		//FocusViewportToTargets();
	}

	return Res;
}

void FLGUIPrefabEditorViewportClient::ProcessClick(FSceneView& View, HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY)
{
	if (ULGUIEditorManagerObject::Instance != nullptr)
	{
		FVector RayOrigin, RayDirection;
		View.DeprojectScreenToWorld(FVector2D(HitX, HitY), View.UnscaledViewRect, View.ViewMatrices.GetInvViewProjectionMatrix(), RayOrigin, RayDirection);
		float LineTraceLength = 100000;
		//find hit UIBatchGeometryRenderable
		auto LineStart = RayOrigin;
		auto LineEnd = RayOrigin + RayDirection * LineTraceLength;
		auto& allUIItems = ULGUIEditorManagerObject::Instance->GetAllUIItem();
		auto prevSelectTarget = LastSelectTarget;
		auto prevSelectActor = LastSelectedActor;
		if (ULGUIEditorManagerObject::RaycastHitUI(this->GetWorld(), allUIItems, LineStart, LineEnd, prevSelectTarget, prevSelectActor, LastSelectTarget, LastSelectedActor))
		{
			GEditor->SelectNone(true, true);
			GEditor->SelectActor(LastSelectedActor.Get(), true, true);
		}
	}


	const FViewportClick Click(&View, this, Key, Event, HitX, HitY);
	if (!ModeTools->HandleClick(this, HitProxy, Click))
	{
		if (HitProxy == NULL)
		{
			GEditor->SelectNone(true, true);
			//SetWidgetMode(FWidget::WM_None);
			return;
		}
		if (HitProxy->IsA(HActor::StaticGetType()))
		{
			HActor* ActorHitProxy = (HActor*)HitProxy;
			AActor* Actor = ActorHitProxy->Actor;
			while (Actor->IsChildActor())
			{
				Actor = Actor->GetParentActor();
			}
			if (Click.GetKey() == EKeys::LeftMouseButton && Actor)
			{
				SetWidgetMode(FWidget::WM_Translate);
			}
		}
	}

}

bool FLGUIPrefabEditorViewportClient::InputWidgetDelta(FViewport* InViewport, EAxisList::Type InCurrentAxis, FVector& Drag, FRotator& Rot, FVector& Scale)
{
	bool bHandled = false;

	// Give the current editor mode a chance to use the input first.  If it does, don't apply it to anything else.
	if (FEditorViewportClient::InputWidgetDelta(InViewport, InCurrentAxis, Drag, Rot, Scale))
	{
		bHandled = true;
	}
	else
	{
		if (InCurrentAxis != EAxisList::None)
		{
			// Skip actors transformation routine in case if any of the selected actors locked
			// but still pretend that we have handled the input
			if (!GEditor->HasLockedActors())
			{
				const bool LeftMouseButtonDown = InViewport->KeyState(EKeys::LeftMouseButton);
				const bool RightMouseButtonDown = InViewport->KeyState(EKeys::RightMouseButton);
				const bool MiddleMouseButtonDown = InViewport->KeyState(EKeys::MiddleMouseButton);

				// We do not want actors updated if we are holding down the middle mouse button.
				if (!MiddleMouseButtonDown)
				{
					ApplyDeltaToActors(Drag, Rot, Scale);
					ApplyDeltaToRotateWidget(Rot);
				}

				ModeTools->PivotLocation += Drag;
				ModeTools->SnappedLocation += Drag;

				if (IsShiftPressed())
				{
					FVector CameraDelta(Drag);
					MoveViewportCamera(CameraDelta, FRotator::ZeroRotator);
				}

				// zachma todo
				//TArray<FEdMode*> ActiveModes;
				//ModeTools->GetActiveModes(ActiveModes);

				//for (int32 ModeIndex = 0; ModeIndex < ActiveModes.Num(); ++ModeIndex)
				//{
				//	ActiveModes[ModeIndex]->UpdateInternalData();
				//}
			}

			bHandled = true;
		}

	}

	return bHandled;
}

void FLGUIPrefabEditorViewportClient::ApplyDeltaToActors(const FVector& InDrag, const FRotator& InRot, const FVector& InScale)
{
	if ((InDrag.IsZero() && InRot.IsZero() && InScale.IsZero()))
	{
		return;
	}

	FVector ModifiedScale = InScale;
	// If we are scaling, we need to change the scaling factor a bit to properly align to grid.
	if (GEditor->UsePercentageBasedScaling())
	{
		USelection* SelectedActors = GEditor->GetSelectedActors();
		const bool bScalingActors = !InScale.IsNearlyZero();

		if (bScalingActors)
		{
			ModifiedScale = InScale * ((GEditor->GetScaleGridSize() / 100.0f) / GEditor->GetGridSize());
		}
	}

	// Transact the actors.
	GEditor->NoteActorMovement();

	// Apply the deltas to any selected actors.
	for (FSelectionIterator SelectedActorIt(GEditor->GetSelectedActorIterator()); SelectedActorIt; ++SelectedActorIt)
	{
		AActor* Actor = static_cast<AActor*>(*SelectedActorIt);
		checkSlow(Actor->IsA(AActor::StaticClass()));

		if (!Actor->bLockLocation)
		{
			// Finally, verify that no actor in the parent hierarchy is also selected
			bool bHasParentInSelection = false;
			AActor* ParentActor = Actor->GetAttachParentActor();
			while (ParentActor != NULL && !bHasParentInSelection)
			{
				if (ParentActor->IsSelected())
				{
					bHasParentInSelection = true;
				}
				ParentActor = ParentActor->GetAttachParentActor();
			}
			if (!bHasParentInSelection)
			{
				ApplyDeltaToActor(Actor, InDrag, InRot, ModifiedScale);	
			}
		}
	}
}

void FLGUIPrefabEditorViewportClient::ApplyDeltaToActor(AActor* InActor, const FVector& InDeltaDrag, const FRotator& InDeltaRot, const FVector& InDeltaScale)
{
	// If we are scaling, we may need to change the scaling factor a bit to properly align to the grid.
	FVector ModifiedDeltaScale = InDeltaScale;

	// we dont scale actors when we only have a very small scale change
	if (InDeltaScale.IsNearlyZero())
	{
		ModifiedDeltaScale = FVector::ZeroVector;
	}

	GEditor->ApplyDeltaToActor(
		InActor,
		true,
		&InDeltaDrag,
		&InDeltaRot,
		&ModifiedDeltaScale,
		IsAltPressed(),
		IsShiftPressed(),
		IsCtrlPressed());
}


void FLGUIPrefabEditorViewportClient::TickWorld(float DeltaSeconds)
{
	PreviewScene->GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
}

void FLGUIPrefabEditorViewportClient::SetWidgetMode(FWidget::EWidgetMode NewMode)
{
	FEditorViewportClient::SetWidgetMode(NewMode);
}

bool FLGUIPrefabEditorViewportClient::FocusViewportToTargets()
{
	FIntPoint ViewportSize(FIntPoint::ZeroValue);
	if (Viewport != nullptr)
	{
		ViewportSize = Viewport->GetSizeXY();
	}

	if (ViewportSize.SizeSquared() <= 0)
	{
		return false;
	}

	if (!PrefabEditorPtr.IsValid())
	{
		return false;
	}

	FBoxSphereBounds Bounds;
	if (PrefabEditorPtr.Pin()->GetSelectedObjectsBounds(Bounds))
	{
		auto TargetLocation = Bounds.GetSphere().Center;
		this->CenterViewportAtPoint(TargetLocation);
		auto ViewDirection = this->GetViewRotation().RotateVector(FVector(1, 0, 0));
		auto Distance = Bounds.GetSphere().W;
		this->SetViewLocation(TargetLocation - ViewDirection * Distance * 1.5f);
	}

	return false;
}

ULGUIPrefab* FLGUIPrefabEditorViewportClient::GetPrefabBeingEdited()const
{
	return PrefabEditorPtr.Pin()->GetPrefabBeingEdited();
}

void FLGUIPrefabEditorViewportClient::TrackingStarted(const struct FInputEventState& InInputState, bool bIsDragging, bool bNudge)
{
	//@TODO: Should push this into FEditorViewportClient
	// Begin transacting.  Give the current editor mode an opportunity to do the transacting.
	const bool bTrackingHandledExternally = ModeTools->StartTracking(this, Viewport);

	if (!bManipulating && !bTrackingHandledExternally)
	{
		BeginTransaction(LOCTEXT("ModificationInViewportTransaction", "Modification in Viewport"));
		bManipulating = true;
	}
}
void FLGUIPrefabEditorViewportClient::TrackingStopped()
{
	// Stop transacting.  Give the current editor mode an opportunity to do the transacting.
	const bool bTransactingHandledByEditorMode = ModeTools->EndTracking(this, Viewport);

	if (bManipulating && !bTransactingHandledByEditorMode)
	{
		EndTransaction();
		bManipulating = false;
	}
}

void FLGUIPrefabEditorViewportClient::BeginTransaction(const FText& SessionName)
{
	if (ScopedTransaction == nullptr)
	{
		ScopedTransaction = new FScopedTransaction(SessionName);

		//auto Prefab = GetPrefabBeingEdited();
		//Prefab->Modify();
	}
}
void FLGUIPrefabEditorViewportClient::MarkTransactionAsDirty()
{
	Invalidate();
	//@TODO: Can add a call to Sprite->PostEditChange here if we want to update the baked sprite data during a drag operation
	// (maybe passing in Interactive - if so, the EndTransaction PostEditChange needs to be a ValueSet)
}
void FLGUIPrefabEditorViewportClient::EndTransaction()
{
	//auto Prefab = GetPrefabBeingEdited();
	//Prefab->PostEditChange();

	if (ScopedTransaction != nullptr)
	{
		delete ScopedTransaction;
		ScopedTransaction = nullptr;
	}
}

#undef LOCTEXT_NAMESPACE
