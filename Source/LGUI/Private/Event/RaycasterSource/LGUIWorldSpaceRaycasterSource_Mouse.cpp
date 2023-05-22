// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Event/RaycasterSource/LGUIWorldSpaceRaycasterSource_Mouse.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "SceneView.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"

#if BUILD_VP_MATRIX_FROM_CAMERA_MANAGER
void LGUIWorldSpaceRaycasterSource_Mouse_BuildProjectionMatrix(FIntPoint RenderTargetSize, ECameraProjectionMode::Type ProjectionType, float FOV, float InOrthoWidth, FMatrix& ProjectionMatrix)
{
	float XAxisMultiplier;
	float YAxisMultiplier;

	if (RenderTargetSize.X > RenderTargetSize.Y)
	{
		// if the viewport is wider than it is tall
		XAxisMultiplier = 1.0f;
		YAxisMultiplier = RenderTargetSize.X / (float)RenderTargetSize.Y;
	}
	else
	{
		// if the viewport is taller than it is wide
		XAxisMultiplier = RenderTargetSize.Y / (float)RenderTargetSize.X;
		YAxisMultiplier = 1.0f;
	}

	if (ProjectionType == ECameraProjectionMode::Orthographic)
	{
		check((int32)ERHIZBuffer::IsInverted);
		const float OrthoWidth = InOrthoWidth / 2.0f;
		const float OrthoHeight = InOrthoWidth / 2.0f * YAxisMultiplier;

		const float NearPlane = 0;
		const float FarPlane = WORLD_MAX / 8.0f;

		const float ZScale = 1.0f / (FarPlane - NearPlane);
		const float ZOffset = -NearPlane;

		ProjectionMatrix = FReversedZOrthoMatrix(
			OrthoWidth,
			OrthoHeight,
			ZScale,
			ZOffset
		);
	}
	else
	{
		if ((int32)ERHIZBuffer::IsInverted)
		{
			ProjectionMatrix = FReversedZPerspectiveMatrix(
				FOV,
				FOV,
				XAxisMultiplier,
				YAxisMultiplier,
				GNearClippingPlane,
				GNearClippingPlane
			);
		}
		else
		{
			ProjectionMatrix = FPerspectiveMatrix(
				FOV,
				FOV,
				XAxisMultiplier,
				YAxisMultiplier,
				GNearClippingPlane,
				GNearClippingPlane
			);
		}
	}
}
FMatrix ULGUIWorldSpaceRaycasterSource_Mouse::ComputeViewProjectionMatrix(APlayerCameraManager* CameraManager, const FIntPoint& ScreenSize)
{
	FMatrix viewProjectionMatrix = FMatrix::Identity;
	if (CameraManager == nullptr)return viewProjectionMatrix;
	FMatrix viewMatrix = CameraManager->GetRootComponent()->GetComponentToWorld().Inverse().ToMatrixNoScale();
	FTransform Transform = CameraManager->GetRootComponent()->GetComponentToWorld();
	FVector ViewLocation = Transform.GetTranslation();

	// Remove the translation from Transform because we only need rotation.
	Transform.SetTranslation(FVector::ZeroVector);
	Transform.SetScale3D(FVector::OneVector);
	FMatrix ViewRotationMatrix = Transform.ToInverseMatrixWithScale();

	// swap axis st. x=z,y=x,z=y (unreal coord space) so that z is up
	ViewRotationMatrix = ViewRotationMatrix * FMatrix(
		FPlane(0, 0, 1, 0),
		FPlane(1, 0, 0, 0),
		FPlane(0, 1, 0, 0),
		FPlane(0, 0, 0, 1));
	const float FOV = CameraManager->GetFOVAngle() * (float)PI / 360.0f;

	FMatrix ProjectionMatrix;
	LGUIWorldSpaceRaycasterSource_Mouse_BuildProjectionMatrix(ScreenSize
		, CameraManager->IsOrthographic() ? ECameraProjectionMode::Orthographic : ECameraProjectionMode::Perspective
		, FOV, CameraManager->GetOrthoWidth(), ProjectionMatrix);

	viewProjectionMatrix = ViewRotationMatrix * ProjectionMatrix;
	return viewProjectionMatrix;
}

