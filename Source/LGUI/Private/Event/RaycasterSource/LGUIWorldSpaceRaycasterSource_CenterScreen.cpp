// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Event/RaycasterSource/LGUIWorldSpaceRaycasterSource_CenterScreen.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "SceneView.h"

bool ULGUIWorldSpaceRaycasterSource_CenterScreen::GenerateRay(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection)
{
	if (auto playerController = this->GetWorld()->GetFirstPlayerController())
	{
		ULocalPlayer* const LocalPlayer = playerController->GetLocalPlayer();
		if (LocalPlayer && LocalPlayer->ViewportClient)
		{
			FVector2D ViewportSize;
			LocalPlayer->ViewportClient->GetViewportSize(ViewportSize);
			// get the projection data
			FSceneViewProjectionData ProjectionData;
			if (LocalPlayer->GetProjectionData(LocalPlayer->ViewportClient->Viewport, eSSP_FULL, /*out*/ ProjectionData))
			{
				auto ViewProMatrix = ProjectionData.ViewRotationMatrix * ProjectionData.ProjectionMatrix;//VieProjectionMatrix without position
				FMatrix const InvViewProjMatrix = ViewProMatrix.InverseFast();
				FSceneView::DeprojectScreenToWorld(ViewportSize * 0.5f, ProjectionData.GetConstrainedViewRect(), InvViewProjMatrix, /*out*/ OutRayOrigin, /*out*/ OutRayDirection);
				OutRayOrigin += ProjectionData.ViewOrigin;//take position out from ViewProjectionMatrix, after deproject calculation, add position to result, this can avoid float precition issue. otherwise result ray will have some obvious bias
				return true;
			}
		}
	}
	return false;
}
bool ULGUIWorldSpaceRaycasterSource_CenterScreen::ShouldStartDrag(ULGUIPointerEventData* InPointerEventData)
{
	if (auto IterObj = GetRaycasterObject())
	{
		FVector2D mousePos = FVector2D(InPointerEventData->pointerPosition);
		FVector2D pressMousePos = FVector2D(InPointerEventData->pressPointerPosition);
		return FVector2D::DistSquared(pressMousePos, mousePos) > IterObj->GetClickThresholdSquare();
	}
	return false;
}
