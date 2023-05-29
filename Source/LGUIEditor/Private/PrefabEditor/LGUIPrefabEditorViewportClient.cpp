// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LGUIPrefabEditorViewportClient.h"
#include "LGUIPrefabEditorViewport.h"
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
#include "LGUIPrefabEditor.h"
#include "MouseDeltaTracker.h"
#include "UnrealEdGlobals.h"
#include "Editor/EditorPerProjectUserSettings.h"
#include "UnrealWidget.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabEditorViewportClient"


FLGUIPrefabEditorViewportClient::FLGUIPrefabEditorViewportClient(FLGUIPrefabPreviewScene& InPreviewScene
	, TWeakPtr<FLGUIPrefabEditor> InPrefabEditorPtr
	, const TSharedRef<SLGUIPrefabEditorViewport>& InEditorViewportPtr)
	: FEditorViewportClient(&GLevelEditorModeTools(), &InPreviewScene, StaticCastSharedRef<SEditorViewport>(InEditorViewportPtr))
	, TrackingTransaction()
{
	this->PrefabEditorPtr = InPrefabEditorPtr;

	GEditor->SelectNone(true, true);

	// The level editor fully supports mode tools and isn't doing any incompatible stuff with the Widget
	ModeTools->SetWidgetMode(UE::Widget::WM_Translate);
	Widget->SetUsesEditorModeTools(ModeTools.Get());

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

	FVector InitialViewLocation;
	FRotator InitialViewRotation;
	FVector InitialViewOrbitLocation;
	InPrefabEditorPtr.Pin()->GetInitialViewLocationAndRotation(InitialViewLocation, InitialViewRotation, InitialViewOrbitLocation);
	SetViewLocation(InitialViewLocation);
	SetViewRotation(InitialViewRotation);
	SetLookAtLocation(InitialViewOrbitLocation);
}

FLGUIPrefabEditorViewportClient::~FLGUIPrefabEditorViewportClient()
{
}

void FLGUIPrefabEditorViewportClient::Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	FEditorViewportClient::Draw(View, PDI);

	if (GUnrealEd != nullptr)
	{
		GUnrealEd->DrawComponentVisualizers(View, PDI);
		//bool bHitTesting = PDI->IsHitTesting();
		//auto AllActors = PrefabEditorPtr.Pin()->GetAllActors();
		//for (auto& PreviewActor : AllActors)
		//{
		//	if (ULGUIEditorManagerObject::IsSelected(PreviewActor))
		//	{
		//		auto& Components = PreviewActor->GetComponents();
		//		for (auto& Comp : Components)
		//		{
		//			if (IsValid(Comp) && Comp->IsRegistered())
		//			{
		//				// Try and find a visualizer
		//				TSharedPtr<FComponentVisualizer> Visualizer = GUnrealEd->FindComponentVisualizer(Comp->GetClass());
		//				if (Visualizer.IsValid())
		//				{
		//					Visualizer->DrawVisualization(Comp, View, PDI);
		//				}
		//			}
		//		}
		//	}
		//}
	}
}
void FLGUIPrefabEditorViewportClient::Draw(FViewport* InViewport, FCanvas* Canvas)
{
	FEditorViewportClient::Draw(InViewport, Canvas);
}
void FLGUIPrefabEditorViewportClient::DrawCanvas(FViewport& InViewport, FSceneView& View, FCanvas& Canvas)
{
	FEditorViewportClient::DrawCanvas(InViewport, View, Canvas);
	if (GUnrealEd != nullptr)
	{
		GUnrealEd->DrawComponentVisualizersHUD(&InViewport, &View, &Canvas);
		//auto AllActors = PrefabEditorPtr.Pin()->GetAllActors();
		//for (auto& PreviewActor : AllActors)
		//{
		//	if (ULGUIEditorManagerObject::IsSelected(PreviewActor))
		//	{
		//		auto& Components = PreviewActor->GetComponents();
		//		for (auto& Comp : Components)
		//		{
		//			if (IsValid(Comp) && Comp->IsRegistered())
		//			{
		//				// Try and find a visualizer
		//				TSharedPtr<FComponentVisualizer> Visualizer = GUnrealEd->FindComponentVisualizer(Comp->GetClass());
		//				if (Visualizer.IsValid())
		//				{
		//					Visualizer->DrawVisualizationHUD(Comp, &InViewport, &View, &Canvas);
		//				}
		//			}
		//		}
		//	}
		//}
	}
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
	bool bHandled = GUnrealEd->ComponentVisManager.HandleInputKey(this, InViewport, Key, Event);
	if (!bHandled)
	{
		bool Res = FEditorViewportClient::InputKey(InViewport, ControllerId, Key, Event, AmountDepressed, bGamepad);

		if (Key == EKeys::F)
		{
			if (Event == IE_Pressed)
			{
				bHandled = FocusViewportToTargets();
			}
		}
	}

	return bHandled;
}

