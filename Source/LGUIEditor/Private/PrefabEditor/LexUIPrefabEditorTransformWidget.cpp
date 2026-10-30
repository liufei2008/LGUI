// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LexUIPrefabEditorTransformWidget.h"
#include "LexUIPrefabEditorViewportClient.h"
#include "Core/LexUIRender/LexUIRenderer.h"
#include "Utils/LexUIUtils.h"
#include "Engine/StaticMesh.h"
#include "ScopedTransaction.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabEditorTransformWidget"

void FLexUITransformWidget::UpdateAxis()
{
	auto SceneView = ViewportClient->CalcSceneView( ViewFamily.Get() );
	auto MouseX = ViewportClient->Viewport->GetMouseX();
	auto MouseY = ViewportClient->Viewport->GetMouseY();

	RenderTransform = ThisTransform;
	float RenderScale = 1;
	if (bIsDragging)
	{
		RenderScale = PressRenderScale;
		RenderTransform.SetScale3D(FVector(RenderScale, RenderScale, RenderScale));
	}
	else
	{
		if (ViewportClient->GetViewportType() != LVT_Perspective)
		{
			RenderScale = ViewportClient->GetViewTransform().GetOrthoZoom() * 0.0001f;
		}
		else
		{
			RenderScale = FVector::Dist(ViewportClient->GetViewLocation(), ThisTransform.GetTranslation()) * 1.5f / ViewportClient->Viewport->GetSizeXY().X;
		}
		if (bIsMousePressedAtThisFrame)
		{
			PressRenderScale = RenderScale; 
		}
	}
	RenderTransform.SetScale3D(FVector(RenderScale, RenderScale, RenderScale));
	
	if (bIsDragging)
	{
		if (SelectedWidget.IsValid())
		{
			FVector RayOrigin, RayDirection;
			FSceneView::DeprojectScreenToWorld(FVector2D(MouseX, MouseY), SceneView->UnscaledViewRect, SceneView->ViewMatrices.GetClipToWorld(), RayOrigin, RayDirection);

			auto Center = ThisTransform.GetTranslation();
			constexpr float Far = 1e6f;
			auto LineStartOfMouse = RayOrigin - RayDirection * Far;
			auto LineEndOfMouse = RayOrigin + RayDirection * Far;
			if (TransformType == ETransformType::Move)
			{
				FVector A = FVector::Zero(), B = FVector(BIG_NUMBER);
				FVector Diff = FVector::ZeroVector;
				switch (MoveAxisType)
				{
				case EMoveAxisType::YZ:
					{
						auto HitPoint = FMath::LinePlaneIntersection(LineStartOfMouse, LineEndOfMouse, Center, ThisTransform.GetUnitAxis(EAxis::X));
						Diff = HitPoint - PressAxisHitPoint;
					}
					break;
				case EMoveAxisType::ZX:
					{
						auto HitPoint = FMath::LinePlaneIntersection(LineStartOfMouse, LineEndOfMouse, Center, ThisTransform.GetUnitAxis(EAxis::Y));
						Diff = HitPoint - PressAxisHitPoint;
					}
					break;
				case EMoveAxisType::XY:
					{
						auto HitPoint = FMath::LinePlaneIntersection(LineStartOfMouse, LineEndOfMouse, Center, ThisTransform.GetUnitAxis(EAxis::Z));
						Diff = HitPoint - PressAxisHitPoint;
					}
					break;
				case EMoveAxisType::X:
					{
						FMath::SegmentDistToSegment(LineStartOfMouse, LineEndOfMouse, ThisTransform.TransformPosition(FVector(-Far, 0, 0)), ThisTransform.TransformPosition(FVector(Far, 0, 0)), A, B);
						Diff = B - PressAxisHitPoint;
					}
					break;
				case EMoveAxisType::Y:
					{
						FMath::SegmentDistToSegment(LineStartOfMouse, LineEndOfMouse, ThisTransform.TransformPosition(FVector(0, -Far, 0)), ThisTransform.TransformPosition(FVector(0, Far, 0)), A, B);
						Diff = B - PressAxisHitPoint;
					}
					break;
				case EMoveAxisType::Z:
					{
						FMath::SegmentDistToSegment(LineStartOfMouse, LineEndOfMouse, ThisTransform.TransformPosition(FVector(0, 0, -Far)), ThisTransform.TransformPosition(FVector(0, 0, Far)), A, B);
						Diff = B - PressAxisHitPoint;
					}
					break;
				}
				ThisTransform.SetTranslation(ThisTransformWhenPress.GetTranslation() + Diff);
				FLexUIUtils::ChangePropertyWithNotify(SelectedWidget.Get(), USceneComponent::GetRelativeLocationPropertyName(), [=, this]
				{
					SelectedWidget->SetWorldLocation(ThisTransform.GetLocation());
				});
			}
			else if (TransformType == ETransformType::Rotate)
			{
				FRotator Diff = FRotator();
				switch (RotateAxisType)
				{
				case ERotateAxisType::X:
					{
						auto LinePlaneIntersectPointX = FMath::LinePlaneIntersection(LineStartOfMouse, LineEndOfMouse, Center, RenderTransform.GetUnitAxis(EAxis::X));
						auto AxisVector = (LinePlaneIntersectPointX - Center).GetSafeNormal();
						auto DotValue = FVector::DotProduct(PressAxisVector, AxisVector);
						auto AngleInDegree = FMath::RadiansToDegrees(FMath::Acos(DotValue));
						auto CrossVector = FVector::CrossProduct(PressAxisVector, AxisVector).GetSafeNormal();
						auto AngleSign = -FMath::Sign(FVector::DotProduct(CrossVector, FVector(1, 0, 0)));
						AngleInDegree *= AngleSign;
						Diff.Roll = AngleInDegree;
					}
					break;
				case ERotateAxisType::Y:
					{
						auto LinePlaneIntersectPointX = FMath::LinePlaneIntersection(LineStartOfMouse, LineEndOfMouse, Center, RenderTransform.GetUnitAxis(EAxis::Y));
						auto AxisVector = (LinePlaneIntersectPointX - Center).GetSafeNormal();
						auto DotValue = FVector::DotProduct(PressAxisVector, AxisVector);
						auto AngleInDegree = FMath::RadiansToDegrees(FMath::Acos(DotValue));
						auto CrossVector = FVector::CrossProduct(PressAxisVector, AxisVector).GetSafeNormal();
						auto AngleSign = -FMath::Sign(FVector::DotProduct(CrossVector, FVector(0, 1, 0)));
						AngleInDegree *= AngleSign;
						Diff.Pitch = AngleInDegree;
					}
					break;
				case ERotateAxisType::Z:
					{
						auto LinePlaneIntersectPointX = FMath::LinePlaneIntersection(LineStartOfMouse, LineEndOfMouse, Center, RenderTransform.GetUnitAxis(EAxis::Z));
						auto AxisVector = (LinePlaneIntersectPointX - Center).GetSafeNormal();
						auto DotValue = FVector::DotProduct(PressAxisVector, AxisVector);
						auto AngleInDegree = FMath::RadiansToDegrees(FMath::Acos(DotValue));
						auto CrossVector = FVector::CrossProduct(PressAxisVector, AxisVector).GetSafeNormal();
						auto AngleSign = FMath::Sign(FVector::DotProduct(CrossVector, FVector(0, 0, 1)));
						AngleInDegree *= AngleSign;
						Diff.Yaw = AngleInDegree;
					}
					break;
				}
				ThisTransform.SetRotation(ThisTransformWhenPress.GetRotation() * Diff.Quaternion());
				FLexUIUtils::ChangePropertyWithNotify(SelectedWidget.Get(), USceneComponent::GetRelativeRotationPropertyName(), [=, this]
				{
					SelectedWidget->SetWorldRotation(ThisTransform.GetRotation());
				});
			}
		}
	}
	else
	{
		if (SelectedWidget.IsValid())
		{
			ThisTransform = SelectedWidget->GetWorldTransform();
		}
		constexpr uint8 AxisAlpha = 255;
		constexpr uint8 PlaneAlpha = 50;
		//reset color
		{
			MoveAxisX->SetColor(ColorAxisX.WithAlpha(AxisAlpha));
			MoveAxisY->SetColor(ColorAxisY.WithAlpha(AxisAlpha));
			MoveAxisZ->SetColor(ColorAxisZ.WithAlpha(AxisAlpha));
			MovePlaneYZ->SetColor(ColorAxisX.WithAlpha(PlaneAlpha));
			MovePlaneZX->SetColor(ColorAxisY.WithAlpha(PlaneAlpha));
			MovePlaneXY->SetColor(ColorAxisZ.WithAlpha(PlaneAlpha));
			RotateAxisX->SetColor(ColorAxisX.WithAlpha(AxisAlpha));
			RotateAxisY->SetColor(ColorAxisY.WithAlpha(AxisAlpha));
			RotateAxisZ->SetColor(ColorAxisZ.WithAlpha(AxisAlpha));
		}
		MoveAxisType = EMoveAxisType::None;
		RotateAxisType = ERotateAxisType::None;
		
		constexpr float Far = 100000000;
		FVector ViewRayOrigin, ViewRayDirection;
		FSceneView::DeprojectScreenToWorld(FVector2D(MouseX, MouseY), SceneView->UnscaledViewRect, SceneView->ViewMatrices.GetClipToWorld(), ViewRayOrigin, ViewRayDirection);
		FVector LineEnd = ViewRayOrigin + ViewRayDirection * Far;

		auto Center = ThisTransform.GetTranslation();
		if (TransformType == ETransformType::Move)
		{
			//yz plane
			{
				auto IntersectPoint = FMath::LinePlaneIntersection(ViewRayOrigin, LineEnd, Center, RenderTransform.GetUnitAxis(EAxis::X));
				auto IntersectPointLocalSpace = RenderTransform.InverseTransformPosition(IntersectPoint);
				bool bIsHit = IntersectPointLocalSpace.Y > 0 && IntersectPointLocalSpace.Y < AxisPlaneSize && IntersectPointLocalSpace.Z > 0 && IntersectPointLocalSpace.Z < AxisPlaneSize;
				MovePlaneYZ->SetColor((bIsHit ? HighlightColor : ColorAxisX).WithAlpha(PlaneAlpha));
				if (bIsHit)
				{
					MoveAxisType = EMoveAxisType::YZ;
					if (bIsMousePressedAtThisFrame)
					{
						PressAxisHitPoint = IntersectPoint;
					}
					return;
				}
			}
			//zx plane
			{
				auto IntersectPoint = FMath::LinePlaneIntersection(ViewRayOrigin, LineEnd, Center, RenderTransform.GetUnitAxis(EAxis::Y));
				auto IntersectPointLocalSpace = RenderTransform.InverseTransformPosition(IntersectPoint);
				bool bIsHit = IntersectPointLocalSpace.Z > 0 && IntersectPointLocalSpace.Z < AxisPlaneSize && IntersectPointLocalSpace.X > 0 && IntersectPointLocalSpace.X < AxisPlaneSize;
				MovePlaneZX->SetColor((bIsHit ? HighlightColor : ColorAxisY).WithAlpha(PlaneAlpha));
				if (bIsHit)
				{
					MoveAxisType = EMoveAxisType::ZX;
					if (bIsMousePressedAtThisFrame)
					{
						PressAxisHitPoint = IntersectPoint;
					}
					return;
				}
			}
			//xy plane
			{
				auto IntersectPoint = FMath::LinePlaneIntersection(ViewRayOrigin, LineEnd, Center, RenderTransform.GetUnitAxis(EAxis::Z));
				auto IntersectPointLocalSpace = RenderTransform.InverseTransformPosition(IntersectPoint);
				bool bIsHit = IntersectPointLocalSpace.X > 0 && IntersectPointLocalSpace.X < AxisPlaneSize && IntersectPointLocalSpace.Y > 0 && IntersectPointLocalSpace.Y < AxisPlaneSize;
				MovePlaneXY->SetColor((bIsHit ? HighlightColor : ColorAxisZ).WithAlpha(PlaneAlpha));
				if (bIsHit)
				{
					MoveAxisType = EMoveAxisType::XY;
					if (bIsMousePressedAtThisFrame)
					{
						PressAxisHitPoint = IntersectPoint;
					}
					return;
				}
			}
		
			FVector A = FVector::Zero(), DistanceXHitPoint = FVector(BIG_NUMBER), DistanceYHitPoint = FVector(BIG_NUMBER), DistanceZHitPoint = FVector(BIG_NUMBER);

			const float HitThreshold = 10.0f * RenderScale;
			FMath::SegmentDistToSegment(ViewRayOrigin, LineEnd, Center, RenderTransform.TransformPosition(FVector(AxisLength, 0, 0)), A, DistanceXHitPoint);
			auto DistanceToX = FVector::Dist(A, DistanceXHitPoint);

			FMath::SegmentDistToSegment(ViewRayOrigin, LineEnd, Center, RenderTransform.TransformPosition(FVector(0, AxisLength, 0)), A, DistanceYHitPoint);
			auto DistanceToY = FVector::Dist(A, DistanceYHitPoint);

			FMath::SegmentDistToSegment(ViewRayOrigin, LineEnd, Center, RenderTransform.TransformPosition(FVector(0, 0, AxisLength)), A, DistanceZHitPoint);
			auto DistanceToZ = FVector::Dist(A, DistanceZHitPoint);

			if (DistanceToX < DistanceToY && DistanceToX < DistanceToZ && DistanceToX < HitThreshold)
			{
				MoveAxisType = EMoveAxisType::X;
				MoveAxisX->SetColor(HighlightColor.WithAlpha(AxisAlpha));
				if (bIsMousePressedAtThisFrame)
				{
					PressAxisHitPoint = DistanceXHitPoint;
				}
			}

			if (DistanceToY < DistanceToX && DistanceToY < DistanceToZ && DistanceToY < HitThreshold)
			{
				MoveAxisType = EMoveAxisType::Y;
				MoveAxisY->SetColor(HighlightColor.WithAlpha(AxisAlpha));
				if (bIsMousePressedAtThisFrame)
				{
					PressAxisHitPoint = DistanceYHitPoint;
				}
			}

			if (DistanceToZ < DistanceToX && DistanceToZ < DistanceToY && DistanceToZ < HitThreshold)
			{
				MoveAxisType = EMoveAxisType::Z;
				MoveAxisZ->SetColor(HighlightColor.WithAlpha(AxisAlpha));
				if (bIsMousePressedAtThisFrame)
				{
					PressAxisHitPoint = DistanceZHitPoint;
				}
			}
		}
		else if (TransformType == ETransformType::Rotate)
		{
			const float HitThreshold = 10.0f * RenderScale, DirThreshold = -0.4f;
			FVector HitPointToCenter, HitPointToCenterDir; double DistToCenter;

			auto LinePlaneIntersectPointX = FMath::LinePlaneIntersection(ViewRayOrigin, LineEnd, Center, RenderTransform.GetUnitAxis(EAxis::X));
			HitPointToCenter = Center - LinePlaneIntersectPointX;
			HitPointToCenter.ToDirectionAndLength(HitPointToCenterDir, DistToCenter);
			auto bIsForwardToX = FVector::DotProduct(ViewRayDirection, HitPointToCenterDir) > DirThreshold;
			auto DistanceToX = FMath::Abs(DistToCenter - RotateAxisRadius * RenderScale);

			auto LinePlaneIntersectPointY = FMath::LinePlaneIntersection(ViewRayOrigin, LineEnd, Center, RenderTransform.GetUnitAxis(EAxis::Y));
			HitPointToCenter = Center - LinePlaneIntersectPointY;
			HitPointToCenter.ToDirectionAndLength(HitPointToCenterDir, DistToCenter);
			DistToCenter = FVector::Dist(LinePlaneIntersectPointY, Center);
			auto bIsForwardToY = FVector::DotProduct(ViewRayDirection, HitPointToCenterDir) > DirThreshold;
			auto DistanceToY = FMath::Abs(DistToCenter - RotateAxisRadius * RenderScale);

			auto LinePlaneIntersectPointZ = FMath::LinePlaneIntersection(ViewRayOrigin, LineEnd, Center, RenderTransform.GetUnitAxis(EAxis::Z));
			HitPointToCenter = Center - LinePlaneIntersectPointZ;
			HitPointToCenter.ToDirectionAndLength(HitPointToCenterDir, DistToCenter);
			DistToCenter = FVector::Dist(LinePlaneIntersectPointZ, Center);
			auto bIsForwardToZ = FVector::DotProduct(ViewRayDirection, HitPointToCenterDir) > DirThreshold;
			auto DistanceToZ = FMath::Abs(DistToCenter - RotateAxisRadius * RenderScale);

			if (DistanceToX < DistanceToY && DistanceToX < DistanceToZ && DistanceToX < HitThreshold && bIsForwardToX)
			{
				RotateAxisType = ERotateAxisType::X;
				RotateAxisX->SetColor(HighlightColor.WithAlpha(AxisAlpha));
				if (bIsMousePressedAtThisFrame)
				{
					PressAxisVector = (LinePlaneIntersectPointX - Center).GetSafeNormal();
				}
			}

			if (DistanceToY < DistanceToX && DistanceToY < DistanceToZ && DistanceToY < HitThreshold && bIsForwardToY)
			{
				RotateAxisType = ERotateAxisType::Y;
				RotateAxisY->SetColor(HighlightColor.WithAlpha(AxisAlpha));
				if (bIsMousePressedAtThisFrame)
				{
					PressAxisVector = (LinePlaneIntersectPointY - Center).GetSafeNormal();
				}
			}

			if (DistanceToZ < DistanceToX && DistanceToZ < DistanceToY && DistanceToZ < HitThreshold && bIsForwardToZ)
			{
				RotateAxisType = ERotateAxisType::Z;
				RotateAxisZ->SetColor(HighlightColor.WithAlpha(AxisAlpha));
				if (bIsMousePressedAtThisFrame)
				{
					PressAxisVector = (LinePlaneIntersectPointZ - Center).GetSafeNormal();
				}
			}
		}
	}
}

