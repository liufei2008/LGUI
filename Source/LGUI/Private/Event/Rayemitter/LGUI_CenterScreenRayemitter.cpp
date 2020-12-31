// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Event/Rayemitter/LGUI_CenterScreenRayemitter.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "SceneView.h"

bool ULGUI_CenterScreenRayemitter::EmitRay(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, TArray<AActor*>& InOutTraceOnlyActors, TArray<AActor*>& InOutTraceIgnoreActors)
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
				currentRayOrigin = OutRayOrigin;
				currentRayDirection = OutRayDirection;
				return true;
			}
		}
	}
	return false;
}
bool ULGUI_CenterScreenRayemitter::ShouldStartDrag(ULGUIPointerEventData* InPointerEventData)
{
	if (ShouldStartDrag_HoldToDrag(InPointerEventData))return true;
	FVector2D mousePos = FVector2D(InPointerEventData->pointerPosition);
	return FVector2D::DistSquared(pressMousePos, mousePos) > clickTresholdSquare;
}
void ULGUI_CenterScreenRayemitter::MarkPress(ULGUIPointerEventData* InPointerEventData)
{
	pressMousePos = FVector2D(InPointerEventData->pointerPosition);
}