void FLGUIPrefabEditorViewportClient::ProcessClick(FSceneView& View, HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY)
{
	const FViewportClick Click(&View, this, Key, Event, HitX, HitY);
	if (GUnrealEd->ComponentVisManager.HandleClick(this, HitProxy, Click))
	{
		return;
	}

	AActor* ClickHitActor = nullptr;
	if (auto ManagerActor = ALGUIManagerActor::GetInstance(this->GetWorld(), false))
	{
		FVector RayOrigin, RayDirection;
		View.DeprojectScreenToWorld(FVector2D(HitX, HitY), View.UnscaledViewRect, View.ViewMatrices.GetInvViewProjectionMatrix(), RayOrigin, RayDirection);
		float LineTraceLength = 100000;
		//find hit UIBatchGeometryRenderable
		auto LineStart = RayOrigin;
		auto LineEnd = RayOrigin + RayDirection * LineTraceLength;
		auto& AllCanvasArray = ManagerActor->GetCanvasArray();
		UUIBaseRenderable* ClickHitUI = nullptr;
		static TArray<UUIItem*> AllUIItemArray;
		AllUIItemArray.Reset();
		for (auto& CanvasItem : AllCanvasArray)
		{
			AllUIItemArray.Append(CanvasItem->GetUIItemArray());
		}
		if (ULGUIEditorManagerObject::RaycastHitUI(this->GetWorld(), AllUIItemArray, LineStart, LineEnd, ClickHitUI, IndexOfClickSelectUI))
		{
			ClickHitActor = ClickHitUI->GetOwner();
		}
	}
	if (ClickHitActor == nullptr)
	{
		if (!ModeTools->HandleClick(this, HitProxy, Click))
		{
			if (HitProxy == NULL)
			{
				GEditor->SelectNone(true, true);
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
				ClickHitActor = Actor;
			}
		}
	}

	{
		GEditor->BeginTransaction(LOCTEXT("ClickToPickActor_Transaction", "Click to pick UI item"));
		GEditor->GetSelectedActors()->Modify();

		// Clear the selection if not multiple selection.
		if (!IsCtrlPressed() && !IsShiftPressed())
		{
			GEditor->SelectNone(false, true, true);
		}

		// We'll batch selection changes instead by using BeginBatchSelectOperation()
		GEditor->GetSelectedActors()->BeginBatchSelectOperation();

		if (ClickHitActor != nullptr)
		{
			GEditor->SelectActor(ClickHitActor, true, false);
		}

		// Commit selection changes
		GEditor->GetSelectedActors()->EndBatchSelectOperation(/*bNotify*/false);
		// Fire selection changed event
		GEditor->NoteSelectionChange();
		GEditor->EndTransaction();
	}
}