FLexUITransformWidget::FLexUITransformWidget(UWorld* InWorld, ULexWidget* InWidget, FLexUIPrefabEditorViewportClient* InViewportClient)
{
	World = InWorld;
	SelectedWidget = InWidget;
	ThisTransform = SelectedWidget->GetWorldTransform();
	LexUIManager = ULexUIManagerWorldSubsystem::GetInstance(InWorld);

	auto MoveAxisMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/LGUI/EditorGizmo/MoveAxis"));
	if (!MoveAxisMesh)return;
	TArray<FLexUIMeshVertex> SrcMeshVertexArray; TArray<FLexUIMeshIndex> SrcMeshIndexArray;
	FLexUIUtils::StaticMeshToLexUIMeshRenderData(MoveAxisMesh, SrcMeshVertexArray, SrcMeshIndexArray);
	{
		auto VertexArray = SrcMeshVertexArray;
		FRotator3f MoveAxisXRot = FRotator3f(-90, 0, 0);
		for (auto& Vertex : VertexArray)
		{
			Vertex.Position = MoveAxisXRot.RotateVector(Vertex.Position);
			Vertex.Color = ColorAxisX;
		}
		MoveAxisX = MakeShared<FLexUIGizmoMesh>(VertexArray, SrcMeshIndexArray, ELexUIGizmoMeshPrimitiveType::Triangle);
		MoveAxisX->UpdateLocalBounds();
	}
	{
		auto VertexArray = SrcMeshVertexArray;
		FRotator3f MoveAxisYRot = FRotator3f(0, 0, 90);
		for (auto& Vertex : VertexArray)
		{
			Vertex.Position = MoveAxisYRot.RotateVector(Vertex.Position);
			Vertex.Color = ColorAxisY;
		}
		MoveAxisY = MakeShared<FLexUIGizmoMesh>(VertexArray, SrcMeshIndexArray, ELexUIGizmoMeshPrimitiveType::Triangle);
		MoveAxisY->UpdateLocalBounds();
	}
	{
		auto VertexArray = SrcMeshVertexArray;
		for (auto& Vertex : VertexArray)
		{
			Vertex.Color = ColorAxisZ;
		}
		MoveAxisZ = MakeShared<FLexUIGizmoMesh>(VertexArray, SrcMeshIndexArray, ELexUIGizmoMeshPrimitiveType::Triangle);
		MoveAxisZ->UpdateLocalBounds();
	}
	
	auto MovePlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/LGUI/EditorGizmo/MovePlane"));
	FLexUIUtils::StaticMeshToLexUIMeshRenderData(MovePlaneMesh, SrcMeshVertexArray, SrcMeshIndexArray);
	{
		auto VertexArray = SrcMeshVertexArray;
		MovePlaneYZCenter = FVector(0, 0.15f, 0.15f);
		for (auto& Vertex : VertexArray)
		{
			Vertex.Color = ColorAxisX;
		}
		MovePlaneYZ = MakeShared<FLexUIGizmoMesh>(VertexArray, SrcMeshIndexArray, ELexUIGizmoMeshPrimitiveType::Triangle);
		MovePlaneYZ->UpdateLocalBounds();
	}
	{
		auto VertexArray = SrcMeshVertexArray;
		MovePlaneZXCenter = FVector(0.15f, 0, 0.15f);
		FRotator3f MovePlaneZXRot = FRotator3f(0, -90, 0);
		for (auto& Vertex : VertexArray)
		{
			Vertex.Position = MovePlaneZXRot.RotateVector(Vertex.Position);
			Vertex.Color = ColorAxisY;
		}
		MovePlaneZX = MakeShared<FLexUIGizmoMesh>(VertexArray, SrcMeshIndexArray, ELexUIGizmoMeshPrimitiveType::Triangle);
		MovePlaneZX->UpdateLocalBounds();
	}
	{
		auto VertexArray = SrcMeshVertexArray;
		MovePlaneXYCenter = FVector(0.15f, 0.15f, 0);
		FRotator3f MovePlaneXYRot = FRotator3f(-90, 0, 0);
		for (auto& Vertex : VertexArray)
		{
			Vertex.Position = MovePlaneXYRot.RotateVector(Vertex.Position);
			Vertex.Color = ColorAxisZ;
		}
		MovePlaneXY = MakeShared<FLexUIGizmoMesh>(VertexArray, SrcMeshIndexArray, ELexUIGizmoMeshPrimitiveType::Triangle);
		MovePlaneXY->UpdateLocalBounds();
	}

	auto RotateAxisMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/LGUI/EditorGizmo/RotateAxis"));
	FLexUIUtils::StaticMeshToLexUIMeshRenderData(RotateAxisMesh, SrcMeshVertexArray, SrcMeshIndexArray);
	{
		auto VertexArray = SrcMeshVertexArray;
		FRotator3f AxisRot = FRotator3f(90, 0, 0);
		for (auto& Vertex : VertexArray)
		{
			Vertex.Position = AxisRot.RotateVector(Vertex.Position);
			Vertex.Color = ColorAxisX;
		}
		RotateAxisX = MakeShared<FLexUIGizmoMesh>(VertexArray, SrcMeshIndexArray, ELexUIGizmoMeshPrimitiveType::Triangle);
		RotateAxisX->UpdateLocalBounds();
	}
	{
		auto VertexArray = SrcMeshVertexArray;
		FRotator3f AxisRot = FRotator3f(0, 0, 90);
		for (auto& Vertex : VertexArray)
		{
			Vertex.Position = AxisRot.RotateVector(Vertex.Position);
			Vertex.Color = ColorAxisY;
		}
		RotateAxisY = MakeShared<FLexUIGizmoMesh>(VertexArray, SrcMeshIndexArray, ELexUIGizmoMeshPrimitiveType::Triangle);
		RotateAxisY->UpdateLocalBounds();
	}
	{
		auto VertexArray = SrcMeshVertexArray;
		FRotator3f AxisRot = FRotator3f(0, 0, 0);
		for (auto& Vertex : VertexArray)
		{
			Vertex.Position = AxisRot.RotateVector(Vertex.Position);
			Vertex.Color = ColorAxisZ;
		}
		RotateAxisZ = MakeShared<FLexUIGizmoMesh>(VertexArray, SrcMeshIndexArray, ELexUIGizmoMeshPrimitiveType::Triangle);
		RotateAxisZ->UpdateLocalBounds();
	}
	
	ViewportClient = InViewportClient;
	ViewFamily = MakeUnique<FSceneViewFamilyContext>(FSceneViewFamily::ConstructionValues(
		InViewportClient->Viewport,
		InViewportClient->GetScene(),
		InViewportClient->EngineShowFlags)
		.SetRealtimeUpdate( true ) );

	if (!GizmoMaterial.IsValid())
	{
		GizmoMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/LGUI/EditorGizmo/GizmoMaterial"));
	}
	if (GizmoMaterial.IsValid())
	{
		MoveAxisX->Material
		= MoveAxisY->Material
		= MoveAxisZ->Material
		= MovePlaneYZ->Material
		= MovePlaneZX->Material
		= MovePlaneXY->Material
		= TStrongObjectPtr(GizmoMaterial.Get());
	}
	if (!RotateGizmoMaterial.IsValid())
	{
		RotateGizmoMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/LGUI/EditorGizmo/RotateGizmoMaterial"));
	}
	if (RotateGizmoMaterial.IsValid())
	{
		RotateAxisX->Material
		= RotateAxisY->Material
		= RotateAxisZ->Material
		= TStrongObjectPtr(RotateGizmoMaterial.Get());
	}

	bCanTick = true;
}

