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
#include "PrefabSystem/LGUIPrefabManager.h"
#include "LGUIPrefabEditorScene.h"
#include "LGUIPrefabEditor.h"
#include "MouseDeltaTracker.h"
#include "Misc/ITransaction.h"
#include "UnrealEdGlobals.h"
#include "Editor/EditorPerProjectUserSettings.h"
#include "UnrealWidget.h"
#include "Elements/Framework/TypedElementRegistry.h"
#include "Elements/Framework/EngineElementsLibrary.h"
#include "Elements/Framework/TypedElementCommonActions.h"
#include "Elements/Framework/TypedElementListObjectUtil.h"
#include "Elements/Framework/TypedElementViewportInteraction.h"
#include "Elements/Actor/ActorElementLevelEditorViewportInteractionCustomization.h"
#include "Elements/Component/ComponentElementLevelEditorViewportInteractionCustomization.h"
#include "InputState.h"
#include "LevelViewportClickHandlers.h"
#include "HModel.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "LGUIPrefabViewportClickHandlers.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabEditorViewportClient"

IMPLEMENT_HIT_PROXY(HLevelSocketProxy, HHitProxy);

FLGUIPrefabEditorViewportClient::FLGUIPrefabEditorViewportClient(FLGUIPrefabEditorScene& InPreviewScene
	, TWeakPtr<FLGUIPrefabEditor> InPrefabEditorPtr
	, const TSharedRef<SLGUIPrefabEditorViewport>& InEditorViewportPtr)
	: FEditorViewportClient(&GLevelEditorModeTools(), nullptr, StaticCastSharedRef<SEditorViewport>(InEditorViewportPtr))
	, PrefabScene(&InPreviewScene)
	, TrackingTransaction()
	, CachedElementsToManipulate(UTypedElementRegistry::GetInstance()->CreateElementList())
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


/**
 * Renders a view frustum specified by the provided frustum parameters
 *
 * @param	PDI					PrimitiveDrawInterface to use to draw the view frustum
 * @param	FrustumColor		Color to draw the view frustum in
 * @param	FrustumAngle		Angle of the frustum
 * @param	FrustumAspectRatio	Aspect ratio of the frustum
 * @param	FrustumStartDist	Start distance of the frustum
 * @param	FrustumEndDist		End distance of the frustum
 * @param	InViewMatrix		View matrix to use to draw the frustum
 */
