// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LGUIBaseRayemitter.h"
#include "LGUI_MainViewportMouseRayEmitter.generated.h"

#define BUILD_VP_MATRIX_FROM_CAMERA_MANAGER 0

//This is for standalone mouse input, it will emit a ray from main viewport mouse position
UCLASS(ClassGroup = LGUI, meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULGUI_MainViewportMouseRayEmitter : public ULGUIBaseRayEmitter
{
	GENERATED_BODY()

public:	
	ULGUI_MainViewportMouseRayEmitter();

public:
	virtual bool EmitRay(FVector& OutRayOrigin, FVector& OutRayDirection, TArray<AActor*>& InOutTraceOnlyActors, TArray<AActor*>& InOutTraceIgnoreActors)override;
	virtual bool ShouldStartDrag(ULGUIPointerEventData* InPointerEventData)override;
	virtual void MarkPress(ULGUIPointerEventData* InPointerEventData)override;
#if BUILD_VP_MATRIX_FROM_CAMERA_MANAGER
private:
	FMatrix ComputeViewProjectionMatrix(APlayerCameraManager* CameraManager, const FIntPoint& ScreenSize);
	void DeprojectViewPointToWorldForMainViewport(const FMatrix& InViewProjectionMatrix, const FVector2D& InViewPoint01, FVector& OutWorldLocation, FVector& OutWorldDirection);
#endif
private:
	FVector2D pressMousePos;
	bool GetMousePosition(FVector2D& OutPos);
};
