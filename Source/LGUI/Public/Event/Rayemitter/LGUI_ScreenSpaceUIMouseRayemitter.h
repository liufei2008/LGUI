// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LGUIBaseRayemitter.h"
#include "LGUI_ScreenSpaceUIMouseRayemitter.generated.h"

class ULGUICanvas;
//For screen space UI mouse input. This component should only attached on a actor which have a LGUICanvas, and RenderMode of LGUICanvas should set to ScreenSpace
UCLASS(ClassGroup = LGUI, meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULGUI_ScreenSpaceUIMouseRayemitter : public ULGUIBaseRayEmitter
{
	GENERATED_BODY()

public:	
	virtual bool EmitRay(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, TArray<AActor*>& InOutTraceOnlyActors, TArray<AActor*>& InOutTraceIgnoreActors)override;
	virtual bool ShouldStartDrag(ULGUIPointerEventData* InPointerEventData)override;
	virtual void MarkPress(ULGUIPointerEventData* InPointerEventData)override;
	static void DeprojectViewPointToWorld(const FMatrix& InViewProjectionMatrix, const FVector2D& InViewPoint01, FVector& OutWorldLocation, FVector& OutWorldDirection);
	void SetRenderCanvas(ULGUICanvas* InCanvas) { RenderCanvas = InCanvas; }
private:
	FVector2D pressMousePos;
	UPROPERTY(Transient)ULGUICanvas* RenderCanvas = nullptr;
};
