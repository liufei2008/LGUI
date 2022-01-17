// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Event/LGUIWorldSpaceRaycaster.h"
#include "LGUIWorldSpaceRaycasterSource_Mouse.generated.h"

#define BUILD_VP_MATRIX_FROM_CAMERA_MANAGER 0

/**
 * This is for standalone mouse input, it will emit a ray from main viewport mouse position
 */
UCLASS(ClassGroup = LGUI, Blueprintable, meta = (DisplayName = "Mouse"))
class LGUI_API ULGUIWorldSpaceRaycasterSource_Mouse : public ULGUIWorldSpaceRaycasterSource
{
	GENERATED_BODY()
public:
	virtual bool GenerateRay(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection)override;
	virtual bool ShouldStartDrag(ULGUIPointerEventData* InPointerEventData)override;
#if BUILD_VP_MATRIX_FROM_CAMERA_MANAGER
private:
	FMatrix ComputeViewProjectionMatrix(APlayerCameraManager* CameraManager, const FIntPoint& ScreenSize);
	void DeprojectViewPointToWorldForMainViewport(const FMatrix& InViewProjectionMatrix, const FVector2D& InViewPoint01, FVector& OutWorldLocation, FVector& OutWorldDirection);
#endif
};
