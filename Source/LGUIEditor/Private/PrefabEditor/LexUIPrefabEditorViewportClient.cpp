// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LexUIPrefabEditorViewportClient.h"
#include "LexUIPrefabEditorViewport.h"
#include "LexUIPrefabEditorTransformWidget.h"
#include "SceneView.h"
#include "LexUIPrefabEditor.h"
#include "Core/LexUIManager.h"
#include "Core/Components/LexCanvas.h"
#include "Core/Components/LexWidget.h"
#include "PrefabSystem/LexUIPrefabInstanceScene.h"

FLexUIPrefabEditorViewportClient::FLexUIPrefabEditorViewportClient(TWeakPtr<FLexUIPrefabEditor> InPrefabEditorPtr
	, const TSharedRef<SLexUIPrefabEditorViewport>& InEditorViewportPtr)
	// nullptr mode tools: the base class then owns a private FAssetEditorModeManager, so level editor modes never see this viewport's input
	: FEditorViewportClient(nullptr, nullptr, StaticCastSharedRef<SEditorViewport>(InEditorViewportPtr))
{
	PrefabEditorPtr = InPrefabEditorPtr;

	bShowWidget = false;

	bUsesDrawHelper = true;

	// DrawHelper set up

	DrawHelper.PerspectiveGridSize = UE_OLD_HALF_WORLD_MAX1;
	DrawHelper.AxesLineThickness = 1.0f;
	DrawHelper.bDrawGrid = true;

	EngineShowFlags.Game = 0;
	EngineShowFlags.ScreenSpaceReflections = 1;
	EngineShowFlags.AmbientOcclusion = 1;
	EngineShowFlags.SetSnap(false);

	SetRealtime(true);

	EngineShowFlags.DisableAdvancedFeatures();
	EngineShowFlags.SetSeparateTranslucency(true);
	EngineShowFlags.SetCompositeEditorPrimitives(true);
	EngineShowFlags.SetParticles(true);
	EngineShowFlags.SetSelection(true);
	EngineShowFlags.SetSelectionOutline(true);

	FVector InitialViewLocation;
	FRotator InitialViewRotation;
	FVector InitialViewOrbitLocation;
	ELevelViewportType InitialViewportType;
	InPrefabEditorPtr.Pin()->GetInitialViewSetting(InitialViewLocation, InitialViewRotation, InitialViewOrbitLocation, InitialViewportType);
	SetViewLocation(InitialViewLocation);
	this->ViewportType = InitialViewportType;
	SetViewRotation(InitialViewRotation);
	SetLookAtLocation(InitialViewOrbitLocation);
	GetPrefabBeingEdited()->GetPrefabInstanceScene()->SetSkyCubeVisibility(IsPerspective());

	OnSelectionChangedDelegateHandle = PrefabEditorPtr.Pin()->OnSelectionChanged.AddLambda([=, this]()
	{
		auto SelectedWidgets = PrefabEditorPtr.Pin()->GetSelectedWidgets();
		if (SelectedWidgets.Num() == 1 && SelectedWidgets[0].IsValid())
		{
			TransformWidget = MakeUnique<FLexUITransformWidget>(GetWorld(), SelectedWidgets[0].Get(), this);
		}
		else
		{
			TransformWidget.Reset();
		}
	});
}

FLexUIPrefabEditorViewportClient::~FLexUIPrefabEditorViewportClient()
{
	if (PrefabEditorPtr.IsValid())
	{
		PrefabEditorPtr.Pin()->OnSelectionChanged.Remove(OnSelectionChangedDelegateHandle);
	}
}

void FLexUIPrefabEditorViewportClient::Tick(float DeltaSeconds)
{
	FEditorViewportClient::Tick(DeltaSeconds);

	GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);

	if (TransformWidget.IsValid())
	{
		TransformWidget->Tick();
	}
}

