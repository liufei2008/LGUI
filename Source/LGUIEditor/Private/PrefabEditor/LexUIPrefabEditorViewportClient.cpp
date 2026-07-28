// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LexUIPrefabEditorViewportClient.h"
#include "LexUIPrefabEditorViewport.h"
#include "Components/DirectionalLightComponent.h"
#include "Animation/AnimationAsset.h"
#include "GameFramework/Actor.h"
#include "Math/Vector.h"
#include "AssetEditorModeManager.h"
#include "EngineUtils.h"
#include "Engine/Selection.h"
#include "SceneView.h"
#include "Editor/UnrealEdEngine.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Editor.h"
#include "LexUIPrefabEditor.h"
#include "MouseDeltaTracker.h"
#include "Misc/ITransaction.h"
#include "UnrealEdGlobals.h"
#include "UnrealWidget.h"
#include "Elements/Framework/TypedElementRegistry.h"
#include "Elements/Framework/EngineElementsLibrary.h"
#include "Elements/Framework/TypedElementViewportInteraction.h"
#include "InputState.h"
#include "ViewportSelectionUtilities.h"
#include "HModel.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "LexUIPrefabViewportClickHandlers.h"
#include "Core/LexUIManager.h"
#include "Core/Components/LexCanvas.h"
#include "Core/Components/LexWidget.h"
#include "Core/LexUIMesh/LexUIGizmoMesh.h"
#include "Core/LexUIRender/LexUIRenderer.h"
#include "PrefabSystem/LexUIPrefabInstanceScene.h"
#include "Utils/LexUIUtils.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabEditorViewportClient"

IMPLEMENT_HIT_PROXY(HLevelSocketProxy, HHitProxy);

class FLexUITransformWidget
{
private:		
	int PressMouseX = 0, PressMouseY = 0; FVector PressAxisHitPoint = FVector::Zero();
	FVector PressAxisVector = FVector::ZeroVector;
	FTransform ThisTransformWhenPress = FTransform::Identity;
	FTransform ThisTransform = FTransform::Identity;
	FTransform RenderTransform = FTransform::Identity;
	float PressRenderScale = 1.0f;
	TWeakObjectPtr<ULexUIManagerWorldSubsystem> LexUIManager;
	TWeakObjectPtr<UWorld> World;
	TSharedPtr<FLexUIGizmoMesh> MoveAxisX;
	TSharedPtr<FLexUIGizmoMesh> MoveAxisY;
	TSharedPtr<FLexUIGizmoMesh> MoveAxisZ;
	TSharedPtr<FLexUIGizmoMesh> MovePlaneYZ;
	TSharedPtr<FLexUIGizmoMesh> MovePlaneZX;
	TSharedPtr<FLexUIGizmoMesh> MovePlaneXY;
	TSharedPtr<FLexUIGizmoMesh> RotateAxisX;
	TSharedPtr<FLexUIGizmoMesh> RotateAxisY;
	TSharedPtr<FLexUIGizmoMesh> RotateAxisZ;
	TWeakObjectPtr<UMaterialInterface> GizmoMaterial;
	TWeakObjectPtr<UMaterialInterface> RotateGizmoMaterial;
	FVector MovePlaneYZCenter;
	FVector MovePlaneZXCenter;
	FVector MovePlaneXYCenter;
	const float AxisLength = 100.0f;
	const float AxisPlaneSize = 30.0f;
	const float RotateAxisRadius = 100.0f;
	FColor ColorAxisX = FColor::Red, ColorAxisY = FColor::Green, ColorAxisZ = FColor::Blue;
	FColor HighlightColor = FColor::Yellow;
	FString DebugName;
	bool bCanTick = false;
	enum class EMoveAxisType
	{
		None, X, Y, Z, YZ, ZX, XY, 
	};
	EMoveAxisType MoveAxisType = EMoveAxisType::None;
	enum class ERotateAxisType
	{
		None, X, Y, Z,
	};
	ERotateAxisType RotateAxisType = ERotateAxisType::None;
	enum class ETransformType
	{
		None, Move, Rotate,
	};
	ETransformType TransformType = ETransformType::Move;
	bool bIsMousePressedAtThisFrame = false;
	bool bIsMouseReleasedAtThisFrame = false;
	bool bIsDragging = false;
	TWeakObjectPtr<ULexWidget> SelectedWidget;
	FLexUIPrefabEditorViewportClient* ViewportClient = nullptr;
	TUniquePtr<FSceneViewFamilyContext> ViewFamily = nullptr;
	void UpdateAxis()
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
				FSceneView::DeprojectScreenToWorld(FVector2D(MouseX, MouseY), SceneView->UnscaledViewRect, SceneView->ViewMatrices.GetInvViewProjectionMatrix(), RayOrigin, RayDirection);

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
			FSceneView::DeprojectScreenToWorld(FVector2D(MouseX, MouseY), SceneView->UnscaledViewRect, SceneView->ViewMatrices.GetInvViewProjectionMatrix(), ViewRayOrigin, ViewRayDirection);
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
	TUniquePtr<FScopedTransaction> Transaction = nullptr;
public:
	FLexUITransformWidget(UWorld* InWorld, ULexWidget* InWidget, FLexUIPrefabEditorViewportClient* InViewportClient)
	{
		World = InWorld;
		SelectedWidget = InWidget;
		ThisTransform = SelectedWidget->GetWorldTransform();
		LexUIManager = ULexUIManagerWorldSubsystem::GetInstance(InWorld);
		DebugName = TEXT("LexUITransformWidget");
		
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
				Vertex.Color = ColorAxisY;
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
	~FLexUITransformWidget()
	{
	}
	void Tick()
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
	bool IsDragging()const{return bIsDragging;}
	bool HandleInputKey(const FInputKeyEventArgs& EventArgs)
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
					PressMouseX = EventArgs.Viewport->GetMouseX();
					PressMouseY = EventArgs.Viewport->GetMouseY();
					ThisTransformWhenPress = ThisTransform;
					Transaction = MakeUnique<FScopedTransaction>(LOCTEXT("MoveWidget", "Move Widget"));
					SelectedWidget->Modify();
					return true;
				}
			}
			else if (EventArgs.Event == IE_Released)
			{
				bIsMouseReleasedAtThisFrame = true;
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
};