static void RenderViewFrustum(FPrimitiveDrawInterface* PDI,
	const FLinearColor& FrustumColor,
	float FrustumAngle,
	float FrustumAspectRatio,
	float FrustumStartDist,
	float FrustumEndDist,
	const FMatrix& InViewMatrix)
{
	FVector Direction(0, 0, 1);
	FVector LeftVector(1, 0, 0);
	FVector UpVector(0, 1, 0);

	FVector Verts[8];

	// FOVAngle controls the horizontal angle.
	float HozHalfAngle = (FrustumAngle) * ((float)PI / 360.f);
	float HozLength = FrustumStartDist * FMath::Tan(HozHalfAngle);
	float VertLength = HozLength / FrustumAspectRatio;

	// near plane verts
	Verts[0] = (Direction * FrustumStartDist) + (UpVector * VertLength) + (LeftVector * HozLength);
	Verts[1] = (Direction * FrustumStartDist) + (UpVector * VertLength) - (LeftVector * HozLength);
	Verts[2] = (Direction * FrustumStartDist) - (UpVector * VertLength) - (LeftVector * HozLength);
	Verts[3] = (Direction * FrustumStartDist) - (UpVector * VertLength) + (LeftVector * HozLength);

	HozLength = FrustumEndDist * FMath::Tan(HozHalfAngle);
	VertLength = HozLength / FrustumAspectRatio;

	// far plane verts
	Verts[4] = (Direction * FrustumEndDist) + (UpVector * VertLength) + (LeftVector * HozLength);
	Verts[5] = (Direction * FrustumEndDist) + (UpVector * VertLength) - (LeftVector * HozLength);
	Verts[6] = (Direction * FrustumEndDist) - (UpVector * VertLength) - (LeftVector * HozLength);
	Verts[7] = (Direction * FrustumEndDist) - (UpVector * VertLength) + (LeftVector * HozLength);

	for (int32 x = 0; x < 8; ++x)
	{
		Verts[x] = InViewMatrix.InverseFast().TransformPosition(Verts[x]);
	}

	const uint8 PrimitiveDPG = SDPG_Foreground;
	PDI->DrawLine(Verts[0], Verts[1], FrustumColor, PrimitiveDPG);
	PDI->DrawLine(Verts[1], Verts[2], FrustumColor, PrimitiveDPG);
	PDI->DrawLine(Verts[2], Verts[3], FrustumColor, PrimitiveDPG);
	PDI->DrawLine(Verts[3], Verts[0], FrustumColor, PrimitiveDPG);

	PDI->DrawLine(Verts[4], Verts[5], FrustumColor, PrimitiveDPG);
	PDI->DrawLine(Verts[5], Verts[6], FrustumColor, PrimitiveDPG);
	PDI->DrawLine(Verts[6], Verts[7], FrustumColor, PrimitiveDPG);
	PDI->DrawLine(Verts[7], Verts[4], FrustumColor, PrimitiveDPG);

	PDI->DrawLine(Verts[0], Verts[4], FrustumColor, PrimitiveDPG);
	PDI->DrawLine(Verts[1], Verts[5], FrustumColor, PrimitiveDPG);
	PDI->DrawLine(Verts[2], Verts[6], FrustumColor, PrimitiveDPG);
	PDI->DrawLine(Verts[3], Verts[7], FrustumColor, PrimitiveDPG);
}
// Frustum parameters for the perspective view.
static float GPerspFrustumAngle=90.f;
static float GPerspFrustumAspectRatio=1.77777f;
static float GPerspFrustumStartDist=GNearClippingPlane;
static float GPerspFrustumEndDist=UE_FLOAT_HUGE_DISTANCE;
static FMatrix GPerspViewMatrix;
void FLGUIPrefabEditorViewportClient::Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	FMemMark Mark(FMemStack::Get());

	FEditorViewportClient::Draw(View, PDI);

	//AGroupActor::DrawBracketsForGroups(PDI, Viewport);

	// Determine if a view frustum should be rendered in the viewport.
	// The frustum should definitely be rendered if the viewport has a view parent.
	bool bRenderViewFrustum = ViewState.GetReference()->HasViewParent();

	// If the viewport doesn't have a view parent, a frustum still should be drawn anyway if the viewport is ortho and level streaming
	// volume previs is enabled in some viewport
	if (!bRenderViewFrustum && IsOrtho())
	{
		for (FLevelEditorViewportClient* CurViewportClient : GEditor->GetLevelViewportClients())
		{
			if (CurViewportClient && IsPerspective() && GetDefault<ULevelEditorViewportSettings>()->bLevelStreamingVolumePrevis)
			{
				bRenderViewFrustum = true;
				break;
			}
		}
	}

	// Draw the view frustum of the view parent or level streaming volume previs viewport, if necessary
	if (bRenderViewFrustum)
	{
		RenderViewFrustum(PDI, FLinearColor(1.0, 0.0, 1.0, 1.0),
			GPerspFrustumAngle,
			GPerspFrustumAspectRatio,
			GPerspFrustumStartDist,
			GPerspFrustumEndDist,
			GPerspViewMatrix);
	}

	if (GEditor->bEnableSocketSnapping)
	{
		const bool bGameViewMode = View->Family->EngineShowFlags.Game && !GEditor->bDrawSocketsInGMode;

		for (FActorIterator It(GetWorld()); It; ++It)
		{
			AActor* Actor = *It;

			if (bGameViewMode || Actor->IsHiddenEd())
			{
				// Don't display sockets on hidden actors...
				continue;
			}

			for (UActorComponent* Component : Actor->GetComponents())
			{
				USceneComponent* SceneComponent = Cast<USceneComponent>(Component);
				if (SceneComponent && SceneComponent->HasAnySockets())
				{
					TArray<FComponentSocketDescription> Sockets;
					SceneComponent->QuerySupportedSockets(Sockets);

					for (int32 SocketIndex = 0; SocketIndex < Sockets.Num(); ++SocketIndex)
					{
						FComponentSocketDescription& Socket = Sockets[SocketIndex];

						if (Socket.Type == EComponentSocketType::Socket)
						{
							const FTransform SocketTransform = SceneComponent->GetSocketTransform(Socket.Name);

							const float DiamondSize = 2.0f;
							const FColor DiamondColor(255, 128, 128);

							PDI->SetHitProxy(new HLevelSocketProxy(*It, SceneComponent, Socket.Name));
							DrawWireDiamond(PDI, SocketTransform.ToMatrixWithScale(), DiamondSize, DiamondColor, SDPG_Foreground);
							PDI->SetHitProxy(NULL);
						}
					}
				}
			}
		}
	}

	//if (this == GCurrentLevelEditingViewportClient)
	//{
	//	FSnappingUtils::DrawSnappingHelpers(View, PDI);
	//}

	if (GUnrealEd != NULL && !IsInGameView())
	{
		GUnrealEd->DrawComponentVisualizers(View, PDI);
	}

	if (GEditor->bDrawParticleHelpers == true)
	{
		if (View->Family->EngineShowFlags.Game)
		{
			extern ENGINE_API void DrawParticleSystemHelpers(const FSceneView * View, FPrimitiveDrawInterface * PDI);
			DrawParticleSystemHelpers(View, PDI);
		}
	}

	Mark.Pop();
}
void FLGUIPrefabEditorViewportClient::DrawCanvas(FViewport& InViewport, FSceneView& View, FCanvas& Canvas)
{
	if (GUnrealEd != nullptr && !IsInGameView())
	{
		GUnrealEd->DrawComponentVisualizersHUD(&InViewport, &View, &Canvas);
	}

	FEditorViewportClient::DrawCanvas(InViewport, View, Canvas);
}