bool FLexUIPrefabEditorViewportClient::InputKey(const FInputKeyEventArgs& EventArgs)
{
	bool bHandled = false;
	if (TransformWidget.IsValid())
	{
		bHandled = TransformWidget->HandleInputKey(EventArgs);
	}
	if (!bHandled)
	{
		FEditorViewportClient::InputKey(EventArgs);

		if (EventArgs.Key == EKeys::F && EventArgs.Event == IE_Pressed)
		{
			bHandled = FocusViewportToTargets();
		}
	}

	return bHandled;
}

void FLexUIPrefabEditorViewportClient::ProcessClick(FSceneView& View, HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY)
{
	FVector RayOrigin, RayDirection;
	View.DeprojectScreenToWorld(FVector2D(HitX, HitY), View.UnscaledViewRect, View.ViewMatrices.GetClipToWorld(), RayOrigin, RayDirection);
	ULexWidget* ClickHitWidget = nullptr;
	if (auto LexUIManager = ULexUIManagerWorldSubsystem::GetInstance(this->GetWorld()))
	{
		float LineTraceLength = 100000000;
		//find hit LexVisualBatchMesh
		auto LineStart = RayOrigin;
		auto LineEnd = RayOrigin + RayDirection * LineTraceLength;
		ULexWidget* ClickHitUI = nullptr;
		TArray<ULexWidget*> AllWidgetArray;
		{
			for (auto& Canvas : LexUIManager->GetAllCanvasArray())
			{
				if (!Canvas->IsRootCanvas())continue;
				auto RootWidget = Canvas->GetWidget();
				ULexWidget::CollectChildrenWidgets(RootWidget, AllWidgetArray);
			}
		}
		if (ULexUIManagerWorldSubsystem::RaycastHitUI(this->GetWorld(), AllWidgetArray, LineStart, LineEnd, ClickHitUI, IndexOfClickSelectUI))
		{
			ClickHitWidget = ClickHitUI;
		}
	}
	if (ClickHitWidget != nullptr)
	{
		const FViewportClick Click(&View, this, Key, Event, HitX, HitY);
		PrefabEditorPtr.Pin()->SelectWidgets({ ClickHitWidget }, Click.IsControlDown());
	}
}

bool FLexUIPrefabEditorViewportClient::InputWidgetDelta(FViewport* InViewport, EAxisList::Type InCurrentAxis, FVector& Drag, FRotator& Rot, FVector& Scale)
{
	if (TransformWidget.IsValid() && TransformWidget->IsDragging())
	{
		return true;
	}
	
	if (IsShiftPressed())
	{
		FVector CameraDelta(Drag);
		MoveViewportCamera(CameraDelta, FRotator::ZeroRotator);
	}

	return false;
}

void FLexUIPrefabEditorViewportClient::SetViewportType(ELevelViewportType InViewportType)
{
	FEditorViewportClient::SetViewportType(InViewportType);
	GetPrefabBeingEdited()->GetPrefabInstanceScene()->SetSkyCubeVisibility(IsPerspective());
}

bool FLexUIPrefabEditorViewportClient::FocusViewportToTargets()
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
UWorld* FLexUIPrefabEditorViewportClient::GetWorld()const
{
	return PrefabEditorPtr.Pin()->GetWorld();
}
// End override because PreviewScene is nullptr

ULexUIPrefab* FLexUIPrefabEditorViewportClient::GetPrefabBeingEdited()const
{
	return PrefabEditorPtr.Pin()->GetPrefabBeingEdited();
}

void FLexUIPrefabEditorViewportClient::CapturedMouseMove(FViewport* InViewport, int32 InMouseX, int32 InMouseY)
{
	FEditorViewportClient::CapturedMouseMove(InViewport, InMouseX, InMouseY);
	
	if (InMouseX != PrevMouseX || InMouseY != PrevMouseY)
	{
		IndexOfClickSelectUI = INDEX_NONE;
	}
	PrevMouseX = InMouseX;
	PrevMouseY = InMouseY;
}