FLexUITransformWidget::~FLexUITransformWidget() = default;

void FLexUITransformWidget::Tick()
{
	if (!bCanTick)return;
	UpdateAxis();

	auto ViewExtension = ULexUIManagerWorldSubsystem::GetViewExtension(World.Get(), true);
	if (!ViewExtension)return;
	
	auto LocalToWorld = RenderTransform.ToMatrixWithScale();
	if (TransformType == ETransformType::Move)
	{
		auto ViewLocation = ViewportClient->GetViewLocation();
		struct FMovePlaneInfo
		{
			double DistanceToCamera;
			TSharedPtr<FLexUIGizmoMesh> RenderData;
		};
		TArray<FMovePlaneInfo> MovePlanes;
		MovePlanes.Add({ FVector::DistSquared(ViewLocation, RenderTransform.TransformPosition(MovePlaneYZCenter)), MovePlaneYZ});
		MovePlanes.Add({ FVector::DistSquared(ViewLocation, RenderTransform.TransformPosition(MovePlaneZXCenter)), MovePlaneZX});
		MovePlanes.Add({ FVector::DistSquared(ViewLocation, RenderTransform.TransformPosition(MovePlaneXYCenter)), MovePlaneXY});
		//simple sort on distance
		MovePlanes.Sort([](const FMovePlaneInfo& A, const FMovePlaneInfo& B)
		{			
			return A.DistanceToCamera > B.DistanceToCamera;
		});
		for (auto& MovePlane : MovePlanes)
		{
			MovePlane.RenderData->LocalToWorldMatrix = LocalToWorld;
			MovePlane.RenderData->Render(ViewExtension, false);
		}

		MoveAxisX->LocalToWorldMatrix = LocalToWorld;
		MoveAxisX->Render(ViewExtension, false);
		MoveAxisY->LocalToWorldMatrix = LocalToWorld;
		MoveAxisY->Render(ViewExtension, false);
		MoveAxisZ->LocalToWorldMatrix = LocalToWorld;
		MoveAxisZ->Render(ViewExtension, false);
	}
	else if (TransformType == ETransformType::Rotate)
	{
		RotateAxisX->LocalToWorldMatrix = LocalToWorld;
		RotateAxisX->Render(ViewExtension, false);
		RotateAxisY->LocalToWorldMatrix = LocalToWorld;
		RotateAxisY->Render(ViewExtension, false);
		RotateAxisZ->LocalToWorldMatrix = LocalToWorld;
		RotateAxisZ->Render(ViewExtension, false);
	}
}