FLexUIPrefabEditorViewportClient::FLexUIPrefabEditorViewportClient(TWeakPtr<FLexUIPrefabEditor> InPrefabEditorPtr
	, const TSharedRef<SLexUIPrefabEditorViewport>& InEditorViewportPtr)
	: FEditorViewportClient(&GLevelEditorModeTools(), nullptr, StaticCastSharedRef<SEditorViewport>(InEditorViewportPtr))
	, TrackingTransaction()
	, CachedElementsToManipulate(UTypedElementRegistry::GetInstance()->CreateElementList())
{
	PrefabEditorPtr = InPrefabEditorPtr;
	// The level editor fully supports mode tools and isn't doing any incompatible stuff with the Widget
	ModeTools->SetWidgetMode(UE::Widget::WM_Translate);
	Widget->SetUsesEditorModeTools(ModeTools.Get());
	bShowWidget = false;

	// GEditorModeTools serves as our draw helper
	bUsesDrawHelper = true;

	// DrawHelper set up

	DrawHelper.PerspectiveGridSize = HALF_WORLD_MAX1;
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
void FLexUIPrefabEditorViewportClient::Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	FMemMark Mark(FMemStack::Get());

	//Draw grid
	{
#if 0
		auto ScreenColorRenderTargetTexture = View->Family->RenderTarget->GetRenderTargetTexture();
		if (ScreenColorRenderTargetTexture != nullptr)
		{
			static UTexture2D* GridTexture = Cast<UTexture2D>(FAppStyle::GetBrush("Checkerboard")->GetResourceObject());
			if (GridTexture == nullptr)
			{
				GridTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineMaterials/DefaultWhiteGrid.DefaultWhiteGrid"), nullptr, LOAD_None, nullptr);
			}
			const bool bAlphaBlend = false;
			Canvas.DrawTile(
				0,
				0,
				InViewport.GetSizeXY().X,
				InViewport.GetSizeXY().Y,
				0.0f,
				0.0f,
				4.0f,
				4.0f,
				FLinearColor(0.15f, 0.15f, 0.15f),
				GridTexture->GetResource(),
				bAlphaBlend);
		}
#endif
	}

	FEditorViewportClient::Draw(View, PDI);

	//AGroupActor::DrawBracketsForGroups(PDI, Viewport);

	// A frustum should be drawn if the viewport is ortho and level streaming volume previs is enabled in some viewport
	if (IsOrtho())
	{
		for (FLevelEditorViewportClient* CurViewportClient : GEditor->GetLevelViewportClients())
		{
			if (CurViewportClient && IsPerspective() && GetDefault<ULevelEditorViewportSettings>()->bLevelStreamingVolumePrevis)
			{
				// Draw the view frustum of the level streaming volume previs viewport.
				RenderViewFrustum(PDI, FLinearColor(1.0, 0.0, 1.0, 1.0),
					GPerspFrustumAngle,
					GPerspFrustumAspectRatio,
					GPerspFrustumStartDist,
					GPerspFrustumEndDist,
					GPerspViewMatrix);

				break;
			}
		}
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
void FLexUIPrefabEditorViewportClient::DrawCanvas(FViewport& InViewport, FSceneView& View, FCanvas& Canvas)
{	
	if (GUnrealEd != nullptr && !IsInGameView())
	{
		GUnrealEd->DrawComponentVisualizersHUD(&InViewport, &View, &Canvas);
	}

	FEditorViewportClient::DrawCanvas(InViewport, View, Canvas);
}

void FLexUIPrefabEditorViewportClient::ReceivedFocus(FViewport* InViewport)
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

void FLexUIPrefabEditorViewportClient::LostFocus(FViewport* InViewport)
{
	FEditorViewportClient::LostFocus(InViewport);

	GEditor->SetPreviewMeshMode(false);
}

void FLexUIPrefabEditorViewportClient::Tick(float DeltaSeconds)
{
	FEditorViewportClient::Tick(DeltaSeconds);

	TickWorld(DeltaSeconds);

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
		bHandled = GUnrealEd->ComponentVisManager.HandleInputKey(this, EventArgs.Viewport, EventArgs.Key, EventArgs.Event);
	}
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

void FLexUIPrefabEditorViewportClient::ProcessClick(FSceneView& View, HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY)
{
	const FViewportClick Click(&View, this, Key, Event, HitX, HitY);

	FVector RayOrigin, RayDirection;
	View.DeprojectScreenToWorld(FVector2D(HitX, HitY), View.UnscaledViewRect, View.ViewMatrices.GetInvViewProjectionMatrix(), RayOrigin, RayDirection);
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
				if (!Canvas->IsRootCanvas())continue;;
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
		PrefabEditorPtr.Pin()->SelectWidgets({ClickHitWidget}, Click.IsControlDown());
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
		LexUIPrefabViewportClickHandlers::ClickViewport(this, Click);
		return;
	}
	if (!ModeTools->HandleClick(this, HitProxy, Click))
	{
		const FTypedElementHandle HitElement = HitProxy ? HitProxy->GetElementHandle() : FTypedElementHandle();

		if (HitProxy == NULL)
		{
			LexUIPrefabViewportClickHandlers::ClickBackdrop(this, Click);
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
		else if (HitElement && LexUIPrefabViewportClickHandlers::ClickElement(this, HitElement, Click))
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
					bComponentSelected = LexUIPrefabViewportClickHandlers::ClickComponent(this, ActorHitProxy, Click);
				}

				if (!bComponentSelected)
				{
					LexUIPrefabViewportClickHandlers::ClickActor(this, ConsideredActor, Click, true);
				}

				// We clicked an actor, allow the pivot to reposition itself.
				// GUnrealEd->SetPivotMovedIndependently(false);
			}
		}
		else if (HitProxy->IsA(HInstancedStaticMeshInstance::StaticGetType()))
		{
			LexUIPrefabViewportClickHandlers::ClickActor(this, ((HInstancedStaticMeshInstance*)HitProxy)->Component->GetOwner(), Click, true);
		}
		//else if (HitProxy->IsA(HBSPBrushVert::StaticGetType()) && ((HBSPBrushVert*)HitProxy)->Brush.IsValid())
		//{
		//	FVector Vertex = FVector(*((HBSPBrushVert*)HitProxy)->Vertex);
		//	LGUIPrefabViewportClickHandlers::ClickBrushVertex(this, ((HBSPBrushVert*)HitProxy)->Brush.Get(), &Vertex, Click);
		//}
		else if (HitProxy->IsA(HStaticMeshVert::StaticGetType()))
		{
			LexUIPrefabViewportClickHandlers::ClickStaticMeshVertex(this, ((HStaticMeshVert*)HitProxy)->Actor, ((HStaticMeshVert*)HitProxy)->Vertex, Click);
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
				LexUIPrefabViewportClickHandlers::ClickSurface(this, ModelHit->GetModel(), SurfaceIndex, Click);
			}
		}
		else if (HitProxy->IsA(HLevelSocketProxy::StaticGetType()))
		{
			LexUIPrefabViewportClickHandlers::ClickLevelSocket(this, HitProxy, Click);
		}
	}
}

