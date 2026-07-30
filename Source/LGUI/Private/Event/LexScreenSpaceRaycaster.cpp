// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Event/LexScreenSpaceRaycaster.h"
#include "Core/Components/LexCanvas.h"
#include "LGUI.h"
#include "Core/LexUISettings.h"
#include "Core/LexWidgetPresenterComponentBase.h"

#define LOCTEXT_NAMESPACE "LGUIScreenSpaceRaycaster"

ULexScreenSpaceRaycaster::ULexScreenSpaceRaycaster()
{
}

void ULexScreenSpaceRaycaster::BeginPlay()
{
	Super::BeginPlay();
	if (!RootCanvas.IsValid())
	{
		auto WidgetPresenter = GetOwner()->FindComponentByClass<ULexWidgetPresenterComponentBase>();
		if (!WidgetPresenter)
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d LexWidgetPresenterComponent is not valid! LexUIScreenSpaceRaycaster can only attach to a Actor which contains a ULexWidgetPresenterComponent!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			return;
		}
		auto Canvas = WidgetPresenter->GetLoadedCanvas();
		if (!IsValid(Canvas) || Canvas->GetActualRenderMode() != ELexRenderMode::ScreenSpaceOverlay)
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d Canvas is not valid! LexUIScreenSpaceRaycaster can only attach to ScreenSpaceUI!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			return;
		}
		RootCanvas = Canvas;
	}
}

bool ULexScreenSpaceRaycaster::GetAffectByGamePause()const
{
#if WITH_EDITOR
	if (GetWorld() && GetWorld()->IsEditorWorld())
	{
		return GetDefault<ULexUISettings>()->bScreenSpaceUIAffectByGamePause;
	}
	else
#endif
	{
		static bool Value = GetDefault<ULexUISettings>()->bScreenSpaceUIAffectByGamePause;
		return Value;
	}
}
bool ULexScreenSpaceRaycaster::ShouldStartDrag(ULexPointerEventData* InPointerEventData)
{
	if (bHoldToDrag)
	{
		if (GetWorld()->TimeSeconds - InPointerEventData->PressTime > HoldToDragTime)
		{
			return true;
		}
	}
	FVector2D mousePos = FVector2D(InPointerEventData->PointerPosition);
	FVector2D pressMousePos = FVector2D(InPointerEventData->PressPointerPosition);
	return FVector2D::DistSquared(pressMousePos, mousePos) > DragThresholdSquare;
}
bool ULexScreenSpaceRaycaster::GenerateRay(ULexPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, float& OutRayLength)
{
	if (!RootCanvas.IsValid())
		return false;

	auto ViewProjectionMatrix = RootCanvas->GetViewProjectionMatrix();
	//Get mouse position, convert to range 0-1
	FVector2D mousePos = FVector2D(InPointerEventData->PointerPosition);
	FVector2D viewportSize = RootCanvas->GetViewportSize();
	FVector2D mousePos01 = mousePos / viewportSize;
	mousePos01.Y = 1.0f - mousePos01.Y;

	DeprojectViewPointToWorld(ViewProjectionMatrix, mousePos01, OutRayOrigin, OutRayDirection);
	OutRayEnd = OutRayOrigin + OutRayDirection * RayLength;
	OutRayLength = RayLength;
	return true;
}

void ULexScreenSpaceRaycaster::Raycast(ULexPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, TArray<FLexUIHitResult>& OutHitResult)
{
	if (!RootCanvas.IsValid())return;
	Super::RaycastUI(InPointerEventData, RootCanvas.Get(), OutRayOrigin, OutRayDirection, OutRayEnd, OutHitResult);
}

void ULexScreenSpaceRaycaster::DeprojectViewPointToWorld(const FMatrix& InViewProjectionMatrix, const FVector2D& InViewPoint01, FVector& OutWorldLocation, FVector& OutWorldDirection)
{
	FMatrix InvViewProjMatrix = InViewProjectionMatrix.Inverse();

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

void ULexScreenSpaceRaycaster::SetRayLength(float Value)
{
	RayLength = Value;
}
void ULexScreenSpaceRaycaster::SetDragThreshold(float Value)
{
	DragThreshold = Value;
	DragThresholdSquare = DragThreshold * DragThreshold;
}
void ULexScreenSpaceRaycaster::SetHoldToDrag(bool Value)
{
	bHoldToDrag = Value;
}
void ULexScreenSpaceRaycaster::SetHoldToDragTime(float Value)
{
	HoldToDragTime = Value;
}

#undef LOCTEXT_NAMESPACE