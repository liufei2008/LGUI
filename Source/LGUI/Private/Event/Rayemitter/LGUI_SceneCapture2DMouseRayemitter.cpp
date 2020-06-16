// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Event/Rayemitter/LGUI_SceneCapture2DMouseRayEmitter.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/SceneCapture2D.h"
#include "Engine/TextureRenderTarget2D.h"

ULGUI_SceneCapture2DMouseRayEmitter::ULGUI_SceneCapture2DMouseRayEmitter()
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

USceneCaptureComponent2D* ULGUI_SceneCapture2DMouseRayEmitter::GetSceneCapture2DComponent()
{
	if (!IsValid(sceneCaptureComp))
	{
		if (!IsValid(SceneCaptureActor))
			return nullptr;
		sceneCaptureComp = SceneCaptureActor->GetCaptureComponent2D();
		if (!IsValid(sceneCaptureComp))
			return nullptr;
	}
	return sceneCaptureComp;
}
bool ULGUI_SceneCapture2DMouseRayEmitter::EmitRay(FVector& OutRayOrigin, FVector& OutRayDirection, TArray<AActor*>& InOutTraceOnlyActors, TArray<AActor*>& InOutTraceIgnoreActors)
{
	if (!IsValid(sceneCaptureComp))
	{
		if (!IsValid(SceneCaptureActor))
			return false;
		sceneCaptureComp = SceneCaptureActor->GetCaptureComponent2D();
		if (!IsValid(sceneCaptureComp))
			return false;
	}
	//Fill InOutTraceOnlyActors and InOutTraceIgnoreActors
	if (sceneCaptureComp->PrimitiveRenderMode == ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList)
	{
		InOutTraceOnlyActors = sceneCaptureComp->ShowOnlyActors;
	}
	else
	{
		InOutTraceIgnoreActors = sceneCaptureComp->HiddenActors;
	}
	//Get mouse position, convert to range 0-1, project to SceneCapture2D
	FVector2D mousePos;
	this->GetWorld()->GetGameViewport()->GetMousePosition(mousePos);
	FVector2D viewportSize;
	this->GetWorld()->GetGameViewport()->GetViewportSize(viewportSize);
	FVector2D mousePos01 = mousePos / viewportSize;
	mousePos01.Y = 1.0f - mousePos01.Y;
	DeprojectViewPointToWorldForSceneCapture2D(sceneCaptureComp, mousePos01, OutRayOrigin, OutRayDirection);
	currentRayOrigin = OutRayOrigin;
	currentRayDirection = OutRayDirection;
	return true;
}

void SceneCapture2DViewport_BuildProjectionMatrix(FIntPoint RenderTargetSize, ECameraProjectionMode::Type ProjectionType, float FOV, float InOrthoWidth, FMatrix& ProjectionMatrix)
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
FMatrix ULGUI_SceneCapture2DMouseRayEmitter::ComputeViewProjectionMatrix(USceneCaptureComponent2D* InSceneCapture2D, bool InWithViewLocation)
{
	FMatrix viewProjectionMatrix = FMatrix::Identity;
	if (InSceneCapture2D == nullptr)return viewProjectionMatrix;
	if (InSceneCapture2D->TextureTarget == nullptr)return viewProjectionMatrix;
	FTransform Transform = InSceneCapture2D->GetComponentToWorld();
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
	const float FOV = InSceneCapture2D->FOVAngle * (float)PI / 360.0f;
	FIntPoint CaptureSize(InSceneCapture2D->TextureTarget->GetSurfaceWidth(), InSceneCapture2D->TextureTarget->GetSurfaceHeight());

	FMatrix ProjectionMatrix;
	if (InSceneCapture2D->bUseCustomProjectionMatrix)
	{
		ProjectionMatrix = InSceneCapture2D->CustomProjectionMatrix;
	}
	else
	{
		SceneCapture2DViewport_BuildProjectionMatrix(CaptureSize, InSceneCapture2D->ProjectionType, FOV, InSceneCapture2D->OrthoWidth, ProjectionMatrix);
	}

	if (InWithViewLocation)
	{
		viewProjectionMatrix = FTranslationMatrix(-ViewLocation) * ViewRotationMatrix * ProjectionMatrix;
	}
	else
	{
		viewProjectionMatrix = ViewRotationMatrix * ProjectionMatrix;
	}
	return viewProjectionMatrix;
}
void ULGUI_SceneCapture2DMouseRayEmitter::DeprojectViewPointToWorldForSceneCapture2D(USceneCaptureComponent2D* InSceneCapture2D, const FVector2D& InViewPoint01, FVector& OutWorldLocation, FVector& OutWorldDirection)
{
	FMatrix viewProjectionMatrix = ComputeViewProjectionMatrix(InSceneCapture2D);
	DeprojectViewPointToWorldForSceneCapture2D(viewProjectionMatrix, InViewPoint01, OutWorldLocation, OutWorldDirection);
	OutWorldLocation += InSceneCapture2D->GetComponentLocation();//take position out from ViewProjectionMatrix, after deproject calculation, add position to result, this can avoid float precition issue. otherwise result ray will have some obvious bias
}
void ULGUI_SceneCapture2DMouseRayEmitter::DeprojectViewPointToWorldForSceneCapture2D(const FMatrix& InViewProjectionMatrix, const FVector2D& InViewPoint01, FVector& OutWorldLocation, FVector& OutWorldDirection)
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

bool ULGUI_SceneCapture2DMouseRayEmitter::ProjectWorldToViewPointForSceneCapture2D(USceneCaptureComponent2D* InSceneCapture2D, const FVector& InWorldPosition, FVector2D& OutViewPoint)
{
	FMatrix viewProjectionMatrix = ComputeViewProjectionMatrix(InSceneCapture2D);
	return ProjectWorldToViewPointForSceneCapture2D(viewProjectionMatrix, InWorldPosition, OutViewPoint);
}
bool ULGUI_SceneCapture2DMouseRayEmitter::ProjectWorldToViewPointForSceneCapture2D(const FMatrix& InViewProjectionMatrix, const FVector& InWorldPosition, FVector2D& OutViewPoint)
{
	auto result = InViewProjectionMatrix.TransformFVector4(FVector4(InWorldPosition, 1.0f));
	if (result.W > 0.0f)
	{
		// the result of this will be x and y coords in -1..1 projection space
		const float RHW = 1.0f / result.W;
		FPlane PosInScreenSpace = FPlane(result.X * RHW, result.Y * RHW, result.Z * RHW, result.W);

		// Move from projection space to normalized 0..1 UI space
		OutViewPoint.X = (PosInScreenSpace.X / 2.f) + 0.5f;
		OutViewPoint.Y = (PosInScreenSpace.Y / 2.f) + 0.5f;

		return true;
	}
	return false;
}
bool ULGUI_SceneCapture2DMouseRayEmitter::ShouldStartDrag(ULGUIPointerEventData* InPointerEventData)
{
	FVector2D mousePos;
	if (this->GetWorld()->GetGameViewport()->GetMousePosition(mousePos))
	{
		return FVector2D::Distance(pressMountPos, mousePos) > clickThreshold;
	}
	return false;
}
void ULGUI_SceneCapture2DMouseRayEmitter::MarkPress(ULGUIPointerEventData* InPointerEventData)
{
	this->GetWorld()->GetGameViewport()->GetMousePosition(pressMountPos);
}