bool FLGUIPrefabEditorViewportClient::InputWidgetDelta(FViewport* InViewport, EAxisList::Type InCurrentAxis, FVector& Drag, FRotator& Rot, FVector& Scale)
{
	if (GUnrealEd->ComponentVisManager.IsActive() && GUnrealEd->ComponentVisManager.HandleInputDelta(this, InViewport, Drag, Rot, Scale))
	{
		return true;
	}

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
UE::Widget::EWidgetMode FLGUIPrefabEditorViewportClient::GetWidgetMode() const
{
	if (GUnrealEd->ComponentVisManager.IsActive() && GUnrealEd->ComponentVisManager.IsVisualizingArchetype())
	{
		return UE::Widget::WM_None;
	}

	return FEditorViewportClient::GetWidgetMode();
}
FVector FLGUIPrefabEditorViewportClient::GetWidgetLocation() const
{
	FVector ComponentVisWidgetLocation;
	if (GUnrealEd->ComponentVisManager.GetWidgetLocation(this, ComponentVisWidgetLocation))
	{
		return ComponentVisWidgetLocation;
	}

	return FEditorViewportClient::GetWidgetLocation();
}
FMatrix FLGUIPrefabEditorViewportClient::GetWidgetCoordSystem() const
{
	FMatrix ComponentVisWidgetCoordSystem;
	if (GUnrealEd->ComponentVisManager.GetCustomInputCoordinateSystem(this, ComponentVisWidgetCoordSystem))
	{
		return ComponentVisWidgetCoordSystem;
	}

	return FEditorViewportClient::GetWidgetCoordSystem();
}
int32 FLGUIPrefabEditorViewportClient::GetCameraSpeedSetting() const
{
	return GetDefault<UEditorPerProjectUserSettings>()->SCSViewportCameraSpeed;
}
void FLGUIPrefabEditorViewportClient::SetCameraSpeedSetting(int32 SpeedSetting)
{
	GetMutableDefault<UEditorPerProjectUserSettings>()->SCSViewportCameraSpeed = SpeedSetting;
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
		if (!Actor->IsLockLocation())
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

bool FLGUIPrefabEditorViewportClient::FocusViewportToTargets()
{
	if (!PrefabEditorPtr.IsValid())
	{
		return false;
	}

	FBoxSphereBounds Bounds = FBoxSphereBounds(EForceInit::ForceInitToZero);
	if (!PrefabEditorPtr.Pin()->GetSelectedObjectsBounds(Bounds))
	{
		Bounds = PrefabEditorPtr.Pin()->GetAllObjectsBounds();
	}
	FocusViewportOnBox(Bounds.GetBox());

	return false;
}

ULGUIPrefab* FLGUIPrefabEditorViewportClient::GetPrefabBeingEdited()const
{
	return PrefabEditorPtr.Pin()->GetPrefabBeingEdited();
}

namespace LevelEditorViewportClientHelper
{
	FProperty* GetEditTransformProperty(UE::Widget::EWidgetMode WidgetMode)
	{
		FProperty* ValueProperty = nullptr;
		switch (WidgetMode)
		{
		case UE::Widget::WM_Translate:
			ValueProperty = FindFProperty<FProperty>(USceneComponent::StaticClass(), USceneComponent::GetRelativeLocationPropertyName());
			break;
		case UE::Widget::WM_Rotate:
			ValueProperty = FindFProperty<FProperty>(USceneComponent::StaticClass(), USceneComponent::GetRelativeRotationPropertyName());
			break;
		case UE::Widget::WM_Scale:
			ValueProperty = FindFProperty<FProperty>(USceneComponent::StaticClass(), USceneComponent::GetRelativeScale3DPropertyName());
			break;
		case UE::Widget::WM_TranslateRotateZ:
			ValueProperty = FindFProperty<FProperty>(USceneComponent::StaticClass(), USceneComponent::GetRelativeLocationPropertyName());
			break;
		case UE::Widget::WM_2D:
			ValueProperty = FindFProperty<FProperty>(USceneComponent::StaticClass(), USceneComponent::GetRelativeLocationPropertyName());
			break;
		default:
			break;
		}
		return ValueProperty;
	}
}

void FLGUIPrefabEditorViewportClient::GetSelectedActorsAndComponentsForMove(TArray<AActor*>& OutActorsToMove, TArray<USceneComponent*>& OutComponentsToMove) const
{
	OutActorsToMove.Reset();
	OutComponentsToMove.Reset();

	// Get the list of parent-most component(s) that are selected
	if (GEditor->GetSelectedComponentCount() > 0)
	{
		// Otherwise, if both a parent and child are selected and the delta is applied to both, the child will actually move 2x delta
		for (FSelectedEditableComponentIterator EditableComponentIt(GEditor->GetSelectedEditableComponentIterator()); EditableComponentIt; ++EditableComponentIt)
		{
			USceneComponent* SceneComponent = Cast<USceneComponent>(*EditableComponentIt);
			if (!SceneComponent)
			{
				continue;
			}

			// Check to see if any parent is selected
			bool bParentAlsoSelected = false;
			USceneComponent* Parent = SceneComponent->GetAttachParent();
			while (Parent != nullptr)
			{
				if (Parent->IsSelected())
				{
					bParentAlsoSelected = true;
					break;
				}

				Parent = Parent->GetAttachParent();
			}

			AActor* ComponentOwner = SceneComponent->GetOwner();
			if (!CanMoveActorInViewport(ComponentOwner))
			{
				continue;
			}

			const bool bIsRootComponent = (ComponentOwner && (ComponentOwner->GetRootComponent() == SceneComponent));
			if (bIsRootComponent)
			{
				// If it is a root component, use the parent actor instead
				OutActorsToMove.Add(ComponentOwner);
			}
			else if (!bParentAlsoSelected)
			{
				// If no parent of this component is also in the selection set, move it
				OutComponentsToMove.Add(SceneComponent);
			}
		}
	}

	// Skip gathering selected actors if we had a valid component selection
	if (OutComponentsToMove.Num() || OutActorsToMove.Num())
	{
		return;
	}

	for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
	{
		AActor* Actor = CastChecked<AActor>(*It);

		// If the root component was selected, this actor is already accounted for
		USceneComponent* RootComponent = Actor->GetRootComponent();
		if (RootComponent && RootComponent->IsSelected())
		{
			continue;
		}

		if (!CanMoveActorInViewport(Actor))
		{
			continue;
		}

		OutActorsToMove.Add(Actor);
	}
}

bool FLGUIPrefabEditorViewportClient::CanMoveActorInViewport(const AActor* InActor) const
{
	if (!GEditor || !InActor)
	{
		return false;
	}

	// The actor cannot be location locked
	if (InActor->IsLockLocation())
	{
		return false;
	}

	// The actor needs to be in the current viewport world
	if (GEditor->PlayWorld)
	{
		if (bIsSimulateInEditorViewport)
		{
			// If the Actor's outer (level) outer (world) is not the PlayWorld then it cannot be moved in this viewport.
			if (!(GEditor->PlayWorld == InActor->GetOuter()->GetOuter()))
			{
				return false;
			}
		}
		else if (!(GEditor->EditorWorld == InActor->GetOuter()->GetOuter()))
		{
			return false;
		}
	}

	return true;
}

#include "UnrealWidget.h"

void FLGUIPrefabEditorViewportClient::CapturedMouseMove(FViewport* InViewport, int32 InMouseX, int32 InMouseY)
{
	// Commit to any pending transactions now
	TrackingTransaction.PromotePendingToActive();

	FEditorViewportClient::CapturedMouseMove(InViewport, InMouseX, InMouseY);

	if (InMouseX != MouseX || InMouseY != MouseY)
	{
		IndexOfClickSelectUI = INDEX_NONE;
	}
	MouseX = InMouseX;
	MouseY = InMouseY;
}

void FLGUIPrefabEditorViewportClient::TrackingStarted(const struct FInputEventState& InInputState, bool bIsDraggingWidget, bool bNudge)
{
	// Begin transacting.  Give the current editor mode an opportunity to do the transacting.
	const bool bTrackingHandledExternally = ModeTools->StartTracking(this, Viewport);

	TrackingTransaction.End();

	const bool bIsDraggingComponents = GEditor->GetSelectedComponentCount() > 0;

	// Create edit property event
	FEditPropertyChain PropertyChain;
	FProperty* TransformProperty = LevelEditorViewportClientHelper::GetEditTransformProperty(GetWidgetMode());
	if (TransformProperty)
	{
		PropertyChain.AddHead(TransformProperty);
	}

	if (bIsDraggingComponents)
	{
		if (bIsDraggingWidget)
		{
			Widget->SetSnapEnabled(true);

			for (FSelectedEditableComponentIterator It(GEditor->GetSelectedEditableComponentIterator()); It; ++It)
			{
				USceneComponent* SceneComponent = Cast<USceneComponent>(*It);
				if (SceneComponent)
				{
					// Notify that this component is beginning to move
					GEditor->BroadcastBeginObjectMovement(*SceneComponent);

					// Broadcast Pre Edit change notification, we can't call PreEditChange directly on Actor or ActorComponent from here since it will unregister the components until PostEditChange
					if (TransformProperty)
					{
						FCoreUObjectDelegates::OnPreObjectPropertyChanged.Broadcast(SceneComponent, PropertyChain);
					}
				}
			}
		}
	}
	else
	{
		for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It && !bIsTrackingBrushModification; ++It)
		{
			AActor* Actor = CastChecked<AActor>(*It);

			if (bIsDraggingWidget)
			{
				// Notify that this actor is beginning to move
				GEditor->BroadcastBeginObjectMovement(*Actor);

				// Broadcast Pre Edit change notification, we can't call PreEditChange directly on Actor or ActorComponent from here since it will unregister the components until PostEditChange
				if (TransformProperty)
				{
					FCoreUObjectDelegates::OnPreObjectPropertyChanged.Broadcast(Actor, PropertyChain);
				}
			}

			Widget->SetSnapEnabled(true);
		}
	}

	// Start a transformation transaction if required
	if (!bTrackingHandledExternally)
	{
		if (bIsDraggingWidget)
		{
			TrackingTransaction.TransCount++;

			FText ObjectTypeBeingTracked = bIsDraggingComponents ? LOCTEXT("TransactionFocus_Components", "Components") : LOCTEXT("TransactionFocus_Actors", "Actors");
			FText TrackingDescription;

			switch (GetWidgetMode())
			{
			case UE::Widget::WM_Translate:
				TrackingDescription = FText::Format(LOCTEXT("MoveTransaction", "Move {0}"), ObjectTypeBeingTracked);
				break;
			case UE::Widget::WM_Rotate:
				TrackingDescription = FText::Format(LOCTEXT("RotateTransaction", "Rotate {0}"), ObjectTypeBeingTracked);
				break;
			case UE::Widget::WM_Scale:
				TrackingDescription = FText::Format(LOCTEXT("ScaleTransaction", "Scale {0}"), ObjectTypeBeingTracked);
				break;
			case UE::Widget::WM_TranslateRotateZ:
				TrackingDescription = FText::Format(LOCTEXT("TranslateRotateZTransaction", "Translate/RotateZ {0}"), ObjectTypeBeingTracked);
				break;
			case UE::Widget::WM_2D:
				TrackingDescription = FText::Format(LOCTEXT("TranslateRotate2D", "Translate/Rotate2D {0}"), ObjectTypeBeingTracked);
				break;
			default:
				if (bNudge)
				{
					TrackingDescription = FText::Format(LOCTEXT("NudgeTransaction", "Nudge {0}"), ObjectTypeBeingTracked);
				}
			}

			if (!TrackingDescription.IsEmpty())
			{
				if (bNudge)
				{
					TrackingTransaction.Begin(TrackingDescription);
				}
				else
				{
					// If this hasn't begun due to a nudge, start it as a pending transaction so that it only really begins when the mouse is moved
					TrackingTransaction.BeginPending(TrackingDescription);
				}
			}
		}

		if (TrackingTransaction.IsActive() || TrackingTransaction.IsPending())
		{
			// Suspend actor/component modification during each delta step to avoid recording unnecessary overhead into the transaction buffer
			GEditor->DisableDeltaModification(true);
		}
	}
}
void FLGUIPrefabEditorViewportClient::TrackingStopped()
{
	const bool AltDown = IsAltPressed();
	const bool ShiftDown = IsShiftPressed();
	const bool ControlDown = IsCtrlPressed();
	const bool LeftMouseButtonDown = Viewport->KeyState(EKeys::LeftMouseButton);
	const bool RightMouseButtonDown = Viewport->KeyState(EKeys::RightMouseButton);
	const bool MiddleMouseButtonDown = Viewport->KeyState(EKeys::MiddleMouseButton);

	// here we check to see if anything of worth actually changed when ending our MouseMovement
	// If the TransCount > 0 (we changed something of value) so we need to call PostEditMove() on stuff
	// if we didn't change anything then don't call PostEditMove()
	bool bDidAnythingActuallyChange = false;

	// Stop transacting.  Give the current editor mode an opportunity to do the transacting.
	const bool bTransactingHandledByEditorMode = ModeTools->EndTracking(this, Viewport);
	if (!bTransactingHandledByEditorMode)
	{
		if (TrackingTransaction.TransCount > 0)
		{
			bDidAnythingActuallyChange = true;
			TrackingTransaction.TransCount--;
		}
	}

	// Notify the selected actors that they have been moved.
	// Don't do this if AddDelta was never called.
	if (bDidAnythingActuallyChange && MouseDeltaTracker->HasReceivedDelta())
	{
		// Create post edit property change event
		FProperty* TransformProperty = LevelEditorViewportClientHelper::GetEditTransformProperty(GetWidgetMode());
		FPropertyChangedEvent PropertyChangedEvent(TransformProperty, EPropertyChangeType::ValueSet);

		// Move components and actors
		{
			TArray<USceneComponent*> ComponentsToMove;
			TArray<AActor*> ActorsToMove;
			GetSelectedActorsAndComponentsForMove(ActorsToMove, ComponentsToMove);

			for (USceneComponent* Component : ComponentsToMove)
			{
				// Broadcast Post Edit change notification, we can't call PostEditChangeProperty directly on Actor or ActorComponent from here since it wasn't pair with a proper PreEditChange
				FCoreUObjectDelegates::OnObjectPropertyChanged.Broadcast(Component, PropertyChangedEvent);
				
				Component->PostEditComponentMove(true);
				GEditor->BroadcastEndObjectMovement(*Component);
			}

			for (AActor* Actor : ActorsToMove)
			{
				// Broadcast Post Edit change notification, we can't call PostEditChangeProperty directly on Actor or ActorComponent from here since it wasn't pair with a proper PreEditChange
				FCoreUObjectDelegates::OnObjectPropertyChanged.Broadcast(Actor, PropertyChangedEvent);
				Actor->PostEditMove(true);
				GEditor->BroadcastEndObjectMovement(*Actor);
			}

			GEditor->BroadcastActorsMoved(ActorsToMove);
		}
	}

	// End the transaction here if one was started in StartTransaction()
	if (TrackingTransaction.IsActive() || TrackingTransaction.IsPending())
	{
		if (!HaveSelectedObjectsBeenChanged())
		{
			TrackingTransaction.Cancel();
		}
		else
		{
			TrackingTransaction.End();
		}

		// Restore actor/component delta modification
		GEditor->DisableDeltaModification(false);
	}

	ModeTools->ActorMoveNotify();

	if (bDidAnythingActuallyChange)
	{
		FScopedLevelDirtied LevelDirtyCallback;
		LevelDirtyCallback.Request();

		RedrawAllViewportsIntoThisScene();
	}
}

void FLGUIPrefabEditorViewportClient::AbortTracking()
{
	if (TrackingTransaction.IsActive())
	{
		// Applying the global undo here will reset the drag operation
		if (GUndo)
		{
			GUndo->Apply();
		}
		TrackingTransaction.Cancel();
		StopTracking();
	}
}

bool FLGUIPrefabEditorViewportClient::HaveSelectedObjectsBeenChanged() const
{
	return (TrackingTransaction.TransCount > 0 || TrackingTransaction.IsActive()) && (MouseDeltaTracker->HasReceivedDelta() || MouseDeltaTracker->WasExternalMovement());
}


#undef LOCTEXT_NAMESPACE