void ULGUIWorldSpaceRaycasterSource_Mouse::DeprojectViewPointToWorldForMainViewport(const FMatrix& InViewProjectionMatrix, const FVector2D& InViewPoint01, FVector& OutWorldLocation, FVector& OutWorldDirection)
{
	FMatrix InvViewProjMatrix = InViewProjectionMatrix.InverseFast();

	const float ScreenSpaceX = (InViewPoint01.X - 0.5f) * 2.0f;
	const float ScreenSpaceY = (InViewPoint01.Y - 0.5f) * 2.0f;

	// The start of the raytrace is defined to be at mousex,mousey,1 in projection space (z=1 is near, z=0 is far - this gives us better precision)
	// To get the direction of the raytrace we need to use any z between the near and the far plane, so let's use (mousex, mousey, 0.5)
	const FVector4 RayStartProjectionSpace = FVector4(ScreenSpaceX, ScreenSpaceY, 1.0f, 1.0f);
	const FVector4 RayEndProjectionSpace = FVector4(ScreenSpaceX, ScreenSpaceY, 0.5f, 1.0f);

	// Projection (changing the W coordinate) is not handled by the FMatrix transforms that work with vectors, so multiplications
	// by the projection matrix should use homogeneous coordinates (i.e. FPlane).
	const FVector4 HGRayStartWorldSpace = InvViewProjMatrix.TransformFVector4(RayStartProjectionSpace);
	const FVector4 HGRayEndWorldSpace = InvViewProjMatrix.TransformFVector4(RayEndProjectionSpace);
	FVector RayStartWorldSpace(HGRayStartWorldSpace.X, HGRayStartWorldSpace.Y, HGRayStartWorldSpace.Z);
	FVector RayEndWorldSpace(HGRayEndWorldSpace.X, HGRayEndWorldSpace.Y, HGRayEndWorldSpace.Z);
	// divide vectors by W to undo any projection and get the 3-space coordinate 
	if (HGRayStartWorldSpace.W != 0.0f)
	{
		RayStartWorldSpace /= HGRayStartWorldSpace.W;
	}
	if (HGRayEndWorldSpace.W != 0.0f)
	{
		RayEndWorldSpace /= HGRayEndWorldSpace.W;
	}
	const FVector RayDirWorldSpace = (RayEndWorldSpace - RayStartWorldSpace).GetSafeNormal();

	// Finally, store the results in the outputs
	OutWorldLocation = RayStartWorldSpace;
	OutWorldDirection = RayDirWorldSpace;
}
#endif


bool ULGUIWorldSpaceRaycasterSource_Mouse::GenerateRay(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection)
{
#if BUILD_VP_MATRIX_FROM_CAMERA_MANAGER
	if (auto pc = GetWorld()->GetFirstPlayerController())
	{
		FIntPoint ScreenSize;
		pc->GetViewportSize(ScreenSize.X, ScreenSize.Y);
		FMatrix viewProjectionMatrix = ComputeViewProjectionMatrix(pc->PlayerCameraManager, ScreenSize);
		FVector2D MousePosition = FVector2D(InPointerEventData->pointerPosition);
		MousePosition.X /= ScreenSize.X;
		MousePosition.Y /= ScreenSize.Y;
		MousePosition.Y = 1.0f - MousePosition.Y;
		DeprojectViewPointToWorldForMainViewport(viewProjectionMatrix, MousePosition, OutRayOrigin, OutRayDirection);
		OutRayOrigin += pc->PlayerCameraManager->GetRootComponent()->GetComponentLocation();//take position out from ViewProjectionMatrix, after deproject calculation, add position to result, this can avoid float precition issue. otherwise result ray will have some obvious bias
		return true;
	}
#else
	if (auto playerController = this->GetWorld()->GetFirstPlayerController())
	{
		ULocalPlayer* const LocalPlayer = playerController->GetLocalPlayer();
		if (LocalPlayer && LocalPlayer->ViewportClient)
		{
			FVector2D ScreenPosition = FVector2D(InPointerEventData->pointerPosition);
			// get the projection data
			FSceneViewProjectionData ProjectionData;
			if (LocalPlayer->GetProjectionData(LocalPlayer->ViewportClient->Viewport, ProjectionData))
			{
				auto ViewProMatrix = ProjectionData.ViewRotationMatrix * ProjectionData.ProjectionMatrix;//VieProjectionMatrix without position
				FMatrix const InvViewProjMatrix = ViewProMatrix.InverseFast();
				FSceneView::DeprojectScreenToWorld(ScreenPosition, ProjectionData.GetConstrainedViewRect(), InvViewProjMatrix, /*out*/ OutRayOrigin, /*out*/ OutRayDirection);
				OutRayOrigin += ProjectionData.ViewOrigin;//take position out from ViewProjectionMatrix, after deproject calculation, add position to result, this can avoid float precition issue. otherwise result ray will have some obvious bias
				return true;
			}
		}
	}
#endif
	return false;
}
bool ULGUIWorldSpaceRaycasterSource_Mouse::ShouldStartDrag(ULGUIPointerEventData* InPointerEventData)
{
	if (auto IterObj = GetRaycasterObject())
	{
		FVector2D mousePos = FVector2D(InPointerEventData->pointerPosition);
		FVector2D pressMousePos = FVector2D(InPointerEventData->pressPointerPosition);
		return FVector2D::DistSquared(pressMousePos, mousePos) > IterObj->GetClickThresholdSquare();
	}
	return false;
}