void FLGUIPrefabEditorViewportClient::ReceivedFocus(FViewport* InViewport)
{
	if (!bReceivedFocusRecently)
	{
		bReceivedFocusRecently = true;

		// A few frames can pass between receiving focus and processing a click, so we use a timer to track whether we have recently received focus.
		FTimerDelegate ResetFocusReceivedTimer;
		ResetFocusReceivedTimer.BindLambda([&]()
			{
				bReceivedFocusRecently = false;
				FocusTimerHandle.Invalidate(); // The timer will only execute once, so we can invalidate now.
			});
		GEditor->GetTimerManager()->SetTimer(FocusTimerHandle, ResetFocusReceivedTimer, 0.1f, false);
	}

	FEditorViewportClient::ReceivedFocus(InViewport);
}

void FLGUIPrefabEditorViewportClient::LostFocus(FViewport* InViewport)
{
	FEditorViewportClient::LostFocus(InViewport);

	GEditor->SetPreviewMeshMode(false);
}

void FLGUIPrefabEditorViewportClient::Tick(float DeltaSeconds)
{
	FEditorViewportClient::Tick(DeltaSeconds);

	float RequestDelta = DeltaSeconds;
	{
		TickWorld(RequestDelta);
	}
}


bool FLGUIPrefabEditorViewportClient::InputKey(const FInputKeyEventArgs& EventArgs)
{
	bool bHandled = GUnrealEd->ComponentVisManager.HandleInputKey(this, EventArgs.Viewport, EventArgs.Key, EventArgs.Event);
	if (!bHandled)
	{
		bool Res = FEditorViewportClient::InputKey(EventArgs);

		if (EventArgs.Key == EKeys::F)
		{
			if (EventArgs.Event == IE_Pressed)
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

	FVector RayOrigin, RayDirection;
	View.DeprojectScreenToWorld(FVector2D(HitX, HitY), View.UnscaledViewRect, View.ViewMatrices.GetInvViewProjectionMatrix(), RayOrigin, RayDirection);
	AActor* ClickHitActor = nullptr;
	ULGUIPrefabManagerObject::OnPrefabEditorViewport_MouseClick.ExecuteIfBound(this->GetWorld(), RayOrigin, RayDirection, ClickHitActor);
	if (ClickHitActor != nullptr)
	{
		LGUIPrefabViewportClickHandlers::ClickActor(this, ClickHitActor, Click, true);
		return;
	}

	// We may have started gizmo manipulation if hot-keys were pressed when we started this click
	// If so, we need to end that now before we potentially update the selection below, 
	// otherwise the usual call in TrackingStopped would include the newly selected element
	if (bHasBegunGizmoManipulation)
	{
		FTypedElementListConstRef ElementsToManipulate = GetElementsToManipulate();
		ViewportInteraction->EndGizmoManipulation(ElementsToManipulate, GetWidgetMode(), ETypedElementViewportInteractionGizmoManipulationType::Click);
		bHasBegunGizmoManipulation = false;
	}

	if (Click.GetKey() == EKeys::MiddleMouseButton && !Click.IsAltDown() && !Click.IsShiftDown())
	{
		LGUIPrefabViewportClickHandlers::ClickViewport(this, Click);
		return;
	}
	if (!ModeTools->HandleClick(this, HitProxy, Click))
	{
		const FTypedElementHandle HitElement = HitProxy ? HitProxy->GetElementHandle() : FTypedElementHandle();

		if (HitProxy == NULL)
		{
			LGUIPrefabViewportClickHandlers::ClickBackdrop(this, Click);
		}
		else if (HitProxy->IsA(HWidgetAxis::StaticGetType()))
		{
			// The user clicked on an axis translation/rotation hit proxy.  However, we want
			// to find out what's underneath the axis widget.  To do this, we'll need to render
			// the viewport's hit proxies again, this time *without* the axis widgets!

			// OK, we need to be a bit evil right here.  Basically we want to hijack the ShowFlags
			// for the scene so we can re-render the hit proxies without any axis widgets.  We'll
			// store the original ShowFlags and modify them appropriately
			const bool bOldModeWidgets1 = EngineShowFlags.ModeWidgets;
			const bool bOldModeWidgets2 = View.Family->EngineShowFlags.ModeWidgets;

			EngineShowFlags.SetModeWidgets(false);
			FSceneViewFamily* SceneViewFamily = const_cast<FSceneViewFamily*>(View.Family);
			SceneViewFamily->EngineShowFlags.SetModeWidgets(false);
			bool bWasWidgetDragging = Widget->IsDragging();
			Widget->SetDragging(false);

			// Invalidate the hit proxy map so it will be rendered out again when GetHitProxy
			// is called
			Viewport->InvalidateHitProxy();

			// This will actually re-render the viewport's hit proxies!
			HHitProxy* HitProxyWithoutAxisWidgets = Viewport->GetHitProxy(HitX, HitY);
			if (HitProxyWithoutAxisWidgets != NULL && !HitProxyWithoutAxisWidgets->IsA(HWidgetAxis::StaticGetType()))
			{
				// Try this again, but without the widget this time!
				ProcessClick(View, HitProxyWithoutAxisWidgets, Key, Event, HitX, HitY);
			}

			// Undo the evil
			EngineShowFlags.SetModeWidgets(bOldModeWidgets1);
			SceneViewFamily->EngineShowFlags.SetModeWidgets(bOldModeWidgets2);

			Widget->SetDragging(bWasWidgetDragging);

			// Invalidate the hit proxy map again so that it'll be refreshed with the original
			// scene contents if we need it again later.
			Viewport->InvalidateHitProxy();
		}
		else if (GUnrealEd->ComponentVisManager.HandleClick(this, HitProxy, Click))
		{
			// Component vis manager handled the click
		}
		else if (HitElement && LGUIPrefabViewportClickHandlers::ClickElement(this, HitElement, Click))
		{
			// Element handled the click
		}
		else if (HitProxy->IsA(HActor::StaticGetType()))
		{
			HActor* ActorHitProxy = (HActor*)HitProxy;
			AActor* ConsideredActor = ActorHitProxy->Actor;
			if (ConsideredActor) // It is possible to be clicking something during level transition if you spam click, and it might not be valid by this point
			{
				while (ConsideredActor->IsChildActor())
				{
					ConsideredActor = ConsideredActor->GetParentActor();
				}

				// We want to process the click on the component only if:
				// 1. The actor clicked is already selected
				// 2. The actor selected is the only actor selected
				// 3. The actor selected is blueprintable
				// 4. No components are already selected and the click was a double click
				// 5. OR, a component is already selected and the click was NOT a double click
				const bool bActorAlreadySelectedExclusively = GEditor->GetSelectedActors()->IsSelected(ConsideredActor) && (GEditor->GetSelectedActorCount() == 1);
				const bool bActorIsBlueprintable = FKismetEditorUtilities::CanCreateBlueprintOfClass(ConsideredActor->GetClass());
				const bool bComponentAlreadySelected = GEditor->GetSelectedComponentCount() > 0;
				const bool bWasDoubleClick = (Click.GetEvent() == IE_DoubleClick);

				const bool bSelectComponent = bActorAlreadySelectedExclusively && bActorIsBlueprintable && (bComponentAlreadySelected != bWasDoubleClick);
				bool bComponentSelected = false;

				if (bSelectComponent)
				{
					bComponentSelected = LGUIPrefabViewportClickHandlers::ClickComponent(this, ActorHitProxy, Click);
				}

				if (!bComponentSelected)
				{
					LGUIPrefabViewportClickHandlers::ClickActor(this, ConsideredActor, Click, true);
				}

				// We clicked an actor, allow the pivot to reposition itself.
				// GUnrealEd->SetPivotMovedIndependently(false);
			}
		}
		else if (HitProxy->IsA(HInstancedStaticMeshInstance::StaticGetType()))
		{
			LGUIPrefabViewportClickHandlers::ClickActor(this, ((HInstancedStaticMeshInstance*)HitProxy)->Component->GetOwner(), Click, true);
		}
		//else if (HitProxy->IsA(HBSPBrushVert::StaticGetType()) && ((HBSPBrushVert*)HitProxy)->Brush.IsValid())
		//{
		//	FVector Vertex = FVector(*((HBSPBrushVert*)HitProxy)->Vertex);
		//	LGUIPrefabViewportClickHandlers::ClickBrushVertex(this, ((HBSPBrushVert*)HitProxy)->Brush.Get(), &Vertex, Click);
		//}
		else if (HitProxy->IsA(HStaticMeshVert::StaticGetType()))
		{
			LGUIPrefabViewportClickHandlers::ClickStaticMeshVertex(this, ((HStaticMeshVert*)HitProxy)->Actor, ((HStaticMeshVert*)HitProxy)->Vertex, Click);
		}
		//else if (BrushSubsystem && BrushSubsystem->ProcessClickOnBrushGeometry(this, HitProxy, Click))
		//{
		//	// Handled by the brush subsystem
		//}
		else if (HitProxy->IsA(HModel::StaticGetType()))
		{
			HModel* ModelHit = (HModel*)HitProxy;

			// Compute the viewport's current view family.
			FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(Viewport, GetScene(), EngineShowFlags));
			FSceneView* SceneView = CalcSceneView(&ViewFamily);

			uint32 SurfaceIndex = INDEX_NONE;
			if (ModelHit->ResolveSurface(SceneView, HitX, HitY, SurfaceIndex))
			{
				LGUIPrefabViewportClickHandlers::ClickSurface(this, ModelHit->GetModel(), SurfaceIndex, Click);
			}
		}
		else if (HitProxy->IsA(HLevelSocketProxy::StaticGetType()))
		{
			LGUIPrefabViewportClickHandlers::ClickLevelSocket(this, HitProxy, Click);
		}
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

/**
 * Returns the horizontal axis for this viewport.
 */

EAxisList::Type FLGUIPrefabEditorViewportClient::GetHorizAxis() const
{
	switch (GetViewportType())
	{
	case LVT_OrthoXY:
	case LVT_OrthoNegativeXY:
		return EAxisList::X;
	case LVT_OrthoXZ:
	case LVT_OrthoNegativeXZ:
		return EAxisList::X;
	case LVT_OrthoYZ:
	case LVT_OrthoNegativeYZ:
		return EAxisList::Y;
	case LVT_OrthoFreelook:
	case LVT_Perspective:
		break;
	}

	return EAxisList::X;
}

/**
 * Returns the vertical axis for this viewport.
 */

EAxisList::Type FLGUIPrefabEditorViewportClient::GetVertAxis() const
{
	switch (GetViewportType())
	{
	case LVT_OrthoXY:
	case LVT_OrthoNegativeXY:
		return EAxisList::Y;
	case LVT_OrthoXZ:
	case LVT_OrthoNegativeXZ:
		return EAxisList::Z;
	case LVT_OrthoYZ:
	case LVT_OrthoNegativeYZ:
		return EAxisList::Z;
	case LVT_OrthoFreelook:
	case LVT_Perspective:
		break;
	}

	return EAxisList::Y;
}
void FLGUIPrefabEditorViewportClient::NudgeSelectedObjects(const struct FInputEventState& InputState)
{
	FViewport* InViewport = InputState.GetViewport();
	EInputEvent Event = InputState.GetInputEvent();
	FKey Key = InputState.GetKey();

	const int32 MouseX = InViewport->GetMouseX();
	const int32 MouseY = InViewport->GetMouseY();

	if (Event == IE_Pressed || Event == IE_Repeat)
	{
		// If this is a pressed event, start tracking.
		if (!bIsTracking && Event == IE_Pressed)
		{
			// without the check for !bIsTracking, the following code would cause a new transaction to be created
			// for each "nudge" that occurred while the key was held down.  Disabling this code prevents the transaction
			// from being constantly recreated while as long as the key is held, so that the entire move is considered an atomic action (and
			// doing undo reverts the entire movement, as opposed to just the last nudge that occurred while the key was held down)
			MouseDeltaTracker->StartTracking(this, MouseX, MouseY, InputState, true);
			bIsTracking = true;
		}

		FIntPoint StartMousePos;
		InViewport->GetMousePos(StartMousePos);
		FKey VirtualKey = EKeys::MouseX;
		EAxisList::Type VirtualAxis = GetHorizAxis();
		float VirtualDelta = GEditor->GetGridSize() * (Key == EKeys::Left ? -1 : 1);
		if (Key == EKeys::Up || Key == EKeys::Down)
		{
			VirtualKey = EKeys::MouseY;
			VirtualAxis = GetVertAxis();
			VirtualDelta = GEditor->GetGridSize() * (Key == EKeys::Up ? 1 : -1);
		}

		bWidgetAxisControlledByDrag = false;
		Widget->SetCurrentAxis(VirtualAxis);
		MouseDeltaTracker->AddDelta(this, VirtualKey, static_cast<int32>(VirtualDelta), 1);
		Widget->SetCurrentAxis(VirtualAxis);
		UpdateMouseDelta();
		InViewport->SetMouse(StartMousePos.X, StartMousePos.Y);
	}
	else if (bIsTracking && Event == IE_Released)
	{
		bWidgetAxisControlledByDrag = false;
		MouseDeltaTracker->EndTracking(this);
		bIsTracking = false;
		Widget->SetCurrentAxis(EAxisList::None);
	}

	RedrawAllViewportsIntoThisScene();
}

void FLGUIPrefabEditorViewportClient::ApplyDeltaToActors(const FVector& InDrag, const FRotator& InRot, const FVector& InScale)
{
	ApplyDeltaToSelectedElements(FTransform(InRot, InDrag, InScale));
}

void FLGUIPrefabEditorViewportClient::ApplyDeltaToActor(AActor* InActor, const FVector& InDeltaDrag, const FRotator& InDeltaRot, const FVector& InDeltaScale)
{
	if (FTypedElementHandle ActorElementHandle = UEngineElementsLibrary::AcquireEditorActorElementHandle(InActor))
	{
		ApplyDeltaToElement(ActorElementHandle, FTransform(InDeltaRot, InDeltaDrag, InDeltaScale));
	}
}

void FLGUIPrefabEditorViewportClient::ApplyDeltaToComponent(USceneComponent* InComponent, const FVector& InDeltaDrag, const FRotator& InDeltaRot, const FVector& InDeltaScale)
{
	if (FTypedElementHandle ComponentElementHandle = UEngineElementsLibrary::AcquireEditorComponentElementHandle(InComponent))
	{
		ApplyDeltaToElement(ComponentElementHandle, FTransform(InDeltaRot, InDeltaDrag, InDeltaScale));
	}
}

void FLGUIPrefabEditorViewportClient::ApplyDeltaToSelectedElements(const FTransform& InDeltaTransform)
{
	if (InDeltaTransform.GetTranslation().IsZero() && InDeltaTransform.Rotator().IsZero() && InDeltaTransform.GetScale3D().IsZero())
	{
		return;
	}

	FTransform ModifiedDeltaTransform = InDeltaTransform;

	{
		FVector AdjustedScale = ModifiedDeltaTransform.GetScale3D();

		// If we are scaling, we need to change the scaling factor a bit to properly align to grid
		if (GEditor->UsePercentageBasedScaling() && !AdjustedScale.IsNearlyZero())
		{
			AdjustedScale *= ((GEditor->GetScaleGridSize() / 100.0f) / GEditor->GetGridSize());
		}

		ModifiedDeltaTransform.SetScale3D(AdjustedScale);
	}

	FInputDeviceState InputState;
	InputState.SetModifierKeyStates(IsShiftPressed(), IsAltPressed(), IsCtrlPressed(), IsCmdPressed());

	FTypedElementListConstRef ElementsToManipulate = GetElementsToManipulate(true);
	ViewportInteraction->UpdateGizmoManipulation(ElementsToManipulate, GetWidgetMode(), Widget ? Widget->GetCurrentAxis() : EAxisList::None, InputState, ModifiedDeltaTransform);
}

void FLGUIPrefabEditorViewportClient::ApplyDeltaToElement(const FTypedElementHandle& InElementHandle, const FTransform& InDeltaTransform)
{
	FInputDeviceState InputState;
	InputState.SetModifierKeyStates(IsShiftPressed(), IsAltPressed(), IsCtrlPressed(), IsCmdPressed());

	ViewportInteraction->ApplyDeltaToElement(InElementHandle, GetWidgetMode(), Widget ? Widget->GetCurrentAxis() : EAxisList::None, InputState, InDeltaTransform);
}

FTypedElementListConstRef FLGUIPrefabEditorViewportClient::GetElementsToManipulate(const bool bForceRefresh)
{
	CacheElementsToManipulate(bForceRefresh);
	return CachedElementsToManipulate;
}

void FLGUIPrefabEditorViewportClient::CacheElementsToManipulate(const bool bForceRefresh)
{
	if (bForceRefresh)
	{
		ResetElementsToManipulate();
	}

	if (!bHasCachedElementsToManipulate)
	{
		const FTypedElementSelectionNormalizationOptions NormalizationOptions = FTypedElementSelectionNormalizationOptions()
			.SetExpandGroups(true)
			.SetFollowAttachment(true);

		const UTypedElementSelectionSet* SelectionSet = GetSelectionSet();
		SelectionSet->GetNormalizedSelection(NormalizationOptions, CachedElementsToManipulate);

		// Remove any elements that cannot be moved
		CachedElementsToManipulate->RemoveAll<ITypedElementWorldInterface>([this](const TTypedElement<ITypedElementWorldInterface>& InWorldElement)
			{
				if (!InWorldElement.CanMoveElement(bIsSimulateInEditorViewport ? ETypedElementWorldType::Game : ETypedElementWorldType::Editor))
				{
					return true;
				}

				// This element must belong to the current viewport world
				if (GEditor->PlayWorld)
				{
					const UWorld* CurrentWorld = InWorldElement.GetOwnerWorld();
					const UWorld* RequiredWorld = bIsSimulateInEditorViewport ? GEditor->PlayWorld : GEditor->EditorWorld;
					if (CurrentWorld != RequiredWorld)
					{
						return true;
					}
				}

				return false;
			});

		bHasCachedElementsToManipulate = true;
	}
}
void FLGUIPrefabEditorViewportClient::ResetElementsToManipulate(const bool bClearList)
{
	if (bClearList)
	{
		CachedElementsToManipulate->Reset();
	}
	bHasCachedElementsToManipulate = false;
}

void FLGUIPrefabEditorViewportClient::ResetElementsToManipulateFromSelectionChange(const UTypedElementSelectionSet* InSelectionSet)
{
	check(InSelectionSet == GetSelectionSet());

	// Don't clear the list immediately, as the selection may change from a construction script running (while we're still iterating the list!)
	// We'll process the clear on the next cache request, or when the typed element registry actually processes its pending deletion
	ResetElementsToManipulate(/*bClearList*/false);
}

void FLGUIPrefabEditorViewportClient::ResetElementsToManipulateFromProcessingDeferredElementsToDestroy()
{
	if (!bHasCachedElementsToManipulate)
	{
		// If we have no cache, make sure the cached list is definitely empty now to ensure it doesn't contain any lingering references to things that are about to be deleted
		CachedElementsToManipulate->Reset();
	}
}

const UTypedElementSelectionSet* FLGUIPrefabEditorViewportClient::GetSelectionSet() const
{
	return GEditor->GetSelectedActors()->GetElementSelectionSet();
}

UTypedElementSelectionSet* FLGUIPrefabEditorViewportClient::GetMutableSelectionSet() const
{
	return GEditor->GetSelectedActors()->GetElementSelectionSet();
}


void FLGUIPrefabEditorViewportClient::TickWorld(float DeltaSeconds)
{
	GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
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


// Begin override because PreviewScene is nullptr
// These implementation are copied from FEditorViewportClient
UWorld* FLGUIPrefabEditorViewportClient::GetWorld()const
{
	return PrefabScene->GetWorld();
}
void FLGUIPrefabEditorViewportClient::AddReferencedObjects(FReferenceCollector& Collector)
{
	FEditorViewportClient::AddReferencedObjects(Collector);
	PrefabScene->AddReferencedObjects(Collector);
}
namespace PreviewLightConstants
{
	const float MovingPreviewLightTimerDuration = 1.0f;

	const float MinMouseRadius = 100.0f;
	const float MinArrowLength = 10.0f;
	const float ArrowLengthToSizeRatio = 0.1f;
	const float MouseLengthToArrowLenghtRatio = 0.2f;

	const float ArrowLengthToThicknessRatio = 0.05f;
	const float MinArrowThickness = 2.0f;

	// Note: MinMouseRadius must be greater than MinArrowLength
}
void FLGUIPrefabEditorViewportClient::DrawPreviewLightVisualization(const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	// Draw the indicator of the current light direction if it was recently moved
	if ((PrefabScene != nullptr) && (PrefabScene->DirectionalLight != nullptr) && (MovingPreviewLightTimer > 0.0f))
	{
		const float A = MovingPreviewLightTimer / PreviewLightConstants::MovingPreviewLightTimerDuration;

		ULightComponent* Light = PrefabScene->DirectionalLight;

		const FLinearColor ArrowColor = Light->LightColor;

		// Figure out where the light is (ignoring position for directional lights)
		const FTransform LightLocalToWorldRaw = Light->GetComponentToWorld();
		FTransform LightLocalToWorld = LightLocalToWorldRaw;
		if (Light->IsA(UDirectionalLightComponent::StaticClass()))
		{
			LightLocalToWorld.SetTranslation(FVector::ZeroVector);
		}
		LightLocalToWorld.SetScale3D(FVector(1.0f));

		// Project the last mouse position during the click into world space
		FVector LastMouseWorldPos;
		FVector LastMouseWorldDir;
		View->DeprojectFVector2D(MovingPreviewLightSavedScreenPos, /*out*/ LastMouseWorldPos, /*out*/ LastMouseWorldDir);

		// The world pos may be nuts due to a super distant near plane for orthographic cameras, so find the closest
		// point to the origin along the ray
		LastMouseWorldPos = FMath::ClosestPointOnLine(LastMouseWorldPos, LastMouseWorldPos + LastMouseWorldDir * WORLD_MAX, FVector::ZeroVector);

		// Figure out the radius to draw the light preview ray at
		const FVector LightToMousePos = LastMouseWorldPos - LightLocalToWorld.GetTranslation();
		const float LightToMouseRadius = FMath::Max<FVector::FReal>(LightToMousePos.Size(), PreviewLightConstants::MinMouseRadius);

		const float ArrowLength = FMath::Max(PreviewLightConstants::MinArrowLength, LightToMouseRadius * PreviewLightConstants::MouseLengthToArrowLenghtRatio);
		const float ArrowSize = PreviewLightConstants::ArrowLengthToSizeRatio * ArrowLength;
		const float ArrowThickness = FMath::Max(PreviewLightConstants::ArrowLengthToThicknessRatio * ArrowLength, PreviewLightConstants::MinArrowThickness);

		const FVector ArrowOrigin = LightLocalToWorld.TransformPosition(FVector(-LightToMouseRadius - 0.5f * ArrowLength, 0.0f, 0.0f));
		const FVector ArrowDirection = LightLocalToWorld.TransformVector(FVector(-1.0f, 0.0f, 0.0f));

		const FQuatRotationTranslationMatrix ArrowToWorld(LightLocalToWorld.GetRotation(), ArrowOrigin);

		DrawDirectionalArrow(PDI, ArrowToWorld, ArrowColor, ArrowLength, ArrowSize, SDPG_World, ArrowThickness);
	}
}
FLinearColor FLGUIPrefabEditorViewportClient::GetBackgroundColor() const
{
	return PrefabScene ? PrefabScene->GetBackgroundColor() : FColor(55, 55, 55);
}
namespace EditorViewportClient
{
	static const float GridSize = 2048.0f;
	static const int8 CellSize = 16;
	static const float LightRotSpeed = 0.22f;
}
class FCachedJoystickState
{
public:
	uint32 JoystickType;
	TMap <FKey, float> AxisDeltaValues;
	TMap <FKey, EInputEvent> KeyEventValues;
};
bool FLGUIPrefabEditorViewportClient::Internal_InputAxis(FViewport* InViewport, FInputDeviceId DeviceID, FKey Key, float Delta, float DeltaTime, int32 NumSamples, bool bGamepad)
{
	if (bDisableInput)
	{
		return true;
	}

	const FPlatformUserId UserId = IPlatformInputDeviceMapper::Get().GetUserForInputDevice(DeviceID);

	// Let the current mode have a look at the input before reacting to it.
	if (ModeTools->InputAxis(this, Viewport, FGenericPlatformMisc::GetUserIndexForPlatformUser(UserId), Key, Delta, DeltaTime))
	{
		return true;
	}

	const bool bMouseButtonDown = InViewport->KeyState(EKeys::LeftMouseButton) || InViewport->KeyState(EKeys::MiddleMouseButton) || InViewport->KeyState(EKeys::RightMouseButton);
	const bool bLightMoveDown = InViewport->KeyState(EKeys::L);

	// Look at which axis is being dragged and by how much
	const float DragX = (Key == EKeys::MouseX) ? Delta : 0.f;
	const float DragY = (Key == EKeys::MouseY) ? Delta : 0.f;

	if (bLightMoveDown && bMouseButtonDown && PrefabScene)
	{
		// Adjust the preview light direction
		FRotator LightDir = PrefabScene->GetLightDirection();

		LightDir.Yaw += -DragX * EditorViewportClient::LightRotSpeed;
		LightDir.Pitch += -DragY * EditorViewportClient::LightRotSpeed;

		PrefabScene->SetLightDirection(LightDir);

		// Remember that we adjusted it for the visualization
		MovingPreviewLightTimer = PreviewLightConstants::MovingPreviewLightTimerDuration;
		MovingPreviewLightSavedScreenPos = FVector2D(LastMouseX, LastMouseY);

		Invalidate();
	}
	else
	{
		/**Save off axis commands for future camera work*/
		FCachedJoystickState* JoystickState = GetJoystickState(DeviceID.GetId());
		if (JoystickState)
		{
			JoystickState->AxisDeltaValues.Add(Key, Delta);
		}

		if (bIsTracking)
		{
			// Accumulate and snap the mouse movement since the last mouse button click.
			MouseDeltaTracker->AddDelta(this, Key, Delta, 0);
		}
	}

	// If we are using a drag tool, paint the viewport so we can see it update.
	if (MouseDeltaTracker->UsingDragTool())
	{
		Invalidate(false, false);
	}

	return true;
}
// End override because PreviewScene is nullptr


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
	
	if (InMouseX != PrevMouseX || InMouseY != PrevMouseY)
	{
		ULGUIPrefabManagerObject::OnPrefabEditorViewport_MouseMove.ExecuteIfBound(this->GetWorld());
	}
	PrevMouseX = InMouseX;
	PrevMouseY = InMouseY;
}

void FLGUIPrefabEditorViewportClient::MouseEnter(FViewport* InViewport, int32 x, int32 y)
{
	FEditorViewportClient::MouseEnter(InViewport, x, y);
}
void FLGUIPrefabEditorViewportClient::MouseMove(FViewport* InViewport, int32 x, int32 y)
{
	FEditorViewportClient::MouseMove(InViewport, x, y);
}
void FLGUIPrefabEditorViewportClient::MouseLeave(FViewport* InViewport)
{
	FEditorViewportClient::MouseLeave(InViewport);
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

			FText TrackingDescription;
			switch (GetWidgetMode())
			{
			case UE::Widget::WM_Translate:
				TrackingDescription = LOCTEXT("MoveTransaction", "Move Elements");
				break;
			case UE::Widget::WM_Rotate:
				TrackingDescription = LOCTEXT("RotateTransaction", "Rotate Elements");
				break;
			case UE::Widget::WM_Scale:
				TrackingDescription = LOCTEXT("ScaleTransaction", "Scale Elements");
				break;
			case UE::Widget::WM_TranslateRotateZ:
				TrackingDescription = LOCTEXT("TranslateRotateZTransaction", "Translate/RotateZ Elements");
				break;
			case UE::Widget::WM_2D:
				TrackingDescription = LOCTEXT("TranslateRotate2D", "Translate/Rotate2D Elements");
				break;
			default:
				if (bNudge)
				{
					TrackingDescription = LOCTEXT("NudgeTransaction", "Nudge Elements");
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