bool FLexUIPrefabEditorViewportClient::InputWidgetDelta(FViewport* InViewport, EAxisList::Type InCurrentAxis, FVector& Drag, FRotator& Rot, FVector& Scale)
{
	if (TransformWidget.IsValid() && TransformWidget->IsDragging())
	{
		return true;
	}
	
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
UE::Widget::EWidgetMode FLexUIPrefabEditorViewportClient::GetWidgetMode() const
{
	if (GUnrealEd->ComponentVisManager.IsActive() && GUnrealEd->ComponentVisManager.IsVisualizingArchetype())
	{
		return UE::Widget::WM_None;
	}

	return FEditorViewportClient::GetWidgetMode();
}
FVector FLexUIPrefabEditorViewportClient::GetWidgetLocation() const
{
	FVector ComponentVisWidgetLocation;
	if (GUnrealEd->ComponentVisManager.GetWidgetLocation(this, ComponentVisWidgetLocation))
	{
		return ComponentVisWidgetLocation;
	}

	return FEditorViewportClient::GetWidgetLocation();
}
FMatrix FLexUIPrefabEditorViewportClient::GetWidgetCoordSystem() const
{
	FMatrix ComponentVisWidgetCoordSystem;
	if (GUnrealEd->ComponentVisManager.GetCustomInputCoordinateSystem(this, ComponentVisWidgetCoordSystem))
	{
		return ComponentVisWidgetCoordSystem;
	}

	return FEditorViewportClient::GetWidgetCoordSystem();
}

void FLexUIPrefabEditorViewportClient::SetViewportType(ELevelViewportType InViewportType)
{
	FEditorViewportClient::SetViewportType(InViewportType);
	GetPrefabBeingEdited()->GetPrefabInstanceScene()->SetSkyCubeVisibility(IsPerspective());
}

/**
 * Returns the horizontal axis for this viewport.
 */

EAxisList::Type FLexUIPrefabEditorViewportClient::GetHorizAxis() const
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

EAxisList::Type FLexUIPrefabEditorViewportClient::GetVertAxis() const
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
void FLexUIPrefabEditorViewportClient::NudgeSelectedObjects(const struct FInputEventState& InputState)
{
	FViewport* InViewport = InputState.GetViewport();
	EInputEvent Event = InputState.GetInputEvent();
	FKey Key = InputState.GetKey();

	const int32 MouseX = InViewport->GetMouseX();
	const int32 MouseY = InViewport->GetMouseY();

	if (Event == IE_Pressed)
	{
		GEditor->BeginTransaction(LOCTEXT("MoveWidget", "Move Widget"));
		for (auto LexWidget : PrefabEditorPtr.Pin()->GetSelectedWidgets())
		{
			LexWidget->Modify();
		}
	}
	else if (Event == IE_Released)
	{
		GEditor->EndTransaction();
	}
	
	if (Event == IE_Pressed || Event == IE_Repeat)
	{
		FVector2D MouseDelta(0,0);
		if (Key == EKeys::Left) MouseDelta.X = -1;
		else if (Key == EKeys::Right) MouseDelta.X = 1;
		else if (Key == EKeys::Up) MouseDelta.Y = 1;
		else if (Key == EKeys::Down) MouseDelta.Y = -1;
		if (GetDefault<ULevelEditorViewportSettings>()->bEnableActorSnap)
		{
			MouseDelta *= GEditor->GetGridSize();
		}
		
		for (auto LexWidget : PrefabEditorPtr.Pin()->GetSelectedWidgets())
		{
			auto AnchoredPos = LexWidget->GetAnchoredPosition();
			AnchoredPos += MouseDelta;
			LexWidget->SetAnchoredPosition(AnchoredPos);
		}
	}

	RedrawAllViewportsIntoThisScene();
}

void FLexUIPrefabEditorViewportClient::ApplyDeltaToActors(const FVector& InDrag, const FRotator& InRot, const FVector& InScale)
{
	ApplyDeltaToSelectedElements(FTransform(InRot, InDrag, InScale));
}

void FLexUIPrefabEditorViewportClient::ApplyDeltaToActor(AActor* InActor, const FVector& InDeltaDrag, const FRotator& InDeltaRot, const FVector& InDeltaScale)
{
	if (FTypedElementHandle ActorElementHandle = UEngineElementsLibrary::AcquireEditorActorElementHandle(InActor))
	{
		ApplyDeltaToElement(ActorElementHandle, FTransform(InDeltaRot, InDeltaDrag, InDeltaScale));
	}
}

void FLexUIPrefabEditorViewportClient::ApplyDeltaToComponent(USceneComponent* InComponent, const FVector& InDeltaDrag, const FRotator& InDeltaRot, const FVector& InDeltaScale)
{
	if (FTypedElementHandle ComponentElementHandle = UEngineElementsLibrary::AcquireEditorComponentElementHandle(InComponent))
	{
		ApplyDeltaToElement(ComponentElementHandle, FTransform(InDeltaRot, InDeltaDrag, InDeltaScale));
	}
}

void FLexUIPrefabEditorViewportClient::ApplyDeltaToSelectedElements(const FTransform& InDeltaTransform)
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

void FLexUIPrefabEditorViewportClient::ApplyDeltaToElement(const FTypedElementHandle& InElementHandle, const FTransform& InDeltaTransform)
{
	FInputDeviceState InputState;
	InputState.SetModifierKeyStates(IsShiftPressed(), IsAltPressed(), IsCtrlPressed(), IsCmdPressed());

	ViewportInteraction->ApplyDeltaToElement(InElementHandle, GetWidgetMode(), Widget ? Widget->GetCurrentAxis() : EAxisList::None, InputState, InDeltaTransform);
}

FTypedElementListConstRef FLexUIPrefabEditorViewportClient::GetElementsToManipulate(const bool bForceRefresh)
{
	CacheElementsToManipulate(bForceRefresh);
	return CachedElementsToManipulate;
}

void FLexUIPrefabEditorViewportClient::CacheElementsToManipulate(const bool bForceRefresh)
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
void FLexUIPrefabEditorViewportClient::ResetElementsToManipulate(const bool bClearList)
{
	if (bClearList)
	{
		CachedElementsToManipulate->Reset();
	}
	bHasCachedElementsToManipulate = false;
}

void FLexUIPrefabEditorViewportClient::ResetElementsToManipulateFromSelectionChange(const UTypedElementSelectionSet* InSelectionSet)
{
	check(InSelectionSet == GetSelectionSet());

	// Don't clear the list immediately, as the selection may change from a construction script running (while we're still iterating the list!)
	// We'll process the clear on the next cache request, or when the typed element registry actually processes its pending deletion
	ResetElementsToManipulate(/*bClearList*/false);
}

void FLexUIPrefabEditorViewportClient::ResetElementsToManipulateFromProcessingDeferredElementsToDestroy()
{
	if (!bHasCachedElementsToManipulate)
	{
		// If we have no cache, make sure the cached list is definitely empty now to ensure it doesn't contain any lingering references to things that are about to be deleted
		CachedElementsToManipulate->Reset();
	}
}

const UTypedElementSelectionSet* FLexUIPrefabEditorViewportClient::GetSelectionSet() const
{
	return GEditor->GetSelectedActors()->GetElementSelectionSet();
}

UTypedElementSelectionSet* FLexUIPrefabEditorViewportClient::GetMutableSelectionSet() const
{
	return GEditor->GetSelectedActors()->GetElementSelectionSet();
}


void FLexUIPrefabEditorViewportClient::TickWorld(float DeltaSeconds)
{
	GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
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
// These implementation are copied from FEditorViewportClient
UWorld* FLexUIPrefabEditorViewportClient::GetWorld()const
{
	return PrefabEditorPtr.Pin()->GetWorld();
}
void FLexUIPrefabEditorViewportClient::AddReferencedObjects(FReferenceCollector& Collector)
{
	FEditorViewportClient::AddReferencedObjects(Collector);
	PrefabEditorPtr.Pin()->GetPreviewScene()->AddReferencedObjects(Collector);
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
void FLexUIPrefabEditorViewportClient::DrawPreviewLightVisualization(const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	// Draw the indicator of the current light direction if it was recently moved
	auto PrefabScene = PrefabEditorPtr.Pin()->GetPreviewScene();
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
FLinearColor FLexUIPrefabEditorViewportClient::GetBackgroundColor() const
{
	auto PrefabScene = PrefabEditorPtr.Pin()->GetPreviewScene();
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
bool FLexUIPrefabEditorViewportClient::Internal_InputAxis(FViewport* InViewport, FInputDeviceId DeviceID, FKey Key, float Delta, float DeltaTime, int32 NumSamples, bool bGamepad)
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

	auto PrefabScene = PrefabEditorPtr.Pin()->GetPreviewScene();
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


ULexUIPrefab* FLexUIPrefabEditorViewportClient::GetPrefabBeingEdited()const
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

void FLexUIPrefabEditorViewportClient::GetSelectedActorsAndComponentsForMove(TArray<AActor*>& OutActorsToMove, TArray<USceneComponent*>& OutComponentsToMove) const
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

bool FLexUIPrefabEditorViewportClient::CanMoveActorInViewport(const AActor* InActor) const
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

void FLexUIPrefabEditorViewportClient::CapturedMouseMove(FViewport* InViewport, int32 InMouseX, int32 InMouseY)
{
	// Commit to any pending transactions now
	TrackingTransaction.PromotePendingToActive();

	FEditorViewportClient::CapturedMouseMove(InViewport, InMouseX, InMouseY);
	
	if (InMouseX != PrevMouseX || InMouseY != PrevMouseY)
	{
		IndexOfClickSelectUI = INDEX_NONE;
	}
	PrevMouseX = InMouseX;
	PrevMouseY = InMouseY;
}

void FLexUIPrefabEditorViewportClient::MouseEnter(FViewport* InViewport, int32 x, int32 y)
{
	FEditorViewportClient::MouseEnter(InViewport, x, y);
}
void FLexUIPrefabEditorViewportClient::MouseMove(FViewport* InViewport, int32 x, int32 y)
{
	FEditorViewportClient::MouseMove(InViewport, x, y);
}
void FLexUIPrefabEditorViewportClient::MouseLeave(FViewport* InViewport)
{
	FEditorViewportClient::MouseLeave(InViewport);
}

void FLexUIPrefabEditorViewportClient::TrackingStarted(const struct FInputEventState& InInputState, bool bIsDraggingWidget, bool bNudge)
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
void FLexUIPrefabEditorViewportClient::TrackingStopped()
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

void FLexUIPrefabEditorViewportClient::AbortTracking()
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

bool FLexUIPrefabEditorViewportClient::HaveSelectedObjectsBeenChanged() const
{
	return (TrackingTransaction.TransCount > 0 || TrackingTransaction.IsActive()) && (MouseDeltaTracker->HasReceivedDelta() || MouseDeltaTracker->WasExternalMovement());
}


#undef LOCTEXT_NAMESPACE
