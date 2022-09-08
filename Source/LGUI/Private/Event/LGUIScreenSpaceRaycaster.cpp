// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Event/LGUIScreenSpaceRaycaster.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "LGUI.h"
#include "Utils/LGUIUtils.h"

#define LOCTEXT_NAMESPACE "LGUIScreenSpaceRaycaster"

ULGUIScreenSpaceRaycaster::ULGUIScreenSpaceRaycaster()
{
	depth = 1000;//commonlly ScreenSpaceRaycaster is on top of WorldSpaceRaycaster, so make it higher
}

void ULGUIScreenSpaceRaycaster::BeginPlay()
{
	Super::BeginPlay();
}

bool ULGUIScreenSpaceRaycaster::ShouldSkipCanvas(class ULGUICanvas* UICanvas)
{
	return !UICanvas->IsRenderToScreenSpace();
}

bool ULGUIScreenSpaceRaycaster::ShouldStartDrag(ULGUIPointerEventData* InPointerEventData)
{
	if (ShouldStartDrag_HoldToDrag(InPointerEventData))return true;
	FVector2D mousePos = FVector2D(InPointerEventData->pointerPosition);
	FVector2D pressMousePos = FVector2D(InPointerEventData->pressPointerPosition);
	return FVector2D::DistSquared(pressMousePos, mousePos) > clickThresholdSquare;
}
bool ULGUIScreenSpaceRaycaster::GenerateRay(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection)
{
	if (!RootCanvas.IsValid())
	{
		auto Canvas = GetOwner()->FindComponentByClass<ULGUICanvas>();
		if (!IsValid(Canvas) || !Canvas->IsRootCanvas())
		{
			auto ErrorMsg = LOCTEXT("CanvasNotValid", "[ULGUIScreenSpaceRaycaster::GenerateRay]Canvas is not valid! LGUIScreenSpaceRaycaster can only attach to ScreenSpaceUIRoot!");
			UE_LOG(LGUI, Error, TEXT("%s"), *ErrorMsg.ToString());
			return false;
		}
		else
		{
			RootCanvas = Canvas;
		}
	}
	if (RootCanvas->GetActualRenderMode() != ELGUIRenderMode::ScreenSpaceOverlay)
		return false;

	auto ViewProjectionMatrix = RootCanvas->GetViewProjectionMatrix();
	//Get mouse position, convert to range 0-1
	FVector2D mousePos = FVector2D(InPointerEventData->pointerPosition);
	FVector2D viewportSize = RootCanvas->GetViewportSize();
	FVector2D mousePos01 = mousePos / viewportSize;
	mousePos01.Y = 1.0f - mousePos01.Y;

	DeprojectViewPointToWorld(ViewProjectionMatrix, mousePos01, OutRayOrigin, OutRayDirection);
	return true;
}

bool ULGUIScreenSpaceRaycaster::Raycast(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult, TArray<USceneComponent*>& OutHoverArray)
{
	return Super::RaycastUI(InPointerEventData, OutRayOrigin, OutRayDirection, OutRayEnd, OutHitResult, OutHoverArray);
}

void ULGUIScreenSpaceRaycaster::DeprojectViewPointToWorld(const FMatrix& InViewProjectionMatrix, const FVector2D& InViewPoint01, FVector& OutWorldLocation, FVector& OutWorldDirection)
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

#undef LOCTEXT_NAMESPACE