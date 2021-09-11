// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LGUIBaseRayemitter.h"
#include "LGUI_CenterScreenRayemitter.generated.h"

/** 
 * Sends trace from the center of the first local player's screen
 */
UCLASS(ClassGroup = LGUI, Blueprintable)
class LGUI_API ULGUI_CenterScreenRayemitter : public ULGUIBaseRayemitter
{
	GENERATED_BODY()

public:
	virtual bool EmitRay(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, TArray<AActor*>& InOutTraceOnlyActors, TArray<AActor*>& InOutTraceIgnoreActors)override;
	virtual bool ShouldStartDrag(ULGUIPointerEventData* InPointerEventData)override;
	virtual void MarkPress(ULGUIPointerEventData* InPointerEventData)override;
private:
	FVector2D pressMousePos;
};
