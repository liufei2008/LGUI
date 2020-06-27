// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Event/Rayemitter/LGUI_ScreenSpaceUIMouseRayemitter.h"
#include "Core/ActorComponent/LGUICanvas.h"

bool ULGUI_ScreenSpaceUIMouseRayemitter::EmitRay(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, TArray<AActor*>& InOutTraceOnlyActors, TArray<AActor*>& InOutTraceIgnoreActors)
{
	if (!IsValid(RenderCanvas))
		return false;
	if (RenderCanvas->GetRenderMode() == ELGUIRenderMode::WorldSpace)
		return false;

	auto ViewProjectionMatrix = RenderCanvas->GetViewProjectionMatrix();
	//Get mouse position, convert to range 0-1
	FVector2D mousePos = FVector2D(InPointerEventData->pointerPosition);
	FVector2D viewportSize = RenderCanvas->GetViewportSize();
	FVector2D mousePos01 = mousePos / viewportSize;
	mousePos01.Y = 1.0f - mousePos01.Y;

	DeprojectViewPointToWorld(ViewProjectionMatrix, mousePos01, OutRayOrigin, OutRayDirection);
	currentRayOrigin = OutRayOrigin;
	currentRayDirection = OutRayDirection;
	return true;
}

void ULGUI_ScreenSpaceUIMouseRayemitter::DeprojectViewPointToWorld(const FMatrix& InViewProjectionMatrix, const FVector2D& InViewPoint01, FVector& OutWorldLocation, FVector& OutWorldDirection)
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

bool ULGUI_ScreenSpaceUIMouseRayemitter::ShouldStartDrag(ULGUIPointerEventData* InPointerEventData)
{
	if (ShouldStartDrag_HoldToDrag(InPointerEventData))return true;
	FVector2D mousePos = FVector2D(InPointerEventData->pointerPosition);
	return FVector2D::DistSquared(pressMousePos, mousePos) > clickTresholdSquare;
}
void ULGUI_ScreenSpaceUIMouseRayemitter::MarkPress(ULGUIPointerEventData* InPointerEventData)
{
	pressMousePos = FVector2D(InPointerEventData->pointerPosition);
}