bool FLexUITransformWidget::HandleInputKey(const FInputKeyEventArgs& EventArgs)
{
	if (EventArgs.Key == EKeys::LeftMouseButton)
	{
		if (EventArgs.Event == IE_Pressed)
		{
			bIsMousePressedAtThisFrame = true;
			UpdateAxis();
			bIsMousePressedAtThisFrame = false;
			if (MoveAxisType != EMoveAxisType::None || RotateAxisType != ERotateAxisType::None)
			{
				bIsDragging = true;
				ThisTransformWhenPress = ThisTransform;
				Transaction = MakeUnique<FScopedTransaction>(LOCTEXT("MoveWidget", "Move Widget"));
				SelectedWidget->Modify();
				return true;
			}
		}
		else if (EventArgs.Event == IE_Released)
		{
			MoveAxisType = EMoveAxisType::None;
			RotateAxisType = ERotateAxisType::None;
			if (bIsDragging)
			{
				bIsDragging = false;
				Transaction.Reset();
				return true;
			}
		}
	}
	else if (EventArgs.Key == EKeys::W)
	{
		if (EventArgs.Event == IE_Pressed)
		{
			TransformType = ETransformType::Move;
			return true;
		}
	}
	else if (EventArgs.Key == EKeys::E)
	{
		if (EventArgs.Event == IE_Pressed)
		{
			TransformType = ETransformType::Rotate;
			return true;
		}
	}
	return false;
}

#undef LOCTEXT_NAMESPACE
