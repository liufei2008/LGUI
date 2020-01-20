// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LGUIBaseRayemitter.h"
#include "LGUI_CenterScreenRayemitter.generated.h"

//Sends trace from the center of the first local player's screen
UCLASS(ClassGroup = LGUI, meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULGUI_CenterScreenRayemitter : public ULGUIBaseRayEmitter
{
	GENERATED_BODY()

public:	
	ULGUI_CenterScreenRayemitter();

public:
	virtual bool EmitRay(FVector& OutRayOrigin, FVector& OutRayDirection, TArray<AActor*>& InOutTraceOnlyActors, TArray<AActor*>& InOutTraceIgnoreActors)override;
	virtual bool ShouldStartDrag(const FLGUIPointerEventData& InPointerEventData)override;
	virtual void MarkPress(const FLGUIPointerEventData& InPointerEventData)override;
private:
	FVector2D pressMousePos;
	bool GetMousePosition(FVector2D& OutPos);
};
